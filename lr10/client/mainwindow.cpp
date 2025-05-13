#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QDataStream>
#include <QTime>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , socket(new QTcpSocket(this))
    , nextBlockSize(0)
{
    ui->setupUi(this);

    // Соединение сигналов
    connect(socket, &QTcpSocket::readyRead, this, &MainWindow::slotReadyRead);
    connect(socket, &QTcpSocket::disconnected, this, [this]() {
        setConnected(false);
        ui->chatBrowser->append("<font color='red'>[" + QTime::currentTime().toString() + "] Отключено от сервера</font>");
    });

    setConnected(false); // Изначально кнопки отключены
}

MainWindow::~MainWindow()
{
    delete ui;
}

// Нажатие кнопки "Подключиться"
void MainWindow::on_connectButton_clicked()
{
    username = ui->nameLineEdit->text().trimmed();
    QString ip = ui->ipLineEdit->text().trimmed();

    if (username.isEmpty() || ip.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Заполните все поля!");
        return;
    }

    socket->connectToHost(ip, 2323);
    if (socket->waitForConnected(3000)) {
        sendToServer("[name]:" + username); // Отправка имени на сервер
        setConnected(true);
        ui->chatBrowser->append("<font color='green'>[" + QTime::currentTime().toString() + "] Подключено к серверу</font>");
    } else {
        QMessageBox::warning(this, "Ошибка", "Не удалось подключиться к серверу");
    }
}

// Нажатие кнопки "Отключиться"
void MainWindow::on_disconnectButton_clicked()
{
    socket->disconnectFromHost();
}

// Отправка сообщения
void MainWindow::on_sendButton_clicked()
{
    QString message = ui->messageLineEdit->text().trimmed();
    if (!message.isEmpty()) {
        QString recipient = ui->userComboBox->currentText();
        if (recipient == "Все") {
            sendToServer(message);
        } else {
            sendToServer("[private]:" + recipient + ":" + message);
        }
        ui->messageLineEdit->clear();
    }
}

// Обработка нажатия Enter в поле сообщения
void MainWindow::on_messageLineEdit_returnPressed()
{
    on_sendButton_clicked();
}

// Чтение данных от сервера
void MainWindow::slotReadyRead()
{
    QDataStream in(socket);
    in.setVersion(QDataStream::Qt_6_2);

    while (true) {
        if (nextBlockSize == 0) {
            if (socket->bytesAvailable() < sizeof(quint16)) break;
            in >> nextBlockSize;
        }
        if (socket->bytesAvailable() < nextBlockSize) break;

        QString message;
        in >> message;
        nextBlockSize = 0;

        // Обработка списка пользователей
        if (message.startsWith("[users]:")) {
            QStringList users = message.mid(8).split(';');
            updateUserList(users);
        }
        // Личное сообщение
        else if (message.startsWith("[private]:")) {
            QString formattedMsg = "<font color='purple'>[" + QTime::currentTime().toString() + "] " + message.mid(10) + "</font>";
            ui->chatBrowser->append(formattedMsg);
        }
        // Обычное сообщение
        else {
            ui->chatBrowser->append("[" + QTime::currentTime().toString() + "] " + message);
        }
    }
}

// Обновление списка пользователей
void MainWindow::updateUserList(const QStringList &users)
{
    ui->userComboBox->clear();
    ui->userComboBox->addItem("Все");
    ui->userComboBox->addItems(users);
}

// Управление состоянием интерфейса
void MainWindow::setConnected(bool connected)
{
    ui->connectButton->setEnabled(!connected);
    ui->disconnectButton->setEnabled(connected);
    ui->sendButton->setEnabled(connected);
    ui->messageLineEdit->setEnabled(connected);
    ui->userComboBox->setEnabled(connected);
}

// Отправка данных на сервер
void MainWindow::sendToServer(const QString &message)
{
    QByteArray data;
    QDataStream out(&data, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_2);
    out << quint16(0) << message;
    out.device()->seek(0);
    out << quint16(data.size() - sizeof(quint16));
    socket->write(data);
}
