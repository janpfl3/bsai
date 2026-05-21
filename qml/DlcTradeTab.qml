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

            // Orderbook card
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: orderbookColumn.implicitHeight + 20
                color: '#1a1a1a'
                border.color: '#262626'
                border.width: 1
                radius: 8

                ColumnLayout {
                    id: orderbookColumn
                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: 12

                    // Instrument selector header
                    ItemDelegate {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 44
                        background: Rectangle {
                            color: '#262626'
                            radius: 4
                        }

                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 8
                            spacing: 8

                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 2

                                Text {
                                    text: self.selectedInstrument
                                    color: '#FFFFFF'
                                    font.pixelSize: 13
                                    font.weight: Font.Bold
                                }

                                Text {
                                    text: self.instrumentExpiryDate
                                    color: '#A0A0A0'
                                    font.pixelSize: 11
                                }
                            }

                            Image {
                                source: 'qrc:/svg2/caret-down.svg'
                                Layout.preferredWidth: 16
                                Layout.preferredHeight: 16
                            }
                        }

                        onClicked: self.showInstrumentPicker()
                    }

                    Text {
                        text: 'Strike Prices (Bid / Ask Premium in sats per contract)'
                        color: '#A0A0A0'
                        font.pixelSize: 11
                        Layout.fillWidth: true
                    }

                    // Strike summary table
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 300
                        color: '#262626'
                        radius: 4

                        ListView {
                            anchors.fill: parent
                            anchors.margins: 1
                            model: self.strikeOrderbooks
                            clip: true

                            header: Rectangle {
                                width: parent.width
                                height: 32
                                color: '#1a1a1a'
                                z: 2

                                RowLayout {
                                    anchors.fill: parent
                                    anchors.margins: 8
                                    spacing: 8

                                    Text {
                                        Layout.fillWidth: true
                                        Layout.preferredWidth: 100
                                        text: 'Strike'
                                        color: '#A0A0A0'
                                        font.pixelSize: 11
                                        font.weight: Font.Bold
                                    }

                                    Text {
                                        Layout.preferredWidth: 80
                                        text: 'Ask (Red)'
                                        color: '#B71C1C'
                                        font.pixelSize: 11
                                        font.weight: Font.Bold
                                        horizontalAlignment: Text.AlignRight
                                    }

                                    Text {
                                        Layout.preferredWidth: 80
                                        text: 'Bid (Green)'
                                        color: '#1B5E20'
                                        font.pixelSize: 11
                                        font.weight: Font.Bold
                                        horizontalAlignment: Text.AlignRight
                                    }
                                }
                            }

                            delegate: ItemDelegate {
                                width: ListView.view.width
                                height: 40
                                background: Rectangle {
                                    color: index % 2 === 0 ? '#1a1a1a' : '#262626'
                                }

                                RowLayout {
                                    anchors.fill: parent
                                    anchors.margins: 8
                                    spacing: 8

                                    Text {
                                        Layout.fillWidth: true
                                        Layout.preferredWidth: 100
                                        text: modelData.strike
                                        color: '#FFFFFF'
                                        font.pixelSize: 12
                                    }

                                    Text {
                                        Layout.preferredWidth: 80
                                        text: modelData.ask
                                        color: '#B71C1C'
                                        font.pixelSize: 12
                                        horizontalAlignment: Text.AlignRight
                                    }

                                    Text {
                                        Layout.preferredWidth: 80
                                        text: modelData.bid
                                        color: '#1B5E20'
                                        font.pixelSize: 12
                                        horizontalAlignment: Text.AlignRight
                                    }
                                }

                                onClicked: self.showStrikeDetail(modelData)
                            }

                            Rectangle {
                                anchors.fill: parent
                                color: 'transparent'
                                visible: parent.count === 0
                                z: 1

                                Text {
                                    anchors.centerIn: parent
                                    text: 'No liquidity on any strike yet'
                                    color: '#A0A0A0'
                                    font.pixelSize: 12
                                }
                            }
                        }
                    }
                }
            }

            // Create order card
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: createOrderColumn.implicitHeight + 20
                color: '#1a1a1a'
                border.color: '#262626'
                border.width: 1
                radius: 8

                ColumnLayout {
                    id: createOrderColumn
                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: 12

                    Text {
                        text: 'Create Order'
                        color: '#FFFFFF'
                        font.pixelSize: 14
                        font.weight: Font.Bold
                    }

                    // Side selector
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        Text {
                            text: 'Side:'
                            color: '#A0A0A0'
                            font.pixelSize: 12
                        }

                        Button {
                            Layout.preferredWidth: 80
                            text: 'Buy'
                            checkable: true
                            checked: self.orderSide === 'buy'
                            onClicked: self.orderSide = 'buy'
                            background: Rectangle {
                                color: parent.checked ? '#1B5E20' : '#262626'
                                radius: 4
                                border.color: parent.checked ? '#1B5E20' : '#262626'
                                border.width: 1
                            }
                            contentItem: Text {
                                text: parent.text
                                color: parent.checked ? '#FFFFFF' : '#A0A0A0'
                                font.pixelSize: 12
                                horizontalAlignment: Text.AlignHCenter
                            }
                        }

                        Button {
                            Layout.preferredWidth: 80
                            text: 'Sell'
                            checkable: true
                            checked: self.orderSide === 'sell'
                            onClicked: self.orderSide = 'sell'
                            background: Rectangle {
                                color: parent.checked ? '#B71C1C' : '#262626'
                                radius: 4
                                border.color: parent.checked ? '#B71C1C' : '#262626'
                                border.width: 1
                            }
                            contentItem: Text {
                                text: parent.text
                                color: parent.checked ? '#FFFFFF' : '#A0A0A0'
                                font.pixelSize: 12
                                horizontalAlignment: Text.AlignHCenter
                            }
                        }
                    }

                    // Strike field
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 4

                        Text {
                            text: 'Strike Price (USD):'
                            color: '#FFFFFF'
                            font.pixelSize: 12
                            font.weight: Font.Bold
                        }

                        RowLayout {
                            spacing: 8

                            TextField {
                                Layout.fillWidth: true
                                text: self.strikePrice
                                placeholderText: '70000'
                                onTextChanged: self.strikePrice = text
                                background: Rectangle {
                                    color: '#262626'
                                    radius: 4
                                    border.color: '#262626'
                                    border.width: 1
                                }
                                color: '#FFFFFF'
                            }

                            Button {
                                text: '⟳'
                                onClicked: self.refreshStrikePrices()
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
                            visible: self.strikePriceError !== ''
                            text: self.strikePriceError
                            color: '#B71C1C'
                            font.pixelSize: 11
                        }
                    }

                    // Quantity field
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 4

                        Text {
                            text: 'Quantity (Contracts):'
                            color: '#FFFFFF'
                            font.pixelSize: 12
                            font.weight: Font.Bold
                        }

                        TextField {
                            Layout.fillWidth: true
                            text: String(self.quantity)
                            placeholderText: '0.01'
                            onTextChanged: self.quantity = parseFloat(text) || 0.01
                            background: Rectangle {
                                color: '#262626'
                                radius: 4
                                border.color: '#262626'
                                border.width: 1
                            }
                            color: '#FFFFFF'
                        }

                        Text {
                            text: 'Minimum 0.01 contracts. 1 contract = 1 BTC'
                            color: '#A0A0A0'
                            font.pixelSize: 10
                        }
                    }

                    // Premium field
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 4

                        Text {
                            text: 'Premium per Contract (sats):'
                            color: '#FFFFFF'
                            font.pixelSize: 12
                            font.weight: Font.Bold
                        }

                        TextField {
                            Layout.fillWidth: true
                            text: String(self.premium)
                            placeholderText: '10000'
                            readOnly: true
                            background: Rectangle {
                                color: '#262626'
                                radius: 4
                                border.color: '#262626'
                                border.width: 1
                                opacity: 0.6
                            }
                            color: '#A0A0A0'
                        }

                        Text {
                            text: 'Informational only; coordinator uses orderbook liquidity'
                            color: '#A0A0A0'
                            font.pixelSize: 10
                        }
                    }

                    // Estimated total
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        Text {
                            text: 'Estimated Total:'
                            color: '#A0A0A0'
                            font.pixelSize: 12
                        }

                        Text {
                            text: self.estimatedTotal
                            color: '#FFFFFF'
                            font.pixelSize: 14
                            font.weight: Font.Bold
                            Layout.fillWidth: true
                            horizontalAlignment: Text.AlignRight
                        }

                        Text {
                            text: 'sats'
                            color: '#A0A0A0'
                            font.pixelSize: 12
                        }
                    }

                    // Create button
                    Button {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 44
                        text: 'Create Order'
                        enabled: !self.actionInProgress && self.isActivated
                        onClicked: self.createOrder()

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
            }

            Item { Layout.fillHeight: true }
        }
    }

    // Properties
    property string selectedInstrument: 'BTC-18MAR26-STRIKE-C'
    property string instrumentExpiryDate: 'Expires: 2026-03-18 16:00 UTC'
    property var strikeOrderbooks: [
        { strike: '$70,000', ask: '2,500', bid: '2,400' },
        { strike: '$71,000', ask: '1,800', bid: '1,700' },
        { strike: '$72,000', ask: '1,200', bid: '1,100' },
        { strike: '$73,000', ask: '800', bid: '700' }
    ]

    property string orderSide: 'buy'
    property string strikePrice: '71000'
    property string strikePriceError: ''
    property real quantity: 0.01
    property int premium: 10000
    property string estimatedTotal: '100 sats'

    property bool isActivated: false
    property bool actionInProgress: false

    // Signals/Slots
    function showInstrumentPicker() {
        console.log('Show instrument picker')
    }

    function showStrikeDetail(strike) {
        console.log('Show strike detail:', strike.strike)
    }

    function refreshStrikePrices() {
        console.log('Refresh strike prices')
    }

    function createOrder() {
        console.log('Create order:', self.orderSide, self.quantity, self.strikePrice)
    }
}

