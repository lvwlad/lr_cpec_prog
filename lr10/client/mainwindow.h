#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QComboBox>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_connectButton_clicked();
    void on_disconnectButton_clicked();
    void on_sendButton_clicked();
    void on_messageLineEdit_returnPressed();
    void slotReadyRead();
    void updateUserList(const QStringList &users);

private:
    Ui::MainWindow *ui;
    QTcpSocket *socket;
    QString username;
    QByteArray Data;
    quint16 nextBlockSize;

    void sendToServer(const QString &message);
    void setConnected(bool connected);
};

#endif // MAINWINDOW_H
