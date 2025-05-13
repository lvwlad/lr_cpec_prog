#include "deviceemulator.h"
#include <QMessageBox>
#include <QDebug>
#include <QFile>
#include <QFileInfo>

DeviceEmulator::DeviceEmulator(const QString &userType, QObject *parent)
    : QObject(parent), userType(userType), server(new QTcpServer(this)), socket(nullptr),
      state(WaitingForHeader), fileStream(nullptr), original_size(0), encrypted_size(0), remainingBytes(0)
{
    uuidToPortMap[QUuid("550e8400-e29b-41d4-a716-446655440000")] = 12345;
    uuidToPortMap[QUuid("550e8400-e29b-41d4-a716-446655440001")] = 12346;

    localUuid = (userType == "A")
        ? QUuid("550e8400-e29b-41d4-a716-446655440000")
        : QUuid("550e8400-e29b-41d4-a716-446655440001");

    if (!server->listen(QHostAddress::LocalHost, uuidToPortMap[localUuid])) {
        qCritical() << "Не удалось запустить сервер:" << server->errorString();
    } else {
        qDebug() << "Сервер запущен на порту" << uuidToPortMap[ localUuid];
    }

    connect(server, &QTcpServer::newConnection, this, &DeviceEmulator::onNewConnection);

    // Установка ключа шифрования
    QByteArray key = QByteArray::fromHex("00112233445566778899aabbccddeeff00112233445566778899aabbccddeeff");
    magma.setKey(key);
}

DeviceEmulator::~DeviceEmulator()
{
    disconnect();
    server->close();
}

bool DeviceEmulator::connectToDevice(const QString &uuid)
{
    QUuid targetUuid(uuid);
    if (!uuidToPortMap.contains(targetUuid) || targetUuid == localUuid) {
        return false;
    }

    if (socket) {
        socket->disconnectFromHost();
        socket->deleteLater();
    }

    socket = new QTcpSocket(this);
    connect(socket, &QTcpSocket::readyRead, this, &DeviceEmulator::onReadyRead);
    connect(socket, &QTcpSocket::disconnected, this, &DeviceEmulator::onDisconnected);
    connect(socket, &QTcpSocket::errorOccurred, this, &DeviceEmulator::onSocketError);

    socket->connectToHost(QHostAddress::LocalHost, uuidToPortMap[targetUuid]);

    if (!socket->waitForConnected(3000)) {
        qWarning() << "Не удалось подключиться:" << socket->errorString();
        return false;
    }

    remoteUuid = targetUuid;
    emit connectionEstablished();
    return true;
}

void DeviceEmulator::sendData(const QString &data)
{
    if (socket && socket->state() == QAbstractSocket::ConnectedState) {
        QString message = "TEXT:" + data + "\n";
        socket->write(message.toUtf8());
        socket->flush();
    }
}

void DeviceEmulator::sendFile(const QString &filePath)
{
    if (!socket || socket->state() != QAbstractSocket::ConnectedState) {
        qWarning() << "Not connected";
        return;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Cannot open file for reading:" << filePath;
        return;
    }

    // Вычисляем размеры
    qint64 originalSize = file.size();
    qint64 blockCount = (originalSize + 7) / 8; // Количество полных блоков
    qint64 paddedSize = blockCount * 8; // Размер с учётом дополнения
    if (originalSize % 8 == 0) {
        paddedSize += 8; // Добавляем блок дополнения для файлов, кратных 8
        blockCount++;
    }

    // Отправляем заголовок
    QString filename = QFileInfo(file).fileName();
    QString header = QString("FILE:%1:%2:%3\n").arg(filename).arg(originalSize).arg(paddedSize);
    socket->write(header.toUtf8());
    socket->flush();

    // Читаем и шифруем файл блоками по 8 байт
    while (!file.atEnd()) {
        QByteArray block = file.read(8);
        qint64 bytesRead = block.size();
        if (bytesRead < 8) {
            // Добавляем PKCS#7 padding
            char padValue = static_cast<char>(8 - bytesRead);
            block.append(QByteArray(8 - bytesRead, padValue));
        }
        QByteArray encrypted = magma.encrypt(block);
        socket->write(encrypted);
        socket->flush();
    }

    // Если размер файла кратен 8, добавляем полный блок дополнения
    if (originalSize % 8 == 0) {
        QByteArray paddingBlock(8, static_cast<char>(8));
        QByteArray encrypted = magma.encrypt(paddingBlock);
        socket->write(encrypted);
        socket->flush();
    }

    file.close();
    qDebug() << "File sent:" << filename << "Original size:" << originalSize << "Padded size:" << paddedSize;
}

