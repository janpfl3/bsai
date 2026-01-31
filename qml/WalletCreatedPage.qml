import Blockstream.Green
import Blockstream.Green.Core
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

StackViewPage {
    signal continueClicked()
    required property Context context
    id: self
    padding: 60
    header: null
    footer: Pane {
        background: null
        bottomPadding: 20
        contentItem: RowLayout {
            HSpacer {
            }
            Label {
                Layout.alignment: Qt.AlignCenter
                Layout.maximumWidth: 450
                horizontalAlignment: Label.AlignHCenter
                font.pixelSize: 14
                opacity: 0.6
                text: 'Warning: If you forget your PIN and have not enabled biometrics or a recovery method, you will lose access to funds.'
                wrapMode: Label.Wrap
            }
            HSpacer {
            }
        }
    }
    contentItem: ColumnLayout {
        VSpacer {
        }
         MultiImage {
            Layout.alignment: Qt.AlignCenter
            Layout.preferredHeight: 295
            Layout.preferredWidth: 311
            foreground: 'qrc:/png/logo_bubbles.png'
        }
        Label {
            Layout.alignment: Qt.AlignCenter
            Layout.topMargin: 20
            font.pixelSize: 26
            font.weight: 600
            text: 'Welcome to Blockstream!'
        }
        Label {
            Layout.alignment: Qt.AlignCenter
            Layout.fillWidth: true
            Layout.preferredWidth: 0
            Layout.maximumWidth: 325
            Layout.topMargin: 10
            horizontalAlignment: Label.AlignHCenter
            font.pixelSize: 14
            font.weight: 400
            text: 'Your wallet is ready. Your journey to financial freedom starts now.'
            wrapMode: Label.Wrap
        }
        PrimaryButton {
            Layout.alignment: Qt.AlignCenter
            Layout.minimumWidth: 325
            Layout.topMargin: 32
            font.weight: 500
            text: qsTrId('id_continue')
            onClicked: self.continueClicked()
        }
        VSpacer {
        }
    }
}

