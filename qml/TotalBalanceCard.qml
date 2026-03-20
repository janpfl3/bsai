import Blockstream.Green
import Blockstream.Green.Core
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "analytics.js" as AnalyticsJS
import "util.js" as UtilJS

WalletHeaderCard {
    id: self
    
    Convert {
        id: convert
        context: self.context
        input: ({ satoshi: String(UtilJS.accounts(self.context).reduce((balance, account) => balance + account.balance, 0)) })
        unit: UtilJS.unit(self.context)
    }

    headerItem: RowLayout {
        Label {
            Layout.alignment: Qt.AlignCenter
            color: '#A0A0A0'
            font.pixelSize: 12
            font.weight: 400
            text: qsTrId('id_bitcoin_balance')
        }
        AbstractButton {
            contentItem: Image {
                Layout.alignment: Qt.AlignCenter
                sourceSize.width: 20
                sourceSize.height: 20
                source: Settings.incognito ? 'qrc:/svg2/eye_closed.svg' : 'qrc:/svg2/eye.svg'
            }
            onClicked: {
                Settings.toggleIncognito()
                if (Settings.incognito) {
                    Analytics.recordEvent('hide_amount', AnalyticsJS.segmentationSession(Settings, self.context))
                }
            }
        }
        HSpacer {
        }
    }

    contentItem: ColumnLayout {
        spacing: 4
        Label {
            topPadding: 4
            font.pixelSize: 22
            font.weight: 600
            text: UtilJS.incognito(Settings.incognito, convert.output.label)
        }
        Label {
            color: '#FFFFFF'
            font.pixelSize: 14
            font.weight: 400
            opacity: 0.6
            text: UtilJS.incognito(Settings.incognito, convert.fiat.label)
        }
        VSpacer {
        }
    }
}
