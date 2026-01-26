import Blockstream.Green
import Blockstream.Green.Core
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "util.js" as UtilJS

StackViewPage {
    signal newWallet()
    signal restoreWallet()
    signal watchOnlyWallet()
    StackView.onActivated: Analytics.recordEvent('wallet_add')
    objectName: "AddWalletPage"
    id: self
    padding: 60
    footer: Pane {
        background: null
        bottomPadding: 20
        contentItem: RowLayout {
            HSpacer {
            }
            LinkButton {
                font.underline: true
                text: qsTrId('id_set_up_watchonly')
                onClicked: self.watchOnlyWallet()
            }
            HSpacer {
            }
        }
    }
    Image {
        Layout.alignment: Qt.AlignCenter
        Layout.preferredHeight: 240
        antialiasing: true
        fillMode: Image.PreserveAspectFit
        mipmap: true
        smooth: true
        source: 'qrc:/svg3/Btc-portal.svg'
    }
    Label {
        Layout.alignment: Qt.AlignCenter
        Layout.maximumWidth: 400
        Layout.topMargin: 20
        color: '#FFF'
        font.pixelSize: 14
        font.weight: 400
        horizontalAlignment: Label.AlignHCenter
        text: 'Create a new wallet to begin your Bitcoin journey.'
        wrapMode: Label.WordWrap
    }
    PrimaryButton {
        Layout.alignment: Qt.AlignCenter
        Layout.minimumWidth: 325
        Layout.topMargin: 40
        text: qsTrId('id_setup_a_new_wallet')
        onClicked: self.newWallet()
    }
    LinkButton {
        Layout.alignment: Qt.AlignCenter
        Layout.topMargin: 20
        font.underline: true
        text: 'Restore an Existing Wallet'
        onClicked: self.restoreWallet()
    }
}
