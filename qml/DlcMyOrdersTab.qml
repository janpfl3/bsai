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
            spacing: 12
            Layout.leftMargin: 20
            Layout.rightMargin: 20
            Layout.topMargin: 20
            Layout.bottomMargin: 20

            // Open Orders Section
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 8

                Text {
                    text: 'Open Orders'
                    color: '#FFFFFF'
                    font.pixelSize: 14
                    font.weight: Font.Bold
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: openOrdersListView.height + 1
                    color: 'transparent'
                    border.color: '#262626'
                    border.width: 1
                    radius: 8
                    clip: true

                    ListView {
                        id: openOrdersListView
                        anchors.fill: parent
                        model: self.openOrders
                        interactive: false
                        spacing: 1

                        delegate: Rectangle {
                            width: ListView.view.width
                            height: 80
                            color: index % 2 === 0 ? '#1a1a1a' : '#262626'

                            ColumnLayout {
                                anchors.fill: parent
                                anchors.margins: 12
                                spacing: 4

                                RowLayout {
                                    Layout.fillWidth: true
                                    spacing: 12

                                    ColumnLayout {
                                        Layout.fillWidth: true
                                        spacing: 2

                                        Text {
                                            text: modelData.instrumentId
                                            color: '#FFFFFF'
                                            font.pixelSize: 12
                                            font.weight: Font.Bold
                                            elideMode: Text.ElideRight
                                        }

                                        RowLayout {
                                            spacing: 8

                                            Rectangle {
                                                Layout.preferredWidth: 32
                                                Layout.preferredHeight: 20
                                                color: modelData.side === 'buy' ? '#1B5E20' : '#B71C1C'
                                                radius: 2

                                                Text {
                                                    anchors.centerIn: parent
                                                    text: modelData.side.toUpperCase()
                                                    color: '#FFFFFF'
                                                    font.pixelSize: 10
                                                    font.weight: Font.Bold
                                                }
                                            }

                                            Text {
                                                text: modelData.createdAt
                                                color: '#A0A0A0'
                                                font.pixelSize: 11
                                            }
                                        }
                                    }

                                    // Info and Cancel buttons
                                    RowLayout {
                                        spacing: 8

                                        Button {
                                            Layout.preferredWidth: 32
                                            Layout.preferredHeight: 32
                                            text: 'ⓘ'
                                            onClicked: self.showOrderInfo(modelData)

                                            background: Rectangle {
                                                color: '#262626'
                                                radius: 4
                                            }

                                            contentItem: Text {
                                                text: parent.text
                                                color: '#A0A0A0'
                                                font.pixelSize: 16
                                                horizontalAlignment: Text.AlignHCenter
                                            }
                                        }

                                        Button {
                                            Layout.preferredWidth: 32
                                            Layout.preferredHeight: 32
                                            text: '✕'
                                            enabled: !self.processingOrder
                                            onClicked: self.cancelOrder(modelData.orderId)

                                            background: Rectangle {
                                                color: parent.enabled ? '#B71C1C' : '#262626'
                                                radius: 4
                                            }

                                            contentItem: Text {
                                                text: parent.text
                                                color: parent.enabled ? '#FFFFFF' : '#A0A0A0'
                                                font.pixelSize: 14
                                                horizontalAlignment: Text.AlignHCenter
                                            }
                                        }
                                    }
                                }

                                Text {
                                    text: modelData.quantity + ' contracts @ $' + modelData.strike + ' - ' + modelData.status
                                    color: '#A0A0A0'
                                    font.pixelSize: 10
                                    Layout.fillWidth: true
                                }
                            }
                        }

                        Rectangle {
                            anchors.fill: parent
                            color: 'transparent'
                            visible: parent.count === 0
                            z: 1

                            Text {
                                anchors.centerIn: parent
                                text: 'No open orders'
                                color: '#A0A0A0'
                                font.pixelSize: 12
                            }
                        }
                    }
                }
            }

            // Live Orders Section
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 8

                Text {
                    text: 'Live Orders'
                    color: '#FFFFFF'
                    font.pixelSize: 14
                    font.weight: Font.Bold
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: liveOrdersListView.height + 1
                    color: 'transparent'
                    border.color: '#262626'
                    border.width: 1
                    radius: 8
                    clip: true

                    ListView {
                        id: liveOrdersListView
                        anchors.fill: parent
                        model: self.liveOrders
                        interactive: false
                        spacing: 1

                        delegate: Rectangle {
                            width: ListView.view.width
                            height: 80
                            color: index % 2 === 0 ? '#1a1a1a' : '#262626'

                            ColumnLayout {
                                anchors.fill: parent
                                anchors.margins: 12
                                spacing: 4

                                RowLayout {
                                    Layout.fillWidth: true
                                    spacing: 12

                                    ColumnLayout {
                                        Layout.fillWidth: true
                                        spacing: 2

                                        Text {
                                            text: modelData.instrumentId
                                            color: '#FFFFFF'
                                            font.pixelSize: 12
                                            font.weight: Font.Bold
                                            elideMode: Text.ElideRight
                                        }

                                        RowLayout {
                                            spacing: 8

                                            Rectangle {
                                                Layout.preferredWidth: 32
                                                Layout.preferredHeight: 20
                                                color: modelData.side === 'buy' ? '#1B5E20' : '#B71C1C'
                                                radius: 2

                                                Text {
                                                    anchors.centerIn: parent
                                                    text: modelData.side.toUpperCase()
                                                    color: '#FFFFFF'
                                                    font.pixelSize: 10
                                                    font.weight: Font.Bold
                                                }
                                            }

                                            Text {
                                                text: modelData.createdAt
                                                color: '#A0A0A0'
                                                font.pixelSize: 11
                                            }
                                        }
                                    }

                                    // Hourglass + Info
                                    RowLayout {
                                        spacing: 8

                                        Image {
                                            source: 'qrc:/svg2/hourglass.svg'
                                            Layout.preferredWidth: 24
                                            Layout.preferredHeight: 24
                                            visible: modelData.inFlight

                                            RotationAnimation on rotation {
                                                from: 0
                                                to: 360
                                                duration: 2000
                                                loops: Animation.Infinite
                                            }

                                            MouseArea {
                                                anchors.fill: parent
                                                onClicked: self.showInFlightDialog(modelData)
                                            }
                                        }

                                        Button {
                                            Layout.preferredWidth: 32
                                            Layout.preferredHeight: 32
                                            text: 'ⓘ'
                                            onClicked: self.showOrderInfo(modelData)

                                            background: Rectangle {
                                                color: '#262626'
                                                radius: 4
                                            }

                                            contentItem: Text {
                                                text: parent.text
                                                color: '#A0A0A0'
                                                font.pixelSize: 16
                                                horizontalAlignment: Text.AlignHCenter
                                            }
                                        }
                                    }
                                }

                                RowLayout {
                                    Layout.fillWidth: true
                                    spacing: 8

                                    Text {
                                        text: modelData.quantity + ' contracts @ $' + modelData.strike
                                        color: '#A0A0A0'
                                        font.pixelSize: 10
                                    }

                                    Text {
                                        text: modelData.status
                                        color: '#00BCFF'
                                        font.pixelSize: 10
                                        font.weight: Font.Bold
                                    }
                                }
                            }
                        }

                        Rectangle {
                            anchors.fill: parent
                            color: 'transparent'
                            visible: parent.count === 0
                            z: 1

                            Text {
                                anchors.centerIn: parent
                                text: 'No live orders'
                                color: '#A0A0A0'
                                font.pixelSize: 12
                            }
                        }
                    }
                }
            }

            // Closed Orders Section
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 8

                Text {
                    text: 'Closed / Settled'
                    color: '#FFFFFF'
                    font.pixelSize: 14
                    font.weight: Font.Bold
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: closedOrdersListView.height + 1
                    color: 'transparent'
                    border.color: '#262626'
                    border.width: 1
                    radius: 8
                    clip: true

                    ListView {
                        id: closedOrdersListView
                        anchors.fill: parent
                        model: self.closedOrders
                        interactive: false
                        spacing: 1

                        delegate: Rectangle {
                            width: ListView.view.width
                            height: 80
                            color: index % 2 === 0 ? '#1a1a1a' : '#262626'

                            ColumnLayout {
                                anchors.fill: parent
                                anchors.margins: 12
                                spacing: 4

                                RowLayout {
                                    Layout.fillWidth: true
                                    spacing: 12

                                    ColumnLayout {
                                        Layout.fillWidth: true
                                        spacing: 2

                                        Text {
                                            text: modelData.instrumentId
                                            color: '#FFFFFF'
                                            font.pixelSize: 12
                                            font.weight: Font.Bold
                                            elideMode: Text.ElideRight
                                        }

                                        RowLayout {
                                            spacing: 8

                                            Rectangle {
                                                Layout.preferredWidth: 32
                                                Layout.preferredHeight: 20
                                                color: modelData.side === 'buy' ? '#1B5E20' : '#B71C1C'
                                                radius: 2

                                                Text {
                                                    anchors.centerIn: parent
                                                    text: modelData.side.toUpperCase()
                                                    color: '#FFFFFF'
                                                    font.pixelSize: 10
                                                    font.weight: Font.Bold
                                                }
                                            }

                                            Text {
                                                text: modelData.settledAt
                                                color: '#A0A0A0'
                                                font.pixelSize: 11
                                            }
                                        }
                                    }

                                    // PnL display
                                    ColumnLayout {
                                        spacing: 2

                                        Text {
                                            text: 'P&L'
                                            color: '#A0A0A0'
                                            font.pixelSize: 9
                                        }

                                        Text {
                                            text: modelData.pnl
                                            color: modelData.pnlPositive ? '#1B5E20' : '#B71C1C'
                                            font.pixelSize: 12
                                            font.weight: Font.Bold
                                        }
                                    }

                                    // Info button
                                    Button {
                                        Layout.preferredWidth: 32
                                        Layout.preferredHeight: 32
                                        text: 'ⓘ'
                                        onClicked: self.showOrderInfo(modelData)

                                        background: Rectangle {
                                            color: '#262626'
                                            radius: 4
                                        }

                                        contentItem: Text {
                                            text: parent.text
                                            color: '#A0A0A0'
                                            font.pixelSize: 16
                                            horizontalAlignment: Text.AlignHCenter
                                        }
                                    }
                                }

                                Text {
                                    text: modelData.quantity + ' contracts @ $' + modelData.strike + ' - ' + modelData.status
                                    color: '#A0A0A0'
                                    font.pixelSize: 10
                                    Layout.fillWidth: true
                                }
                            }
                        }

                        Rectangle {
                            anchors.fill: parent
                            color: 'transparent'
                            visible: parent.count === 0
                            z: 1

                            Text {
                                anchors.centerIn: parent
                                text: 'No closed orders'
                                color: '#A0A0A0'
                                font.pixelSize: 12
                            }
                        }
                    }
                }
            }

            Item { Layout.fillHeight: true }
        }
    }

    // Properties
    property var openOrders: [
        {
            orderId: 'ord-001',
            instrumentId: 'BTC-18MAR26-STRIKE-C',
            side: 'buy',
            quantity: '0.5',
            strike: '71000',
            premium: '2500',
            createdAt: '2 hours ago',
            status: 'Waiting for match',
            inFlight: false
        }
    ]

    property var liveOrders: [
        {
            orderId: 'ord-002',
            instrumentId: 'BTC-18MAR26-STRIKE-C',
            side: 'sell',
            quantity: '0.25',
            strike: '72000',
            premium: '1800',
            createdAt: '1 hour ago',
            status: 'Pending signatures',
            inFlight: true,
            inFlightPhase: 'makerSigningDlc'
        }
    ]

    property var closedOrders: [
        {
            orderId: 'ord-100',
            instrumentId: 'BTC-15MAR26-STRIKE-C',
            side: 'buy',
            quantity: '1.0',
            strike: '70000',
            premium: '2200',
            settledAt: '5 days ago',
            status: 'Settled',
            pnl: '+45,000 sats',
            pnlPositive: true
        },
        {
            orderId: 'ord-101',
            instrumentId: 'BTC-08MAR26-STRIKE-P',
            side: 'sell',
            quantity: '0.5',
            strike: '69000',
            premium: '1500',
            settledAt: '3 days ago',
            status: 'Settled',
            pnl: '-15,000 sats',
            pnlPositive: false
        }
    ]

    property bool processingOrder: false

    // Signals/Slots
    function showOrderInfo(order) {
        console.log('Show order info:', order.orderId)
    }

    function showInFlightDialog(order) {
        console.log('Show in-flight dialog for:', order.orderId, 'phase:', order.inFlightPhase)
    }

    function cancelOrder(orderId) {
        console.log('Cancel order:', orderId)
    }
}

