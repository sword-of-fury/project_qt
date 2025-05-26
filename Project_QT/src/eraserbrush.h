#ifndef ERASERBRUSH_H
#define ERASERBRUSH_H

#include "brush.h" // Inherits from Brush
#include "tile.h"  // For interacting with Tile objects

// Forward declarations
class MapView;
class Map;

/**
 * @brief The EraserBrush class provides functionality to remove items from the map.
 * It removes all items from a tile at the current layer or from the top if configured.
 * This class directly translates the structure and core functionality from Source/src/eraser_brush.cpp/h.
 */
class EraserBrush : public Brush
{
    Q_OBJECT
public:
    explicit EraserBrush(QObject* parent = nullptr);
    virtual ~EraserBrush() = default;

    // Override virtual methods from Brush base class
    void mousePressEvent(QMouseEvent* event, MapView* view) override;
    void mouseMoveEvent(QMouseEvent* event, MapView* view) override;
    void mouseReleaseEvent(QMouseEvent* event, MapView* view) override;
    
    // Draw preview of the brush (e.g., an X or transparent square)
    void drawPreview(QPainter& painter, const QPoint& pos, double zoom) override;

    // Brush type and name identification
    Type getType() const override { return Type::Eraser; }
    QString getName() const override { return tr("Eraser"); }
    QIcon getIcon() override; // Loads icon for toolbar/palette

private:
    // Helper method to perform the actual erasing on the tile
    void eraseTile(MapView* view, const QPoint& tilePos);

    QPoint lastErasePos; // To avoid redundant erasing on the same tile when dragging.
};

#endif // ERASERBRUSH_H