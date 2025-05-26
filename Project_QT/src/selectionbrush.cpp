#include "selectionbrush.h"
#include "mapview.h" // For MapView context
#include "map.h"     // For Map model interaction
#include "mainwindow.h" // For access to Undo Stack
#include "deleteselectioncommand.h" // For delete operation undo
#include "additemcommand.h" // Potentially for paste command (or other item-specific commands)
#include "clearitemscommand.h" // For clearing areas during paste

#include <QDebug>
#include <QMouseEvent>
#include <QKeyEvent> // For keyboard event handling
#include <QJsonDocument> // For JSON parsing/generating clipboard data
#include <QJsonArray>
#include <QJsonObject>
#include <QClipboard> // For actual system clipboard

// Dummy global ClipboardData instance for this compilation, if not properly linked as a full module
// It is better to use the header definition or a fully integrated `CopyBuffer`
// ClipboardData `getInstance()` is now directly provided in `selectionbrush.h`.

// Constructor for the SelectionBrush
SelectionBrush::SelectionBrush(QObject* parent) :
    Brush(parent),
    currentSelection(QRect()),
    startDragPos(QPoint(-1, -1)),
    isDragging(false),
    selectionMode(Replace), // Default mode
    shiftPressed(false),
    ctrlPressed(false),
    altPressed(false)
{
    setType(Type::Selection);
    setName(tr("Selection"));
    setIcon(QIcon(":/images/selection.png")); // Icon from resources
}

// Mouse Press Event: Initiates a selection drag or a single-tile toggle.
void SelectionBrush::mousePressEvent(QMouseEvent* event, MapView* view)
{
    if (event->button() == Qt::LeftButton) {
        // Update modifier key states
        shiftPressed = event->modifiers().testFlag(Qt::ShiftModifier);
        ctrlPressed = event->modifiers().testFlag(Qt::ControlModifier);
        altPressed = event->modifiers().testFlag(Qt::AltModifier);

        // Determine selection mode based on modifier keys, mimicking Source/selection.cpp
        if (shiftPressed && ctrlPressed) {
            selectionMode = Subtract; // Shift+Ctrl for Subtract (or custom combination)
        } else if (shiftPressed) {
            selectionMode = Add; // Shift for Add
        } else if (ctrlPressed) {
            selectionMode = Toggle; // Ctrl for Toggle
        } else {
            selectionMode = Replace; // No modifiers for Replace
        }
        
        // If in Replace mode and not toggling, clear current selection (mimics Source/selection.cpp)
        if (selectionMode == Replace && !event->modifiers().testFlag(Qt::ControlModifier)) { // Original used Control for toggle/multi.
            view->clearSelection();
        }

        startDragPos = view->mapToTile(event->pos());
        isDragging = true;
        // The Map::setSelection/addToSelection/etc methods will be called by `applySelection` in `mouseMoveEvent`/`mouseReleaseEvent`.

        event->accept();
    }
}

// Mouse Move Event: Updates the selection rectangle dynamically during drag.
void SelectionBrush::mouseMoveEvent(QMouseEvent* event, MapView* view)
{
    if (isDragging && event->buttons() & Qt::LeftButton) {
        QPoint currentTilePos = view->mapToTile(event->pos());
        // Calculate the rectangle formed by startDragPos and currentTilePos.
        // QRect::normalized() handles when currentTilePos is less than startDragPos.
        QRect newSelectionRect = QRect(startDragPos, currentTilePos).normalized();

        // Pass to applySelection for logic (which then updates Map and MapScene)
        applySelection(view, newSelectionRect);

        event->accept();
    }
}

// Mouse Release Event: Finalizes the selection.
void SelectionBrush::mouseReleaseEvent(QMouseEvent* event, MapView* view)
{
    if (isDragging && event->button() == Qt::LeftButton) {
        QPoint currentTilePos = view->mapToTile(event->pos());
        QRect finalSelectionRect = QRect(startDragPos, currentTilePos).normalized();

        if (finalSelectionRect.width() == 1 && finalSelectionRect.height() == 1 && !isDragging) {
            // Single click (no drag) for toggle mode or add/subtract
            // The isDragging flag helps distinguish clicks from drags if only single-tile action is wanted on click.
            if (selectionMode == Toggle || selectionMode == Add || selectionMode == Subtract) {
                toggleSingleTile(view, finalSelectionRect.topLeft());
            }
        } else {
            // Drag operation, apply final selection.
            applySelection(view, finalSelectionRect, true); // True for final release.
        }

        isDragging = false;
        // MapView will call update().
        event->accept();
    }
}