void DeviceEmulator::disconnect()
{
    if (socket) {
        socket->disconnectFromHost();
    }
}

bool DeviceEmulator::isConnected() const
{
    return socket && socket->state() == QAbstractSocket::ConnectedState;
}

void DeviceEmulator::onNewConnection()
{
    if (socket) {
        socket->disconnectFromHost();
        socket->deleteLater();
    }

    socket = server->nextPendingConnection();
    connect(socket, &QTcpSocket::readyRead, this, &DeviceEmulator::onReadyRead);
    connect(socket, &QTcpSocket::disconnected, this, &DeviceEmulator::onDisconnected);
    connect(socket, &QTcpSocket::errorOccurred, this, &DeviceEmulator::onSocketError);

    quint16 peerPort = socket->peerPort();
    remoteUuid = (peerPort == 12345)
        ? QUuid("550e8400-e29b-41d4-a716-446655440000")
        : QUuid("550e8400-e29b-41d4-a716-446655440001");

    emit connectionEstablished();
}

void DeviceEmulator::onReadyRead()
{
    if (!socket) return;

    buffer += socket->readAll();

    while (!buffer.isEmpty()) {
        if (state == WaitingForHeader) {
            int newlinePos = buffer.indexOf('\n');
            if (newlinePos == -1) break; // Ждём полный заголовок

            QByteArray headerData = buffer.left(newlinePos);
            buffer.remove(0, newlinePos + 1);
            QString header = QString::fromUtf8(headerData);

            if (header.startsWith("TEXT:")) {
                QString message = header.mid(5);
                emit dataReceived(message);
            } else if (header.startsWith("FILE:")) {
                QStringList parts = header.split(':');
                if (parts.size() != 4) {
                    qWarning() << "Invalid file header:" << header;
                    break;
                }

                currentFilename = parts[1];
                bool ok1, ok2;
                original_size = parts[2].toLongLong(&ok1);
                encrypted_size = parts[3].toLongLong(&ok2);

                if (!ok1 || !ok2 || original_size < 0 || encrypted_size < 0 || encrypted_size % 8 != 0) {
                    qWarning() << "Invalid sizes in header:" << header;
                    break;
                }

                remainingBytes = encrypted_size;
                state = ReceivingFile;

                // Открываем файл для записи
                fileStream = new QFile("received_" + currentFilename);
                if (!fileStream->open(QIODevice::WriteOnly)) {
                    qWarning() << "Cannot open file for writing:" << fileStream->fileName();
                    delete fileStream;
                    fileStream = nullptr;
                    state = WaitingForHeader;
                    break;
                }
                qDebug() << "Receiving file:" << currentFilename << "Original size:" << original_size << "Encrypted size:" << encrypted_size;
            } else {
                qWarning() << "Unknown header:" << header;
                break;
            }
        } else if (state == ReceivingFile) {
            if (!fileStream) {
                state = WaitingForHeader;
                break;
            }

            while (remainingBytes >= 8 && buffer.size() >= 8) {
                QByteArray encryptedBlock = buffer.left(8);
                buffer.remove(0, 8);
                QByteArray decryptedBlock = magma.decrypt(encryptedBlock);
                fileStream->write(decryptedBlock);
                remainingBytes -= 8;
            }

            if (remainingBytes == 0) {
                // Усекаем файл до оригинального размера
                fileStream->resize(original_size);
                fileStream->close();
                qDebug() << "File received:" << currentFilename << "Size:" << original_size;
                emit fileReceived(fileStream->fileName());
                delete fileStream;
                fileStream = nullptr;
                state = WaitingForHeader;
            }
        }
    }
}

void DeviceEmulator::onDisconnected()
{
    if (state == ReceivingFile && fileStream) {
        fileStream->close();
        fileStream->remove(); // Удаляем частично полученный файл
        qDebug() << "Removed incomplete file:" << fileStream->fileName();
        delete fileStream;
        fileStream = nullptr;
    }
    state = WaitingForHeader;
    emit connectionLost();
}

void DeviceEmulator::onSocketError(QAbstractSocket::SocketError error)
{
    qWarning() << "Socket error:" << error;
    if (state == ReceivingFile && fileStream) {
        fileStream->close();
        fileStream->remove();
        qDebug() << "Removed incomplete file due to socket error:" << fileStream->fileName();
        delete fileStream;
        fileStream = nullptr;
    }
    state = WaitingForHeader;
    emit connectionLost();
}
