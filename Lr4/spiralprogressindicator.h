#ifndef SPIRALPROGRESSINDICATOR_H
#define SPIRALPROGRESSINDICATOR_H

#include <QWidget>
#include <QPainter>
#include <QWheelEvent>
#include <QTimer>
#include <QPainterPath>

class SpiralProgressIndicator : public QWidget
{
    Q_OBJECT

public:
    explicit SpiralProgressIndicator(QWidget *parent = nullptr);

    void setProgressValue(int value);
    int progressValue() const;

    void setMaximumValue(int value);
    int maximumValue() const;

    void setMinimumValue(int value);
    int minimumValue() const;

protected:
    void paintEvent(QPaintEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private slots:
    void updateAnimation();

private:
    int m_progressValue;       // Текущее значение прогресса
    int m_maximumValue;       // Максимальное значение
    int m_minimumValue;       // Минимальное значение
    int m_targetValue;        // Целевое значение для анимации
    QTimer *m_animationTimer; // Таймер для анимации
};

#endif // SPIRALPROGRESSINDICATOR_H
