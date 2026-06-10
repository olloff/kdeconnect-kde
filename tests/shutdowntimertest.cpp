/**
 * SPDX-FileCopyrightText: 2026 olloff <olloff@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "../plugins/shutdowntimer/shutdowntimer.h"

#include <QDateTime>
#include <QSignalSpy>
#include <QTest>

/**
 * Tests the ShutdownTimer state machine used by the shutdowntimer plugin.
 * The platform power actions themselves (PowerManagement) are deliberately
 * not exercised here.
 */
class ShutdownTimerTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void actionStringRoundTrip();
    void actionFromInvalidString();
    void initialStateIsInactive();
    void scheduleActivatesTimer();
    void scheduleRejectsOutOfRangeDelays();
    void rescheduleReplacesPendingTimer();
    void cancelStopsTimer();
    void cancelWithoutTimerIsRejected();
    void expiryEmitsExpiredAndResetsState();
};

void ShutdownTimerTest::actionStringRoundTrip()
{
    const auto actions = {ShutdownTimer::Action::Shutdown, ShutdownTimer::Action::Reboot, ShutdownTimer::Action::Suspend};
    for (ShutdownTimer::Action action : actions) {
        const auto parsed = ShutdownTimer::actionFromString(ShutdownTimer::actionToString(action));
        QVERIFY(parsed.has_value());
        QCOMPARE(*parsed, action);
    }
}

void ShutdownTimerTest::actionFromInvalidString()
{
    QVERIFY(!ShutdownTimer::actionFromString(QStringLiteral("hibernate")).has_value());
    QVERIFY(!ShutdownTimer::actionFromString(QStringLiteral("Shutdown")).has_value()); // case-sensitive
    QVERIFY(!ShutdownTimer::actionFromString(QString()).has_value());
}

void ShutdownTimerTest::initialStateIsInactive()
{
    ShutdownTimer timer;
    QVERIFY(!timer.isActive());
    QCOMPARE(timer.deadline(), 0);
}

void ShutdownTimerTest::scheduleActivatesTimer()
{
    ShutdownTimer timer;
    QSignalSpy changedSpy(&timer, &ShutdownTimer::changed);

    const qint64 before = QDateTime::currentMSecsSinceEpoch();
    QVERIFY(timer.schedule(ShutdownTimer::Action::Reboot, 60));
    const qint64 after = QDateTime::currentMSecsSinceEpoch();

    QVERIFY(timer.isActive());
    QCOMPARE(timer.action(), ShutdownTimer::Action::Reboot);
    QVERIFY(timer.deadline() >= before + 60 * 1000);
    QVERIFY(timer.deadline() <= after + 60 * 1000);
    QCOMPARE(changedSpy.count(), 1);
}

void ShutdownTimerTest::scheduleRejectsOutOfRangeDelays()
{
    ShutdownTimer timer;
    QSignalSpy changedSpy(&timer, &ShutdownTimer::changed);

    QVERIFY(!timer.schedule(ShutdownTimer::Action::Shutdown, -1));
    QVERIFY(!timer.schedule(ShutdownTimer::Action::Shutdown, ShutdownTimer::maxSeconds + 1));

    QVERIFY(!timer.isActive());
    QCOMPARE(changedSpy.count(), 0);
}

void ShutdownTimerTest::rescheduleReplacesPendingTimer()
{
    ShutdownTimer timer;
    QVERIFY(timer.schedule(ShutdownTimer::Action::Shutdown, 3600));
    const qint64 firstDeadline = timer.deadline();

    QVERIFY(timer.schedule(ShutdownTimer::Action::Suspend, 60));

    QVERIFY(timer.isActive());
    QCOMPARE(timer.action(), ShutdownTimer::Action::Suspend);
    QVERIFY(timer.deadline() < firstDeadline);
}

void ShutdownTimerTest::cancelStopsTimer()
{
    ShutdownTimer timer;
    QVERIFY(timer.schedule(ShutdownTimer::Action::Shutdown, 3600));

    QSignalSpy changedSpy(&timer, &ShutdownTimer::changed);
    QSignalSpy expiredSpy(&timer, &ShutdownTimer::expired);

    QVERIFY(timer.cancel());

    QVERIFY(!timer.isActive());
    QCOMPARE(timer.deadline(), 0);
    QCOMPARE(changedSpy.count(), 1);
    QCOMPARE(expiredSpy.count(), 0);
}

void ShutdownTimerTest::cancelWithoutTimerIsRejected()
{
    ShutdownTimer timer;
    QSignalSpy changedSpy(&timer, &ShutdownTimer::changed);

    QVERIFY(!timer.cancel());
    QCOMPARE(changedSpy.count(), 0);
}

void ShutdownTimerTest::expiryEmitsExpiredAndResetsState()
{
    ShutdownTimer timer;
    QSignalSpy expiredSpy(&timer, &ShutdownTimer::expired);

    QVERIFY(timer.schedule(ShutdownTimer::Action::Suspend, 0));
    QVERIFY(expiredSpy.wait(1000));

    QCOMPARE(expiredSpy.count(), 1);
    QCOMPARE(expiredSpy.constFirst().constFirst().value<ShutdownTimer::Action>(), ShutdownTimer::Action::Suspend);
    QVERIFY(!timer.isActive());
    QCOMPARE(timer.deadline(), 0);
}

QTEST_GUILESS_MAIN(ShutdownTimerTest)

#include "shutdowntimertest.moc"
