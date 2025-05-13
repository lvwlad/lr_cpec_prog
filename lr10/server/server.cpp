#include "server.h"
#include <QTime>
#include <QVBoxLayout>
#include <QLabel>
#include <QMessageBox>

Server::Server(QWidget *parent)
    : QMainWindow(parent),
      tcpServer(nullptr),
      nextBlockSize(0)
{
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(centralWidget);

    toggleButton = new QPushButton("Запустить сервер", this);
    clientList = new QListWidget(this);
    log = new QTextEdit(this);
    log->setReadOnly(true);

    layout->addWidget(toggleButton);
    layout->addWidget(new QLabel("Подключенные клиенты:"));
    layout->addWidget(clientList);
    layout->addWidget(new QLabel("Логи:"));
    layout->addWidget(log);

    setCentralWidget(centralWidget);
    connect(toggleButton, &QPushButton::clicked, this, &Server::toggleServer);
}

Server::~Server() {
    for (QTcpSocket *socket : clients.keys()) {
        socket->disconnectFromHost();
        socket->deleteLater();
    }
    clients.clear();

    if (tcpServer) {
        tcpServer->close();
        delete tcpServer;
    }
}

void Server::toggleServer() {
    if (!tcpServer) {
        tcpServer = new QTcpServer(this);
        if (tcpServer->listen(QHostAddress::Any, 2323)) {
            connect(tcpServer, &QTcpServer::newConnection, this, &Server::updateClientList);
            logMessage("Сервер запущен на порту 2323");
            toggleButton->setText("Остановить сервер");
        } else {
            logMessage("Ошибка запуска: " + tcpServer->errorString());
        }
    } else {
        tcpServer->close();
        delete tcpServer;
        tcpServer = nullptr;
        logMessage("Сервер остановлен");
        toggleButton->setText("Запустить сервер");
    }
}

void Server::updateClientList() {
    QTcpSocket *socket = tcpServer->nextPendingConnection();
    connect(socket, &QTcpSocket::readyRead, this, &Server::slotReadyRead);
    connect(socket, &QTcpSocket::disconnected, this, &Server::clientDisconnected);
    clients.insert(socket, "");
    updateClientListWidget();
    logMessage("Новое подключение: " + QString::number(socket->socketDescriptor()));
}

void Server::slotReadyRead() {
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
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

        if (message.startsWith("[name]:")) {
            QString name = message.section(':', 1);
            clients[socket] = name;
            updateClientListWidget();
            logMessage(name + " присоединился");
            broadcastMessage(name + " подключился");
            sendUserList();
        } else if (message.startsWith("[private]:")) {
            QString recipient = message.section(':', 1, 1);
            QString text = message.section(':', 2);
            QString senderName = clients[socket];

            // Отправить получателю
            for (auto it = clients.begin(); it != clients.end(); ++it) {
                if (it.value() == recipient) {
                    sendToClient(it.key(), "Отправлено пользователем:" + senderName + ": " + text);
                    break;
                }
            }

            // Отправить копию отправителю
            sendToClient(socket, "Отправлено пользователю:" + recipient + ": " + text);
        } else {
            broadcastMessage(clients[socket] + ": " + message);
        }
    }
}

void Server::clientDisconnected() {
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    QString name = clients.value(socket, "Неизвестный");
    clients.remove(socket);
    updateClientListWidget();
    logMessage(name + " отключился");
    broadcastMessage(name + " покинул чат");
    sendUserList();
    socket->deleteLater();
}

void Server::sendToClient(QTcpSocket *socket, const QString &message) {
    QByteArray data;
    QDataStream out(&data, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_2);
    out << quint16(0) << message;
    out.device()->seek(0);
    out << quint16(data.size() - sizeof(quint16));
    socket->write(data);
}

void Server::broadcastMessage(const QString &message) {
    for (QTcpSocket *socket : clients.keys()) {
        sendToClient(socket, message);
    }
}

void Server::sendUserList() {
    QStringList userList;
    for (const QString &name : clients.values()) {
        if (!name.isEmpty()) userList << name;
    }
    broadcastMessage("[users]:" + userList.join(";"));
}

void Server::logMessage(const QString &message) {
    log->append(QTime::currentTime().toString("hh:mm:ss") + " | " + message);
}

void Server::updateClientListWidget() {
    clientList->clear();
    for (const QString &name : clients.values()) {
        if (!name.isEmpty()) {
            clientList->addItem(name);
        }
    }
}
