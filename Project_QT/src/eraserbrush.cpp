#include "floodfillbrush.h"
#include "mapview.h"
#include "map.h"
#include "tile.h"
#include "item.h"
#include "clearitemscommand.h" // For removing items during fill (if target is 'empty')
#include "additemcommand.h"    // For adding items during fill
#include "mainwindow.h"        // For undo stack and dispatching signals
#include "bordersystem.h"      // For automagic border updates

#include <QDebug>
#include <QMouseEvent>
#include <QStack> // For iterative (non-recursive) flood fill (avoids stack overflow on large maps)
#include <QSet> // For tracking visited positions

// Constructor
FloodFillBrush::FloodFillBrush(QObject *parent) : 
    Brush(parent),
    currentItem(nullptr),
    currentLayer(Layer::Type::Ground) // Default to ground layer
{
    setType(Type::FloodFill);
    setName(tr("Flood Fill"));
    setIcon(QIcon(":/images/floodfill.png")); // Icon from resources
}

// Mouse Press Event: Stores the starting position for the flood fill.
void FloodFillBrush::mousePressEvent(QMouseEvent* event, MapView* view)
{
    if (event->button() == Qt::LeftButton) {
        QPoint tilePos = view->mapToTile(event->pos());
        // Start the flood fill here directly or when mouse is released for final trigger.
        // For classic flood fill, trigger on press (or release if confirmation needed).
        // Original RME calls it on click. So, call the main floodFill here.
        floodFill(tilePos);
        event->accept();
    }
}

// Mouse Move Event: Flood fill does not actively paint while moving.
void FloodFillBrush::mouseMoveEvent(QMouseEvent* event, MapView* view)
{
    // Flood fill doesn't need to respond to mouse moves directly for painting.
    // It is primarily a click-based tool.
    event->accept();
}

// Mouse Release Event: No specific action usually, the fill happened on press.
void FloodFillBrush::mouseReleaseEvent(QMouseEvent* event, MapView* view)
{
    // No additional logic for mouse release for flood fill.
    event->accept();
}

