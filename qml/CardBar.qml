import Blockstream.Green
import Blockstream.Green.Core
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "analytics.js" as AnalyticsJS
import "util.js" as UtilJS

Pane {
    signal sendClicked()
    signal jadeDetailsClicked()
    signal promoClicked(Promo promo)
    required property Context context
    id: self
    clip: true
    leftPadding: 16
    rightPadding: 16
    topPadding: 0
    bottomPadding: 0
    background: Rectangle {
        radius: 4
        color: '#181818'
        border.width: 1
        border.color: '#262626'
    }
    contentItem: RowLayout {
        Flickable {
            Layout.fillHeight: true
            Layout.fillWidth: true
            id: flickable
            contentWidth: layout.implicitWidth
            implicitHeight: layout.implicitHeight
            RowLayout {
                id: layout
                spacing: 0
                TotalBalanceCard {
                    context: self.context
                }
                Separator {
                    visible: jade_card.visible
                }
                JadeCard {
                    id: jade_card
                    context: self.context
                    onDetailsClicked: self.jadeDetailsClicked()
                }
                Separator {
                }
            }
        }
        RowLayout {
            Layout.alignment: Qt.AlignCenter
            spacing: 5
            TransactButton {
                action.shortcut: 'Ctrl+B'
                action.onTriggered: {
                    const drawer = buy_bitcoin_drawer_component.createObject(self, { context: self.context })
                    drawer.open()
                }
                enabled: !!UtilJS.accounts(self.context).find(account => !account.network.liquid)
                icon.source: 'qrc:/svg/coin.svg'
                text: 'Buy'
                visible: self.context.mainnet
            }
            TransactButton {
                action.shortcut: 'Ctrl+S'
                action.onTriggered: openSendDrawer()
                enabled: !self.context.watchonly
                icon.source: 'qrc:/svg/send.svg'
                text: qsTrId('id_send')
                // TODO move to send page
                // self.checkDeviceMatches() && self.currentAccount && !(self.currentAccount.session.config?.twofactor_reset?.is_active ?? false)
            }
            TransactButton {
                action.shortcut: 'Ctrl+R'
                action.onTriggered: openReceiveDrawer()
                icon.source: 'qrc:/svg/receive.svg'
                text: qsTrId('id_receive')
                // TODO move to receive page
                // self.checkDeviceMatches() && self.currentAccount && !(self.currentAccount.session.config?.twofactor_reset?.is_active ?? false)
            }
            TransactButton {
                action.shortcut: 'Ctrl+W'
                action.onTriggered: openSwapDrawer()
                enabled: !self.context.watchonly && !self.context.wallet.login.device
                icon.source: 'qrc:/000000/24/arrows-down-up.svg'
                text: qsTrId('id_swap')
                visible: self.context.mainnet
            }
        }
    }
    Component {
        id: buy_bitcoin_drawer_component
        BuyDrawer {
        }
    }
    // Image {
    //     source: 'qrc:/svg2/arrow_right.svg'
    //     anchors.verticalCenter: parent.verticalCenter
    //     anchors.left: parent.left
    //     visible: flickable.contentX > 0
    //     rotation: 180
    //     opacity: 0.5
    //     TapHandler {
    //         onTapped: flickable.flick(2000, 0)
    //     }
    // }
    // Image {
    //     source: 'qrc:/svg2/arrow_right.svg'
    //     anchors.verticalCenter: parent.verticalCenter
    //     anchors.right: parent.right
    //     visible: flickable.contentWidth - flickable.contentX > flickable.width
    //     opacity: 0.5
    //     TapHandler {
    //         onTapped: flickable.flick(-2000, 0)
    //     }
    // }
    component TransactButton: PrimaryButton {
        Layout.minimumWidth: 150
        action: Action {
            enabled: UtilJS.effectiveVisible(self)
        }
        leftPadding: 8
        rightPadding: 8
    }

    component Separator: Rectangle {
        Layout.minimumWidth: 1
        Layout.maximumWidth: 1
        Layout.leftMargin: 20
        Layout.rightMargin: 20
        Layout.fillHeight: true
        color: '#FFF'
        opacity: 0.04
    }
}
