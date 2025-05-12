#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSlider>
#include <QVBoxLayout>
#include "spiralprogressindicator.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    SpiralProgressIndicator *progressIndicator;
    QSlider *slider;
};

#endif // MAINWINDOW_H