// Main Flood Fill Function: Initiates the flood-fill algorithm.
void FloodFillBrush::floodFill(const QPoint& startPos)
{
    Map* map = Map::getInstancePtr(); // Get the Map singleton.
    if (!map || !view || !currentItem) { // Needs MapView and an item to fill with.
        qWarning() << "FloodFillBrush: Cannot perform flood fill. Map/View/Item is null.";
        return;
    }

    // Ensure startPos is within map bounds.
    if (startPos.x() < 0 || startPos.x() >= map->getSize().width() ||
        startPos.y() < 0 || startPos.y() >= map->getSize().height()) {
        qDebug() << "FloodFillBrush: Start position outside map bounds at" << startPos;
        return;
    }

    Tile* startTile = map->getTile(startPos.x(), startPos.y(), currentLayer);
    if (!startTile) {
        qWarning() << "FloodFillBrush: Start tile is null at " << startPos;
        return;
    }

    // Determine the "target item" for comparison (what we are replacing/filling over).
    // This could be the topmost item on the start tile in the `currentLayer`,
    // or an "empty" state if we are filling an empty area.
    Item targetItem; // Default to a "null" or "empty" item for comparison.
    bool targetIsEmpty = startTile->isEmpty(); // For empty vs content check.

    if (!targetIsEmpty) {
        // If the start tile is not empty, get the primary item on `currentLayer` to compare against.
        QVector<Item*> itemsOnStartLayer = startTile->getItemsByLayer(currentLayer);
        if (!itemsOnStartLayer.isEmpty()) {
            targetItem = *itemsOnStartLayer.first(); // Use first item as the "target".
        } else {
            // If itemsOnStartLayer is empty, but targetIsEmpty is false, it means other layers have items,
            // or there is a creature on this layer (not covered by items list here).
            // For robust flood fill, this is a tricky spot. Default to empty if specific item isn't found.
            targetIsEmpty = true; // Effectively filling an empty slot if no specific item.
        }
    }
    
    // Check if the currentItem (fill item) is the same as the targetItem. If so, do nothing.
    if (!targetIsEmpty && targetItem.getId() == currentItem->getId() && targetItem.getName() == currentItem->getName()) {
        qDebug() << "FloodFillBrush: Fill item is same as target item. Doing nothing.";
        return;
    }

    // Start Undo Macro.
    MainWindow* mainWin = qobject_cast<MainWindow*>(view->parentWidget());
    if (mainWin && mainWin->getUndoStack()) {
        // `beginMacro` for multi-step undo.
        mainWin->getUndoStack()->beginMacro(tr("Flood Fill from (%1,%2)").arg(startPos.x()).arg(startPos.y()));
    }

    QSet<QPoint> visitedPositions; // To avoid revisiting tiles and infinite loops.
    QStack<QPoint> stack; // Use an explicit stack for iterative BFS/DFS to avoid deep recursion.

    stack.push(startPos); // Start from the initial tile.
    visitedPositions.insert(startPos); // Mark as visited.

    // Directions for 4-directional flood fill (North, East, South, West).
    // For 8-directional, add diagonals.
    QVector<QPoint> directions = {
        QPoint(0, -1), // North
        QPoint(1, 0),  // East
        QPoint(0, 1),  // South
        QPoint(-1, 0)  // West
    };

    while (!stack.isEmpty()) {
        QPoint currentTilePos = stack.pop();

        // Get the tile object at the current position.
        Tile* currentTile = map->getTile(currentTilePos.x(), currentTilePos.y(), currentLayer);
        if (!currentTile) continue; // Should not happen if bounds check done at `Map::getTile` call.

        // Determine if this `currentTile` matches the `targetItem` condition.
        bool currentTileMatchesTarget = false;
        if (targetIsEmpty) {
            currentTileMatchesTarget = currentTile->isEmpty();
        } else {
            // Check if any item on `currentLayer` of `currentTile` matches `targetItem`.
            // More complex check needed for multiple items per layer/tile, e.g. "top-most item is target".
            QVector<Item*> itemsOnCurrentLayer = currentTile->getItemsByLayer(currentLayer);
            if (!itemsOnCurrentLayer.isEmpty()) {
                currentTileMatchesTarget = (itemsOnCurrentLayer.first()->getId() == targetItem.getId()); // Simple ID match
            } else {
                currentTileMatchesTarget = currentTile->isEmpty(); // Treat as empty if no items.
            }
        }
        
        // If the current tile *does not* match the target item/empty condition, or already visited, skip.
        // It's important to only push unvisited items onto stack for flood fill.
        // Also: This condition (`currentTileMatchesTarget`) means `currentTile` is still in the "fillable zone".
        // It means we can fill `currentTilePos` and proceed to neighbors.

        // If currentTile doesn't match the target item, it's a boundary; do not fill, and do not continue to its neighbors.
        if (!currentTileMatchesTarget && currentTilePos != startPos) continue; // Skip startPos only if already filled or different content
                                                                           // Don't fill start pos itself if it matches target but then current item doesn't replace it
                                                                           // If it doesn't match target and is *not* startPos, then it's a border/different content

        // Apply the fill operation.
        // Clear items or add new items, creating appropriate undo commands.
        // Order: clear old, add new.

        if (mainWin && mainWin->getUndoStack()) {
            // If target item is "empty" and we are placing an item: clear and add.
            // If target is an item and fill is empty: clear.
            // If target is an item and fill is different item: clear and add.
            
            // Check if current tile needs modification by `currentItem`.
            // Check if current tile already contains `currentItem`.
            bool alreadyContainsFillItem = false;
            if (!currentTile->isEmpty()) { // Only check if currentTile has content
                for (const Item& existingItem : currentTile->getItems()) { // Iterate items for check
                    if (existingItem.getId() == currentItem->getId() &&
                        existingItem.getName() == currentItem->getName()) { // Deep check needed.
                        alreadyContainsFillItem = true;
                        break;
                    }
                }
            }

            // Only modify if the fill item is different from current.
            if (!alreadyContainsFillItem || currentTileMatchesTarget) { // if `currentTileMatchesTarget` is true means fillable target state

                // Clear items in the target layer of this tile for clean fill.
                ClearItemsCommand* clearCmd = new ClearItemsCommand(map, currentTilePos, currentLayer);
                mainWin->getUndoStack()->push(clearCmd);

                // Add the new fill item (currentItem) if it's not empty.
                // Assuming currentItem is not nullptr (checked at func start).
                AddItemCommand* addCmd = new AddItemCommand(map, currentTilePos, currentLayer, *currentItem);
                mainWin->getUndoStack()->push(addCmd); // Add to undo stack.

                 map->setModified(true); // Mark map as modified.
                 map->emit tileChanged(currentTilePos); // Request MapView redraw.
            }

        } else {
            // Direct modification fallback (no undo).
            currentTile->clearLayer(currentLayer); // Clear the old items.
            currentTile->addItem(*currentItem);    // Add the new item.
            map->setModified(true);
            map->emit tileChanged(currentTilePos);
            qDebug() << "FloodFillBrush: Tile" << currentTilePos << "filled directly (no undo).";
        }

        // Trigger automagic border updates for the modified tile.
        if (map->getBorderSystem()->isEnabled()) {
            map->getBorderSystem()->applyBorders(currentTilePos, currentLayer);
        }

        // Explore neighbors.
        for (const QPoint& dir : directions) {
            QPoint neighborPos = currentTilePos + dir;

            // Ensure neighbor is within map bounds.
            if (neighborPos.x() >= 0 && neighborPos.x() < map->getSize().width() &&
                neighborPos.y() >= 0 && neighborPos.y() < map->getSize().height())
            {
                // Ensure neighbor has not been visited.
                if (!visitedPositions.contains(neighborPos)) {
                    Tile* neighborTile = map->getTile(neighborPos.x(), neighborPos.y(), currentLayer);
                    // Determine if neighbor also matches target condition.
                    bool neighborMatchesTarget = false;
                    if (neighborTile) {
                        if (targetIsEmpty) {
                            neighborMatchesTarget = neighborTile->isEmpty();
                        } else {
                            QVector<Item*> itemsOnNeighborLayer = neighborTile->getItemsByLayer(currentLayer);
                            if (!itemsOnNeighborLayer.isEmpty()) {
                                neighborMatchesTarget = (itemsOnNeighborLayer.first()->getId() == targetItem.getId());
                            } else {
                                neighborMatchesTarget = neighborTile->isEmpty(); // Treat as empty if no specific item.
                            }
                        }
                    }

                    if (neighborMatchesTarget) { // If the neighbor matches the target, add to stack for processing.
                        stack.push(neighborPos);
                        visitedPositions.insert(neighborPos); // Mark as visited.
                    }
                }
            }
        }
    }

    if (mainWin && mainWin->getUndoStack()) {
        mainWin->getUndoStack()->endMacro(); // End Undo Macro.
    }
}


// Draw preview: a transparent square with fill item inside.
void FloodFillBrush::drawPreview(QPainter& painter, const QPoint& pos, double zoom)
{
    Q_UNUSED(pos);
    
    painter.setOpacity(0.6); // Semi-transparent for preview.
    
    // Draw the currently selected fill item.
    if (currentItem) {
        currentItem->draw(painter, QPointF(0, 0), 1.0); // Item draws itself on local 0,0.
    } else {
        // Fallback: draw a basic fill pattern.
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(100, 100, 255, 80)); // Transparent blue.
        painter.drawRect(0, 0, MapTileItem::TilePixelSize * size, MapTileItem::TilePixelSize * size); // Default brush size (1x1).
    }

    // Draw outline.
    painter.setOpacity(1.0); // Reset opacity.
    painter.setPen(QPen(Qt::white, 1));
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(0, 0, MapTileItem::TilePixelSize -1 , MapTileItem::TilePixelSize -1 ); // Fixed 1x1 outline for preview.
}

// Get Icon: Flood Fill brush icon.
QIcon FloodFillBrush::getIcon()
{
    if (icon.isNull()) { // Load icon only if not already loaded.
        icon = QIcon(":/images/floodfill.png");
    }
    return icon;
}