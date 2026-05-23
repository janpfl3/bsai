import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: orderBookView
    color: "transparent"

    property string instrumentId

    ColumnLayout {
        anchors.fill: parent
        
        // Bids (buy side)
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#1a1a1a"
            
            Column {
                anchors.fill: parent
                Text {
                    text: qsTr("Buy Orders")
                    font.bold: true
                    padding: 8
                }
                
                ListView {
                    width: parent.width
                    height: parent.height - 32
                    model: bidModel
                    
                    delegate: Row {
                        width: parent.width
                        padding: 8
                        
                        Text { text: model.price; width: parent.width * 0.33 }
                        Text { text: model.size; width: parent.width * 0.33 }
                        Text { text: model.total; width: parent.width * 0.34 }
                    }
                }
            }
        }

        // Asks (sell side)
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#1a1a1a"
            
            Column {
                anchors.fill: parent
                Text {
                    text: qsTr("Sell Orders")
                    font.bold: true
                    padding: 8
                }
                
                ListView {
                    width: parent.width
                    height: parent.height - 32
                    model: askModel
                    
                    delegate: Row {
                        width: parent.width
                        padding: 8
                        
                        Text { text: model.price; width: parent.width * 0.33 }
                        Text { text: model.size; width: parent.width * 0.33 }
                        Text { text: model.total; width: parent.width * 0.34 }
                    }
                }
            }
        }
    }
}
