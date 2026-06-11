/**
 * SPDX-FileCopyrightText: 2026 olloff <olloff@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.kirigami 2.20 as Kirigami
import org.kde.kdeconnect 1.0

Kirigami.ScrollablePage {
    property string device

    Kirigami.FormLayout {

        KdeConnectPluginConfig {
            id: config
            deviceId: device
            pluginName: "kdeconnect_shutdowntimer"
        }

        Component.onCompleted: {
            warningMinutes.value = config.getInt("warningSeconds", 300) / 60
        }

        QQC2.SpinBox {
            id: warningMinutes
            Kirigami.FormData.label: i18nd("kdeconnect-plugins", "Warn this many minutes before the timer ends (0 to disable):")
            from: 0
            to: 120
            onValueModified: config.set("warningSeconds", value * 60)
        }
    }
}
