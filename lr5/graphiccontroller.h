#ifndef GRAPHICCONTROLLER_H
#define GRAPHICCONTROLLER_H

#include <QObject>
#include <QColor>
#include <QPointF>
#include <QUndoStack>
#include "graphicmodel.h"
#include "shape.h"

enum class EditorMode {
    Select,
    CreateLine,
    CreateRect,
    CreateEllipse,
    CreateText,
    CreateTrapezoid
};

class GraphicController : public QObject {
    Q_OBJECT
public:
    explicit GraphicController(GraphicModel* model,
                               QUndoStack* undoStack,
                               QObject* parent = nullptr);

    void setEditorMode(EditorMode mode);
    void setCurrentColor(const QColor& color);
    QColor getCurrentColor() const;
    void changeSelectedItemsColor(const QColor& color);

    void mousePressed(const QPointF& pos);
    void mouseMoved(const QPointF& pos);
    void mouseReleased();

    void deleteSelectedItems();
    void clearAll();

private:
    GraphicModel* model;
    QUndoStack*   undoStack;
    EditorMode    currentMode;
    QColor        currentColor;

    // для создания
    Shape* currentShape;
    bool   isDrawing;

    // для перемещения
    Shape*  movingShape;
    bool    isMoving;
    QPointF moveStartPos;
};

#endif // GRAPHICCONTROLLER_H
