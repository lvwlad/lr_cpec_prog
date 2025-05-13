#ifndef DEVICESELECTIONDIALOG_H
#define DEVICESELECTIONDIALOG_H

#include <QDialog>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QUuid>

class DeviceSelectionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DeviceSelectionDialog(const QString &userType, QWidget *parent = nullptr);
    ~DeviceSelectionDialog() override;

signals:
    void deviceSelected(const QString &uuid);

private slots:
    void on_connectButton_clicked();

private:
    QListWidget *devicesList;
    QPushButton *connectButton;
    QString userType;
};

#endif // DEVICESELECTIONDIALOG_H
