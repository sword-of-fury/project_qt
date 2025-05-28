#ifndef NORMALBRUSH_H
#define NORMALBRUSH_H

#include "brush.h"
#include <QIcon> // For QIcon return type

class QMouseEvent; // Forward declaration
class QPainter;    // Forward declaration
class MapView;     // Forward declaration
class Item;        // Forward declaration

class NormalBrush : public Brush
{
    Q_OBJECT
public:
    explicit NormalBrush(QObject* parent = nullptr);

    void mousePressEvent(QMouseEvent* event, MapView* view) override;
    void mouseMoveEvent(QMouseEvent* event, MapView* view) override;
    void mouseReleaseEvent(QMouseEvent* event, MapView* view) override;
    void drawPreview(QPainter& painter, const QPoint& pos, double zoom) override;
    QIcon getIcon() override;

    // Method to set the item to be drawn by this brush (conceptual for now)
    void setCurrentItem(Item* item);

private:
    Item* currentItemToDraw; // The item type this brush will draw
};

#endif // NORMALBRUSH_H
