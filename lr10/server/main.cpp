#include <QApplication>
#include "server.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv); // Создание QApplication
    Server server;
    server.show(); // Показать GUI сервера
    return a.exec();
}
