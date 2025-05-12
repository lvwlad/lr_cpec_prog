#include "graphicmodel.h"

GraphicModel::GraphicModel(QObject* parent) : QObject(parent) {
    scene = new CustomGraphicsScene(this);
    scene->setSceneRect(-500, -500, 1000, 1000);
}

Shape* GraphicModel::addShape(ShapeType type, const QPointF& startPos, const QColor& color) {
    Shape* shape = new Shape(type, startPos, color);
    shapes.append(shape);
    scene->addItem(shape);
    emit sceneUpdated();
    return shape;
}

void GraphicModel::addShape(Shape* shape) {
    shapes.append(shape);
    scene->addItem(shape);
    emit sceneUpdated();
}

void GraphicModel::removeShape(Shape* shape) {
    if (shapes.removeOne(shape)) {
        scene->removeItem(shape);
        emit sceneUpdated();
    }
}

void GraphicModel::clear() {
    for (Shape* shape : shapes) {
        scene->removeItem(shape);
        delete shape;
    }
    shapes.clear();
    emit sceneUpdated();
}

QList<Shape*> GraphicModel::getShapes() const {
    return shapes;
}

CustomGraphicsScene* GraphicModel::getScene() const {
    return scene;
}

