#include "mapview.h"
#include "map.h"
#include "mainwindow.h"
#include "brushmanager.h"
#include "selectionbrush.h"
#include "normalbrush.h"
#include "floodfillbrush.h"
#include "itemmanager.h" // For ItemManager::getSprite() (client/server IDs)
#include "clientversion.h" // For ClientVersion data

#include <QApplication>
#include <QGraphicsPixmapItem>
#include <QTimer>
#include <QDebug>
#include <QScrollBar>
#include <QGraphicsSceneMouseEvent>
#include <QClipboard>
#include <QMessageBox>
#include <algorithm> // For std::min, std::max

// QGraphicsView takes ownership of mapScene due to `this` parent in constructor.
MapView::MapView(QWidget* parent) :
    QGraphicsView(parent),
    mapScene(new MapScene(this)), // QGraphicsScene is owned by MapView
    currentMap(nullptr),
    currentBrush(nullptr),
    currentItem(nullptr),
    currentCreature(nullptr),
    cursorItem(nullptr),
    zoom(1.0),
    currentLayer(static_cast<int>(Layer::Type::Ground)), // Initialize with actual Layer::Type enum
    isSelecting(false),
    startSelectPos(QPoint(0,0))
{
    setScene(mapScene);
    setRenderHint(QPainter::Antialiasing);
    setDragMode(QGraphicsView::RubberBandDrag); // Default selection mode
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setResizeAnchor(QGraphicsView::AnchorUnderMouse);
    setOptimizationFlag(QGraphicsView::DontSavePainterState);
    setCacheMode(QGraphicsView::CacheBackground); // Cache static background to improve performance

    // Mouse state initializations
    draggingMap = false;
    drawingActive = false;
    selectionActive = false;
    boundingBoxSelection = false;
    activeMouseButton = Qt::NoButton;
    mouseInsideView = false;
    setMouseTracking(true); // Important for hover effects and cursor updates

    // Connect MapView's scrollbar changes to update visible tiles in MapScene.
    connect(horizontalScrollBar(), &QScrollBar::valueChanged, this, &MapView::updateVisibleTiles);
    connect(verticalScrollBar(), &QScrollBar::valueChanged, this, &MapView::updateVisibleTiles);
    // Connect MapScene's sceneRect changes back to MapView for correct scrollbar ranges.
    connect(mapScene, &QGraphicsScene::sceneRectChanged, this, [this](const QRectF& rect){
        setSceneRect(rect); // Sync QGraphicsView's internal sceneRect.
    });

    // Setup brush preview item, owned by mapScene.
    cursorItem = mapScene->addPixmap(QPixmap());
    cursorItem->setZValue(1000); // Always on top of map tiles.

    // Timer for periodic updates, owned by MapView.
    updateTimer = new QTimer(this);
    connect(updateTimer, &QTimer::timeout, this, &MapView::updateCursor); // updateCursor needs to be aware of mouseInsideView
    updateTimer->start(100); // Refresh interval for cursor etc.
    
    // Attempt to set the map from MainWindow if it's already available.
    // This might be better handled by MainWindow explicitly calling setMap post-construction.
    if (auto mainWin = qobject_cast<MainWindow*>(parentWidget())) {
        if (mainWin->getMap()) {
            setMap(mainWin->getMap());
        }
    }
}

MapView::~MapView()
{
    // QGraphicsScene and its items are owned by MapView/QGraphicsScene itself.
    // Pointers like currentMap, currentBrush are not owned by MapView.
}

void MapView::setMap(Map* map)
{
    if (currentMap == map) return;

    if (currentMap) {
        // Disconnect existing map signals before assigning a new map.
        disconnect(currentMap, &Map::tileChanged, mapScene, &MapScene::updateTile);
        disconnect(currentMap, &Map::selectionChanged, this, &MapView::onSelectionChanged);
        disconnect(currentMap, &Map::mapChanged, static_cast<MainWindow*>(parentWidget()), &MainWindow::updateWindowTitle);
        disconnect(currentMap, &Map::selectionChanged, static_cast<MainWindow*>(parentWidget()), &MainWindow::onSelectionChanged);
        disconnect(currentMap, &Map::mapChanged, mapScene, &MapScene::update);
        disconnect(currentMap, &Map::mapChanged, this, &MapView::updateVisibleTiles);
        // Also disconnect specific layer signals
        for (int i = 0; i < Map::LayerCount; ++i) {
             Layer* layerObj = currentMap->getLayer(static_cast<Layer::Type>(i));
             if (layerObj) {
                 disconnect(layerObj, &Layer::visibilityChanged, mapScene, &MapScene::update); // Generic scene update
             }
         }
    }

    currentMap = map;
    mapScene->setMap(map); // Propagate map instance to MapScene for item management.

    if (currentMap) {
        // Connect new map signals.
        connect(currentMap, &Map::tileChanged, mapScene, &MapScene::updateTile);
        connect(currentMap, &Map::selectionChanged, this, &MapView::onSelectionChanged);
        connect(currentMap, &Map::mapChanged, static_cast<MainWindow*>(parentWidget()), &MainWindow::updateWindowTitle);
        connect(currentMap, &Map::selectionChanged, static_cast<MainWindow*>(parentWidget()), &MainWindow::onSelectionChanged);
        connect(currentMap, &Map::mapChanged, mapScene, &MapScene::update);
        connect(currentMap, &Map::mapChanged, this, &MapView::updateVisibleTiles);

        // Connect Layer visibility changes to update map scene.
        for (int i = 0; i < Map::LayerCount; ++i) {
            Layer* layerObj = currentMap->getLayer(static_cast<Layer::Type>(i));
            if (layerObj) {
                // When a layer's visibility changes, trigger MapScene to re-evaluate and redraw visible tiles.
                // updateVisibleTiles will internally call updateTile on all currently visible MapTileItems.
                connect(layerObj, &Layer::visibilityChanged, this, &MapView::updateVisibleTiles);
                connect(layerObj, &Layer::visibilityChanged, mapScene, &MapScene::update); // To make sure backgrounds/grid re-evaluates.
            }
        }
        
        // Initial setup for the new map.
        updateVisibleTiles(); 
        mapScene->update(); 
    }
}

Map* MapView::getMap() const
{
    return currentMap;
}

void MapView::setBrush(Brush* brush)
{
    currentBrush = brush;
    if (currentBrush) {
        currentBrush->setLayer(currentLayer); // Brushes operate on the current active layer.
        // Specialization for painting brushes to assign current item/creature.
        if (currentBrush->getType() == Brush::Type::Normal || currentBrush->getType() == Brush::Type::FloodFill) {
             if (NormalBrush* normalBrush = qobject_cast<NormalBrush*>(currentBrush)) {
                 normalBrush->setCurrentItem(currentItem);
             } else if (FloodFillBrush* floodBrush = qobject_cast<FloodFillBrush*>(currentBrush)) {
                 floodBrush->setCurrentItem(currentItem);
                 floodBrush->setCurrentLayer(static_cast<Tile::Layer>(currentLayer)); // Flood fill cares about Tile layer for context
             }
        }
        setCursor(currentBrush->getCursor()); // Update system cursor based on selected brush.
    } else {
        unsetCursor(); // Revert to default cursor if no brush.
    }
    updateCursor(); // Redraw brush preview.
}

