import Blockstream.Green
import Blockstream.Green.Core
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "analytics.js" as AnalyticsJS
import "util.js" as UtilJS

WalletDrawer {
    property Asset asset
    property url url
    id: self
    minimumContentWidth: 450
    contentItem: GStackView {
        id: stack_view
        title: qsTrId('id_send')
        initialItem: RecipientPage {
            context: self.context
            account: null
            asset: null
            onCloseClicked: self.close()
        }
    }
    AnalyticsView {
        name: 'Send'
        active: true
        segmentation: AnalyticsJS.segmentationSession(Settings, self.context)
    }
    onClosed: {
        if (self.url && stack_view.currentItem instanceof SendPage) {
            WalletManager.openUrl = self.url
        }
    }
}
