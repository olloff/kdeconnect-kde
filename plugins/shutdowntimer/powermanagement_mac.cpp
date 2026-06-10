/**
 * SPDX-FileCopyrightText: 2026 olloff <olloff@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "powermanagement.h"

#include "plugin_shutdowntimer_debug.h"

#include <QProcess>

bool PowerManagement::isActionSupported(ShutdownTimer::Action action)
{
    Q_UNUSED(action);
    return true;
}

bool PowerManagement::execute(ShutdownTimer::Action action)
{
    // Going through System Events gives applications the chance to prompt
    // for unsaved changes instead of being killed, and works without root.
    QString appleScript;
    switch (action) {
    case ShutdownTimer::Action::Shutdown:
        appleScript = QStringLiteral("tell application \"System Events\" to shut down");
        break;
    case ShutdownTimer::Action::Reboot:
        appleScript = QStringLiteral("tell application \"System Events\" to restart");
        break;
    case ShutdownTimer::Action::Suspend:
        appleScript = QStringLiteral("tell application \"System Events\" to sleep");
        break;
    }

    if (!QProcess::startDetached(QStringLiteral("osascript"), {QStringLiteral("-e"), appleScript})) {
        qCWarning(KDECONNECT_PLUGIN_SHUTDOWNTIMER) << "Could not run osascript for action" << ShutdownTimer::actionToString(action);
        return false;
    }
    return true;
}