void MapView::setCurrentTool(Brush::Type type)
{
    // Fetches the specified brush and sets it as the active tool.
    BrushManager& brushManager = BrushManager::getInstance();
    Brush* newBrush = brushManager.getBrush(type);
    if (newBrush) {
        setBrush(newBrush); // Uses setBrush to update active tool and cursor.
    }
}

void MapView::setCurrentLayer(int layer)
{
    // Updates the active editing layer in MapView, and propagates to MapScene/Brush.
    if (currentLayer == layer) return;

    currentLayer = layer;
    if (mapScene) {
        mapScene->setCurrentLayer(currentLayer); // MapScene updates its item's rendering.
    }
    if (currentBrush) {
        currentBrush->setLayer(currentLayer); // Brush operates on new active layer.
    }
    updateCursor(); // Cursor preview might change depending on new layer's content/context.
    qobject_cast<MainWindow*>(parentWidget())->onLayerChanged(currentLayer); // Notify MainWindow (e.g., status bar).
}

int MapView::getCurrentLayer() const
{
    return currentLayer;
}

void MapView::setCurrentItem(Item* item)
{
    // Sets the currently selected item from the palette, propagates to relevant brushes.
    currentItem = item;
    if (currentBrush) {
         if (NormalBrush* normalBrush = qobject_cast<NormalBrush*>(currentBrush)) {
             normalBrush->setCurrentItem(item);
         } else if (FloodFillBrush* floodBrush = qobject_cast<FloodFillBrush*>(currentBrush)) {
             floodBrush->setCurrentItem(item);
         }
    }
    updateCursor(); // Update brush preview if item changes.
}

void MapView::setCurrentCreature(Creature* creature)
{
    // Sets the currently selected creature from the palette, propagates to relevant brushes.
    currentCreature = creature;
    // Potentially propagate to CreatureBrush here once implemented.
    updateCursor(); // Update brush preview if creature changes.
}

void MapView::setShowGrid(bool show)
{
    // Delegates grid visibility control to MapScene.
    if (mapScene) {
        mapScene->setShowGrid(show);
    }
}

void MapView::setShowCollisions(bool show)
{
    // Delegates collision highlight visibility control to MapScene.
    if (mapScene) {
        mapScene->setShowCollisions(show);
    }
}

void MapView::setZoom(double newZoom)
{
    // Sets the zoom level, clamps values, and applies transformation to QGraphicsView.
    if (newZoom <= 0.0) return;
    if (newZoom < 0.1) newZoom = 0.1; 
    if (newZoom > 5.0) newZoom = 5.0; 

    scale(newZoom / zoom, newZoom / zoom);
    zoom = newZoom;

    qobject_cast<MainWindow*>(parentWidget())->onZoomChanged(zoom); // Notify MainWindow (e.g., status bar).
    mapScene->updateGridLines(); // Force grid lines to redraw with new zoom level.
    updateVisibleTiles(); // Re-evaluate and re-populate visible tiles for optimal rendering.
}

QPoint MapView::mapToTile(const QPoint& pos) const
{
    // Converts a QWidget coordinate (from mouse event) to a tile coordinate (e.g., 0,0 for top-left tile).
    QPointF scenePos = mapToScene(pos);
    return QPoint(static_cast<int>(floor(scenePos.x() / MapTileItem::TilePixelSize)),
                  static_cast<int>(floor(scenePos.y() / MapTileItem::TilePixelSize)));
}

QPoint MapView::tileToMap(const QPoint& pos) const
{
    // Converts a tile coordinate to a QWidget coordinate (top-left pixel of the tile in view).
    QPointF scenePos(pos.x() * MapTileItem::TilePixelSize, pos.y() * MapTileItem::TilePixelSize);
    return mapFromScene(scenePos).toPoint();
}

void MapView::clearSelection()
{
    // Clears visual selection and propagates to Map model.
    if (currentMap) {
        currentMap->clearSelection();
        mapScene->clearSelection(); // Tell scene to clear visual selection.
    }
}

void MapView::selectTile(const QPoint& position)
{
    // Selects a single tile and updates Map model and MapScene visually.
    if (currentMap) {
        currentMap->setSelection(QRect(position, QSize(1, 1))); // Model update.
        mapScene->selectTile(position); // Visual update.
    }
}

void MapView::selectTiles(const QRect& rect)
{
    // Selects a rectangular area of tiles and updates Map model and MapScene visually.
    if (currentMap) {
        currentMap->setSelection(rect); // Model update.
        mapScene->selectTiles(rect); // Visual update.
    }
}

QList<QPoint> MapView::getSelectedTiles() const
{
    // Returns a list of currently selected tile positions. Queries the Map model directly.
    QList<QPoint> selected;
    if (currentMap) {
        QRect selectionRect = currentMap->getSelection();
        for (int x = selectionRect.left(); x < selectionRect.left() + selectionRect.width(); ++x) {
            for (int y = selectionRect.top(); y < selectionRect.top() + selectionRect.height(); ++y) {
                selected.append(QPoint(x, y));
            }
        }
    }
    return selected;
}

void MapView::resizeEvent(QResizeEvent* event)
{
    // Overrides base class resize event.
    QGraphicsView::resizeEvent(event);
    updateVisibleTiles(); // Update visible area to load/unload items efficiently after resize.
}

