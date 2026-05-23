import Blockstream.Green
import Blockstream.Green.Core
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Page {
    id: self
    background: null
    leftPadding: 0
    rightPadding: 0
    topPadding: 0
    bottomPadding: 0
    focusPolicy: Qt.ClickFocus

    required property Context context

    ScrollView {
        anchors.fill: parent

        ColumnLayout {
            width: self.width
            spacing: 16
            leftMargin: 20
            rightMargin: 20
            topMargin: 20
            bottomMargin: 20

            // Coordinator trading hint (conditional)
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: hintColumn.implicitHeight + 16
                visible: self.coordinatorTradingHint !== ''
                color: 'transparent'
                border.color: '#B71C1C'
                border.width: 1
                radius: 8

                ColumnLayout {
                    id: hintColumn
                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: 8

                    RowLayout {
                        spacing: 8

                        Image {
                            source: 'qrc:/svg2/warning.svg'
                            Layout.preferredWidth: 20
                            Layout.preferredHeight: 20
                        }

                        Text {
                            text: 'Configuration Issue'
                            color: '#B71C1C'
                            font.pixelSize: 13
                            font.weight: Font.Bold
                            Layout.fillWidth: true
                        }
                    }

                    Text {
                        text: self.coordinatorTradingHint
                        color: '#A0A0A0'
                        font.pixelSize: 12
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                    }
                }
            }

            // Wallet activation card
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: activationColumn.implicitHeight + 20
                color: '#1a1a1a'
                border.color: '#262626'
                border.width: 1
                radius: 8

                ColumnLayout {
                    id: activationColumn
                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: 12

                    RowLayout {
                        spacing: 8

                        Image {
                            source: 'qrc:/svg2/wallet.svg'
                            Layout.preferredWidth: 24
                            Layout.preferredHeight: 24
                        }

                        Text {
                            text: self.isActivated ? 'Wallet Registered' : 'Activate Wallet for DLC'
                            color: '#FFFFFF'
                            font.pixelSize: 14
                            font.weight: Font.Bold
                            Layout.fillWidth: true
                        }
                    }

                    Text {
                        visible: !self.isActivated
                        text: 'Register your Bitcoin wallet to start trading DLC options. Your private keys remain on your device.'
                        color: '#A0A0A0'
                        font.pixelSize: 12
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                    }

                    // Activation UI (not activated)
                    ColumnLayout {
                        visible: !self.isActivated
                        Layout.fillWidth: true
                        spacing: 8

                        Text {
                            text: 'Select Bitcoin Wallet:'
                            color: '#FFFFFF'
                            font.pixelSize: 12
                            font.weight: Font.Bold
                        }

                        ComboBox {
                            id: walletCombo
                            Layout.fillWidth: true
                            model: self.availableWallets
                            textRole: 'label'
                        }

                        Button {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 40
                            text: 'Activate'
                            enabled: walletCombo.currentIndex >= 0 && !self.actionInProgress
                            onClicked: self.activateWallet(self.availableWallets[walletCombo.currentIndex].id)

                            background: Rectangle {
                                color: parent.enabled ? '#00BCFF' : '#262626'
                                radius: 4
                            }

                            contentItem: Text {
                                text: parent.text
                                color: parent.enabled ? '#121416' : '#A0A0A0'
                                font.pixelSize: 14
                                font.weight: Font.Bold
                                horizontalAlignment: Text.AlignHCenter
                            }
                        }
                    }

                    // Activated UI
                    ColumnLayout {
                        visible: self.isActivated
                        Layout.fillWidth: true
                        spacing: 8

                        Text {
                            text: 'Active Wallet: ' + self.activeWalletLabel
                            color: '#FFFFFF'
                            font.pixelSize: 12
                            Layout.fillWidth: true
                        }

                        Text {
                            visible: self.tokenExpiryDate !== ''
                            text: 'Token expires: ' + self.tokenExpiryDate
                            color: '#A0A0A0'
                            font.pixelSize: 11
                        }

                        Button {
                            Layout.fillWidth: true
                            text: 'Switch Wallet'
                            onClicked: self.showWalletSwitchDialog()

                            background: Rectangle {
                                color: '#262626'
                                border.color: '#00BCFF'
                                border.width: 1
                                radius: 4
                            }

                            contentItem: Text {
                                text: parent.text
                                color: '#00BCFF'
                                font.pixelSize: 13
                                horizontalAlignment: Text.AlignHCenter
                            }
                        }
                    }
                }
            }

            // Summary stat cards row
            RowLayout {
                Layout.fillWidth: true
                spacing: 12

                // PnL Card
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 140
                    color: '#1a1a1a'
                    border.color: '#262626'
                    border.width: 1
                    radius: 8

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 16
                        spacing: 8
                        justifyContent: Layout.AlignVCenter

                        Text {
                            text: 'Wallet PnL'
                            color: '#A0A0A0'
                            font.pixelSize: 12
                        }

                        Text {
                            text: self.isActivated ? self.walletPnlDisplay : '—'
                            color: self.walletPnlPositive ? '#1B5E20' : '#B71C1C'
                            font.pixelSize: 24
                            font.weight: Font.Bold
                            Layout.fillHeight: true
                            verticalAlignment: Text.AlignVCenter
                        }

                        Text {
                            text: 'sats'
                            color: '#A0A0A0'
                            font.pixelSize: 11
                        }
                    }
                }

                // Order Counts Card
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 140
                    color: '#1a1a1a'
                    border.color: '#262626'
                    border.width: 1
                    radius: 8

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 16
                        spacing: 8
                        justifyContent: Layout.AlignVCenter

                        Text {
                            text: 'Order Counts'
                            color: '#A0A0A0'
                            font.pixelSize: 12
                        }

                        RowLayout {
                            Layout.fillHeight: true
                            Layout.fillWidth: true
                            spacing: 16
                            verticalAlignment: Layout.AlignVCenter

                            ColumnLayout {
                                Text {
                                    text: 'Open'
                                    color: '#A0A0A0'
                                    font.pixelSize: 11
                                }
                                Text {
                                    text: self.isActivated ? String(self.openOrderCount) : '—'
                                    color: '#FFFFFF'
                                    font.pixelSize: 18
                                    font.weight: Font.Bold
                                }
                            }

                            ColumnLayout {
                                Text {
                                    text: 'Live'
                                    color: '#A0A0A0'
                                    font.pixelSize: 11
                                }
                                Text {
                                    text: self.isActivated ? String(self.liveOrderCount) : '—'
                                    color: '#FFFFFF'
                                    font.pixelSize: 18
                                    font.weight: Font.Bold
                                }
                            }

                            ColumnLayout {
                                Text {
                                    text: 'Closed'
                                    color: '#A0A0A0'
                                    font.pixelSize: 11
                                }
                                Text {
                                    text: self.isActivated ? String(self.closedOrderCount) : '—'
                                    color: '#FFFFFF'
                                    font.pixelSize: 18
                                    font.weight: Font.Bold
                                }
                            }
                        }
                    }
                }
            }

            // Balance Split Card
            Rectangle {
                Layout.fillWidth: true
                color: '#1a1a1a'
                border.color: '#262626'
                border.width: 1
                radius: 8

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 16
                    spacing: 12

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 4

                        Text {
                            text: 'Coordinator Balance'
                            color: '#FFFFFF'
                            font.pixelSize: 13
                            font.weight: Font.Bold
                        }

                        Text {
                            text: 'Balances visible to the DLC coordinator'
                            color: '#A0A0A0'
                            font.pixelSize: 11
                        }
                    }

                    // Progress bar
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 8
                        color: '#262626'
                        radius: 4

                        Rectangle {
                            height: parent.height
                            radius: 4
                            color: '#00BCFF'
                            width: parent.width * (self.availableBalanceSat / Math.max(1, self.availableBalanceSat + self.reservedBalanceSat))

                            Behavior on width {
                                SmoothedAnimation {
                                    velocity: 100
                                }
                            }
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 16

                        ColumnLayout {
                            spacing: 2

                            Text {
                                text: 'Available'
                                color: '#A0A0A0'
                                font.pixelSize: 11
                            }

                            Text {
                                text: self.isActivated ? String(self.availableBalanceSat) + ' sats' : '—'
                                color: '#1B5E20'
                                font.pixelSize: 12
                                font.weight: Font.Bold
                            }
                        }

                        ColumnLayout {
                            spacing: 2

                            Text {
                                text: 'Reserved'
                                color: '#A0A0A0'
                                font.pixelSize: 11
                            }

                            Text {
                                text: self.isActivated ? String(self.reservedBalanceSat) + ' sats' : '—'
                                color: '#B71C1C'
                                font.pixelSize: 12
                                font.weight: Font.Bold
                            }
                        }
                    }
                }
            }

            // Risk Disclosure Card
            Rectangle {
                Layout.fillWidth: true
                color: 'transparent'
                border.color: '#262626'
                border.width: 1
                radius: 8

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 16
                    spacing: 8

                    Text {
                        text: 'Risk Disclosure'
                        color: '#FFFFFF'
                        font.pixelSize: 13
                        font.weight: Font.Bold
                    }

                    Text {
                        text: 'DLC options lock collateral for the contract duration. Only xpubs and signatures leave your device. Your seed phrase remains local and is never transmitted.'
                        color: '#A0A0A0'
                        font.pixelSize: 11
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                    }
                }
            }

            Item { Layout.fillHeight: true }
        }
    }

    // Properties (simulated; would connect to DlcPageModel)
    property bool isActivated: false
    property string activeWalletLabel: 'My Bitcoin Wallet'
    property string tokenExpiryDate: '2026-05-21'
    property var availableWallets: [
        { id: 'wallet1', label: 'My Bitcoin Wallet (registered)' },
        { id: 'wallet2', label: 'Backup Wallet (not registered)' }
    ]

    property string coordinatorTradingHint: ''
    property int openOrderCount: 2
    property int liveOrderCount: 1
    property int closedOrderCount: 5
    property string walletPnlDisplay: '+1,234,567'
    property bool walletPnlPositive: true
    property int availableBalanceSat: 5000000
    property int reservedBalanceSat: 1000000

    property bool actionInProgress: false

    // Signals/Slots
    function activateWallet(walletId) {
        console.log('Activating wallet:', walletId)
        self.isActivated = true
    }

    function showWalletSwitchDialog() {
        console.log('Show wallet switch dialog')
    }
}
