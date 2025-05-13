import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Qt.labs.platform 1.1

ApplicationWindow {
    id: window
    width: 800
    height: 600
    visible: true
    title: qsTr("Библиотека")

    property int currentBookId: -1
    property int currentReaderId: -1

    TabBar {
        id: tabBar
        width: parent.width

        TabButton {
            text: "Книги"
        }
        TabButton {
            text: "Читатели"
        }
    }

    StackLayout {
        anchors.top: tabBar.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        currentIndex: tabBar.currentIndex

        // Вкладка "Книги"
        ColumnLayout {
            spacing: 10

            RowLayout {
                Layout.fillWidth: true
                Layout.leftMargin: 10
                Layout.rightMargin: 10
                spacing: 10

                TextField {
                    id: searchField
                    placeholderText: "Поиск по названию, автору или жанру"
                    Layout.fillWidth: true
                    onTextChanged: refreshBookList()
                }

                CheckBox {
                    id: availableFilter
                    text: "Только доступные"
                    onCheckedChanged: refreshBookList()
                }

                Button {
                    text: "Расширенный поиск"
                    onClicked: advancedSearchPopup.open()
                }

                Button {
                    text: "Экспорт в CSV"
                    onClicked: fileDialog.open()
                }

                Button {
                    text: "Добавить книгу"
                    onClicked: {
                        currentBookId = -1
                        titleField.text = ""
                        authorField.text = ""
                        yearField.text = ""
                        genreField.text = ""
                        availableCheckBox.checked = true
                        bookFormPopup.open()
                    }
                }
            }

            ScrollView {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.leftMargin: 10
                Layout.rightMargin: 10

                ListView {
                    id: bookListView
                    width: parent.width
                    height: parent.height
                    clip: true
                    spacing: 5
                    model: ListModel { id: bookModel }

                    delegate: ItemDelegate {
                        width: bookListView.width
                        height: 80
                        leftPadding: 10
                        rightPadding: 10

                        background: Rectangle {
                            color: "transparent"
                            border.color: "#eee"
                            radius: 5
                        }

                        RowLayout {
                            anchors.fill: parent
                            spacing: 10

                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 5

                                Label {
                                    text: title
                                    font.bold: true
                                    elide: Text.ElideRight
                                    Layout.fillWidth: true
                                }

                                Label {
                                    text: author + " (" + year + ")"
                                    elide: Text.ElideRight
                                    Layout.fillWidth: true
                                }

                                Label {
                                    text: genre + (reader_name ? " (Выдана: " + reader_name + ")" : "")
                                    color: "gray"
                                    elide: Text.ElideRight
                                    Layout.fillWidth: true
                                }
                            }

                            Label {
                                text: available ? "Доступна" : "Недоступна"
                                color: available ? "green" : "red"
                            }

                            RowLayout {
                                spacing: 5

                                Button {
                                    text: available ? "Выдать" : "Вернуть"
                                    onClicked: {
                                        if (available) {
                                            currentBookId = id
                                            issueBookPopup.open()
                                        } else {
                                            if (database.returnBook(id)) {
                                                refreshBookList()
                                            }
                                        }
                                    }
                                }

                                Button {
                                    text: "Изменить"
                                    onClicked: {
                                        currentBookId = id
                                        titleField.text = title
                                        authorField.text = author
                                        yearField.text = year
                                        genreField.text = genre
                                        availableCheckBox.checked = available
                                        bookFormPopup.open()
                                    }
                                }

                                Button {
                                    text: "Удалить"
                                    onClicked: {
                                        console.log("Attempting to delete book with ID:", id)
                                        deleteBookPopup.bookId = id
                                        deleteBookPopup.open()
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        // Вкладка "Читатели"
        ColumnLayout {
            spacing: 10

            RowLayout {
                Layout.fillWidth: true
                Layout.leftMargin: 10
                Layout.rightMargin: 10
                spacing: 10

                Button {
                    text: "Добавить читателя"
                    onClicked: {
                        currentReaderId = -1
                        readerNameField.text = ""
                        readerContactField.text = ""
                        readerFormPopup.open()
                    }
                }
            }

            ScrollView {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.leftMargin: 10
                Layout.rightMargin: 10

                ListView {
                    id: readerListView
                    width: parent.width
                    height: parent.height
                    clip: true
                    spacing: 5
                    model: ListModel { id: readerModel }

                    delegate: ItemDelegate {
                        width: readerListView.width
                        height: 60
                        leftPadding: 10
                        rightPadding: 10

                        background: Rectangle {
                            color: "transparent"
                            border.color: "#eee"
                            radius: 5
                        }

                        RowLayout {
                            anchors.fill: parent
                            spacing: 10

                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 5

                                Label {
                                    text: name
                                    font.bold: true
                                    elide: Text.ElideRight
                                    Layout.fillWidth: true
                                }

                                Label {
                                    text: contact
                                    color: "gray"
                                    elide: Text.ElideRight
                                    Layout.fillWidth: true
                                }
                            }

                            RowLayout {
                                spacing: 5

                                Button {
                                    text: "Изменить"
                                    onClicked: {
                                        currentReaderId = id
                                        readerNameField.text = name
                                        readerContactField.text = contact
                                        readerFormPopup.open()
                                    }
                                }

                                Button {
                                    text: "Удалить"
                                    onClicked: {
                                        console.log("Attempting to delete reader with ID:", id)
                                        deleteReaderPopup.readerId = id
                                        deleteReaderPopup.open()
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    Popup {
        id: bookFormPopup
        width: Math.min(window.width * 0.9, 400)
        height: Math.min(window.height * 0.9, 350)
        x: (window.width - width) / 2
        y: (window.height - height) / 2
        modal: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        padding: 10

        ColumnLayout {
            anchors.fill: parent
            spacing: 10

            Label {
                text: currentBookId === -1 ? "Добавить книгу" : "Изменить книгу"
                font.bold: true
                font.pixelSize: 18
            }

            TextField {
                id: titleField
                placeholderText: "Название"
                Layout.fillWidth: true
            }

            TextField {
                id: authorField
                placeholderText: "Автор"
                Layout.fillWidth: true
            }

            TextField {
                id: yearField
                placeholderText: "Год издания"
                validator: IntValidator { bottom: 0; top: 2100 }
                Layout.fillWidth: true
            }

            TextField {
                id: genreField
                placeholderText: "Жанр"
                Layout.fillWidth: true
            }

            CheckBox {
                id: availableCheckBox
                text: "Доступна"
                checked: true
            }

            RowLayout {
                Button {
                    text: "Отмена"
                    Layout.fillWidth: true
                    onClicked: bookFormPopup.close()
                }

                Button {
                    text: currentBookId === -1 ? "Добавить" : "Сохранить"
                    Layout.fillWidth: true
                    onClicked: {
                        if (currentBookId === -1) {
                            if (database.addBook(titleField.text, authorField.text,
                                               parseInt(yearField.text),
                                               genreField.text,
                                               availableCheckBox.checked)) {
                                refreshBookList()
                                bookFormPopup.close()
                            }
                        } else {
                            if (database.updateBook(currentBookId, titleField.text,
                                                   authorField.text,
                                                   parseInt(yearField.text),
                                                   genreField.text,
                                                   availableCheckBox.checked)) {
                                refreshBookList()
                                bookFormPopup.close()
                            }
                        }
                    }
                }
            }
        }
    }

    Popup {
        id: deleteBookPopup
        width: Math.min(window.width * 0.9, 300)
        height: Math.min(window.height * 0.9, 200)
        x: (window.width - width) / 2
        y: (window.height - height) / 2
        modal: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        padding: 10
        property int bookId: -1

        ColumnLayout {
            anchors.fill: parent
            spacing: 10

            Label {
                text: "Вы уверены, что хотите удалить эту книгу?"
                wrapMode: Text.Wrap
                Layout.fillWidth: true
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 10

                Button {
                    text: "Отмена"
                    Layout.fillWidth: true
                    onClicked: {
                        console.log("Deletion cancelled for book ID:", deleteBookPopup.bookId)
                        deleteBookPopup.close()
                    }
                }

                Button {
                    text: "Удалить"
                    Layout.fillWidth: true
                    onClicked: {
                        console.log("Confirmed deletion for book ID:", deleteBookPopup.bookId)
                        if (database.deleteBook(deleteBookPopup.bookId)) {
                            console.log("Book deleted successfully")
                            refreshBookList()
                            resultPopup.text = "Книга успешно удалена"
                        } else {
                            console.log("Failed to delete book")
                            resultPopup.text = "Ошибка при удалении книги"
                        }
                        deleteBookPopup.close()
                        resultPopup.open()
                    }
                }
            }
        }
    }

    Popup {
        id: resultPopup
        width: Math.min(window.width * 0.9, 300)
        height: Math.min(window.height * 0.9, 150)
        x: (window.width - width) / 2
        y: (window.height - height) / 2
        modal: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        padding: 10
        property string text: ""

        ColumnLayout {
            anchors.fill: parent
            spacing: 10

            Label {
                text: resultPopup.text
                wrapMode: Text.Wrap
                Layout.fillWidth: true
            }

            Button {
                text: "ОК"
                Layout.fillWidth: true
                onClicked: resultPopup.close()
            }
        }
    }

    FileDialog {
        id: fileDialog
        title: "Сохранить CSV"
        nameFilters: ["CSV files (*.csv)"]
        fileMode: FileDialog.SaveFile
        onAccepted: {
            var path = fileDialog.file.toString().replace("file://", "")
            if (database.exportToCSV(path)) {
                console.log("Exported to", path)
            } else {
                console.log("Export failed")
            }
        }
    }

    Popup {
        id: advancedSearchPopup
        width: Math.min(window.width * 0.9, 400)
        height: Math.min(window.height * 0.9, 400)
        x: (window.width - width) / 2
        y: (window.height - height) / 2
        modal: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        padding: 10

        ColumnLayout {
            anchors.fill: parent
            spacing: 10

            Label {
                text: "Расширенный поиск"
                font.bold: true
                font.pixelSize: 18
            }

            TextField {
                id: minYearField
                placeholderText: "Минимальный год"
                validator: IntValidator { bottom: 0; top: 2100 }
                Layout.fillWidth: true
            }

            TextField {
                id: maxYearField
                placeholderText: "Максимальный год"
                validator: IntValidator { bottom: 0; top: 2100 }
                Layout.fillWidth: true
            }

            TextField {
                id: authorFieldAdv
                placeholderText: "Автор"
                Layout.fillWidth: true
            }

            TextField {
                id: genreFieldAdv
                placeholderText: "Жанр"
                Layout.fillWidth: true
            }

            CheckBox {
                id: availableFilterAdv
                text: "Только доступные"
                checked: availableFilter.checked
            }

            RowLayout {
                Button {
                    text: "Отмена"
                    Layout.fillWidth: true
                    onClicked: advancedSearchPopup.close()
                }

                Button {
                    text: "Применить"
                    Layout.fillWidth: true
                    onClicked: {
                        var minYear = minYearField.text ? parseInt(minYearField.text) : 0;
                        var maxYear = maxYearField.text ? parseInt(maxYearField.text) : 0;
                        bookModel.clear()
                        var books = database.searchBooksAdvanced(searchField.text, minYear, maxYear, availableFilterAdv.checked, authorFieldAdv.text, genreFieldAdv.text)
                        for (var i = 0; i < books.length; i++) {
                            bookModel.append(books[i])
                        }
                        advancedSearchPopup.close()
                    }
                }
            }
        }
    }

    Popup {
        id: readerFormPopup
        width: Math.min(window.width * 0.9, 400)
        height: Math.min(window.height * 0.9, 250)
        x: (window.width - width) / 2
        y: (window.height - height) / 2
        modal: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        padding: 10

        ColumnLayout {
            anchors.fill: parent
            spacing: 10

            Label {
                text: currentReaderId === -1 ? "Добавить читателя" : "Изменить читателя"
                font.bold: true
                font.pixelSize: 18
            }

            TextField {
                id: readerNameField
                placeholderText: "Имя"
                Layout.fillWidth: true
            }

            TextField {
                id: readerContactField
                placeholderText: "Контакт (телефон/email)"
                Layout.fillWidth: true
            }

            RowLayout {
                Button {
                    text: "Отмена"
                    Layout.fillWidth: true
                    onClicked: readerFormPopup.close()
                }

                Button {
                    text: currentReaderId === -1 ? "Добавить" : "Сохранить"
                    Layout.fillWidth: true
                    onClicked: {
                        if (currentReaderId === -1) {
                            if (database.addReader(readerNameField.text, readerContactField.text)) {
                                refreshReaderList()
                                readerFormPopup.close()
                            }
                        } else {
                            if (database.updateReader(currentReaderId, readerNameField.text, readerContactField.text)) {
                                refreshReaderList()
                                readerFormPopup.close()
                            }
                        }
                    }
                }
            }
        }
    }

    Popup {
        id: deleteReaderPopup
        width: Math.min(window.width * 0.9, 300)
        height: Math.min(window.height * 0.9, 200)
        x: (window.width - width) / 2
        y: (window.height - height) / 2
        modal: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        padding: 10
        property int readerId: -1

        ColumnLayout {
            anchors.fill: parent
            spacing: 10

            Label {
                text: "Вы уверены, что хотите удалить этого читателя?"
                wrapMode: Text.Wrap
                Layout.fillWidth: true
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 10

                Button {
                    text: "Отмена"
                    Layout.fillWidth: true
                    onClicked: {
                        console.log("Deletion cancelled for reader ID:", deleteReaderPopup.readerId)
                        deleteReaderPopup.close()
                    }
                }

                Button {
                    text: "Удалить"
                    Layout.fillWidth: true
                    onClicked: {
                        console.log("Confirmed deletion for reader ID:", deleteReaderPopup.readerId)
                        if (database.deleteReader(deleteReaderPopup.readerId)) {
                            console.log("Reader deleted successfully")
                            refreshReaderList()
                            resultPopup.text = "Читатель успешно удален"
                        } else {
                            console.log("Failed to delete reader")
                            resultPopup.text = "Ошибка при удалении читателя"
                        }
                        deleteReaderPopup.close()
                        resultPopup.open()
                    }
                }
            }
        }
    }

    Popup {
        id: issueBookPopup
        width: Math.min(window.width * 0.9, 400)
        height: Math.min(window.height * 0.9, 300)
        x: (window.width - width) / 2
        y: (window.height - height) / 2
        modal: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        padding: 10

        onOpened: {
            console.log("Opening issue book popup for book ID:", currentBookId)
            readerComboModel.clear()
            var readers = database.getAllReaders()
            for (var i = 0; i < readers.length; i++) {
                readerComboModel.append(readers[i])
            }
            if (readers.length > 0) {
                readerComboBox.currentIndex = 0
                currentReaderId = readers[0].id
            }
        }

        ColumnLayout {
            anchors.fill: parent
            spacing: 10

            Label {
                text: "Выдать книгу"
                font.bold: true
                font.pixelSize: 18
            }

            ComboBox {
                id: readerComboBox
                Layout.fillWidth: true
                model: ListModel { id: readerComboModel }
                textRole: "name"
                valueRole: "id"
                onActivated: {
                    currentReaderId = currentValue
                    console.log("Selected reader ID:", currentReaderId)
                }
            }

            RowLayout {
                Button {
                    text: "Отмена"
                    Layout.fillWidth: true
                    onClicked: issueBookPopup.close()
                }

                Button {
                    text: "Выдать"
                    Layout.fillWidth: true
                    onClicked: {
                        console.log("Issuing book ID:", currentBookId, "to reader ID:", currentReaderId)
                        if (database.issueBook(currentBookId, currentReaderId)) {
                            refreshBookList()
                            issueBookPopup.close()
                        } else {
                            console.log("Failed to issue book")
                        }
                    }
                }
            }
        }
    }

    function refreshBookList() {
        console.log("Refreshing book list")
        bookModel.clear()
        var books = database.searchBooksAdvanced(searchField.text, 0, 0, availableFilter.checked, "", "")
        for (var i = 0; i < books.length; i++) {
            bookModel.append(books[i])
        }
    }

    function refreshReaderList() {
        console.log("Refreshing reader list")
        readerModel.clear()
        var readers = database.getAllReaders()
        for (var i = 0; i < readers.length; i++) {
            readerModel.append(readers[i])
        }
    }

    Component.onCompleted: {
        console.log("Application started, initializing lists")
        refreshBookList()
        refreshReaderList()
    }
}
