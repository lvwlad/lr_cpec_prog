#include "mainwindow.h"
#include <QUndoStack>
#include <QAction>
#include <QKeySequence>
#include <QFontDialog>  // Для диалога выбора шрифта

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    model = new GraphicModel(this);
    undoStack  = new QUndoStack(this);
    undoAction = new QAction(tr("Undo"), this);
    undoAction->setShortcut(QKeySequence::Undo);
    undoAction->setEnabled(false);
    connect(undoAction, &QAction::triggered, undoStack, &QUndoStack::undo);
    connect(undoStack, &QUndoStack::canUndoChanged, undoAction, &QAction::setEnabled);
    redoAction = new QAction(tr("Redo"), this);
    redoAction->setShortcut(QKeySequence::Redo);
    redoAction->setEnabled(false);
    connect(redoAction, &QAction::triggered, undoStack, &QUndoStack::redo);
    connect(undoStack, &QUndoStack::canRedoChanged, redoAction, &QAction::setEnabled);
    controller = new GraphicController(model, undoStack, this);

    setupUI();
    setupToolBar();
    setupConnections();
}

MainWindow::~MainWindow() {}

void MainWindow::setupUI() {
    view = new QGraphicsView(this);
    view->setScene(model->getScene());
    view->setRenderHint(QPainter::Antialiasing);
    view->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    view->setDragMode(QGraphicsView::RubberBandDrag);
    view->setInteractive(true);
    setCentralWidget(view);

    toolBar = new QToolBar("Tools", this);
    addToolBar(Qt::LeftToolBarArea, toolBar);
}

void MainWindow::setupToolBar() {
    QAction* selectAction = toolBar->addAction("Select");
    QAction* lineAction = toolBar->addAction("Line");
    QAction* rectAction = toolBar->addAction("Rectangle");
    QAction* ellipseAction = toolBar->addAction("Ellipse");
    QAction* trapezoidAction = toolBar->addAction("Trapezoid");
    QAction* textAction = toolBar->addAction("Text");
    toolBar->addSeparator();
    QAction* colorAction = toolBar->addAction("Color");
    QAction* fontAction = toolBar->addAction("Font");  // Кнопка "Шрифт"
    toolBar->addSeparator();
    QAction* deleteAction = toolBar->addAction("Delete");
    QAction* clearAction = toolBar->addAction("Clear");
    toolBar->addSeparator();
    toolBar->addAction(undoAction);
    toolBar->addAction(redoAction);

    connect(selectAction, &QAction::triggered, this, &MainWindow::onSelectAction);
    connect(lineAction, &QAction::triggered, this, &MainWindow::onLineAction);
    connect(rectAction, &QAction::triggered, this, &MainWindow::onRectAction);
    connect(ellipseAction, &QAction::triggered, this, &MainWindow::onEllipseAction);
    connect(trapezoidAction, &QAction::triggered, this, &MainWindow::onTrapezoidAction);
    connect(textAction, &QAction::triggered, this, &MainWindow::onTextAction);
    connect(colorAction, &QAction::triggered, this, &MainWindow::onColorAction);
    connect(fontAction, &QAction::triggered, this, &MainWindow::onFontAction);  // Подключение слота для шрифта
    connect(deleteAction, &QAction::triggered, this, &MainWindow::onDeleteAction);
    connect(clearAction, &QAction::triggered, this, &MainWindow::onClearAction);
}

void MainWindow::setupConnections() {
    connect(model->getScene(), &CustomGraphicsScene::sceneMousePressed,
            this, &MainWindow::handleMousePressed);
    connect(model->getScene(), &CustomGraphicsScene::sceneMouseMoved,
            this, &MainWindow::handleMouseMoved);
    connect(model->getScene(), &CustomGraphicsScene::sceneMouseReleased,
            this, &MainWindow::handleMouseReleased);
}

void MainWindow::onSelectAction() {
    controller->setEditorMode(EditorMode::Select);
    view->setDragMode(QGraphicsView::RubberBandDrag);
}

void MainWindow::onLineAction() {
    controller->setEditorMode(EditorMode::CreateLine);
    view->setDragMode(QGraphicsView::NoDrag);
}

void MainWindow::onRectAction() {
    controller->setEditorMode(EditorMode::CreateRect);
    view->setDragMode(QGraphicsView::NoDrag);
}

void MainWindow::onEllipseAction() {
    controller->setEditorMode(EditorMode::CreateEllipse);
    view->setDragMode(QGraphicsView::NoDrag);
}

void MainWindow::onTrapezoidAction() {
    controller->setEditorMode(EditorMode::CreateTrapezoid);
    view->setDragMode(QGraphicsView::NoDrag);
}

void MainWindow::onTextAction() {
    controller->setEditorMode(EditorMode::CreateText);
    view->setDragMode(QGraphicsView::NoDrag);
}

void MainWindow::onColorAction() {
    QColor color = QColorDialog::getColor(controller->getCurrentColor(),
                                        this, "Выберите цвет");
    if (color.isValid()) {
        controller->setCurrentColor(color);
        controller->changeSelectedItemsColor(color);
    }
}

void MainWindow::onFontAction() {
    bool ok;
    QFont font = QFontDialog::getFont(&ok, QFont("Sans Serif", 14), this, "Выберите шрифт");
    if (ok) {
        // Применяем шрифт к выделенным текстовым объектам
        for (Shape* shape : model->getShapes()) {
            if (shape->isSelected() && shape->getType() == ShapeType::Text) {
                shape->setFont(font);
            }
        }
    }
}

void MainWindow::onDeleteAction() {
    controller->deleteSelectedItems();
}

void MainWindow::onClearAction() {
    controller->clearAll();
}

void MainWindow::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Delete) {
        controller->deleteSelectedItems();
    }
    QMainWindow::keyPressEvent(event);
}

void MainWindow::handleMousePressed(const QPointF& pos) {
    controller->mousePressed(pos);
}

void MainWindow::handleMouseMoved(const QPointF& pos) {
    controller->mouseMoved(pos);
}

void MainWindow::handleMouseReleased() {
    controller->mouseReleased();
}
