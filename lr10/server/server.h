#ifndef SERVER_H
#define SERVER_H

#include <QMainWindow>
#include <QTcpServer>
#include <QTcpSocket>
#include <QMap>
#include <QListWidget>
#include <QTextEdit>
#include <QPushButton>

class Server : public QMainWindow {
    Q_OBJECT

public:
    Server(QWidget *parent = nullptr);
    virtual ~Server();

private slots:
    void toggleServer();
    void updateClientList();
    void slotReadyRead();
    void clientDisconnected();

private:
    QTcpServer *tcpServer;
    QMap<QTcpSocket*, QString> clients;
    QByteArray Data;
    quint16 nextBlockSize;

    // GUI элементы
    QPushButton *toggleButton;
    QListWidget *clientList;
    QTextEdit *log;

    void logMessage(const QString &message);
    void sendToClient(QTcpSocket *socket, const QString &message);
    void broadcastMessage(const QString &message);
    void sendUserList();
    void updateClientListWidget();
};

#endif // SERVER_H