// Key Press Event: Handles keyboard shortcuts for selection transformations.
void SelectionBrush::keyPressEvent(QKeyEvent* event, MapView* view)
{
    Qt::Key key = static_cast<Qt::Key>(event->key());
    Qt::KeyboardModifiers modifiers = event->modifiers();

    // Update modifier states (for consistency with mouse events)
    shiftPressed = modifiers.testFlag(Qt::ShiftModifier);
    ctrlPressed = modifiers.testFlag(Qt::ControlModifier);
    altPressed = modifiers.testFlag(Qt::AltModifier);

    // Delegate to helper for transformation keys.
    handleTransformKey(view, key, modifiers);

    event->accept();
}

void SelectionBrush::keyReleaseEvent(QKeyEvent* event, MapView* view)
{
    // Update modifier states on key release.
    shiftPressed = event->modifiers().testFlag(Qt::ShiftModifier);
    ctrlPressed = event->modifiers().testFlag(Qt::ControlModifier);
    altPressed = event->modifiers().testFlag(Qt::AltModifier);

    event->accept();
}

// Private helper: Applies the selection to the map model.
void SelectionBrush::applySelection(MapView* view, const QRect& newRect, bool isFinalRelease)
{
    if (!view->getMap()) return;

    Map* map = view->getMap();

    // Store the previous selection for the undo macro if this is a final action.
    MainWindow* mainWin = qobject_cast<MainWindow*>(view->parentWidget());
    if (mainWin && mainWin->getUndoStack()) {
        // Need to encapsulate a complex selection change in a QUndoCommand
        // For simplicity, directly modify Map state (and Map::selectionChanged signal takes care of MapScene visual)
        // If SelectionCommand supported setting explicit set of points (like Map::selectedPositions) it would be used here.
    }

    if (selectionMode == Replace) {
        map->setSelection(newRect);
    } else if (selectionMode == Add) {
        map->addToSelection(newRect);
    } else if (selectionMode == Subtract) {
        map->removeFromSelection(newRect);
    } else if (selectionMode == Toggle) {
        // For Toggle mode during drag, toggle each individual tile only on mouseRelease (or as clicks).
        // This makes it less practical for drag and typically means it's handled in `mouseReleaseEvent`'s `toggleSingleTile` logic for clicks.
        // For rectangular drag in Toggle mode, a loop over tiles in `newRect` calling `toggleSelectionAt` for each might be desired if `isFinalRelease`.
        if (isFinalRelease) { // Only apply toggle logic on release of drag.
             for (int y = newRect.top(); y <= newRect.bottom(); ++y) {
                for (int x = newRect.left(); x <= newRect.right(); ++x) {
                    map->toggleSelectionAt(QPoint(x, y));
                }
            }
        }
        // During drag, map->setSelection() or a visual rubberband for selection by Qt will provide feedback.
    }

    // `map->selectionChanged` will trigger `MapView::onSelectionChanged` and then `MapScene::selectTiles`, updating visuals.
    view->getScene()->update(); // Force a scene update for changes not covered by selection item's updates.
}

// Private helper: Toggles selection of a single tile (used in toggle mode clicks).
void SelectionBrush::toggleSingleTile(MapView* view, const QPoint& tilePos) {
    if (!view->getMap()) return;
    view->getMap()->toggleSelectionAt(tilePos); // Directly toggle in map model.
    view->getScene()->update();
}

// Draw preview: Display current selection rectangle as drawn by QGraphicsView (RubberBand).
void SelectionBrush::drawPreview(QPainter& painter, const QPoint& pos, double zoom)
{
    // The selection visual is primarily handled by MapSelectionItem within MapScene
    // and QGraphicsView's RubberBandDrag. This preview might show the brush icon.
    Q_UNUSED(painter); Q_UNUSED(pos); Q_UNUSED(zoom);
    // Draw brush icon over cursor position if that's desired (like original RME).
    // if (icon.isNull()) { /* load icon */ }
    // painter.drawPixmap(pos.x(), pos.y(), icon.pixmap(32, 32)); // Example.
}

// Global selection operations: (move, rotate, flip) - often pushed onto undo stack.
// These methods will act on the currently `Map::getSelection()`.
void SelectionBrush::moveSelection(MapView* view, const QPoint& offset, bool copy)
{
    if (!view->getMap() || view->getMap()->getSelection().isEmpty()) return;
    
    // For Undo/Redo, this would wrap `Map::moveSelection` in a command.
    // e.g. `mainWin->getUndoStack()->push(new MoveSelectionCommand(view->getMap(), view->getMap()->getSelection(), offset, copy));`
    view->getMap()->moveSelection(offset, copy); // Actual map data modification (placeholder for full logic).
    view->getMap()->setModified(true);
    view->getMap()->clearSelection(); // Clear visual selection after transform for effect.
    view->getScene()->update(); // Force visual update.
    qDebug() << "SelectionBrush: Moved selection by" << offset << ", copy:" << copy << ". (Stub logic).";
}