void MapView::mousePressEvent(QMouseEvent* event)
{
    // Captures mouse press events, identifies actions, and delegates to brushes/handlers.
    QGraphicsView::mousePressEvent(event); // Always call base class.

    QPoint mousePos = event->pos();
    lastMousePos = mousePos; // Store for dragging calculations
    activeMouseButton = event->button(); // Store which button initiated this sequence

    QPoint tilePos = mapToTile(mousePos); // mapToTile uses event->pos() internally if needed

    MainWindow* mainWin = qobject_cast<MainWindow*>(parentWidget());
    bool switchMouseButtons = false; // Placeholder for g_settings.getInteger(Config::SWITCH_MOUSEBUTTONS)
    if (mainWin) {
        // switchMouseButtons = mainWin->settings().getSwitchMouseButtons(); // Example
    }

    // --- Panning (Middle Button or Right if switched, or Ctrl+Left) ---
    bool panKeyPressed = (event->button() == Qt::MiddleButton && !switchMouseButtons) ||
                         (event->button() == Qt::RightButton && switchMouseButtons) ||
                         (event->button() == Qt::LeftButton && event->modifiers() & Qt::ControlModifier);

    if (panKeyPressed) {
        draggingMap = true;
        setDragMode(QGraphicsView::ScrollHandDrag);
        // Let QGraphicsView handle the ScrollHandDrag initiation.
        // We store lastPanPos in case we need it for other logic, but QGraphicsView manages its own.
        lastPanPos = event->pos(); 
        QGraphicsView::mousePressEvent(event); // Crucial to let base class initiate ScrollHandDrag
        event->accept();
        return;
    } else {
        // Default drag mode when not panning
        if (currentBrush && currentBrush->getType() == Brush::Type::Selection) {
            setDragMode(QGraphicsView::RubberBandDrag); // For selection tool
        } else {
            setDragMode(QGraphicsView::NoDrag); // For drawing tools
        }
    }

    // --- Context Menu (Right Button or Middle if switched) ---
    bool contextMenuKeyPressed = (event->button() == Qt::RightButton && !switchMouseButtons) ||
                                 (event->button() == Qt::MiddleButton && switchMouseButtons);
    if (contextMenuKeyPressed) {
        if (mainWin && currentBrush && currentBrush->getType() != Brush::Type::Selection) {
            // mainWin->setSelectionMode(); // Equivalent of g_gui.SetSelectionMode()
        }
        if (currentMap) { // Select tile under cursor for context menu
            currentMap->clearSelection(); // Clear previous selection
            currentMap->selectTile(tilePos.x(), tilePos.y(), currentLayer);
            // mapScene->updateTileSelection(tilePos); // Visual update for single tile selection
        }
        createContextMenu(event->globalPos());
        event->accept();
        return;
    }

    // --- Left Button Actions (Drawing or Selection) ---
    if (event->button() == Qt::LeftButton) {
        dragStartTile = tilePos;
        dragStartScenePos = mapToScene(mousePos);

        if (currentBrush && currentMap) {
            if (currentBrush->getType() == Brush::Type::Selection) {
                selectionActive = true;
                SelectionBrush* selBrush = static_cast<SelectionBrush*>(currentBrush);
                
                // Porting logic from MapCanvas::OnMouseActionClick (selection part)
                if (event->modifiers() & Qt::ShiftModifier) { // Boundbox selection
                    boundingBoxSelection = true;
                    if (!(event->modifiers() & Qt::ControlModifier)) { // Shift only, not Ctrl+Shift
                        currentMap->clearSelection();
                    }
                } else if (event->modifiers() & Qt::ControlModifier) { // Ctrl only (toggle selection)
                    currentMap->toggleTileSelection(tilePos.x(), tilePos.y(), currentLayer);
                     // No drag for toggle. selectionActive might be set false by brush.
                } else { // No modifiers
                    Tile* clickedTile = currentMap->getTile(tilePos.x(), tilePos.y(), currentLayer);
                    if (clickedTile && currentMap->isTileSelected(tilePos.x(), tilePos.y(), currentLayer)) {
                        // Clicked on already selected tile, prepare for drag-move
                        // selBrush->prepareMoveSelection(); // Brush handles this state
                    } else {
                        currentMap->clearSelection();
                        currentMap->selectTile(tilePos.x(), tilePos.y(), currentLayer);
                        // selBrush->prepareMoveSelection(); // Brush handles this state
                    }
                }
                selBrush->mousePressEvent(event, this); // Let brush handle its internal state
            } else { // Drawing mode
                drawingActive = true;
                // Logic from MapCanvas::OnMouseActionClick (drawing part)
                currentBrush->mousePressEvent(event, this); // Brush handles actual drawing
            }
            // mapScene->update(); // Often handled by map signals now
            event->accept();
            return;
        }
    }
    QGraphicsView::mousePressEvent(event);
}

void MapView::mouseDoubleClickEvent(QMouseEvent* event)
{
    QGraphicsView::mouseDoubleClickEvent(event); // Call base first

    MainWindow* mainWin = qobject_cast<MainWindow*>(parentWidget());
    bool doubleClickProperties = true; // Placeholder for g_settings.getInteger(Config::DOUBLECLICK_PROPERTIES)
    // if(mainWin) { doubleClickProperties = mainWin->settings().getDoubleClickProperties(); }


    if (event->button() == Qt::LeftButton && doubleClickProperties) {
        QPoint tilePos = mapToTile(event->pos());
        Tile* tile = currentMap ? currentMap->getTile(tilePos.x(), tilePos.y(), currentLayer) : nullptr;
        
        if (tile) {
            // Determine what was double-clicked (top item, creature, spawn, or tile itself)
            // This logic mirrors MapCanvas::OnMouseLeftDoubleClick
            // For simplicity, this example primarily focuses on items.
            // In a full implementation, check for spawns and creatures too.

            Item* topItem = tile->getTopItem(); // Assuming Tile::getTopItem() exists

            if (topItem) {
                // Emit a signal for MainWindow to show properties for this item
                emit itemSelected(topItem); 
            } else if (!tile->getCreatures().isEmpty()) {
                 emit creatureSelected(tile->getCreatures().first());
            // } else if (tile->getSpawn()) {
                // emit spawnSelected(tile->getSpawn()); // If you have a signal for spawns
            } else {
                // No specific item/creature, show properties for the tile itself
                emit tileSelected(tile);
            }
            event->accept();
        }
    }
}


void MapView::mouseMoveEvent(QMouseEvent* event)
{
    QPoint currentMousePos = event->pos();
    QPoint currentTilePos = mapToTile(currentMousePos);
    MainWindow* mainWin = qobject_cast<MainWindow*>(parentWidget());

    if (lastMousePos != currentMousePos) {
        emit mousePositionChanged(currentTilePos);
        if (mouseInsideView) {
            updateCursor(); // Update brush preview based on new position
        }
    }

    // --- Panning (if activeMouseButton is set appropriately) ---
    if (draggingMap) {
        // QGraphicsView::ScrollHandDrag handles the actual scrolling when active.
        // We just need to ensure the event is passed to the base class if ScrollHandDrag is active.
        if (dragMode() == QGraphicsView::ScrollHandDrag) {
            QGraphicsView::mouseMoveEvent(event);
        } else {
            // Custom panning logic if ScrollHandDrag is not used (e.g. if it was implemented manually)
            // QPoint delta = currentMousePos - lastMousePos;
            // horizontalScrollBar()->setValue(horizontalScrollBar()->value() - delta.x());
            // verticalScrollBar()->setValue(verticalScrollBar()->value() - delta.y());
        }
        lastMousePos = currentMousePos;
        event->accept();
        return;
    }

    // --- Drawing or Selection Drag (Left Button) ---
    if (activeMouseButton == Qt::LeftButton && (event->buttons() & Qt::LeftButton) && currentBrush && currentMap) {
        if (selectionActive) {
            // In selection mode and left button is down
            // Delegate to SelectionBrush to handle drag (e.g., rubber band, move preview)
            if (SelectionBrush* selBrush = qobject_cast<SelectionBrush*>(currentBrush)) {
                selBrush->mouseMoveEvent(event, this); // Pass this MapView instance
            }
        } else if (drawingActive) {
            // In drawing mode and left button is down
            // Delegate to current drawing brush
            currentBrush->mouseMoveEvent(event, this);
        }
        // mapScene->update(); // Often handled by brush or map signals now
        event->accept();
        lastMousePos = currentMousePos; // Update lastMousePos for continuous drag
        return;
    }
    
    lastMousePos = currentMousePos;
    QGraphicsView::mouseMoveEvent(event); // Call base for other hover effects etc.
}


