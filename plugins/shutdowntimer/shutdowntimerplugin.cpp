/**
 * SPDX-FileCopyrightText: 2026 olloff <olloff@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "shutdowntimerplugin.h"

#include <QDateTime>

#include <KFormat>
#include <KLocalizedString>
#include <KPluginFactory>

#include <core/daemon.h>
#include <core/device.h>

#include "plugin_shutdowntimer_debug.h"
#include "powermanagement.h"

K_PLUGIN_CLASS_WITH_JSON(ShutdownTimerPlugin, "kdeconnect_shutdowntimer.json")

ShutdownTimerPlugin::ShutdownTimerPlugin(QObject *parent, const QVariantList &args)
    : KdeConnectPlugin(parent, args)
{
    connect(&m_localTimer, &ShutdownTimer::changed, this, &ShutdownTimerPlugin::sendState);
    connect(&m_localTimer, &ShutdownTimer::expired, this, &ShutdownTimerPlugin::handleExpired);
}

bool ShutdownTimerPlugin::isActive() const
{
    return m_remoteActive;
}

QString ShutdownTimerPlugin::action() const
{
    return m_remoteAction;
}

qint64 ShutdownTimerPlugin::deadline() const
{
    return m_remoteDeadline;
}

void ShutdownTimerPlugin::scheduleShutdown(const QString &action, qint64 seconds)
{
    NetworkPacket np(PACKET_TYPE_SHUTDOWNTIMER_REQUEST,
                     {
                         {QStringLiteral("setAction"), action},
                         {QStringLiteral("setSeconds"), seconds},
                     });
    sendPacket(np);
}

void ShutdownTimerPlugin::cancelShutdown()
{
    NetworkPacket np(PACKET_TYPE_SHUTDOWNTIMER_REQUEST, {{QStringLiteral("cancel"), true}});
    sendPacket(np);
}

void ShutdownTimerPlugin::connected()
{
    sendState();

    NetworkPacket np(PACKET_TYPE_SHUTDOWNTIMER_REQUEST, {{QStringLiteral("requestStatus"), true}});
    sendPacket(np);
}

void ShutdownTimerPlugin::receivePacket(const NetworkPacket &np)
{
    if (np.type() == PACKET_TYPE_SHUTDOWNTIMER) {
        // Status broadcast: the remote device's timer changed
        m_remoteActive = np.get<bool>(QStringLiteral("isActive"), false);
        m_remoteAction = m_remoteActive ? np.get<QString>(QStringLiteral("action")) : QString();
        m_remoteDeadline = m_remoteActive ? np.get<qint64>(QStringLiteral("deadline"), 0) : 0;
        Q_EMIT timerChanged();
    } else if (np.type() == PACKET_TYPE_SHUTDOWNTIMER_REQUEST) {
        handleRequest(np);
    }
}

void ShutdownTimerPlugin::handleRequest(const NetworkPacket &np)
{
    if (np.has(QStringLiteral("cancel"))) {
        if (m_localTimer.cancel()) {
            Daemon::instance()->sendSimpleNotification(QStringLiteral("shutdownTimerCancelled"),
                                                       device()->name(),
                                                       i18n("Scheduled power action cancelled"),
                                                       QStringLiteral("system-shutdown"));
        }
    }

    if (np.has(QStringLiteral("setAction")) && np.has(QStringLiteral("setSeconds"))) {
        const QString actionString = np.get<QString>(QStringLiteral("setAction"));
        const qint64 seconds = np.get<qint64>(QStringLiteral("setSeconds"), -1);

        const auto action = ShutdownTimer::actionFromString(actionString);
        if (!action || !PowerManagement::isActionSupported(*action)) {
            qCWarning(KDECONNECT_PLUGIN_SHUTDOWNTIMER) << "Rejecting unsupported action" << actionString << "from" << device()->name();
            sendState(); // unchanged status doubles as the negative acknowledgement
        } else if (m_localTimer.schedule(*action, seconds)) {
            notifyScheduled();
        } else {
            qCWarning(KDECONNECT_PLUGIN_SHUTDOWNTIMER) << "Rejecting out-of-range delay" << seconds << "from" << device()->name();
            sendState();
        }
    }

    if (np.has(QStringLiteral("requestStatus"))) {
        sendState();
    }
}

void ShutdownTimerPlugin::sendState()
{
    NetworkPacket np(PACKET_TYPE_SHUTDOWNTIMER, {{QStringLiteral("isActive"), m_localTimer.isActive()}});
    if (m_localTimer.isActive()) {
        np.set(QStringLiteral("action"), ShutdownTimer::actionToString(m_localTimer.action()));
        np.set(QStringLiteral("deadline"), m_localTimer.deadline());
    }
    sendPacket(np);
}

void ShutdownTimerPlugin::handleExpired(ShutdownTimer::Action action)
{
    qCDebug(KDECONNECT_PLUGIN_SHUTDOWNTIMER) << "Timer expired, executing" << ShutdownTimer::actionToString(action);
    if (!PowerManagement::execute(action)) {
        Daemon::instance()->reportError(device()->name(), i18n("Failed to perform the scheduled power action"));
    }
}

void ShutdownTimerPlugin::notifyScheduled()
{
    const qint64 remainingMsecs = m_localTimer.deadline() - QDateTime::currentMSecsSinceEpoch();
    const QString duration = KFormat().formatSpelloutDuration(qMax<qint64>(remainingMsecs, 0));

    QString text;
    switch (m_localTimer.action()) {
    case ShutdownTimer::Action::Shutdown:
        text = i18n("This device will shut down in %1", duration);
        break;
    case ShutdownTimer::Action::Reboot:
        text = i18n("This device will reboot in %1", duration);
        break;
    case ShutdownTimer::Action::Suspend:
        text = i18n("This device will suspend in %1", duration);
        break;
    }

    Daemon::instance()->sendSimpleNotification(QStringLiteral("shutdownTimerScheduled"), device()->name(), text, QStringLiteral("system-shutdown"));
}

QString ShutdownTimerPlugin::dbusPath() const
{
    return QLatin1String("/modules/kdeconnect/devices/%1/shutdowntimer").arg(device()->id());
}

#include "moc_shutdowntimerplugin.cpp"
#include "shutdowntimerplugin.moc"
