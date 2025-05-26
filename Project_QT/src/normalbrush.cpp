#include "normalbrush.h"
#include "mapview.h" // Needs MapView context for map/editor access
#include "map.h"     // Needs Map access to modify tiles
#include "item.h"    // Needs Item access for type and data
#include "additemcommand.h" // For undo/redo support
#include "mainwindow.h" // For access to undo stack
#include "bordersystem.h" // For automagic borders (from Map)

#include <QDebug> // For debugging
#include <QMouseEvent> // For event types

// Constructor: Initializes the normal brush.
NormalBrush::NormalBrush(QObject* parent)
    : Brush(parent),
      currentItem(nullptr),
      lastPaintPos(QPoint(-1, -1)) // Initialize with an invalid position
{
    setType(Type::Normal); // Set the brush type
    setName(tr("Normal Brush"));
    setIcon(QIcon(":/images/brush.png")); // Load icon from resources
}

// Mouse Press Event: Initiates the drawing/painting process.
void NormalBrush::mousePressEvent(QMouseEvent* event, MapView* view)
{
    if (event->button() == Qt::LeftButton) {
        QPoint tilePos = view->mapToTile(event->pos());
        if (!currentItem) { // If no item is selected, possibly just remove base tile, or do nothing.
            // Behavior if no item is selected (e.g. don't draw anything)
            // or perhaps default to clearing top items depending on context.
            qDebug() << "NormalBrush: No item selected. Cannot draw.";
            return;
        }
        if (lastPaintPos != tilePos) { // Only draw if position changed
            drawBrush(view, tilePos);
            lastPaintPos = tilePos; // Store current position
        }
        event->accept();
    }
}

// Mouse Move Event: Continues the drawing/painting process when mouse is dragged.
void NormalBrush::mouseMoveEvent(QMouseEvent* event, MapView* view)
{
    if (event->buttons() & Qt::LeftButton) { // If left mouse button is held down
        QPoint tilePos = view->mapToTile(event->pos());
        if (lastPaintPos != tilePos) { // Only draw if position changed
            drawBrush(view, tilePos);
            lastPaintPos = tilePos; // Update last painted position
        }
        event->accept();
    }
}

// Mouse Release Event: Finalizes the drawing/painting process.
void NormalBrush::mouseReleaseEvent(QMouseEvent* event, MapView* view)
{
    // Reset last position when mouse is released to allow immediate draw on next click
    lastPaintPos = QPoint(-1, -1);
    event->accept();
}

// Private helper method: Performs the actual item placement on the map.
void NormalBrush::drawBrush(MapView* view, const QPoint& tilePos)
{
    if (!currentItem || !view->getMap()) return; // Needs item and map

    // Ensure tilePos is within map bounds before modifying the map.
    if (tilePos.x() < 0 || tilePos.x() >= view->getMap()->getSize().width() ||
        tilePos.y() < 0 || tilePos.y() >= view->getMap()->getSize().height()) {
        qDebug() << "NormalBrush: Attempted to draw outside map bounds at" << tilePos;
        return;
    }

    // Get current tile from the map model. Use `currentLayer` from MapView for editing context.
    // The Tile pointer refers to the specific X,Y,Z (layer) instance in the Map.
    Tile* targetTile = view->getMap()->getTile(tilePos.x(), tilePos.y(), layer); 
    if (!targetTile) { // If tile does not exist for some reason, maybe create a new one.
        // In this implementation, Map::getTile guarantees a Tile* will be returned for valid bounds.
        qWarning() << "NormalBrush: Target tile is null at " << tilePos;
        return;
    }

    // Prepare an undoable command (AddItemCommand) from Source.
    // Ensure `AddItemCommand` handles appending the `currentItem` to `targetTile`.
    // It should check if an item with the same ID/properties already exists to avoid duplicates
    // unless the item itself supports multiple stacking (stackable flag in `Item`).

    // Create an AddItemCommand to support Undo/Redo.
    // It's the command's responsibility to manage adding/removing the item from the Tile.
    MainWindow* mainWin = qobject_cast<MainWindow*>(view->parentWidget());
    if (mainWin && mainWin->getUndoStack()) {
        // We pass the Map instance, the tile position, the layer, and a copy of the item to place.
        // `AddItemCommand` should internally manage adding `currentItem` to `targetTile` via `map->addItem`.
        AddItemCommand* command = new AddItemCommand(view->getMap(), tilePos, layer, *currentItem);
        mainWin->getUndoStack()->push(command); // Push the command onto the undo stack.
    } else {
        // Fallback for no undo stack (direct application to map).
        targetTile->addItem(*currentItem); // Adds a copy to the Tile's internal vector.
        view->getMap()->setModified(true); // Mark map as modified.
        view->getMap()->emit tileChanged(tilePos); // Inform MapView to redraw this tile.
        qDebug() << "NormalBrush: Item" << currentItem->getName() << "drawn directly at" << tilePos << ". No Undo support.";
    }

    // Trigger automagic border system if enabled (from Source/src/normal_brush.cpp).
    // Access BorderSystem from Map singleton.
    if (view->getMap()->getBorderSystem()->isEnabled()) {
        view->getMap()->getBorderSystem()->applyBorders(tilePos, layer); // Apply borders around changed tile.
    }
}


// Draw preview of the brush on the map (called by MapView::updateCursor).
void NormalBrush::drawPreview(QPainter& painter, const QPoint& pos, double zoom)
{
    // Draw the main item (currentItem) if available, with its proper scale and transparency.
    // Position `pos` is usually (0,0) as we're drawing into a temporary pixmap.
    Q_UNUSED(pos); 
    
    painter.setOpacity(0.6); // Slightly transparent for preview.
    if (currentItem) {
        // Call the Item's draw method, which handles its sprite blitting using QPainter.
        // We pass 1.0 zoom because the `MapView` will scale the entire pixmap.
        currentItem->draw(painter, QPointF(0, 0), 1.0); 
    } else {
        // Draw a generic square to indicate brush size even if no item is selected.
        painter.setPen(QPen(Qt::blue, 2));
        painter.setBrush(QColor(0, 0, 255, 60)); // Light transparent blue
        // Brush::getSize() controls how many tiles the brush affects.
        painter.drawRect(0, 0, MapTileItem::TilePixelSize * size, MapTileItem::TilePixelSize * size);
    }

    // Also draw a transparent border for the preview.
    painter.setOpacity(1.0); // Reset opacity for border
    painter.setPen(QPen(Qt::white, 1));
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(0, 0, MapTileItem::TilePixelSize * size -1 , MapTileItem::TilePixelSize * size -1 ); // Draw rectangle based on brush size
}

// Provides the icon for the brush (e.g., in toolbar/palette).
QIcon NormalBrush::getIcon() const
{
    if (icon.isNull() && currentItem) {
        // Dynamically create icon from currentItem if no default is set.
        return QIcon(currentItem->getIcon()); // Assuming Item::getIcon() returns QIcon directly.
    }
    return icon; // Return default icon if set or item is null.
}