void MapView::mouseReleaseEvent(QMouseEvent* event)
{
    Qt::MouseButton releasedButton = event->button();
    MainWindow* mainWin = qobject_cast<MainWindow*>(parentWidget());
    bool switchMouseButtons = false; // Placeholder for g_settings.getInteger(Config::SWITCH_MOUSEBUTTONS)
    // if (mainWin) { switchMouseButtons = mainWin->settings().getSwitchMouseButtons(); }


    // --- Panning Release ---
    bool panButtonReleased = (releasedButton == Qt::MiddleButton && !switchMouseButtons) ||
                             (releasedButton == Qt::RightButton && switchMouseButtons) ||
                             (releasedButton == Qt::LeftButton && event->modifiers() & Qt::ControlModifier && draggingMap);

    if (draggingMap && panButtonReleased) {
        draggingMap = false;
        // If ScrollHandDrag was active, it handles its own release.
        // Reset to NoDrag or RubberBandDrag as appropriate.
        if (currentBrush && currentBrush->getType() == Brush::Type::Selection) {
             setDragMode(QGraphicsView::RubberBandDrag);
        } else {
             setDragMode(QGraphicsView::NoDrag);
        }
        QGraphicsView::mouseReleaseEvent(event); // Propagate for ScrollHandDrag to finalize
        setCursor(Qt::ArrowCursor);
        if (activeMouseButton == releasedButton) activeMouseButton = Qt::NoButton;
        event->accept();
        return;
    }

    // --- Drawing or Selection Release (Left Button) ---
    if (activeMouseButton == Qt::LeftButton && releasedButton == Qt::LeftButton && currentBrush && currentMap) {
        if (selectionActive) {
            if (SelectionBrush* selBrush = qobject_cast<SelectionBrush*>(currentBrush)) {
                selBrush->mouseReleaseEvent(event, this); // Finalize selection
            }
            selectionActive = false;
            boundingBoxSelection = false;
        } else if (drawingActive) {
            currentBrush->mouseReleaseEvent(event, this); // Finalize drawing stroke
            drawingActive = false;
        }
        // mapScene->update(); // Often handled by brush or map signals
        if (activeMouseButton == releasedButton) activeMouseButton = Qt::NoButton;
        event->accept();
        return;
    }
    
    if (activeMouseButton == releasedButton) {
        activeMouseButton = Qt::NoButton;
    }
    QGraphicsView::mouseReleaseEvent(event);
}

void MapView::wheelEvent(QWheelEvent* event)
{
    MainWindow* mainWin = qobject_cast<MainWindow*>(parentWidget());
    if (!mainWin) {
        QGraphicsView::wheelEvent(event);
        return;
    }

    Qt::KeyboardModifiers modifiers = event->modifiers();
    int delta = event->angleDelta().y(); // Standard way to get wheel delta

    if (modifiers & Qt::ControlModifier) { // Ctrl + Wheel: Change floor
        if (delta > 0) mainWin->changeFloor(currentLayer + 1);
        else mainWin->changeFloor(currentLayer - 1);
        event->accept();
    } else if (modifiers & Qt::AltModifier) { // Alt + Wheel: Change brush size
        if (delta > 0) mainWin->increaseBrushSize();
        else mainWin->decreaseBrushSize();
        event->accept();
    } else { // Default: Zoom
        const double zoomFactorBase = 1.15;
        double zoomFactor = (delta > 0) ? zoomFactorBase : 1.0 / zoomFactorBase;
        
        setZoom(zoom * zoomFactor); // setZoom handles clamping and scene updates
        // Centering on mouse is handled by setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
        event->accept();
    }
    // mapScene->update(); // setZoom should trigger necessary updates.
    // updateCursor(); // setZoom or brush size change should trigger this.
}

void MapView::enterEvent(QEvent* event) {
    QGraphicsView::enterEvent(event);
    mouseInsideView = true;
    updateCursor(); // Show cursor when mouse enters
    // Could also grab keyboard focus: setFocus(Qt::MouseFocusReason);
}

void MapView::leaveEvent(QEvent* event) {
    QGraphicsView::leaveEvent(event);
    mouseInsideView = false;
    if (cursorItem) {
        cursorItem->setPixmap(QPixmap()); // Hide custom cursor item
    }
    // Reset any hover states if necessary
    // If a drag operation is not active, reset activeMouseButton
    if (activeMouseButton != Qt::NoButton && !(QApplication::mouseButtons() & activeMouseButton)) {
        activeMouseButton = Qt::NoButton;
        drawingActive = false;
        selectionActive = false;
        draggingMap = false;
    }
}

