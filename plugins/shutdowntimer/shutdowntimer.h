/**
 * SPDX-FileCopyrightText: 2026 olloff <olloff@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#pragma once

#include <QObject>
#include <QString>
#include <QTimer>

#include <limits>
#include <optional>

/**
 * State machine for a single pending power action.
 *
 * The timer is owned by the process that will perform the action, so the
 * deadline can be read and the timer cancelled at any point before expiry.
 * Executing the action itself is left to the owner of this object (see
 * PowerManagement), which keeps this class free of platform code and
 * testable in isolation.
 */
class ShutdownTimer : public QObject
{
    Q_OBJECT

public:
    enum class Action {
        Shutdown,
        Reboot,
        Suspend,
    };
    Q_ENUM(Action)

    explicit ShutdownTimer(QObject *parent = nullptr);

    /**
     * Maps the wire representation used in kdeconnect.shutdowntimer packets
     * ("shutdown", "reboot", "suspend") to an Action, or std::nullopt for
     * anything else.
     */
    static std::optional<Action> actionFromString(const QString &action);
    static QString actionToString(Action action);

    bool isActive() const;

    /**
     * The pending action. Only meaningful while isActive() returns true.
     */
    Action action() const;

    /**
     * When the pending action fires, in milliseconds since epoch (UTC),
     * or 0 while no timer is active.
     */
    qint64 deadline() const;

    /**
     * Schedules @p action to fire in @p seconds seconds, replacing any
     * pending timer. Returns false (and leaves any pending timer untouched)
     * if @p seconds is out of range.
     */
    bool schedule(Action action, qint64 seconds);

    /**
     * Aborts the pending timer. Returns false if no timer was active.
     */
    bool cancel();

    /**
     * Upper bound for schedule()'s @p seconds, chosen to keep the deadline
     * within QTimer's millisecond range (just under 25 days).
     */
    static constexpr qint64 maxSeconds = std::numeric_limits<int>::max() / 1000;

Q_SIGNALS:
    /**
     * Emitted whenever isActive(), action() or deadline() change, including
     * on expiry (after which isActive() is false again).
     */
    void changed();

    /**
     * Emitted when the timer expires. The state is reset to inactive before
     * this is emitted, so a status read from a connected slot already
     * reports no pending timer.
     */
    void expired(ShutdownTimer::Action action);

private:
    void onTimeout();

    QTimer m_timer;
    Action m_action = Action::Shutdown;
    qint64 m_deadline = 0;
};
