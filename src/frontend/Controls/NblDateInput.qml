import QtQuick

NblTextBox {

    property var date: new Date()
    property string format: "yyyy/MM/dd"

    function getTime() {
        return date.getTime()
    }

    function setTime(time) {
        date.setTime(time)
    }

    signal dateEdited

    placeholderText: format
    text: Qt.formatDate(date, format)
    validator: RegularExpressionValidator { 
        regularExpression: /^\d{4}\/(0[1-9]|1[0-2])\/(0[1-9]|[12]\d|3[01])$/ 
    }

    onDateChanged: {
        text = date.toLocaleDateString(Qt.locale(), format)
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