void MapView::keyPressEvent(QKeyEvent* event)
{
    // Handles keyboard shortcuts and navigation.
    Qt::Key key = static_cast<Qt::Key>(event->key());
    Qt::KeyboardModifiers modifiers = event->modifiers();

    // Standard Navigation
    int scrollAmount = MapTileItem::TilePixelSize; // Scroll by one tile size
    if (key == Qt::Key_Up) {
        verticalScrollBar()->setValue(verticalScrollBar()->value() - scrollAmount);
    } else if (key == Qt::Key_Down) {
        verticalScrollBar()->setValue(verticalScrollBar()->value() + scrollAmount);
    } else if (key == Qt::Key_Left) {
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() - scrollAmount);
    } else if (key == Qt::Key_Right) {
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() + scrollAmount);
    }

    // Delegate Common Shortcuts to MainWindow
    MainWindow* mainWin = qobject_cast<MainWindow*>(parentWidget());
    if (mainWin) {
        if (event->matches(QKeySequence::Undo)) { mainWin->undo(); event->accept(); return;}
        if (event->matches(QKeySequence::Redo)) { mainWin->redo(); event->accept(); return;}
        if (event->matches(QKeySequence::Cut)) { mainWin->cutSelection(); event->accept(); return;}
        if (event->matches(QKeySequence::Copy)) { mainWin->copySelection(); event->accept(); return;}
        if (event->matches(QKeySequence::Paste)) { mainWin->pasteSelection(); event->accept(); return;}
        if (key == Qt::Key_Delete) { mainWin->deleteSelection(); event->accept(); return;}
        if (event->matches(QKeySequence::SelectAll)) { mainWin->selectAll(); event->accept(); return;}
        
        // Handle Escape to cancel paste or other modes
        if (key == Qt::Key_Escape && mainWin->IsPasting()) {
            mainWin->EndPasting();
            event->accept(); return;
        }
        // Specific Hotkeys from original RME `map_display.cpp::OnKeyDown`
        if (key == Qt::Key_A && modifiers == Qt::NoModifier) {
             mainWin->toggleBorderSystem(!mainWin->getBorderSystem()->isEnabled()); // Toggle Automagic
             event->accept(); return;
        }
        if (key == Qt::Key_Q && modifiers == Qt::NoModifier) {
            mainWin->selectPreviousBrush(); // Select Previous Brush
            event->accept(); return;
        }
        if (key == Qt::Key_Plus || key == Qt::Key_Add) { // Plus/Numpad Plus
            mainWin->increaseBrushSize();
            event->accept(); return;
        }
        if (key == Qt::Key_Minus || key == Qt::Key_Subtract) { // Minus/Numpad Minus
            mainWin->decreaseBrushSize();
            event->accept(); return;
        }
        if (key == Qt::Key_PageDown) { // Floor Down
            mainWin->changeFloor(currentLayer - 1);
            event->accept(); return;
        }
        if (key == Qt::Key_PageUp) { // Floor Up
            mainWin->changeFloor(currentLayer + 1);
            event->accept(); return;
        }
        if (key == Qt::Key_Space) { // Toggle Selection/Drawing Mode
            mainWin->switchToolMode();
            event->accept(); return;
        }
        if ((key == Qt::Key_X || key == Qt::Key_Z) && modifiers == Qt::NoModifier) { // Rotate Item (Original Z/X)
            // This is specific to `Item` rotation logic within the map.
            // Delegate to Map/Editor class with current selection context.
            if (currentMap && !currentMap->getSelection().isEmpty()) {
                 currentMap->rotateSelection((key == Qt::Key_Z) ? -90 : 90); // Z: -90, X: +90 degrees (example)
                 event->accept(); return;
            }
        }
    }
    
    // Pass event to current brush (e.g., selection brush for keyboard operations).
    if (currentBrush) {
        currentBrush->keyPressEvent(event, this);
        mapScene->update(); // Brush interaction might affect visuals.
        updateCursor(); // Brush interaction might affect cursor.
        event->accept(); return;
    }

    QGraphicsView::keyPressEvent(event); // Call base class for unhandled events.
}

void MapView::updateVisibleTiles()
{
    // Updates MapScene with the currently visible area of the viewport.
    // MapScene then creates/removes MapTileItems as needed for performance.
    if (!mapScene || !currentMap) return;

    QRectF visibleRect = mapToScene(viewport()->rect()).boundingRect();
    mapScene->updateVisibleTiles(visibleRect);
}

void MapView::updateCursor()
{
    // Updates the position and appearance of the brush preview cursor.
    if (!cursorItem || !currentMap || !isVisible() || !mouseInsideView) { // Check mouseInsideView
        if (cursorItem) cursorItem->setPixmap(QPixmap()); // Hide cursor if view not visible or mouse outside
        return;
    }

    QPoint viewPos = mapFromGlobal(QCursor::pos());
    if (!viewport()->rect().contains(viewPos)) { // Check if cursor is within the viewport bounds
        cursorItem->setPixmap(QPixmap());
        return;
    }
    
    QPoint tilePos = mapToTile(viewPos);
    
    if (tilePos.x() < 0 || tilePos.x() >= currentMap->getSize().width() ||
        tilePos.y() < 0 || tilePos.y() >= currentMap->getSize().height()) {
         if (cursorItem) cursorItem->setPixmap(QPixmap());
         return;
    }

    cursorItem->setPos(tilePos.x() * MapTileItem::TilePixelSize, tilePos.y() * MapTileItem::TilePixelSize);

    QPixmap cursorPixmap;
    if (currentBrush) {
        // Get brush size in tiles (e.g., 1 for 1x1, 2 for 3x3 (center + 1 around), 0 for point)
        // This needs to align with how Brush::getSize() is defined.
        // Assuming Brush::getSize() returns radius for now (0 means 1x1, 1 means 3x3)
        int brushRadius = currentBrush->getSize(); 
        int previewDiameter = (brushRadius * 2) + 1; // Diameter in tiles. If size is 0, diameter is 1.
        
        QSize pixmapSize(previewDiameter * MapTileItem::TilePixelSize, 
                         previewDiameter * MapTileItem::TilePixelSize);
        cursorPixmap = QPixmap(pixmapSize);
        cursorPixmap.fill(Qt::transparent); // Start with transparent background.

        QPainter painter(&cursorPixmap);
        currentBrush->drawPreview(painter, QPoint(0, 0), zoom); // Brush draws itself on (0,0) of this pixmap.
        painter.end();
    } else {
        // Default crosshairs for unselected brush or unknown tool.
        cursorPixmap = QPixmap(MapTileItem::TilePixelSize, MapTileItem::TilePixelSize);
        cursorPixmap.fill(Qt::transparent);
        QPainter painter(&cursorPixmap);
        painter.setPen(QPen(Qt::white, 1));
        painter.drawRect(0,0,MapTileItem::TilePixelSize-1,MapTileItem::TilePixelSize-1);
        painter.drawLine(0, MapTileItem::TilePixelSize / 2, MapTileItem::TilePixelSize, MapTileItem::TilePixelSize / 2);
        painter.drawLine(MapTileItem::TilePixelSize / 2, 0, MapTileItem::TilePixelSize / 2, MapTileItem::TilePixelSize);
        painter.end();
    }
    cursorItem->setPixmap(cursorPixmap);
}

// Helper function to create and add actions to a menu.
QAction* MapView::createAction(QMenu* menu, const QString& text, QObject* receiver, const char* member, bool enabled, QKeySequence shortcut) {
    QAction* action = menu->addAction(text, receiver, member);
    action->setEnabled(enabled);
    if (shortcut.isValid()) {
        action->setShortcut(shortcut);
    }
    return action;
}

