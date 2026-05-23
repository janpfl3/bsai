Dialog {
    id: dlcErrorDialog
    
    property string errorCode
    property string errorMessage
    property string dlcId
    
    ColumnLayout {
        anchors.fill: parent
        
        Text {
            text: qsTr("DLC Error")
            font.bold: true
            font.pointSize: 14
        }
        
        Text {
            text: {
                switch(errorCode) {
                    case "COORDINATOR_UNREACHABLE":
                        return qsTr("Cannot reach the DLC Coordinator. Check your internet connection.")
                    case "SIGNATURE_VERIFICATION_FAILED":
                        return qsTr("The coordinator's signature could not be verified. This DLC may be compromised.")
                    case "INSUFFICIENT_COLLATERAL":
                        return qsTr("Your account doesn't have enough Bitcoin for this order.")
                    case "FUNDING_TX_FAILED":
                        return qsTr("The funding transaction could not be broadcast.")
                    default:
                        return errorMessage
                }
            }
            wrapMode: Text.Wrap
        }
        
        // Recommended actions
        Text {
            text: qsTr("Recommended Action:")
            font.bold: true
        }
        
        Text {
            text: {
                switch(errorCode) {
                    case "COORDINATOR_UNREACHABLE":
                        return qsTr("• Check your internet connection\n• Verify coordinator URL in settings\n• Retry in a moment")
                    case "SIGNATURE_VERIFICATION_FAILED":
                        return qsTr("• Contact support immediately\n• Do not proceed with this DLC\n• Check coordinator security status")
                    case "INSUFFICIENT_COLLATERAL":
                        return qsTr("• Receive more Bitcoin to your account\n• Reduce order size\n• Try a different instrument")
                    default:
                        return ""
                }
            }
            wrapMode: Text.Wrap
            font.pointSize: 9
        }
        
        RowLayout {
            Layout.fillWidth: true
            
            Button {
                text: qsTr("Copy Error Details")
                onClicked: copyToClipboard(dlcId + ": " + errorCode + " - " + errorMessage)
            }
            Button {
                text: qsTr("OK")
                onClicked: dlcErrorDialog.close()
            }
        }
    }
}
