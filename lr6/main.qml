import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Shapes 1.15
import QtCore
import "WeatherFetcher.js" as WeatherFetcher

ApplicationWindow {
    visible: true
    width: 400
    height: 600
    title: qsTr("Погодный информатор")

    Settings {
        id: appSettings
        property string lastCity: ""
    }

    ScrollView {
        anchors.fill: parent
        clip: true

        ColumnLayout {
            width: parent.width
            spacing: 10

            TextField {
                id: cityInput
                placeholderText: qsTr("Введите город")
                Layout.preferredWidth: 300
                Layout.alignment: Qt.AlignHCenter
                font.pointSize: 16
                background: Rectangle {
                    color: "#ffffff"
                    radius: 5
                    border.color: "#cccccc"
                }
            }

            ComboBox {
                id: unitsCombo
                model: [
                    { text: qsTr("Цельсий"), value: "metric" },
                    { text: qsTr("Фаренгейт"), value: "imperial" }
                ]
                textRole: "text"
                valueRole: "value"
                Layout.preferredWidth: 300
                Layout.alignment: Qt.AlignHCenter
                font.pointSize: 16
                currentIndex: 0
            }

            Button {
                text: qsTr("Обновить погоду")
                font.pointSize: 16
                Layout.preferredWidth: 300
                Layout.alignment: Qt.AlignHCenter
                background: Rectangle {
                    color: "#4caf50"
                    radius: 5
                }
                onClicked: {
                    WeatherFetcher.fetchWeather(cityInput.text, unitsCombo.currentValue)
                    WeatherFetcher.fetchForecast(cityInput.text, unitsCombo.currentValue)
                    WeatherFetcher.fetchHourlyForecast(cityInput.text, unitsCombo.currentValue)
                    appSettings.lastCity = cityInput.text
                }
            }

            ColumnLayout {
                id: weatherBlock
                spacing: 10
                opacity: 0.0
                Layout.alignment: Qt.AlignHCenter
                visible: !
                errorMessage.visible

                Behavior on opacity {
                    NumberAnimation { duration: 500 }
                }

                Text {
                    id: cityName
                    text: qsTr("Город: -")
                    font.pointSize: 18
                    font.bold: true
                    color: "#333"
                }

                Text {
                    id: temperature
                    text: qsTr("Температура: -")
                    font.pointSize: 18
                    color: "#ff5722"
                    font.bold: true
                }

                Text {
                    id: description
                    text: qsTr("Описание: -")
                    font.pointSize: 14
                    color: "#757575"
                }

                Text {
                    id: humidity
                    text: qsTr("Влажность: -")
                    font.pointSize: 14
                    color: "#757575"
                }

                Text {
                    id: wind
                    text: qsTr("Ветер: -")
                    font.pointSize: 14
                    color: "#757575"
                }
            }

            Text {
                text: qsTr("Прогноз на 3 дня:")
                font.pointSize: 14
                font.bold: true
                color: "#333"
                Layout.alignment: Qt.AlignHCenter
                visible: !
                errorMessage.visible
            }

            ListView {
                id: forecastList
                Layout.preferredWidth: 300
                Layout.preferredHeight: 120
                Layout.alignment: Qt.AlignHCenter
                model: ListModel {
                    id: forecastModel
                }
                delegate: Text {
                    text: date + " | " + tempMin + "° - " + tempMax + "° | " + desc + " | Осадки: " + pop + "%"
                    font.pointSize: 12
                    color: "#333"
                    width: forecastList.width
                    wrapMode: Text.Wrap
                }
                visible: !
                errorMessage.visible
            }

            ComboBox {
                id: graphTypeCombo
                model: [
                    { text: qsTr("Температура"), value: "temperature" },
                    { text: qsTr("Влажность"), value: "humidity" },
                    { text: qsTr("Скорость ветра"), value: "wind" }
                ]
                textRole: "text"
                valueRole: "value"
                Layout.preferredWidth: 300
                Layout.alignment: Qt.AlignHCenter
                font.pointSize: 16
                currentIndex: 0
                onCurrentValueChanged: {
                    WeatherFetcher.updateGraph(graphTypeCombo.currentValue)
                }
                visible: !
                errorMessage.visible
            }

            Text {
                id: yAxisLabel
                text: graphTypeCombo.currentValue === "temperature" ? qsTr("Температура (°C)") :
                      graphTypeCombo.currentValue === "humidity" ? qsTr("Влажность (%)") :
                      qsTr("Скорость ветра (м/с)")
                font { pointSize: 12; bold: true }
                color: "#333"
                Layout.alignment: Qt.AlignHCenter
                visible: !
                errorMessage.visible
            }

            Canvas {
                id: weatherChart
                Layout.preferredWidth: 300
                Layout.preferredHeight: 180
                Layout.alignment: Qt.AlignHCenter
                property var dataPoints: []
                property var times: []
                property string graphType: graphTypeCombo.currentValue
                property string units: unitsCombo.currentValue
                visible: !
                errorMessage.visible

                onPaint: {
                    var ctx = getContext("2d")
                    ctx.clearRect(0, 0, width, height)
                    if (dataPoints.length === 0) return

                    var maxValue = Math.max(...dataPoints)
                    var minValue = Math.min(...dataPoints)
                    var range = maxValue - minValue || 1

                    var stepX = width / (dataPoints.length - 1)
                    var stepY = height / range

                    // Отрисовка сетки и осей
                    ctx.strokeStyle = "#888"
                    ctx.lineWidth = 0.5
                    ctx.font = "10px Arial"

                    // Горизонтальные линии (Y)
                    var ySteps = 5
                    for (var i = 0; i <= ySteps; i++) {
                        var yVal = minValue + (range * i / ySteps)
                        var yPos = height - (yVal - minValue) * stepY

                        ctx.beginPath()
                        ctx.moveTo(0, yPos)
                        ctx.lineTo(width, yPos)
                        ctx.stroke()

                        ctx.fillStyle = "#000"
                        ctx.fillText(yVal.toFixed(1), 5, yPos - 5)
                    }

                    // Вертикальные линии (X)
                    times.forEach(function (time, index) {
                        var xPos = index * stepX
                        ctx.beginPath()
                        ctx.moveTo(xPos, 0)
                        ctx.lineTo(xPos, height)
                        ctx.stroke()
                    })

                    // Оси X и Y
                    ctx.beginPath()
                    ctx.moveTo(0, height)
                    ctx.lineTo(width, height)
                    ctx.moveTo(0, 0)
                    ctx.lineTo(0, height)
                    ctx.strokeStyle = "#000"
                    ctx.lineWidth = 1
                    ctx.stroke()

                    // Подписи времени на оси X
                    ctx.font = "10px Arial"
                    ctx.fillStyle = "#000"
                    times.forEach(function (time, index) {
                        ctx.fillText(time, index * stepX, height - 5)
                    })

                    // Отрисовка графика
                    var gradient = ctx.createLinearGradient(0, 0, 0, height)
                    if (graphType === "temperature") {
                        gradient.addColorStop(0, "blue")
                        gradient.addColorStop(1, "red")
                    } else if (graphType === "humidity") {
                        gradient.addColorStop(0, "green")
                        gradient.addColorStop(1, "blue")
                    } else {
                        gradient.addColorStop(0, "gray")
                        gradient.addColorStop(1, "orange")
                    }

                    ctx.beginPath()
                    ctx.moveTo(0, height - (dataPoints[0] - minValue) * stepY)
                    for (var j = 1; j < dataPoints.length; j++) {
                        ctx.lineTo(j * stepX, height - (dataPoints[j] - minValue) * stepY)
                    }
                    ctx.strokeStyle = gradient
                    ctx.lineWidth = 2
                    ctx.stroke()
                }
            }

            Text {
                id: xAxisLabel
                text: qsTr("Время")
                font { pointSize: 12; bold: true }
                color: "#333"
                Layout.alignment: Qt.AlignHCenter
                visible: !
                errorMessage.visible
            }

            Text {
                id: errorMessage
                color: "red"
                font.pointSize: 12
                wrapMode: Text.Wrap
                visible: false
                Layout.preferredWidth: 300
                Layout.alignment: Qt.AlignHCenter
                horizontalAlignment: Text.AlignHCenter
            }
        }

        Component.onCompleted: {
            if (appSettings.lastCity !== "") {
                cityInput.text = appSettings.lastCity
                WeatherFetcher.fetchWeather(appSettings.lastCity, unitsCombo.currentValue)
                WeatherFetcher.fetchForecast(appSettings.lastCity, unitsCombo.currentValue)
                WeatherFetcher.fetchHourlyForecast(appSettings.lastCity, unitsCombo.currentValue)
            }
        }
    }
}
