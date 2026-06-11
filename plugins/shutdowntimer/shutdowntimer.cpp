/**
 * SPDX-FileCopyrightText: 2026 olloff <olloff@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "shutdowntimer.h"

#include <QDateTime>

ShutdownTimer::ShutdownTimer(QObject *parent)
    : QObject(parent)
{
    m_timer.setSingleShot(true);
    m_timer.setTimerType(Qt::PreciseTimer);
    connect(&m_timer, &QTimer::timeout, this, &ShutdownTimer::onTimeout);
}

std::optional<ShutdownTimer::Action> ShutdownTimer::actionFromString(const QString &action)
{
    if (action == QLatin1String("shutdown")) {
        return Action::Shutdown;
    }
    if (action == QLatin1String("reboot")) {
        return Action::Reboot;
    }
    if (action == QLatin1String("suspend")) {
        return Action::Suspend;
    }
    return std::nullopt;
}

QString ShutdownTimer::actionToString(Action action)
{
    switch (action) {
    case Action::Shutdown:
        return QStringLiteral("shutdown");
    case Action::Reboot:
        return QStringLiteral("reboot");
    case Action::Suspend:
        return QStringLiteral("suspend");
    }
    Q_UNREACHABLE();
}

bool ShutdownTimer::isActive() const
{
    return m_timer.isActive();
}

ShutdownTimer::Action ShutdownTimer::action() const
{
    return m_action;
}

qint64 ShutdownTimer::deadline() const
{
    return m_deadline;
}

bool ShutdownTimer::schedule(Action action, qint64 seconds)
{
    if (seconds < 0 || seconds > maxSeconds) {
        return false;
    }

    m_action = action;
    m_deadline = QDateTime::currentMSecsSinceEpoch() + seconds * 1000;
    m_timer.start(static_cast<int>(seconds * 1000));

    Q_EMIT changed();
    return true;
}

bool ShutdownTimer::cancel()
{
    if (!m_timer.isActive()) {
        return false;
    }

    m_timer.stop();
    m_deadline = 0;

    Q_EMIT changed();
    return true;
}

void ShutdownTimer::onTimeout()
{
    const Action expiredAction = m_action;
    m_deadline = 0;

    Q_EMIT changed();
    Q_EMIT expired(expiredAction);
}
