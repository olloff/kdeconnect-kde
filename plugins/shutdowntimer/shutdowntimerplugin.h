/**
 * SPDX-FileCopyrightText: 2026 olloff <olloff@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include <QTimer>

#include <core/kdeconnectplugin.h>

#include "shutdowntimer.h"

#define PACKET_TYPE_SHUTDOWNTIMER QStringLiteral("kdeconnect.shutdowntimer")
#define PACKET_TYPE_SHUTDOWNTIMER_REQUEST QStringLiteral("kdeconnect.shutdowntimer.request")

/**
 * Schedule, inspect and cancel delayed power actions (shutdown, reboot,
 * suspend) on the paired device, and perform them locally when the paired
 * device asks for it. See the README in this directory for the protocol.
 *
 * The D-Bus properties and methods operate on the *remote* device's timer,
 * mirroring the convention of the lockdevice plugin.
 */
class ShutdownTimerPlugin : public KdeConnectPlugin
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.kdeconnect.device.shutdowntimer")
    Q_PROPERTY(bool isActive READ isActive NOTIFY timerChanged)
    Q_PROPERTY(QString action READ action NOTIFY timerChanged)
    Q_PROPERTY(qint64 deadline READ deadline NOTIFY timerChanged)

public:
    explicit ShutdownTimerPlugin(QObject *parent, const QVariantList &args);

    bool isActive() const;
    QString action() const;
    qint64 deadline() const;

    /**
     * Schedules @p action ("shutdown", "reboot" or "suspend") on the remote
     * device in @p seconds seconds, replacing any pending timer there.
     */
    Q_SCRIPTABLE void scheduleShutdown(const QString &action, qint64 seconds);

    /**
     * Cancels the pending timer on the remote device, if any.
     */
    Q_SCRIPTABLE void cancelShutdown();

    QString dbusPath() const override;
    void connected() override;
    void receivePacket(const NetworkPacket &np) override;

Q_SIGNALS:
    Q_SCRIPTABLE void timerChanged();

private:
    void sendState();
    void handleRequest(const NetworkPacket &np);
    void handleExpired(ShutdownTimer::Action action);
    void notifyScheduled();
    void onLocalTimerChanged();
    void showWarning();
    QString pendingActionText(const QString &duration) const;

    ShutdownTimer m_localTimer;

    // Re-broadcasts the local timer status while it is active, so a peer
    // that missed a status packet converges within one interval.
    QTimer m_syncTimer;

    // Fires the "about to shut down" notification, a configurable time
    // (config key warningSeconds, 0 to disable) before the local deadline.
    QTimer m_warningTimer;

    // Last known state of the remote device's timer
    bool m_remoteActive = false;
    QString m_remoteAction;
    qint64 m_remoteDeadline = 0;
};
