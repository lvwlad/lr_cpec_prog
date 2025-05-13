var apiKey = "53f2e1b43a53b9ec86018cdf1e816cfa"
var baseUrl = "https://api.openweathermap.org/data/2.5/weather"
var forecastUrl = "https://api.openweathermap.org/data/2.5/forecast"

function fetchWeather(city, units) {
    if (!city) {
        errorMessage.text = qsTr("Введите город")
        errorMessage.visible = true
        return
    }

    var xhr = new XMLHttpRequest()
    var url = baseUrl + "?q=" + encodeURIComponent(city) +
              "&appid=" + apiKey + "&units=" + units + "&lang=ru"

    xhr.onreadystatechange = function () {
        if (xhr.readyState === XMLHttpRequest.DONE) {
            if (xhr.status === 200) {
                var response = JSON.parse(xhr.responseText)
                cityName.text = qsTr("Город: ") + response.name
                temperature.text = qsTr("Температура: ") + response.main.temp + (units === "metric" ? "°C" : "°F")
                description.text = qsTr("Описание: ") + response.weather[0].description
                humidity.text = qsTr("Влажность: ") + response.main.humidity + "%"
                var windSpeed = units === "metric" ? response.wind.speed : response.wind.speed * 0.44704
                wind.text = qsTr("Ветер: ") + windSpeed.toFixed(1) + " м/с"
                errorMessage.visible = false
                weatherBlock.opacity = 1.0
            } else {
                errorMessage.text = qsTr("Ошибка: проверьте название города")
                errorMessage.visible = true
                weatherBlock.opacity = 0.0
            }
        }
    }

    xhr.open("GET", url)
    xhr.send()
}

function fetchForecast(city, units) {
    if (!city) {
        errorMessage.text = qsTr("Введите город")
        errorMessage.visible = true
        return
    }

    var xhr = new XMLHttpRequest()
    var url = forecastUrl + "?q=" + encodeURIComponent(city) +
              "&appid=" + apiKey + "&units=" + units + "&lang=ru"

    xhr.onreadystatechange = function () {
        if (xhr.readyState === XMLHttpRequest.DONE) {
            if (xhr.status === 200) {
                var response = JSON.parse(xhr.responseText)
                processForecastData(response)
            } else {
                errorMessage.text = qsTr("Ошибка: проверьте название города")
                errorMessage.visible = true
            }
        }
    }

    xhr.open("GET", url)
    xhr.send()
}

function processForecastData(data) {
    var forecastList = data.list
    var dailyData = {}

    forecastList.forEach(function (item) {
        var date = new Date(item.dt * 1000)
        var day = date.toLocaleDateString()

        if (!dailyData[day]) {
            dailyData[day] = {
                temps: [],
                descriptions: [],
                pops: []
            }
        }

        dailyData[day].temps.push(item.main.temp)
        dailyData[day].descriptions.push(item.weather[0].description)
        dailyData[day].pops.push(item.pop * 100)
    })

    forecastModel.clear()
    var count = 0
    for (var day in dailyData) {
        if (count >= 3) break
        var temps = dailyData[day].temps
        var minTemp = Math.min(...temps)
        var maxTemp = Math.max(...temps)
        var description = dailyData[day].descriptions[0]
        var pop = Math.max(...dailyData[day].pops).toFixed(0)
        forecastModel.append({
            date: day,
            tempMin: minTemp.toFixed(1),
            tempMax: maxTemp.toFixed(1),
            desc: description,
            pop: pop
        })
        count++
    }
}

function fetchHourlyForecast(city, units) {
    if (!city) {
        errorMessage.text = qsTr("Введите город")
        errorMessage.visible = true
        return
    }

    var xhr = new XMLHttpRequest()
    var url = forecastUrl + "?q=" + encodeURIComponent(city) +
              "&appid=" + apiKey + "&units=" + units + "&lang=ru"

    xhr.onreadystatechange = function () {
        if (xhr.readyState === XMLHttpRequest.DONE) {
            if (xhr.status === 200) {
                var response = JSON.parse(xhr.responseText)
                processHourlyData(response)
} else {
                errorMessage.text = qsTr("Ошибка: проверьте название города")
                errorMessage.visible = true
            }
        }
    }

    xhr.open("GET", url)
    xhr.send()
}

var hourlyTemps = []
var hourlyHumidities = []
var hourlyWinds = []
var hourlyTimes = []

function processHourlyData(data) {
    var hourlyData = data.list.slice(0, 8)
    hourlyTemps = hourlyData.map(item => item.main.temp)
    hourlyHumidities = hourlyData.map(item => item.main.humidity)
    hourlyWinds = hourlyData.map(item => {
        return weatherChart.units === "metric" ? item.wind.speed : item.wind.speed * 0.44704
    })
    hourlyTimes = hourlyData.map(item => {
        var date = new Date(item.dt * 1000)
        return date.getHours() + ":00"
    })
    updateGraph(weatherChart.graphType)
}

function updateGraph(graphType) {
    if (graphType === "temperature") {
        weatherChart.dataPoints = hourlyTemps
    } else if (graphType === "humidity") {
        weatherChart.dataPoints = hourlyHumidities
    } else {
        weatherChart.dataPoints = hourlyWinds
    }
    weatherChart.times = hourlyTimes
    weatherChart.requestPaint()
}
