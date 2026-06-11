/**
 * SPDX-FileCopyrightText: 2026 olloff <olloff@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "powermanagement.h"

#include "plugin_shutdowntimer_debug.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusReply>

namespace
{
QString methodForAction(ShutdownTimer::Action action)
{
    switch (action) {
    case ShutdownTimer::Action::Shutdown:
        return QStringLiteral("PowerOff");
    case ShutdownTimer::Action::Reboot:
        return QStringLiteral("Reboot");
    case ShutdownTimer::Action::Suspend:
        return QStringLiteral("Suspend");
    }
    Q_UNREACHABLE();
}
}

bool PowerManagement::isActionSupported(ShutdownTimer::Action action)
{
    Q_UNUSED(action);
    return true;
}

bool PowerManagement::execute(ShutdownTimer::Action action)
{
    QDBusMessage message = QDBusMessage::createMethodCall(QStringLiteral("org.freedesktop.login1"),
                                                          QStringLiteral("/org/freedesktop/login1"),
                                                          QStringLiteral("org.freedesktop.login1.Manager"),
                                                          methodForAction(action));
    message.setArguments({false}); // non-interactive: don't prompt for polkit authorization

    const QDBusReply<void> reply = QDBusConnection::systemBus().call(message);
    if (!reply.isValid()) {
        qCWarning(KDECONNECT_PLUGIN_SHUTDOWNTIMER) << "logind" << methodForAction(action) << "failed:" << reply.error().message();
        return false;
    }
    return true;
}
