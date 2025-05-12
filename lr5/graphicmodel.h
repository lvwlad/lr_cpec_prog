#ifndef GRAPHICMODEL_H
#define GRAPHICMODEL_H

#include <QObject>
#include <QList>
#include "customgraphicsscene.h"
#include "shape.h"

class GraphicModel : public QObject {
    Q_OBJECT
public:
    explicit GraphicModel(QObject* parent = nullptr);

    Shape* addShape(ShapeType type, const QPointF& startPos, const QColor& color);
    void addShape(Shape* shape);
    void removeShape(Shape* shape);
    void clear();

    QList<Shape*> getShapes() const;
    CustomGraphicsScene* getScene() const;

signals:
    void sceneUpdated();

private:
    CustomGraphicsScene* scene;
    QList<Shape*> shapes;
};

#endif // GRAPHICMODEL_H
