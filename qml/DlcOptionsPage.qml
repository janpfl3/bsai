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

    // Tab indices
    enum Tab {
        Overview = 0,
        Trade = 1,
        MyOrders = 2,
        Simulate = 3
    }

    contentItem: ColumnLayout {
        spacing: 0

        // Top navigation bar with tabs
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 56
            color: '#1a1a1a'
            border.color: '#262626'
            border.width: 1

            RowLayout {
                anchors.fill: parent
                anchors.margins: 8
                spacing: 8

                Repeater {
                    model: [
                        { text: 'Overview', index: DlcOptionsPage.Tab.Overview },
                        { text: 'Trade', index: DlcOptionsPage.Tab.Trade },
                        { text: 'My Orders', index: DlcOptionsPage.Tab.MyOrders },
                        { text: 'Simulate', index: DlcOptionsPage.Tab.Simulate }
                    ]

                    Button {
                        Layout.preferredHeight: 40
                        text: modelData.text
                        enabled: !self.loading && !self.actionInProgress
                        onClicked: self.currentTab = modelData.index

                        background: Rectangle {
                            color: self.currentTab === modelData.index ? '#00BCFF' : '#262626'
                            border.color: '#00BCFF'
                            border.width: self.currentTab === modelData.index ? 1 : 0
                            radius: 4
                        }

                        contentItem: Text {
                            text: parent.text
                            color: self.currentTab === modelData.index ? '#121416' : '#A0A0A0'
                            font.pixelSize: 14
                            font.weight: self.currentTab === modelData.index ? Font.Bold : Font.Normal
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                    }
                }

                Item { Layout.fillWidth: true }
            }
        }

        // Error message banner
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: errorLabel.implicitHeight + 16
            color: '#B71C1C'
            visible: self.errorMessage !== ''

            RowLayout {
                anchors.fill: parent
                anchors.margins: 8
                spacing: 8

                Image {
                    source: 'qrc:/svg2/exclamation-circle.svg'
                    Layout.preferredWidth: 20
                    Layout.preferredHeight: 20
                }

                Text {
                    id: errorLabel
                    text: self.errorMessage
                    color: '#FFFFFF'
                    font.pixelSize: 13
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                }

                Button {
                    text: '✕'
                    font.pixelSize: 16
                    onClicked: self.clearTransientMessages()
                    background: Rectangle { color: 'transparent' }
                }
            }
        }

        // Info message banner
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: infoLabel.implicitHeight + 16
            color: '#00BCFF'
            visible: self.infoMessage !== ''

            RowLayout {
                anchors.fill: parent
                anchors.margins: 8
                spacing: 8

                Image {
                    source: 'qrc:/svg2/info.svg'
                    Layout.preferredWidth: 20
                    Layout.preferredHeight: 20
                }

                Text {
                    id: infoLabel
                    text: self.infoMessage
                    color: '#121416'
                    font.pixelSize: 13
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                }

                Button {
                    text: '✕'
                    font.pixelSize: 16
                    onClicked: self.clearTransientMessages()
                    background: Rectangle { color: 'transparent' }
                }
            }
        }

        // Tab content
        StackLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: self.currentTab

            DlcOverviewTab {
                id: overviewTab
                context: self.context
            }

            DlcTradeTab {
                id: tradeTab
                context: self.context
            }

            DlcMyOrdersTab {
                id: myOrdersTab
                context: self.context
            }

            DlcSimulateTab {
                id: simulateTab
                context: self.context
            }
        }
    }

    // Loading overlay
    Rectangle {
        anchors.fill: parent
        color: '#000000'
        opacity: 0.6
        visible: self.actionInProgress || self.orderbookRefreshing

        BusyIndicator {
            anchors.centerIn: parent
            width: 80
            height: 80
            running: parent.visible
        }
    }

    // Properties (would normally come from DlcPageModel in C++)
    property int currentTab: DlcOptionsPage.Tab.Overview
    property bool loading: false
    property bool actionInProgress: false
    property bool orderbookRefreshing: false
    property string errorMessage: ''
    property string infoMessage: ''

    // Slots (would normally connect to DlcPageModel methods)
    function clearTransientMessages() {
        self.errorMessage = ''
        self.infoMessage = ''
    }
}
<<<<<<< HEAD
=======

>>>>>>> 2a70e837 (initial with DLC)
