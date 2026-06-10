/**
 * SPDX-FileCopyrightText: 2026 olloff <olloff@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "powermanagement.h"

#include "plugin_shutdowntimer_debug.h"

#include <Windows.h>

#include <PowrProf.h>

namespace
{
/**
 * ExitWindowsEx requires the calling process to hold the SE_SHUTDOWN_NAME
 * privilege, which is present but disabled by default.
 */
bool acquireShutdownPrivilege()
{
    HANDLE token;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &token)) {
        return false;
    }

    TOKEN_PRIVILEGES privileges = {};
    privileges.PrivilegeCount = 1;
    privileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    bool success = LookupPrivilegeValueW(nullptr, SE_SHUTDOWN_NAME, &privileges.Privileges[0].Luid)
        && AdjustTokenPrivileges(token, FALSE, &privileges, 0, nullptr, nullptr) && GetLastError() == ERROR_SUCCESS;

    CloseHandle(token);
    return success;
}
}

bool PowerManagement::isActionSupported(ShutdownTimer::Action action)
{
    if (action == ShutdownTimer::Action::Suspend) {
        return IsPwrSuspendAllowed();
    }
    return true;
}

bool PowerManagement::execute(ShutdownTimer::Action action)
{
    if (action == ShutdownTimer::Action::Suspend) {
        // (suspend, force unused since Vista, disable wake events)
        if (!SetSuspendState(FALSE, FALSE, FALSE)) {
            qCWarning(KDECONNECT_PLUGIN_SHUTDOWNTIMER) << "SetSuspendState failed:" << GetLastError();
            return false;
        }
        return true;
    }

    if (!acquireShutdownPrivilege()) {
        qCWarning(KDECONNECT_PLUGIN_SHUTDOWNTIMER) << "Could not acquire SE_SHUTDOWN_NAME privilege:" << GetLastError();
        return false;
    }

    const UINT flags = (action == ShutdownTimer::Action::Reboot ? EWX_REBOOT : EWX_POWEROFF) | EWX_FORCEIFHUNG;
    if (!ExitWindowsEx(flags, SHTDN_REASON_MAJOR_OTHER | SHTDN_REASON_FLAG_PLANNED)) {
        qCWarning(KDECONNECT_PLUGIN_SHUTDOWNTIMER) << "ExitWindowsEx failed:" << GetLastError();
        return false;
    }
    return true;
}
