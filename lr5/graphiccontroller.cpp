// graphiccontroller.cpp
#include "graphiccontroller.h"
#include "customgraphicsscene.h"
#include <QInputDialog>
#include <QFontDialog>
#include <QUndoCommand>
#include <QFont>

namespace {

// ... (остальные классы команд Undo/Redo остаются без изменений)

} // namespace

// === GraphicController implementation ===

GraphicController::GraphicController(GraphicModel* model,
                                     QUndoStack* undoStack,
                                     QObject* parent)
    : QObject(parent)
    , model(model)
    , undoStack(undoStack)
    , currentMode(EditorMode::Select)
    , currentColor(Qt::black)
    , currentShape(nullptr)
    , isDrawing(false)
    , movingShape(nullptr)
    , isMoving(false)
    , moveStartPos()
{}

void GraphicController::setEditorMode(EditorMode mode) {
    currentMode = mode;
}

void GraphicController::setCurrentColor(const QColor& color) {
    currentColor = color;
}

QColor GraphicController::getCurrentColor() const {
    return currentColor;
}

void GraphicController::changeSelectedItemsColor(const QColor& color) {
    for (Shape* sh : model->getShapes()) {
        if (sh->isSelected()) {
            sh->setColor(color);
        }
    }
}

void GraphicController::mousePressed(const QPointF& pos) {
    if (currentMode == EditorMode::Select) {
        // начало перемещения
        for (QGraphicsItem* it : model->getScene()->items(pos)) {
            if (auto sh = dynamic_cast<Shape*>(it)) {
                movingShape  = sh;
                isMoving     = true;
                moveStartPos = sh->pos();
                return;
            }
        }
    } else {
        // добавление новой фигуры
        ShapeType t = ShapeType::Line;
        switch (currentMode) {
        case EditorMode::CreateLine:     t = ShapeType::Line;      break;
        case EditorMode::CreateRect:     t = ShapeType::Rectangle; break;
        case EditorMode::CreateEllipse:  t = ShapeType::Ellipse;   break;
        case EditorMode::CreateText:     t = ShapeType::Text;      break;
        case EditorMode::CreateTrapezoid: t = ShapeType::Trapezoid; break;
        default: return;
        }

        if (t == ShapeType::Text) {
            // Ввод только текста, без выбора шрифта
            bool txtOk;
            QString txt = QInputDialog::getText(nullptr, "Введите текст", "Текст:",
                                                QLineEdit::Normal, "", &txtOk);
            if (!txtOk || txt.isEmpty()) return;

            undoStack->push(new AddShapeCmd(model, t, pos, currentColor));
            currentShape = model->getShapes().last();
            currentShape->setText(txt);
            // Устанавливаем шрифт по умолчанию
            currentShape->setFont(QFont("Sans Serif", 14));
        } else {
            undoStack->push(new AddShapeCmd(model, t, pos, currentColor));
            currentShape = model->getShapes().last();
        }

        isDrawing = true;
    }
}

void GraphicController::mouseMoved(const QPointF& pos) {
    if (isMoving && movingShape) {
        // двигаем «вживую»
        movingShape->setPos(pos - movingShape->boundingRect().center());
    }
    else if (isDrawing && currentShape) {
        currentShape->setEndPos(pos);
    }
}

void GraphicController::mouseReleased() {
    if (isMoving && movingShape) {
        QPointF endPos = movingShape->pos();
        if (endPos != moveStartPos) {
            undoStack->push(new MoveShapeCmd(movingShape, moveStartPos, endPos));
        }
    }
    // сброс флагов
    isMoving     = false;
    movingShape  = nullptr;
    isDrawing    = false;
    currentShape = nullptr;
}

void GraphicController::deleteSelectedItems() {
    for (Shape* sh : model->getShapes()) {
        if (sh->isSelected()) {
            undoStack->push(new RemoveShapeCmd(model, sh));
        }
    }
}

void GraphicController::clearAll() {
    for (Shape* sh : model->getShapes()) {
        undoStack->push(new RemoveShapeCmd(model, sh));
    }
}
