import QtQuick

NblTextBox {

    property var date: new Date()
    property string format: Theme.dateFormat
    property bool utc: true

    function getTime() {
        return date.getTime()
    }

    function setTime(time) {
        date.setTime(time)
        text = getDateString()
    }

    function getDateString() {
        if(utc) {
            return Qt.formatDate(new Date(date.getTime()), format)
        }
        return date.toLocaleDateString(Qt.locale(), format)
    }

    signal dateEdited

    placeholderText: format
    text: getDateString()
    validator: RegularExpressionValidator { 
        regularExpression: /^\d{4}\/(0[1-9]|1[0-2])\/(0[1-9]|[12]\d|3[01])$/ 
    }

    onDateChanged: {
        text = getDateString()
    }

    onTextEdited: {

        if(text.length != 0) {
            let parsedDate = Date.fromLocaleDateString(Qt.locale(), text, format)
            if (isNaN(parsedDate.getTime())) {
                return;
            } 
            date = parsedDate
            dateEdited()
        }

    }

}
