#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QString userType = (argc > 1) ? argv[1] : "A";

    MainWindow w(userType);
    w.show();

    return a.exec();
}
