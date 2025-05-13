#ifndef DEVICEEMULATOR_H
#define DEVICEEMULATOR_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QUuid>
#include <QFile>
#include "magma.h"

class DeviceEmulator : public QObject
{
    Q_OBJECT

public:
    explicit DeviceEmulator(const QString &userType, QObject *parent = nullptr);
    ~DeviceEmulator();

    bool connectToDevice(const QString &uuid);
    void sendData(const QString &data);
    void sendFile(const QString &filePath);
    void disconnect();
    bool isConnected() const;

signals:
    void dataReceived(const QString &data);
    void fileReceived(const QString &filename);
    void connectionEstablished();
    void connectionLost();

private slots:
    void onNewConnection();
    void onReadyRead();
    void onDisconnected();
    void onSocketError(QAbstractSocket::SocketError error);

private:
    QTcpServer *server;
    QTcpSocket *socket;
    QString userType;
    QUuid localUuid;
    QUuid remoteUuid;
    QMap<QUuid, quint16> uuidToPortMap;
    Magma magma;
    QByteArray buffer;
    enum State { WaitingForHeader, ReceivingFile } state;
    QString currentFilename;
    qint64 original_size;
    qint64 encrypted_size;
    qint64 remainingBytes;
    QFile *fileStream;
};

#endif // DEVICEEMULATOR_H
