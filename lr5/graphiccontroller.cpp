// graphiccontroller.cpp
#include "graphiccontroller.h"
#include "customgraphicsscene.h"
#include <QInputDialog>
#include <QFontDialog>
#include <QUndoCommand>
#include <QFont>

namespace {

/// Добавление фигуры
class AddShapeCmd : public QUndoCommand {
public:
    AddShapeCmd(GraphicModel* m, ShapeType t, const QPointF& p, const QColor& c)
        : model(m), type(t), pos(p), col(c), shape(nullptr) {
        setText("Add Shape");
    }
    void redo() override {
        if (!shape) shape = model->addShape(type, pos, col);
        else        model->addShape(shape);
    }
    void undo() override {
        model->removeShape(shape);
    }
private:
    GraphicModel* model;
    ShapeType     type;
    QPointF       pos;
    QColor        col;
    Shape*        shape;
};

/// Удаление фигуры
class RemoveShapeCmd : public QUndoCommand {
public:
    RemoveShapeCmd(GraphicModel* m, Shape* s)
        : model(m), shape(s) {
        setText("Remove Shape");
        // запомним параметры для undo
        type = shape->getType();
        pos  = shape->pos();
        col  = shape->getColor();
        txt  = shape->getText();
    }
    void redo() override {
        model->removeShape(shape);
    }
    void undo() override {
        shape = model->addShape(type, pos, col);
        if (type == ShapeType::Text)
            shape->setText(txt);
    }
private:
    GraphicModel* model;
    Shape*        shape;
    ShapeType     type;
    QPointF       pos;
    QColor        col;
    QString       txt;
};

/// Перемещение фигуры
class MoveShapeCmd : public QUndoCommand {
public:
    MoveShapeCmd(Shape* s, const QPointF& from, const QPointF& to)
        : shape(s), oldPos(from), newPos(to) {
        setText("Move Shape");
    }
    void redo() override { shape->setPos(newPos); }
    void undo() override { shape->setPos(oldPos); }
private:
    Shape*   shape;
    QPointF  oldPos, newPos;
};

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
            // выбор шрифта
            bool fontOk;
            QFont f = QFontDialog::getFont(&fontOk, QFont(), nullptr, "Select Font");
            if (!fontOk) return;
            // ввод текста
            bool txtOk;
            QString txt = QInputDialog::getText(nullptr, "Enter Text", "Text:",
                                                QLineEdit::Normal, "", &txtOk);
            if (!txtOk || txt.isEmpty()) return;

            undoStack->push(new AddShapeCmd(model, t, pos, currentColor));
            currentShape = model->getShapes().last();
            currentShape->setFont(f);
            currentShape->setText(txt);
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
