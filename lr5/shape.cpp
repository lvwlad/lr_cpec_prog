// shape.cpp (обновлённый метод paint)
void Shape::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    Q_UNUSED(option);
    Q_UNUSED(widget);

    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(QPen(color, 2));

    switch(type) {
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
        if (!text.isEmpty()) {
            painter->setFont(font);  // Применяем выбранный шрифт
            painter->setPen(Qt::red); // Устанавливаем цвет текста (для видимости)
            painter->drawText(startPos, text); // Рисуем текст
        } else {
            painter->setPen(QPen(Qt::gray, 1, Qt::DashLine));
            QRectF textRect = boundingRect();
            painter->drawRect(textRect); // Рисуем рамку для пустого текста
        }
        break;
    case ShapeType::Trapezoid:
        // Код для трапеции (оставляем без изменений)
        break;
    }

    if (isSelected() || isEditing) {
        painter->setPen(QPen(Qt::blue, 1, Qt::DashLine));
        QRectF rect = boundingRect();
        painter->drawRect(rect);
        if (type != ShapeType::Text) {
            painter->setBrush(Qt::white);
            painter->setPen(QPen(Qt::black, 1));
            for (int i = 1; i <= 4; ++i) {
                QRectF handle = getHandleRect(static_cast<ResizeHandle>(i));
                painter->drawRect(handle);
            }
        }
    }
}
