import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Card {
    id: positionCard
    
    required property string dlcId
    required property string instrumentId
    required property string role  // "maker" or "taker"
    required property long collateralSats
    required property string status
    required property string fundingTxId
    required property string settlementOutcome

    Column {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8

        // Header: Instrument & Status
        RowLayout {
            width: parent.width
            
            Text {
                text: instrumentId
                font.bold: true
                Layout.fillWidth: true
            }
            
            Badge {
                text: status
                color: status === "ACTIVE" ? "#00aa00" : 
                       status === "SETTLED" ? "#0066ff" : "#ffaa00"
            }
        }

        Divider { Layout.fillWidth: true }

        // Contract Details
        GridLayout {
            columns: 2
            width: parent.width
            
            Text { text: qsTr("Role:") }
            Text { text: role; font.bold: true }
            
            Text { text: qsTr("Collateral:") }
            Text { text: formatSats(collateralSats); font.bold: true }
            
            Text { text: qsTr("Funding Tx:") }
            Text { 
                text: fundingTxId.substring(0, 16) + "..."
                font.pointSize: 9
                MouseArea {
                    anchors.fill: parent
                    onClicked: copyToClipboard(fundingTxId)
                }
            }
        }

        // Status timeline
        ColumnLayout {
            spacing: 4
            
            StatusTimelineItem { 
                label: qsTr("Contract Signed")
                completed: status !== "AWAITING_SIGNATURES"
            }
            StatusTimelineItem {
                label: qsTr("Funding Confirmed")
                completed: ["ACTIVE", "SETTLED"].includes(status)
            }
            StatusTimelineItem {
                label: qsTr("Settled")
                completed: status === "SETTLED"
                outcome: settlementOutcome
            }
        }

        // Actions
        RowLayout {
            Layout.fillWidth: true
            
            Button {
                text: qsTr("View Details")
                Layout.fillWidth: true
                onClicked: showPositionDetails(dlcId)
            }
        }
    }
}