void SelectionBrush::rotateSelection(MapView* view, int degrees)
{
    if (!view->getMap() || view->getMap()->getSelection().isEmpty()) return;
    view->getMap()->rotateSelection(degrees); // Placeholder for actual rotation logic.
    view->getMap()->setModified(true);
    view->getMap()->clearSelection();
    view->getScene()->update();
    qDebug() << "SelectionBrush: Rotated selection by" << degrees << "degrees. (Stub logic).";
}

void SelectionBrush::flipSelectionHorizontally(MapView* view)
{
    if (!view->getMap() || view->getMap()->getSelection().isEmpty()) return;
    view->getMap()->flipSelectionHorizontally(); // Placeholder for actual flip logic.
    view->getMap()->setModified(true);
    view->getMap()->clearSelection();
    view->getScene()->update();
    qDebug() << "SelectionBrush: Flipped selection horizontally. (Stub logic).";
}

void SelectionBrush::flipSelectionVertically(MapView* view)
{
    if (!view->getMap() || view->getMap()->getSelection().isEmpty()) return;
    view->getMap()->flipSelectionVertically(); // Placeholder for actual flip logic.
    view->getMap()->setModified(true);
    view->getMap()->clearSelection();
    view->getScene()->update();
    qDebug() << "SelectionBrush: Flipped selection vertically. (Stub logic).";
}

void SelectionBrush::copySelection(MapView* view)
{
    if (!view->getMap() || view->getMap()->getSelection().isEmpty()) return;

    // Mimic Source/selection.cpp::copy(). This is complex (gathering items from tiles, converting to JSON).
    // The Source version reads tiles from the selection bounding box and converts them to a `QJsonObject` clipboard.
    
    QJsonObject clipboardJson;
    QRect selection = view->getMap()->getSelection();
    QJsonArray tilesArray;

    for (int y = selection.top(); y <= selection.bottom(); ++y) {
        for (int x = selection.left(); x <= selection.right(); ++x) {
            // Get all items at this X,Y,Z layer from the Map model.
            // Loop all layers as Source copy logic is usually multi-floor.
            QJsonArray itemsOnTileArray;
            for(int z = 0; z < Map::LayerCount; ++z) {
                Tile* tile = view->getMap()->getTile(x, y, z);
                if (tile) {
                    for(const Item& item : tile->getItems()) {
                        QJsonObject itemObj;
                        itemObj["id"] = item.getId();
                        itemObj["x"] = item.getPosition().x() - selection.left(); // Store relative position for paste
                        itemObj["y"] = item.getPosition().y() - selection.top();
                        itemObj["z"] = item.getLayer(); // Store item's actual layer.
                        // Add more item attributes (actionId, uniqueId, text etc.) if implemented.
                        // itemObj["attributes"] = QJsonObject::fromVariantMap(item.getAttributes().toVariantMap()); // If item attributes are map to variantmap

                        itemsOnTileArray.append(itemObj);
                    }
                }
            }
            if (!itemsOnTileArray.isEmpty()) {
                QJsonObject tileObj;
                tileObj["x"] = x - selection.left(); // Relative to selection's top-left.
                tileObj["y"] = y - selection.top();
                // Add any tile properties (e.g. MapFlags, ZoneIds) if applicable.
                tileObj["items"] = itemsOnTileArray;
                tilesArray.append(tileObj);
            }
        }
    }

    clipboardJson["selectionBoundingBox"] = QJsonObject { // Store full bounding box for validation on paste.
        {"x", selection.x()}, {"y", selection.y()},
        {"width", selection.width()}, {"height", selection.height()}
    };
    clipboardJson["tiles"] = tilesArray;
    
    // Copy the JSON string representation to the system clipboard
    QJsonDocument doc(clipboardJson);
    QApplication::clipboard()->setText(doc.toJson(QJsonDocument::Compact)); // Copy to system clipboard.
    
    // Also store in our internal ClipboardData singleton
    ClipboardData::getInstance().copyData(clipboardJson, selection);
    
    qDebug() << "Copied selection data: " << doc.toJson(QJsonDocument::Compact).mid(0, 100) << "...";
}

