#ifndef PENCILBRUSH_H
#define PENCILBRUSH_H

#include "brush.h" // Inherits from Brush
#include "item.h" // Needs to know about Item for placement

// Forward declarations
class MapView;
class Map;
class Item;

/**
 * @brief The PencilBrush class provides functionality for drawing lines on the map.
 * It places the currently selected item onto each tile along the mouse drag path.
 * This class directly translates the structure and core functionality from Source/src/pencil_brush.cpp/h.
 */
class PencilBrush : public Brush
{
    Q_OBJECT
public:
    explicit PencilBrush(QObject* parent = nullptr);
    virtual ~PencilBrush() = default;

    // Override virtual methods from Brush base class
    void mousePressEvent(QMouseEvent* event, MapView* view) override;
    void mouseMoveEvent(QMouseEvent* event, MapView* view) override;
    void mouseReleaseEvent(QMouseEvent* event, MapView* view) override;
    
    // Draw preview (a single square of the item).
    void drawPreview(QPainter& painter, const QPoint& pos, double zoom) override;

    // Getter/Setter for the item to be placed (set from palette)
    void setCurrentItem(Item* item) { currentItem = item; }
    Item* getCurrentItem() const { return currentItem; }

    // Brush type and name identification
    Type getType() const override { return Type::Pencil; }
    QString getName() const override { return tr("Pencil"); }
    QIcon getIcon() override; // Provides icon for toolbar/palette

private:
    Item* currentItem;      // Item to be placed by the brush (not owned)
    QPoint lastPaintPos;    // Stores the last painted tile position for line drawing

    // Private helper method: Performs the actual item placement on a single tile
    void drawTile(MapView* view, const QPoint& tilePos);

    // Private helper for drawing a line between two points using Bresenham's algorithm.
    void drawLine(MapView* view, const QPoint& p1, const QPoint& p2);
};

#endif // PENCILBRUSH_H