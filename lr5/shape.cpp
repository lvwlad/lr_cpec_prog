// shape.cpp
#include "shape.h"
#include <QPainter>
#include <QCursor>
#include <QGraphicsSceneMouseEvent>
#include <QFontMetrics>

Shape::Shape(ShapeType type, const QPointF& startPos, const QColor& color, QGraphicsItem* parent)
    : QGraphicsItem(parent)
    , type(type)
    , startPos(startPos)
    , endPos(startPos)
    , color(color)
    , text()
    , isEditing(false)
    , currentHandle(None)
    , isResizing(false)
    , font(QFont())  // дефолтный шрифт
{
    setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsMovable);
    setAcceptHoverEvents(true);
}

void Shape::setEndPos(const QPointF& endPos) {
    prepareGeometryChange();
    this->endPos = endPos;
    update();
}

void Shape::setText(const QString& text) {
    this->text = text;
    update();
}

void Shape::setColor(const QColor& color) {
    this->color = color;
    update();
}

void Shape::setEditing(bool editing) {
    isEditing = editing;
    update();
}

void Shape::setFont(const QFont& f) {
    font = f;
    update();
}

QFont Shape::getFont() const {
    return font;
}

ShapeType Shape::getType() const {
    return type;
}

QColor Shape::getColor() const {
    return color;
}

QString Shape::getText() const {
    return text;
}

QRectF Shape::boundingRect() const {
    if (type == ShapeType::Text) {
        // размер текста по выбранному шрифту
        QFontMetrics fm(font);
        QRect tr = fm.boundingRect(text);
        qreal pad = 4;
        return QRectF(startPos.x() + tr.x() - pad,
                      startPos.y() + tr.y() - pad,
                      tr.width() + pad*2,
                      tr.height() + pad*2);
    } else {
        QRectF rect(startPos, endPos);
        return rect.normalized().adjusted(-10, -10, 10, 10);
    }
}

void Shape::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    Q_UNUSED(option);
    Q_UNUSED(widget);

    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(QPen(color, 2));

    switch (type) {
    case ShapeType::Line:
        painter->drawLine(startPos, endPos);
        break;
    case ShapeType::Rectangle:
        painter->drawRect(QRectF(startPos, endPos));
        break;
    case ShapeType::Ellipse:
        painter->drawEllipse(QRectF(startPos, endPos));
        break;
    case ShapeType::Text:
        if (!isEditing && !text.isEmpty()) {
            painter->setFont(font);
            QFontMetrics fm(font);
            qreal pad = 4;
            QRectF trRect(startPos.x() - pad,
                          startPos.y() - fm.ascent() - pad,
                          fm.horizontalAdvance(text) + pad*2,
                          fm.height() + pad*2);
            painter->drawText(trRect.topLeft() + QPointF(pad, pad + fm.ascent()), text);
        }
        break;
    case ShapeType::Trapezoid: {
        QRectF rect(startPos, endPos);
        rect = rect.normalized();
        qreal w = rect.width();
        qreal h = rect.height();
        // Определяем точки трапеции
        QPointF p1(rect.left() + w * 0.2, rect.top());     // Верхняя левая
        QPointF p2(rect.right() - w * 0.2, rect.top());    // Верхняя правая
        QPointF p3(rect.right(), rect.bottom());           // Нижняя правая
        QPointF p4(rect.left(), rect.bottom());            // Нижняя левая
        QPolygonF poly;
        poly << p1 << p2 << p3 << p4;
        painter->setPen(QPen(color, 2));
        painter->drawPolygon(poly);
        break;
    }
    }

    // рамка и «ручки» для выделения
    if (isSelected() || isEditing) {
        painter->setPen(QPen(Qt::blue, 1, Qt::DashLine));
        QRectF br = boundingRect();
        painter->drawRect(br);

        if (type != ShapeType::Text) {
            painter->setBrush(Qt::white);
            painter->setPen(QPen(Qt::black, 1));
            for (int i = 1; i <= 4; ++i) {
                QRectF h = getHandleRect(static_cast<ResizeHandle>(i));
                painter->drawRect(h);
            }
        }
    }
}

Shape::ResizeHandle Shape::getResizeHandle(const QPointF& pos) const {
    if (type == ShapeType::Text) return None;
    for (int i = 1; i <= 4; ++i) {
        if (getHandleRect(static_cast<ResizeHandle>(i)).contains(pos))
            return static_cast<ResizeHandle>(i);
    }
    return None;
}

QRectF Shape::getHandleRect(ResizeHandle handle) const {
    QRectF rect(startPos, endPos);
    rect = rect.normalized();
    const qreal s = 8;
    switch (handle) {
    case TopLeft:
        return QRectF(rect.topLeft()   - QPointF(s/2, s/2), QSizeF(s, s));
    case TopRight:
        return QRectF(rect.topRight()  - QPointF(s/2, s/2), QSizeF(s, s));
    case BottomLeft:
        return QRectF(rect.bottomLeft()- QPointF(s/2, s/2), QSizeF(s, s));
    case BottomRight:
        return QRectF(rect.bottomRight()-QPointF(s/2, s/2), QSizeF(s, s));
    default:
        return QRectF();
    }
}

void Shape::hoverMoveEvent(QGraphicsSceneHoverEvent* event) {
    ResizeHandle h = getResizeHandle(event->pos());
    switch (h) {
    case TopLeft:
    case BottomRight: setCursor(Qt::SizeFDiagCursor); break;
    case TopRight:
    case BottomLeft: setCursor(Qt::SizeBDiagCursor); break;
    default:         setCursor(Qt::ArrowCursor);
    }
    QGraphicsItem::hoverMoveEvent(event);
}

void Shape::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        currentHandle = getResizeHandle(event->pos());
        isResizing = (currentHandle != None);
        if (isResizing) {
            resizeStartPos = startPos;
            resizeStartEnd = endPos;
        }
    }
    QGraphicsItem::mousePressEvent(event);
}

void Shape::mouseMoveEvent(QGraphicsSceneMouseEvent* event) {
    if (isResizing && (event->buttons() & Qt::LeftButton)) {
        prepareGeometryChange();
        QPointF delta = event->scenePos() - event->lastScenePos();
        switch (currentHandle) {
        case TopLeft:     startPos += delta; break;
        case TopRight:    endPos.setX(endPos.x() + delta.x()); startPos.setY(startPos.y() + delta.y()); break;
        case BottomLeft:  startPos.setX(startPos.x() + delta.x()); endPos.setY(endPos.y() + delta.y()); break;
        case BottomRight: endPos += delta; break;
        default: break;
        }
        update();
    } else {
        QGraphicsItem::mouseMoveEvent(event);
    }
}

void Shape::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
    isResizing = false;
    currentHandle = None;
    QGraphicsItem::mouseReleaseEvent(event);
}
