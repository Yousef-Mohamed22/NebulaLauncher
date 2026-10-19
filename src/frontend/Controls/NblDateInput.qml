// Copyright 2026 jwall64
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

import QtQuick

import Nebula

NblTextBox {

    property var date: new Date()
    property string format: Theme.dateInputFormat
    property bool utc: true
    property var time: date.getTime()

    onTimeChanged: {
        date.setTime(time)
    }

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
        time = date.getTime()
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
