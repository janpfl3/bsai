import Blockstream.Green
import Blockstream.Green.Core
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

WalletHeaderCard {
    signal detailsClicked
    readonly property JadeDevice device: self.context?.device instanceof JadeDevice ? self.context.device : null
    readonly property var latestFirmware: {
        for (const firmware of controller.firmwares) {
            if (firmware.latest) {
                return firmware
            }
        }
        return null
    }
    readonly property bool runningLatest: {
        return self.device?.version === self.latestFirmware?.version
    }
    readonly property bool debug: Qt.application.arguments.indexOf('--debugjade') > 0

    function isJadeLocked() {
        return self.device?.connected && self.device?.state === JadeDevice.StateLocked
    }

    function getJadeStatusOptions () {
        if (!self.device?.connected) {
            return { text: 'Disconnected', color: '#FF6467' }
        }

        if (self.isJadeLocked()) {
            return { text: 'Locked', color: '#FDC700' }
        }

        return { text: 'Connected', color: '#00C60D' }
    }

    Component.onCompleted: firmware_controller.check(self.device)
    JadeFirmwareController {
        id: firmware_controller
    }
    JadeFirmwareCheckController {
        id: controller
        index: firmware_controller.index
        device: self.device
    }
    id: self
    visible: self.context.wallet.login?.device?.type === 'jade'
    TapHandler {
        enabled: self.device?.connected ?? false
        onTapped: {
          if (self.debug) {
            self.detailsClicked()
          } else {
            update_firmware_dialog.createObject(self).open()
          }
        }
    }
    background: Item {
        Image {
            id: image
            x: details_column.width
            anchors.verticalCenter: parent.verticalCenter
            source: {
                if (self.device) {
                    const type = self.device.versionInfo?.BOARD_TYPE
                    return type === 'JADE_V2' ? 'qrc:/png/jade2_card.png' : 'qrc:/png/jade_card.png'
                }
                return ''
            }
        }
    }
    headerItem: RowLayout {
        Label {
            Layout.alignment: Qt.AlignCenter
            color: '#FFF'
            font.capitalization: Font.AllUppercase
            font.pixelSize: 12
            font.weight: 400
            opacity: 0.6
            text: qsTrId('id_hardware_wallet')
        }
        HSpacer {
            Layout.minimumHeight: 28
        }
        Label {
            Layout.alignment: Qt.AlignCenter
            Layout.minimumWidth: fw_column.width
            font.capitalization: Font.AllUppercase
            font.pixelSize: 12
            font.weight: 400
            opacity: 0.6
            text: qsTrId('id_firmware')
            visible: !self.runningLatest
        }
    }
    contentItem: RowLayout {
        ColumnLayout {
            Layout.rightMargin: image.width
            id: details_column
            RowLayout {
                Label {
                    font.capitalization: Font.AllUppercase
                    font.pixelSize: 20
                    font.weight: 600
                    text: self.context.wallet.login?.device?.name ?? ''
                }
                Image {
                    id: seal_check
                    width: 24
                    height: 24
                    source: 'qrc:/svg2/seal-check.svg'
                    visible: {
                        if (!self.device) return false
                        const board_type = self.device.versionInfo?.BOARD_TYPE
                        if (board_type !== 'JADE_V2') return false
                        const efusemac = self.device.versionInfo?.EFUSEMAC
                        if (!efusemac) return false
                        return Settings.isEventRegistered({ efusemac, result: 'genuine', type: 'jade_genuine_check' })
                    }
                }
            }
            RowLayout {
                Layout.fillHeight: false
                spacing: 4
                Rectangle {
                    Layout.alignment: Qt.AlignCenter
                    color: self.getJadeStatusOptions().color
                    implicitWidth: 8
                    implicitHeight: 8
                    radius: 4
                }
                Label {
                    font.pixelSize: 14
                    font.weight: 400
                    color: '#A0A0A0'
                    text: self.getJadeStatusOptions().text
                }
                LinkButton {
                    text: qsTrId('id_unlock')
                    visible: self.isJadeLocked()
                    enabled: unlock_controller.monitor?.idle ?? true
                    onClicked: unlock_controller.unlock()
                }
            }
            VSpacer {
            }
        }
        ColumnLayout {
            id: fw_column
            visible: !self.runningLatest
            Label {
                font.capitalization: Font.AllUppercase
                font.pixelSize: 20
                font.weight: 600
                text: self.device?.version ?? ''
            }
            PrimaryButton {
                text: qsTrId('id_firmware_upgrade')
                onClicked: update_firmware_dialog.createObject(self).open()
            }
            VSpacer {
            }
        }
    }

    Component {
        id: update_firmware_dialog
        JadeUpdateDialog2 {
            context: self.context
            device: self.device
        }
    }
    JadeUnlockController {
        id: unlock_controller
        context: self.context
        device: self.device
        onHttpRequest: (request) => {
            const dialog = http_request_dialog.createObject(self, { request, context: self.context })
            dialog.open()
        }
    }
    Component {
        id: http_request_dialog
        JadeHttpRequestDialog {
        }
    }
}
