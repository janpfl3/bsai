import QtQuick
import QtQuick.Controls

ListView {
    ScrollIndicator.vertical: ScrollIndicator {
    }
    id: self
    clip: true
    focusPolicy: Qt.ClickFocus
}