void MapView::createContextMenu(const QPoint& globalPos)
{
    QMenu contextMenu(this); // The menu.

    QPoint tileMapPos = mapToTile(mapFromGlobal(globalPos)); // Tile position under the mouse.
    Tile* currentTile = currentMap ? currentMap->getTile(tileMapPos.x(), tileMapPos.y(), currentLayer) : nullptr;
    
    // Check various states for enabling/disabling actions.
    bool hasMapSelection = currentMap ? !currentMap->getSelection().isEmpty() : false;
    bool singleTileSelected = currentMap ? (currentMap->getSelection().width() == 1 && currentMap->getSelection().height() == 1) : false;
    bool hasClipboardData = ClipboardData::getInstance().hasClipboardData(); // Requires ClipboardData class to be integrated.

    // --- Core Edit Actions (Undo/Redo, Cut/Copy/Paste/Delete) ---
    // These generally call corresponding methods in MainWindow or delegate actions directly.
    createAction(&contextMenu, tr("Undo"), this, SLOT(onUndoAction()), qobject_cast<MainWindow*>(parentWidget())->getUndoStack()->canUndo(), QKeySequence::Undo);
    createAction(&contextMenu, tr("Redo"), this, SLOT(onRedoAction()), qobject_cast<MainWindow*>(parentWidget())->getUndoStack()->canRedo(), QKeySequence::Redo);
    contextMenu.addSeparator();

    createAction(&contextMenu, tr("Cut"), this, SLOT(onCutSelectionAction()), hasMapSelection, QKeySequence::Cut);
    createAction(&contextMenu, tr("Copy"), this, SLOT(onCopySelectionAction()), hasMapSelection, QKeySequence::Copy);
    createAction(&contextMenu, tr("Paste"), this, SLOT(onPasteSelectionAction()), hasClipboardData, QKeySequence::Paste);
    createAction(&contextMenu, tr("Delete"), this, SLOT(onDeleteSelectionAction()), hasMapSelection, QKeySequence::Delete);
    contextMenu.addSeparator();

    // --- Tile/Item/Creature Specific Actions ---
    // Actions enabled if a valid tile (with optional content) is under the mouse.
    if (currentTile) {
        // Tile properties
        createAction(&contextMenu, tr("Properties"), this, SLOT(onTilePropertiesAction()));
        
        // Item specific actions (Source/src/map_display.cpp - MapPopupMenu::Update, On* methods)
        if (!currentTile->getItems().isEmpty()) {
            contextMenu.addSeparator();
            // Try to get the item instance if there is one on the current layer for advanced checks.
            Item currentTileItem = currentTile->getItems().first(); // Simplified, take first item for checks.

            createAction(&contextMenu, tr("Item Properties"), this, SLOT(onItemPropertiesAction()));

            QAction* rotateItem = createAction(&contextMenu, tr("Rotate Item"), this, SLOT(onRotateItemAction()));
            // Check if item has property IsRotatable based on Source/src/item.h.
            rotateItem->setEnabled(currentTileItem.hasProperty(static_cast<ItemProperty>(ItemProperty::IsRotatable)));
            
            QAction* switchDoor = createAction(&contextMenu, tr("Switch Door State"), this, SLOT(onSwitchDoorAction()));
            switchDoor->setEnabled(currentTileItem.hasProperty(static_cast<ItemProperty>(ItemProperty::IsDoor))); // Check if item is a door.

            QAction* gotoDest = createAction(&contextMenu, tr("Go To Destination"), this, SLOT(onGotoDestinationAction()));
            gotoDest->setEnabled(currentTileItem.hasProperty(static_cast<ItemProperty>(ItemProperty::IsTeleport))); // Check if item is a teleport.

            // Submenu for copying item information
            QMenu* copyItemInfoMenu = contextMenu.addMenu(tr("Copy Item Info"));
            createAction(copyItemInfoMenu, tr("Copy Position (XYZ)"), this, SLOT(onCopyPositionAction()));
            createAction(copyItemInfoMenu, tr("Copy Server ID"), this, SLOT(onCopyServerIdAction()));
            createAction(copyItemInfoMenu, tr("Copy Client ID"), this, SLOT(onCopyClientIdAction()));
            createAction(copyItemInfoMenu, tr("Copy Name"), this, SLOT(onCopyNameAction()));
        }
        
        // Creature specific actions
        if (!currentTile->getCreatures().isEmpty()) {
            contextMenu.addSeparator();
            createAction(&contextMenu, tr("Creature Properties"), this, SLOT(onCreaturePropertiesAction()));
        }

        // --- Common Editor Operations ---
        contextMenu.addSeparator();
        createAction(&contextMenu, tr("Browse Tile..."), this, SLOT(onBrowseTileAction()));
        createAction(&contextMenu, tr("Find Similar Items..."), this, SLOT(onFindSimilarItemsAction()));
        contextMenu.addSeparator();
        createAction(&contextMenu, tr("Generate Island..."), this, SLOT(onGenerateIslandAction()));
        createAction(&contextMenu, tr("Create House..."), this, SLOT(onCreateHouseAction()));
        contextMenu.addSeparator();
        
        createAction(&contextMenu, tr("Fill"), this, SLOT(onFillAction()), (currentBrush && currentBrush->getType() == Brush::Type::FloodFill));
        contextMenu.addSeparator();

        // --- Brush Selection Actions ---
        // These will set the current tool based on properties of the tile/item under the mouse.
        // Similar to `Source/src/map_display.cpp`'s `OnSelectRAWBrush`, `OnSelectGroundBrush`, etc.
        // Only if a single tile is clicked/selected and has content.
        if (currentTile && singleTileSelected) {
            Item *groundItem = nullptr;
            for (const Item& item : currentTile->getItems()) { // Iterate to find ground item for type checks.
                if (item.hasProperty(static_cast<ItemProperty>(ItemProperty::IsGroundTile))) { // Find a ground item.
                    groundItem = const_cast<Item*>(&item); // Safely get non-const ptr for ground property checks
                    break;
                }
            }

            if (groundItem) {
                 createAction(&contextMenu, tr("Select Ground Brush"), this, SLOT(onSelectGroundBrushAction()));
                 if (groundItem->getGroundBrush()) { // If specific ground brush, show.
                    createAction(&contextMenu, tr("Select Specific Ground Brush"), this, SLOT(onSelectGroundBrushAction())); // Could map to specific ground brush.
                 }
            }

            // Loop through all items on the tile for specific brush selections.
            for (const Item& item : currentTile->getItems()) {
                 if (item.getType() == "RAW") { // Simplified check based on raw item type string or special flag
                     createAction(&contextMenu, tr("Select RAW Brush"), this, SLOT(onSelectRawBrushAction()));
                 }
                 if (item.hasProperty(static_cast<ItemProperty>(ItemProperty::IsDoodad))) {
                     createAction(&contextMenu, tr("Select Doodad Brush"), this, SLOT(onSelectDoodadBrushAction()));
                 }
                 if (item.hasProperty(static_cast<ItemProperty>(ItemProperty::IsDoor))) {
                     createAction(&contextMenu, tr("Select Door Brush"), this, SLOT(onSelectDoorBrushAction()));
                 }
                 if (item.hasProperty(static_cast<ItemProperty>(ItemProperty::IsWall))) { // Check for IsWall property
                     createAction(&contextMenu, tr("Select Wall Brush"), this, SLOT(onSelectWallBrushAction()));
                 }
                 if (item.hasProperty(static_cast<ItemProperty>(ItemProperty::IsCarpet))) {
                     createAction(&contextMenu, tr("Select Carpet Brush"), this, SLOT(onSelectCarpetBrushAction()));
                 }
                 if (item.hasProperty(static_cast<ItemProperty>(ItemProperty::IsTable))) {
                     createAction(&contextMenu, tr("Select Table Brush"), this, SLOT(onSelectTableBrushAction()));
                 }
                 if (item.hasProperty(static_cast<ItemProperty>(ItemProperty::IsCreature))) { // Assuming creature items have this property.
                    createAction(&contextMenu, tr("Select Creature Brush"), this, SLOT(onSelectCreatureBrushAction()));
                 }
                 // Handle specific creature brushes via separate item types in creature brushes, etc.
                 if (item.hasProperty(static_cast<ItemProperty>(ItemProperty::IsHouse))) {
                    createAction(&contextMenu, tr("Select House Brush"), this, SLOT(onSelectHouseBrushAction()));
                 }
                 if (item.hasProperty(static_cast<ItemProperty>(ItemProperty::IsWaypoint))) {
                    createAction(&contextMenu, tr("Select Waypoint Brush"), this, SLOT(onSelectMoveToAction())); // 'Move To' or Waypoint brush
                 }
                 // Collection and Spawn brushes might need specific detection
            }
             if (currentTile->getCreatures().size() > 0) { // Separate creature brushes from items.
                createAction(&contextMenu, tr("Select Creature Brush"), this, SLOT(onSelectCreatureBrushAction())); // Based on creature at tile.
                // Could be `onSelectSpawnBrushAction()` if creature is part of spawn.
            }
            contextMenu.addSeparator(); // Separator for Brush selection group.
        } // End if (singleTileSelected)


        // --- Map Level Actions (Always present if currentMap exists) ---
        createAction(&contextMenu, tr("Map Properties..."), this, SLOT(onMapPropertiesAction()));
        createAction(&contextMenu, tr("Map Statistics..."), qobject_cast<MainWindow*>(parentWidget()), SLOT(showMapStatisticsDialog()));
        createAction(&contextMenu, tr("Go To Position..."), qobject_cast<MainWindow*>(parentWidget()), SLOT(showGotoPositionDialog()));

        contextMenu.addSeparator();
    } // End if (currentTile)

    // Global Exit action.
    createAction(&contextMenu, tr("Exit Application"), qApp, SLOT(quit()));

    // Show the menu at the global cursor position.
    contextMenu.exec(globalPos);
}


