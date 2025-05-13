#include "deviceselectiondialog.h"

DeviceSelectionDialog::DeviceSelectionDialog(const QString &userType, QWidget *parent)
    : QDialog(parent), userType(userType)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    devicesList = new QListWidget(this);
    connectButton = new QPushButton("Подключиться", this);

    if (userType != "A") {
        devicesList->addItem("Устройство A (550e8400-e29b-41d4-a716-446655440000)");
    }
    if (userType != "B") {
        devicesList->addItem("Устройство B (550e8400-e29b-41d4-a716-446655440001)");
    }

    mainLayout->addWidget(devicesList);
    mainLayout->addWidget(connectButton);

    setLayout(mainLayout);
    setWindowTitle("Выбор устройства");
    resize(400, 200);

    connect(connectButton, &QPushButton::clicked, this, &DeviceSelectionDialog::on_connectButton_clicked);
}

DeviceSelectionDialog::~DeviceSelectionDialog()
{
}

void DeviceSelectionDialog::on_connectButton_clicked()
{
    if (devicesList->currentItem()) {
        QString selectedDevice = devicesList->currentItem()->text();
        QString uuid;

        if (selectedDevice.contains("Устройство A")) {
            uuid = "550e8400-e29b-41d4-a716-446655440000";
        } else {
            uuid = "550e8400-e29b-41d4-a716-446655440001";
        }

        emit deviceSelected(uuid);
        accept();
    }
}
