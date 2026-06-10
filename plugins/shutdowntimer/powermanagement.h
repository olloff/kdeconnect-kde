/**
 * SPDX-FileCopyrightText: 2026 olloff <olloff@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include "shutdowntimer.h"

/**
 * Platform backends for performing the power action of an expired
 * ShutdownTimer. Implemented in powermanagement_linux.cpp (logind),
 * powermanagement_win.cpp (Win32) and powermanagement_mac.cpp.
 */
namespace PowerManagement
{
/**
 * Whether this platform backend can perform @p action at all. Used to
 * reject requests up front instead of failing when the timer expires.
 */
bool isActionSupported(ShutdownTimer::Action action);

/**
 * Performs @p action immediately. Returns false if the request could not
 * be issued; note that a true return value only means the request was
 * accepted by the OS, which may still veto it (e.g. an unsaved-changes
 * dialog blocking shutdown).
 */
bool execute(ShutdownTimer::Action action);
}
