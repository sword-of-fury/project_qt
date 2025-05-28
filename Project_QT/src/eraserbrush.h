#ifndef ERASERBRUSH_H
#define ERASERBRUSH_H

#include "brush.h"
#include <QIcon> // For QIcon return type

class QMouseEvent; // Forward declaration
class QPainter;    // Forward declaration
class MapView;     // Forward declaration

class EraserBrush : public Brush
{
    Q_OBJECT
public:
    explicit EraserBrush(QObject* parent = nullptr);

    void mousePressEvent(QMouseEvent* event, MapView* view) override;
    void mouseMoveEvent(QMouseEvent* event, MapView* view) override;
    void mouseReleaseEvent(QMouseEvent* event, MapView* view) override;
    void drawPreview(QPainter& painter, const QPoint& pos, double zoom) override;
    QIcon getIcon() override;
};

#endif // ERASERBRUSH_H
