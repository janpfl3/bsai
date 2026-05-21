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

            // Input card
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

                    Text {
                        text: 'Payout Simulation'
                        color: '#FFFFFF'
                        font.pixelSize: 14
                        font.weight: Font.Bold
                    }

                    Text {
                        text: 'Calculate theoretical profit or loss for different Bitcoin price scenarios'
                        color: '#A0A0A0'
                        font.pixelSize: 11
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                    }

                    // Conversational input layout
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 12

                        // "I am the [Buyer|Seller] of [Qty] contracts of [PUT|CALL]"
                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 4

                            RowLayout {
                                spacing: 8

                                Text {
                                    text: 'I am the'
                                    color: '#A0A0A0'
                                    font.pixelSize: 12
                                }

                                Button {
                                    text: 'Buyer'
                                    checkable: true
                                    checked: self.buyerSide
                                    onClicked: self.buyerSide = true
                                    background: Rectangle {
                                        color: parent.checked ? '#1B5E20' : '#262626'
                                        radius: 4
                                    }
                                    contentItem: Text {
                                        text: parent.text
                                        color: parent.checked ? '#FFFFFF' : '#A0A0A0'
                                        font.pixelSize: 11
                                    }
                                }

                                Button {
                                    text: 'Seller'
                                    checkable: true
                                    checked: !self.buyerSide
                                    onClicked: self.buyerSide = false
                                    background: Rectangle {
                                        color: parent.checked ? '#B71C1C' : '#262626'
                                        radius: 4
                                    }
                                    contentItem: Text {
                                        text: parent.text
                                        color: parent.checked ? '#FFFFFF' : '#A0A0A0'
                                        font.pixelSize: 11
                                    }
                                }

                                Text {
                                    text: 'of'
                                    color: '#A0A0A0'
                                    font.pixelSize: 12
                                }

                                SpinBox {
                                    value: self.simQty * 100
                                    from: 1
                                    to: 1000
                                    stepSize: 1
                                    onValueChanged: self.simQty = value / 100
                                    textFromValue: function(value) {
                                        return (value / 100).toFixed(2)
                                    }
                                }

                                Text {
                                    text: 'contracts of'
                                    color: '#A0A0A0'
                                    font.pixelSize: 12
                                }

                                Button {
                                    text: 'CALL'
                                    checkable: true
                                    checked: self.isCall
                                    onClicked: self.isCall = true
                                    background: Rectangle {
                                        color: parent.checked ? '#00BCFF' : '#262626'
                                        radius: 4
                                    }
                                    contentItem: Text {
                                        text: parent.text
                                        color: parent.checked ? '#121416' : '#A0A0A0'
                                        font.pixelSize: 11
                                    }
                                }

                                Button {
                                    text: 'PUT'
                                    checkable: true
                                    checked: !self.isCall
                                    onClicked: self.isCall = false
                                    background: Rectangle {
                                        color: parent.checked ? '#00BCFF' : '#262626'
                                        radius: 4
                                    }
                                    contentItem: Text {
                                        text: parent.text
                                        color: parent.checked ? '#121416' : '#A0A0A0'
                                        font.pixelSize: 11
                                    }
                                }
                            }
                        }

                        // Role selector
                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 8

                            Text {
                                text: 'Role:'
                                color: '#A0A0A0'
                                font.pixelSize: 12
                            }

                            Button {
                                text: 'Maker'
                                checkable: true
                                checked: self.isMaker
                                onClicked: self.isMaker = true
                                background: Rectangle {
                                    color: parent.checked ? '#00BCFF' : '#262626'
                                    radius: 4
                                }
                                contentItem: Text {
                                    text: parent.text
                                    color: parent.checked ? '#121416' : '#A0A0A0'
                                    font.pixelSize: 11
                                }
                            }

                            Button {
                                text: 'Taker'
                                checkable: true
                                checked: !self.isMaker
                                onClicked: self.isMaker = false
                                background: Rectangle {
                                    color: parent.checked ? '#00BCFF' : '#262626'
                                    radius: 4
                                }
                                contentItem: Text {
                                    text: parent.text
                                    color: parent.checked ? '#121416' : '#A0A0A0'
                                    font.pixelSize: 11
                                }
                            }
                        }

                        // Strike price
                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 4

                            Text {
                                text: 'Strike Price (USD):'
                                color: '#FFFFFF'
                                font.pixelSize: 12
                                font.weight: Font.Bold
                            }

                            TextField {
                                Layout.fillWidth: true
                                text: String(self.simStrike)
                                placeholderText: '70000'
                                onTextChanged: self.simStrike = parseInt(text) || 70000
                                background: Rectangle {
                                    color: '#262626'
                                    radius: 4
                                    border.color: '#262626'
                                    border.width: 1
                                }
                                color: '#FFFFFF'
                            }
                        }

                        // Premium
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
                                text: String(self.simPremium)
                                placeholderText: '10000'
                                onTextChanged: self.simPremium = parseInt(text) || 10000
                                background: Rectangle {
                                    color: '#262626'
                                    radius: 4
                                    border.color: '#262626'
                                    border.width: 1
                                }
                                color: '#FFFFFF'
                            }
                        }

                        // Outcome price
                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 4

                            Text {
                                text: 'Bitcoin Price at Expiry (USD):'
                                color: '#FFFFFF'
                                font.pixelSize: 12
                                font.weight: Font.Bold
                            }

                            SpinBox {
                                Layout.fillWidth: true
                                value: self.simOutcome
                                from: 10000
                                to: 500000
                                stepSize: 1000
                                onValueChanged: self.simOutcome = value
                            }
                        }

                        // Network fees
                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 4

                            Text {
                                text: 'Network Fees (sats):'
                                color: '#FFFFFF'
                                font.pixelSize: 12
                                font.weight: Font.Bold
                            }

                            TextField {
                                Layout.fillWidth: true
                                text: String(self.simFees)
                                placeholderText: '0'
                                onTextChanged: self.simFees = parseInt(text) || 0
                                background: Rectangle {
                                    color: '#262626'
                                    radius: 4
                                    border.color: '#262626'
                                    border.width: 1
                                }
                                color: '#FFFFFF'
                            }
                        }
                    }

                    // Simulate button
                    Button {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 44
                        text: 'SIMULATE'
                        enabled: !self.simulateLoading
                        onClicked: self.runSimulate()

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

            // Results card (conditional)
            Rectangle {
                Layout.fillWidth: true
                visible: self.hasResults
                color: '#1a1a1a'
                border.color: '#262626'
                border.width: 1
                radius: 8

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 16
                    spacing: 12

                    Text {
                        text: 'Simulation Results'
                        color: '#FFFFFF'
                        font.pixelSize: 14
                        font.weight: Font.Bold
                    }

                    // Payout chart placeholder
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 250
                        color: '#262626'
                        radius: 4

                        Text {
                            anchors.centerIn: parent
                            text: '[Payout Chart would render here]\n\nStrike: $' + self.simStrike + '\nOutcome: $' + self.simOutcome
                            color: '#A0A0A0'
                            font.pixelSize: 12
                            horizontalAlignment: Text.AlignHCenter
                        }
                    }

                    // Metrics rows
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 16

                            Text {
                                text: 'PnL (Rounded)'
                                color: '#A0A0A0'
                                font.pixelSize: 12
                                Layout.fillWidth: true
                            }

                            Text {
                                text: self.resultPnL
                                color: self.resultPnLPositive ? '#1B5E20' : '#B71C1C'
                                font.pixelSize: 13
                                font.weight: Font.Bold
                                horizontalAlignment: Text.AlignRight
                            }
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 1
                            color: '#262626'
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 16

                            Text {
                                text: 'Posted Collateral'
                                color: '#A0A0A0'
                                font.pixelSize: 12
                                Layout.fillWidth: true
                            }

                            Text {
                                text: self.resultCollateral + ' sats'
                                color: '#FFFFFF'
                                font.pixelSize: 12
                                horizontalAlignment: Text.AlignRight
                            }
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 16

                            Text {
                                text: 'Premium Paid/Received'
                                color: '#A0A0A0'
                                font.pixelSize: 12
                                Layout.fillWidth: true
                            }

                            Text {
                                text: self.resultPremium + ' sats'
                                color: '#FFFFFF'
                                font.pixelSize: 12
                                horizontalAlignment: Text.AlignRight
                            }
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 16

                            Text {
                                text: 'Network Fees'
                                color: '#A0A0A0'
                                font.pixelSize: 12
                                Layout.fillWidth: true
                            }

                            Text {
                                text: self.resultFees + ' sats'
                                color: '#FFFFFF'
                                font.pixelSize: 12
                                horizontalAlignment: Text.AlignRight
                            }
                        }
                    }
                }
            }

            // Info banner
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: infoBanner.implicitHeight + 12
                visible: self.simulateLoading
                color: '#00BCFF'
                radius: 4

                ColumnLayout {
                    id: infoBanner
                    anchors.fill: parent
                    anchors.margins: 8
                    spacing: 4

                    Text {
                        text: 'Simulating payout...'
                        color: '#121416'
                        font.pixelSize: 12
                    }

                    BusyIndicator {
                        Layout.preferredWidth: 24
                        Layout.preferredHeight: 24
                        running: true
                    }
                }
            }

            Item { Layout.fillHeight: true }
        }
    }

    // Properties
    property bool buyerSide: true
    property real simQty: 1.0
    property bool isCall: true
    property bool isMaker: true
    property int simStrike: 70000
    property int simPremium: 10000
    property int simOutcome: 75000
    property int simFees: 0

    property bool simulateLoading: false
    property bool hasResults: false

    property string resultPnL: '+25,000'
    property bool resultPnLPositive: true
    property string resultCollateral: '500,000'
    property string resultPremium: '10,000'
    property string resultFees: '500'

    // Signals/Slots
    function runSimulate() {
        console.log('Run simulation')
        self.simulateLoading = true
        self.hasResults = false
        
        // Simulate async work
        Timer {
            interval: 1500
            running: self.simulateLoading
            onTriggered: {
                self.simulateLoading = false
                self.hasResults = true
            }
        }
    }
}

