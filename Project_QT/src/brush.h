#ifndef BRUSH_H
#define BRUSH_H

#include <QObject>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QPainter>
#include <QIcon>
#include <QString>

#include "mapview.h" // For MapView reference in mouse/key events. (Forward declared here in MapView::Type definition)
#include "item.h" // For Item properties used in brush operations (Item::draw for example).
#include "tile.h" // For Tile properties (e.g., hasProperty, isEmpty).
#include "mapscene.h" // For MapTileItem::TilePixelSize.

// Forward declarations of concrete brush types for brush casting in Brush base class.
// This allows safe downcasting within `Brush::asX()` methods.
class NormalBrush;
class EraserBrush;
class FloodFillBrush;
class SelectionBrush;
class PencilBrush; // New pencil brush type.

// Additional brushes from source/brush.h
class CarpetBrush;
class CreatureBrush;
class DoodadBrush;
class GroundBrush;
class HouseBrush;
class HouseExitBrush;
class RAWBrush; // This usually points to NormalBrush as RAW is a category/item type
class SpawnBrush;
class TableBrush;
class WallBrush;
class WaypointBrush;
class FlagBrush; // For zone flags

/**
 * @brief The Brush class is the abstract base for all map editing brushes.
 * It defines the common interface for how brushes interact with mouse/keyboard
 * events on the MapView and how they draw their previews.
 * This class directly translates the common interface from Source/src/brush.h.
 */
class Brush : public QObject
{
    Q_OBJECT
public:
    // Brush Types, mirroring `Brush::Type` from original `Source/src/brush_enums.h` conceptually.
    // Ensure this matches the `Brush::Type` used by BrushManager and Toolbar.
    enum Type {
        Normal = 0,    // Standard item placement
        Eraser = 1,    // Item removal
        Selection = 2, // Area selection and manipulation
        FloodFill = 3, // Contiguous area filling
        Pencil = 4,    // Line drawing (pixel by pixel or tile by tile)

        // Specialized brushes from Source/src/brush_enums.h, or general category mapping:
        Ground,       // Specific ground painting (may be NormalBrush with ground item)
        Wall,         // Wall drawing (may be NormalBrush with wall item, or a complex WallBrush)
        Door,         // Door placement (may be NormalBrush with door item)
        Table,        // Table placement
        Carpet,       // Carpet placement
        Creature,     // Creature placement
        Spawn,        // Spawn placement
        House,        // House tile drawing
        HouseExit,    // House exit drawing
        Waypoint,     // Waypoint placement
        Flag,         // Zone flag drawing (PZ, NOPVP)
        Doodad,       // Complex item placement (doodads.xml)
        RAW,          // Raw item ID brush (direct item drawing)
        MoveTo,       // Special tool, might be handled by selection brush
        // If not a distinct brush type, these could map to Normal/Selection brush with special Item* or config.

        Count // Number of brush types
    };

    explicit Brush(QObject* parent = nullptr);
    virtual ~Brush() = default;

    // --- Core Interaction Interface (overridden by concrete brushes) ---
    virtual void mousePressEvent(QMouseEvent* event, MapView* view) = 0;
    virtual void mouseMoveEvent(QMouseEvent* event, MapView* view) = 0;
    virtual void mouseReleaseEvent(QMouseEvent* event, MapView* view) = 0;
    virtual void keyPressEvent(QKeyEvent* event, MapView* view) { Q_UNUSED(event); Q_UNUSED(view); } // Optional
    virtual void keyReleaseEvent(QKeyEvent* event, MapView* view) { Q_UNUSED(event); Q_UNUSED(view); } // Optional

    // --- Visual Interface ---
    virtual void drawPreview(QPainter& painter, const QPoint& pos, double zoom) = 0; // For MapView cursor
    virtual QIcon getIcon() = 0; // For Toolbar/Palette display

    // --- Properties & Type Identification ---
    void setType(Type newType) { type = newType; emit typeChanged(newType); }
    Type getType() const { return type; }

    QString getName() const { return name; }
    void setName(const QString& newName) { name = newName; }

    void setIcon(const QIcon& newIcon) { icon = newIcon; } // To set pre-defined icon.
    // Note: getIcon() is pure virtual.

    // Brush-specific parameters (common across some brushes, can be overridden).
    void setSize(int newSize) { size = newSize; emit sizeChanged(newSize); }
    int getSize() const { return size; } // Multi-tile brush size.

    void setLayer(int newLayer) { layer = newLayer; emit layerChanged(newLayer); }
    int getLayer() const { return layer; } // Active editing layer.

    // For tools that use Item (e.g. NormalBrush, FloodFill).
    // void setItem(Item* item) { currentItem = item; } // Needs specific setter on sub-brush, not generic.

    // Used in Source/src/brush.h to provide information about brush capabilities.
    virtual bool needBorders() const { return true; } // Does this brush trigger automagic borders?
    virtual bool canDrag() const { return true; }     // Can apply while dragging? (true for Normal/Pencil)
    virtual bool canSmear() const { return true; }    // Is a smearable tool? (e.g. spray paint).
    virtual bool oneSizeFitsAll() const { return false; } // Can this brush adjust to target item size?
    virtual int32_t getMaxVariation() const { return 0; } // For random variants.
    virtual bool isRaw() const { return false; } // For raw item IDs.
    // isDoodad, isTerrain, isGround, isWall, isWallDecoration, isTable, isCarpet, isDoor,
    // isOptionalBorder, isCreature, isSpawn, isHouse, isHouseExit, isWaypoint, isFlag, isEraser.
    // These return specific brush subclass types for casting:
    virtual NormalBrush* asNormal() { return nullptr; }
    virtual EraserBrush* asEraser() { return nullptr; }
    virtual FloodFillBrush* asFloodFill() { return nullptr; }
    virtual SelectionBrush* asSelection() { return nullptr; }
    virtual PencilBrush* asPencil() { return nullptr; } // New pencil brush.
    // And others from Source/src/brush.h for concrete types.

    void setCursor(const QCursor& newCursor) { brushCursor = newCursor; }
    QCursor getCursor() const { return brushCursor; }

signals:
    // Signals emitted when brush properties change, to update UI/other components.
    void typeChanged(Type type);
    void iconChanged(const QIcon& icon); // If icon is dynamically updated.
    void sizeChanged(int size);
    void layerChanged(int layer);
    // void itemChanged(Item* item); // For NormalBrush/FloodFill, specific signal in derived class.
    void changed(); // Generic signal for any internal brush state change.

protected:
    Type type;
    QString name;
    QIcon icon;
    int size;   // Brush size (e.g., 1x1, 3x3)
    int layer;  // Current active layer this brush operates on
    QCursor brushCursor; // Cursor shape for MapView to display.
};

#endif // BRUSH_H