// --- Context Menu Action Handlers (Delegating to MainWindow or performing local operations) ---

void MapView::onUndoAction() { qobject_cast<MainWindow*>(parentWidget())->undo(); }
void MapView::onRedoAction() { qobject_cast<MainWindow*>(parentWidget())->redo(); }

void MapView::onCutSelectionAction() {
    // Emits signal to MainWindow to perform the cut, including data in Map::getSelection().
    if (currentMap && !currentMap->getSelection().isEmpty()) {
        emit cutRequest(currentMap->getSelection());
    }
}
void MapView::onCopySelectionAction() {
    // Emits signal to MainWindow to perform the copy.
    if (currentMap && !currentMap->getSelection().isEmpty()) {
        emit copyRequest(currentMap->getSelection());
    }
}
void MapView::onPasteSelectionAction() {
    // Emits signal to MainWindow to perform the paste, using current mouse position as target.
    if (currentMap && ClipboardData::getInstance().hasClipboardData()) {
        emit pasteRequest(mapToTile(mapFromGlobal(QCursor::pos())));
    } else {
        QMessageBox::information(this, tr("Paste Error"), tr("No content to paste in clipboard."));
    }
}
void MapView::onDeleteSelectionAction() {
    // Emits signal to MainWindow to perform the delete.
    if (currentMap && !currentMap->getSelection().isEmpty()) {
        emit deleteRequest(currentMap->getSelection());
    }
}

void MapView::onCopyPositionAction() {
    QPoint tilePos = mapToTile(mapFromGlobal(QCursor::pos()));
    QString positionString = QString("%1 %2 %3").arg(tilePos.x()).arg(tilePos.y()).arg(currentLayer);
    QApplication::clipboard()->setText(positionString);
    qDebug() << "Copied position:" << positionString;
}

void MapView::onCopyServerIdAction() {
    QPoint tilePos = mapToTile(mapFromGlobal(QCursor::pos()));
    Tile* tile = currentMap->getTile(tilePos.x(), tilePos.y(), currentLayer);
    if (tile && !tile->getItems().isEmpty()) {
        QString serverId = QString::number(tile->getItems().first().getId());
        QApplication::clipboard()->setText(serverId);
        qDebug() << "Copied server ID:" << serverId;
    }
}

void MapView::onCopyClientIdAction() {
    QPoint tilePos = mapToTile(mapFromGlobal(QCursor::pos()));
    Tile* tile = currentMap->getTile(tilePos.x(), tilePos.y(), currentLayer);
    if (tile && !tile->getItems().isEmpty()) {
        // Client ID might be different from Server ID in Tibia. Assuming they are same for now.
        // In a real Tibia editor, you'd lookup `ItemManager::getInstance().getClientId(tile->getItems().first().getId())`.
        QString clientId = QString::number(tile->getItems().first().getId());
        QApplication::clipboard()->setText(clientId);
        qDebug() << "Copied client ID:" << clientId;
    }
}

void MapView::onCopyNameAction() {
    QPoint tilePos = mapToTile(mapFromGlobal(QCursor::pos()));
    Tile* tile = currentMap->getTile(tilePos.x(), tilePos.y(), currentLayer);
    if (tile && !tile->getItems().isEmpty()) {
        QString itemName = tile->getItems().first().getName(); // Assuming Item has getName() method
        QApplication::clipboard()->setText(itemName);
        qDebug() << "Copied name:" << itemName;
    }
}

void MapView::onRotateItemAction() {
    qDebug() << "Rotate Item action triggered.";
    // Delegate to Map to rotate the current selection (or first item on clicked tile).
    // `Source/editor.cpp::rotateSelection()`
    if (currentMap && !currentMap->getSelection().isEmpty()) {
        currentMap->rotateSelection(90); // Default 90 degrees rotation.
    }
}

void MapView::onSwitchDoorAction() {
    qDebug() << "Switch Door State action triggered.";
    // Locate the door item on the clicked tile/selected tile and call its `switchDoor()`
    // This typically needs more specific integration with a `Door` Item subclass logic.
}

