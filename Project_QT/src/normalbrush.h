#ifndef NORMALBRUSH_H
#define NORMALBRUSH_H

#include "brush.h" // Inherits from base Brush class
#include "item.h" // Needs to know about Item for placement

// Forward declarations if needed for Brush class methods or Tile methods
class MapView; // For mouse events if overriding directly (already in brush.h)
class Tile;    // For tile manipulation (in Map, via MapView)
class Item;    // Item to draw
class Map;     // Map for operations
class BorderSystem; // For automagic border updates

/**
 * @brief The NormalBrush class represents a standard painting brush.
 * It places the currently selected item onto the map.
 * This class directly translates the structure and core functionality from Source/src/normal_brush.cpp/h.
 */
class NormalBrush : public Brush
{
    Q_OBJECT
public:
    explicit NormalBrush(QObject* parent = nullptr);
    virtual ~NormalBrush() = default; // Use default destructor

    // Override virtual methods from Brush base class
    void mousePressEvent(QMouseEvent* event, MapView* view) override;
    void mouseMoveEvent(QMouseEvent* event, MapView* view) override;
    void mouseReleaseEvent(QMouseEvent* event, MapView* view) override;
    
    // Draw preview of the brush (called by MapView::updateCursor)
    void drawPreview(QPainter& painter, const QPoint& pos, double zoom) override;

    // Getter/Setter for the item to be placed (set from palette)
    void setCurrentItem(Item* item) { currentItem = item; }
    Item* getCurrentItem() const { return currentItem; }

    // Brush type and name identification
    Type getType() const override { return Type::Normal; }
    QString getName() const override { return tr("Normal Brush"); }
    QIcon getIcon() const override; // Provides icon for toolbar/palette

private:
    Item* currentItem; // Item to be placed by the brush (not owned)
    QPoint lastPaintPos; // Keep track of the last painted tile to avoid re-drawing on same tile if mouse is held down
    
    // Private helper for actual map modification (places an item on a tile)
    void drawBrush(MapView* view, const QPoint& tilePos);
};

#endif // NORMALBRUSH_H