void SelectionBrush::pasteSelection(MapView* view)
{
    if (!view->getMap() || !ClipboardData::getInstance().hasClipboardData()) return;

    QPoint targetTilePos = view->mapToTile(view->mapFromGlobal(QCursor::pos())); // Get current cursor pos in tiles
    QJsonObject clipboardJson = ClipboardData::getInstance().getCopyData();
    QRect copiedBoundingBox = ClipboardData::getInstance().getBoundingBox(); // Original bounding box for relative position

    QJsonArray tilesArray = clipboardJson["tiles"].toArray();
    
    // For Undo/Redo, this would typically involve a `PasteSelectionCommand`.
    // The `PasteSelectionCommand` then calls `Map::addItem` with Undoable operations.
    MainWindow* mainWin = qobject_cast<MainWindow*>(view->parentWidget());
    if (mainWin && mainWin->getUndoStack()) {
        // Clear old selection
        view->getMap()->clearSelection(); // Clear current visual selection for paste.

        // We can create a compound command or macro for paste to clear & add many items.
        // It's the command's job to reconstruct `Tile`s and place them.
        // If PasteSelectionCommand exists (Project_QT/src/pasteselectioncommand.h)
        // Pass the actual ClipboardData and target paste position.
        // The command iterates through ClipboardData, finds new target positions (relative to pastePos), and calls Map's item ops.
        // `mainWin->getUndoStack()->push(new PasteSelectionCommand(view->getMap(), targetTilePos, ClipboardData::getInstance()));`
        qDebug() << "SelectionBrush: Paste triggered. Actual paste logic will be in PasteSelectionCommand or related service.";
        
        // Manual basic paste (without undo) if command is not fully ready:
        // Begin paste sequence (e.g., in Editor or Map class)
        // This is simplified, just directly applies paste logic.
        for (const QJsonValue& tileValue : tilesArray) {
            QJsonObject tileObj = tileValue.toObject();
            int relX = tileObj["x"].toInt(); // Relative x from original bounding box.
            int relY = tileObj["y"].toInt(); // Relative y from original bounding box.

            QPoint newPos = QPoint(targetTilePos.x() + relX, targetTilePos.y() + relY);

            // Bounds check for new position.
            if (newPos.x() < 0 || newPos.x() >= view->getMap()->getSize().width() ||
                newPos.y() < 0 || newPos.y() >= view->getMap()->getSize().height()) {
                qDebug() << "Skipping paste item outside bounds at" << newPos;
                continue;
            }

            // Remove existing items at new position (simple replacement for paste for now).
            // This ideally would be handled by PasteSelectionCommand to support Undo/Redo of replacements.
            for (int z = 0; z < Map::LayerCount; ++z) { // Clear all layers on this new target tile.
                view->getMap()->getTile(newPos.x(), newPos.y(), z)->clearItems(); // Clear for replace behavior
                view->getMap()->getTile(newPos.x(), newPos.y(), z)->clearCreatures(); // Clear creatures too
            }
            
            QJsonArray itemsOnTileArray = tileObj["items"].toArray();
            for (const QJsonValue& itemValue : itemsOnTileArray) {
                QJsonObject itemObj = itemValue.toObject();
                int itemId = itemObj["id"].toInt();
                // int itemZ = itemObj["z"].toInt(); // Get item's relative layer.

                Item* itemToPlace = ItemManager::getInstance().getItemById(itemId); // Get actual Item object.
                if (itemToPlace) {
                    view->getMap()->addItem(newPos.x(), newPos.y(), static_cast<Layer::Type>(itemToPlace->getLayer()), *itemToPlace);
                    // Add attributes if stored in JSON (actionId, uniqueId, text etc.)
                    // item.setActionID(itemObj["actionid"].toInt());
                } else {
                    qDebug() << "Warning: Could not find item ID" << itemId << "for paste at" << newPos;
                }
            }
        }
        
        view->getMap()->setModified(true);
        view->getMap()->emit mapChanged(); // Full redraw for map content change.
        view->getScene()->update();
        qDebug() << "SelectionBrush: Pasted selection directly. (No full undo).";

    } else {
        qDebug() << "SelectionBrush: Paste triggered but no data in internal clipboard.";
    }
}