void MapView::onGotoDestinationAction() {
    qDebug() << "Go To Destination action triggered (needs teleport item context).";
    // This would identify a teleport item on the clicked tile, extract its destination, and
    // then call `MainWindow::showGotoPositionDialog` or directly manipulate the view.
    qobject_cast<MainWindow*>(parentWidget())->showGotoPositionDialog();
}

void MapView::onBrowseTileAction() {
    qDebug() << "Browse Tile action triggered.";
    // This would open a dialog (`BrowseTileWindow` from original `Source`) showing items on the tile.
    qobject_cast<MainWindow*>(parentWidget())->showFindItemDialog(); // Placeholder for a generic search/browse dialog.
}

void MapView::onFindSimilarItemsAction() {
    qDebug() << "Find Similar Items action triggered.";
    // Delegates to a generic search dialog.
    qobject_cast<MainWindow*>(parentWidget())->showFindItemDialog(); // Placeholder.
}

void MapView::onGenerateIslandAction() {
    qDebug() << "Generate Island action triggered.";
    // Delegates to the Island Generator Dialog.
    qobject_cast<MainWindow*>(parentWidget())->showMapPropertiesDialog(); // Placeholder, needs actual IslandGen dialog.
}

void MapView::onCreateHouseAction() {
    qDebug() << "Create House action triggered.";
    // Delegates to the Create House dialog/tool.
    qobject_cast<MainWindow*>(parentWidget())->showMapStatisticsDialog(); // Placeholder, needs actual House Creation dialog.
}

void MapView::onTilePropertiesAction() {
    qDebug() << "Tile properties action triggered.";
    // Emits signal for MainWindow to open `PropertyEditorDock` to edit `Tile` properties.
    QPoint tilePos = mapToTile(mapFromGlobal(QCursor::pos()));
    Tile* tile = currentMap->getTile(tilePos.x(), tilePos.y(), currentLayer);
    emit tileSelected(tile);
}

void MapView::onItemPropertiesAction() {
    qDebug() << "Item properties action triggered.";
    // Emits signal for `MainWindow` to open `PropertyEditorDock` for item.
    QPoint tilePos = mapToTile(mapFromGlobal(QCursor::pos()));
    Tile* tile = currentMap->getTile(tilePos.x(), tilePos.y(), currentLayer);
    if (tile && !tile->getItems().isEmpty()) {
        emit itemSelected(&(tile->getItems().first())); // Selects the first item for now.
    }
}

void MapView::onCreaturePropertiesAction() {
    qDebug() << "Creature properties action triggered.";
    // Emits signal for `MainWindow` to open `PropertyEditorDock` for creature.
    QPoint tilePos = mapToTile(mapFromGlobal(QCursor::pos()));
    Tile* tile = currentMap->getTile(tilePos.x(), tilePos.y(), currentLayer);
    if (tile && !tile->getCreatures().isEmpty()) {
        emit creatureSelected(tile->getCreatures().first()); // Selects the first creature for now.
    }
}

void MapView::onMapPropertiesAction() {
    qDebug() << "Map properties action triggered.";
    // Delegates to `MainWindow` to open `MapPropertiesDialog`.
    qobject_cast<MainWindow*>(parentWidget())->showMapPropertiesDialog();
}

void MapView::onFillAction() {
    qDebug() << "Fill action triggered.";
    if (currentBrush && currentBrush->getType() == Brush::Type::FloodFill) {
        // Execute the flood fill starting from the clicked tile.
        QPoint tilePos = mapToTile(mapFromGlobal(QCursor::pos()));
        static_cast<FloodFillBrush*>(currentBrush)->floodFill(tilePos);
        mapScene->update(); // Redraw scene.
    } else {
        QMessageBox::information(this, tr("Fill Tool"), tr("Please select the Flood Fill brush to use this action."));
    }
}

// --- Brush Selection Actions ---
// These slots receive signals from context menu actions and delegate to MainWindow
// to set the globally active brush.

void MapView::onSelectRawBrushAction() {
    qDebug() << "Select RAW Brush action.";
    qobject_cast<MainWindow*>(parentWidget())->setCurrentTool(Brush::Type::Normal); // RAW is typically Normal brush mode
    // Additional logic might be needed to set item ID based on current tile or open dialog
}
void MapView::onSelectGroundBrushAction() {
    qDebug() << "Select Ground Brush action.";
    qobject_cast<MainWindow*>(parentWidget())->setCurrentTool(Brush::Type::Normal); // Ground brush uses Normal brush mode.
}
void MapView::onSelectDoodadBrushAction() {
    qDebug() << "Select Doodad Brush action.";
    qobject_cast<MainWindow*>(parentWidget())->setCurrentTool(Brush::Type::Normal); // Doodad uses Normal brush mode.
}
void MapView::onSelectDoorBrushAction() {
    qDebug() << "Select Door Brush action.";
    qobject_cast<MainWindow*>(parentWidget())->setCurrentTool(Brush::Type::Normal); // Door uses Normal brush mode.
}
void MapView::onSelectWallBrushAction() {
    qDebug() << "Select Wall Brush action.";
    qobject_cast<MainWindow*>(parentWidget())->setCurrentTool(Brush::Type::Normal); // Wall uses Normal brush mode.
}
void MapView::onSelectCarpetBrushAction() {
    qDebug() << "Select Carpet Brush action.";
    qobject_cast<MainWindow*>(parentWidget())->setCurrentTool(Brush::Type::Normal); // Carpet uses Normal brush mode.
}
void MapView::onSelectTableBrushAction() {
    qDebug() << "Select Table Brush action.";
    qobject_cast<MainWindow*>(parentWidget())->setCurrentTool(Brush::Type::Normal); // Table uses Normal brush mode.
}
void MapView::onSelectCreatureBrushAction() {
    qDebug() << "Select Creature Brush action.";
    qobject_cast<MainWindow*>(parentWidget())->setCurrentTool(Brush::Type::Normal); // Creature uses Normal brush mode.
}
void MapView::onSelectSpawnBrushAction() {
    qDebug() << "Select Spawn Brush action.";
    qobject_cast<MainWindow*>(parentWidget())->setCurrentTool(Brush::Type::Normal); // Spawn uses Normal brush mode.
}
void MapView::onSelectHouseBrushAction() {
    qDebug() << "Select House Brush action.";
    qobject_cast<MainWindow*>(parentWidget())->setCurrentTool(Brush::Type::Normal); // House uses Normal brush mode.
}
void MapView::onSelectCollectionBrushAction() {
    qDebug() << "Select Collection Brush action.";
    qobject_cast<MainWindow*>(parentWidget())->setCurrentTool(Brush::Type::Normal); // Collection uses Normal brush mode.
}
void MapView::onSelectMoveToAction() {
    qDebug() << "Select Move To action.";
    qobject_cast<MainWindow*>(parentWidget())->setCurrentTool(Brush::Type::Selection); // Move To uses Selection brush mode.
}