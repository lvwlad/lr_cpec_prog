#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsView>
#include <QToolBar>
#include <QColorDialog>
#include <QInputDialog>
#include <QKeyEvent>
#include "graphicmodel.h"
#include "graphiccontroller.h"
#include <QUndoStack>
#include <QAction>
#include <QKeySequence>

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

protected:
    void keyPressEvent(QKeyEvent* event) override;

private slots:
    void onSelectAction();
    void onLineAction();
    void onRectAction();
    void onEllipseAction();
    void onTextAction();
    void onTrapezoidAction();
    void onColorAction();
    void onDeleteAction();
    void onClearAction();

    void handleMousePressed(const QPointF& pos);
    void handleMouseMoved(const QPointF& pos);
    void handleMouseReleased();

private:
    void setupUI();
    void setupToolBar();
    void setupConnections();

    QGraphicsView* view;
    QToolBar* toolBar;

    GraphicModel* model;
    GraphicController* controller;
    QUndoStack* undoStack;
    QAction* undoAction;
    QAction* redoAction;
};

#endif // MAINWINDOW_H