void SelectionBrush::deleteSelection(MapView* view)
{
    if (!view->getMap() || view->getMap()->getSelection().isEmpty()) return;
    
    // Get currently selected bounding box.
    QRect selectionRect = view->getMap()->getSelection();
    
    // For Undo/Redo, use DeleteSelectionCommand.
    MainWindow* mainWin = qobject_cast<MainWindow*>(view->parentWidget());
    if (mainWin && mainWin->getUndoStack()) {
        DeleteSelectionCommand* command = new DeleteSelectionCommand(view->getMap(), selectionRect);
        mainWin->getUndoStack()->push(command);
        qDebug() << "SelectionBrush: DeleteSelectionCommand pushed for" << selectionRect;
    } else {
        // Fallback for no undo stack (direct delete).
        // This clears all items (and creatures) in the selection range, layer by layer.
        for (int x = selectionRect.left(); x <= selectionRect.right(); ++x) {
            for (int y = selectionRect.top(); y <= selectionRect.bottom(); ++y) {
                for (int z = 0; z < Map::LayerCount; ++z) {
                    Tile* tile = view->getMap()->getTile(x, y, z);
                    if (tile) {
                        tile->clearItems(); // Clear all items.
                        tile->clearCreatures(); // Clear all creatures.
                    }
                }
            }
        }
        view->getMap()->setModified(true);
        view->getMap()->emit mapChanged(); // Trigger full redraw.
        qDebug() << "SelectionBrush: Cleared selected area directly. No Undo support.";
    }
    
    view->clearSelection(); // Clear the visual selection rectangle/highlight.
    view->getScene()->update(); // Force visual update.
}


// Selection operations via toolbar or direct hotkeys that impact Map's state.
void SelectionBrush::clearSelection(MapView* view)
{
    if (!view->getMap()) return;
    view->getMap()->clearSelection(); // Delegates to Map class to clear selection model.
    view->getScene()->update(); // Trigger visual update of selection in scene.
    emit selectionCleared(); // Notify UI components (e.g. status bar, toolbar)
}

// Private helper to handle transformation keys (Move, Rotate, Flip).
void SelectionBrush::handleTransformKey(MapView* view, Qt::Key key, Qt::KeyboardModifiers modifiers)
{
    if (!view->getMap() || view->getMap()->getSelection().isEmpty()) {
        return;
    }

    QPoint offset(0, 0);
    int degrees = 0;
    bool isFlip = false;
    bool isMove = false;
    
    // Movement with Arrow Keys (Source used 32 pixels for non-Ctrl, 1 for Ctrl, no direct offset for units in map)
    // Here, we use 1 tile unit (32 pixels).
    int step = 1;
    if (modifiers == Qt::NoModifier || modifiers == Qt::ShiftModifier) { // Normal move
        if (key == Qt::Key_Up) offset = QPoint(0, -step);
        else if (key == Qt::Key_Down) offset = QPoint(0, step);
        else if (key == Qt::Key_Left) offset = QPoint(-step, 0);
        else if (key == Qt::Key_Right) offset = QPoint(step, 0);
        isMove = true;
    } else if (modifiers.testFlag(Qt::ControlModifier) && (key >= Qt::Key_Up && key <= Qt::Key_Right)) { // Smaller moves/alternative
         step = 1; // Example, original may have a smaller step (like 1 pixel if possible).
         offset = QPoint(0,0); // For now, use same 1-tile step, or map a separate movement command to smaller pixel step if applicable.
         // Source does 32 pixels for non-Ctrl and 1 pixel for Ctrl in Move/Draw
         // But that's specific to Mouse move in map_display, not global transform.
         // Stick to 1-tile moves here as standard for selections.
    }

    // Rotation (Z/X from original)
    if (key == Qt::Key_Z && modifiers == Qt::NoModifier) { // Z for counter-clockwise
        degrees = -90; // Rotate -90 degrees
    } else if (key == Qt::Key_X && modifiers == Qt::NoModifier) { // X for clockwise
        degrees = 90; // Rotate +90 degrees
    }
    
    // Flipping (H/V from original)
    // Source `main_menubar.cpp::OnFlipSelectionHorizontally`, `OnFlipSelectionVertically`
    // This is from Edit menu, so likely distinct QActions connected to these, not brush hotkeys.
    // However, if the intent is to tie directly to `SelectionBrush`, uncomment/re-add logic.
    /*
    if (key == Qt::Key_H && modifiers == Qt::NoModifier) { // H for Horizontal Flip
        flipSelectionHorizontally(view); isFlip = true;
    } else if (key == Qt::Key_V && modifiers == Qt::NoModifier) { // V for Vertical Flip
        flipSelectionVertically(view); isFlip = true;
    }
    */
    
    if (isMove && offset != QPoint(0,0)) {
        // If Shift is pressed while moving, perform a COPY operation (Source/main_menubar.cpp::OnCopySelection)
        bool performCopy = modifiers.testFlag(Qt::ShiftModifier);
        moveSelection(view, offset, performCopy);
    } else if (degrees != 0) {
        rotateSelection(view, degrees);
    } else if (isFlip) {
        // Already handled directly above.
    }
}