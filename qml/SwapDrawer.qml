import Blockstream.Green
import Blockstream.Green.Core
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

WalletDrawer {
    id: self
    minimumContentWidth: 450
    contentItem: GStackView {
        id: stack_view
        title: qsTrId('id_swap')
        initialItem: ChainSwapCreatePage {
            context: self.context
            onCloseClicked: self.close()
        }
    }
}
