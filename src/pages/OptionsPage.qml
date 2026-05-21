import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Page {
    id: optionsPage
    title: qsTr("Options")

    Column {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 16

        TabBar {
            id: tabBar
            width: parent.width
            
            TabBarButton {
                text: qsTr("Discover")
            }
            TabBarButton {
                text: qsTr("Orders")
            }
            TabBarButton {
                text: qsTr("Positions")
            }
            TabBarButton {
                text: qsTr("Activity")
            }
        }

        StackLayout {
            width: parent.width
            currentIndex: tabBar.currentIndex

            // 1. Discover Instruments
            DiscoverInstrumentsPanel {
                onInstrumentSelected: {
                    orderSheet.instrument = instrument
                    orderSheet.open()
                }
            }

            // 2. Open Orders
            OrdersPanel {
                onOrderCancelled: orderManager.cancelOrder(orderId)
            }

            // 3. Active Positions (DLCs)
            PositionsPanel {
                onPositionClicked: positionDetailDialog.dlcId = dlcId
            }

            // 4. Activity / History
            ActivityPanel {}
        }
    }

    // Modal: Place Order
    OrderEntrySheet {
        id: orderSheet
    }

    // Modal: Position Details
    PositionDetailDialog {
        id: positionDetailDialog
    }
}
