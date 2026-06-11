/*
 * SPDX-FileCopyrightText: 2026 olloff <olloff@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.kdeconnect

Kirigami.ScrollablePage
{
    id: root
    title: i18nd("kdeconnect-app", "Shutdown timer")
    property QtObject pluginInterface
    property QtObject device

    property double now: Date.now()

    Timer {
        interval: 1000
        running: root.pluginInterface.isActive
        repeat: true
        onTriggered: root.now = Date.now()
    }

    Connections {
        target: root.pluginInterface
        function onTimerChangedProxy() {
            root.now = Date.now();
        }
    }

    function formatCountdown(msecs) {
        const total = Math.max(0, Math.round(msecs / 1000));
        const h = Math.floor(total / 3600);
        const m = Math.floor((total % 3600) / 60);
        const s = total % 60;
        const pad = n => n < 10 ? "0" + n : "" + n;
        return h > 0 ? h + ":" + pad(m) + ":" + pad(s) : m + ":" + pad(s);
    }

    function pendingText(action, countdown) {
        switch (action) {
        case "reboot":
            return i18nd("kdeconnect-app", "Rebooting in %1", countdown);
        case "suspend":
            return i18nd("kdeconnect-app", "Suspending in %1", countdown);
        default:
            return i18nd("kdeconnect-app", "Shutting down in %1", countdown);
        }
    }

    ColumnLayout {
        spacing: Kirigami.Units.largeSpacing

        Kirigami.Heading {
            Layout.fillWidth: true
            level: 1
            text: root.pluginInterface.isActive
                  ? root.pendingText(root.pluginInterface.action,
                                     root.formatCountdown(root.pluginInterface.deadline - root.now))
                  : i18nd("kdeconnect-app", "No timer active")
        }

        Label {
            visible: root.pluginInterface.isActive
            text: i18nd("kdeconnect-app", "at %1",
                        new Date(Number(root.pluginInterface.deadline)).toLocaleTimeString(Qt.locale(), Locale.ShortFormat))
            color: Kirigami.Theme.disabledTextColor
        }

        Button {
            visible: root.pluginInterface.isActive
            icon.name: "dialog-cancel"
            text: i18nd("kdeconnect-app", "Cancel timer")
            onClicked: root.pluginInterface.cancelShutdown()
        }

        Kirigami.Separator {
            Layout.fillWidth: true
        }

        Kirigami.FormLayout {
            Layout.fillWidth: true

            ComboBox {
                id: actionCombo
                Kirigami.FormData.label: i18nd("kdeconnect-app", "Action:")
                textRole: "text"
                valueRole: "value"
                model: [
                    { text: i18nd("kdeconnect-app", "Shut down"), value: "shutdown" },
                    { text: i18nd("kdeconnect-app", "Reboot"), value: "reboot" },
                    { text: i18nd("kdeconnect-app", "Suspend"), value: "suspend" },
                ]
            }

            SpinBox {
                id: delayMinutes
                Kirigami.FormData.label: i18nd("kdeconnect-app", "Delay (minutes):")
                from: 1
                to: 24 * 60
                value: 30
                editable: true
            }

            Button {
                icon.name: "chronometer"
                text: root.pluginInterface.isActive
                      ? i18nd("kdeconnect-app", "Replace timer")
                      : i18nd("kdeconnect-app", "Start timer")
                onClicked: root.pluginInterface.scheduleShutdown(actionCombo.currentValue, delayMinutes.value * 60)
            }
        }
    }
}
