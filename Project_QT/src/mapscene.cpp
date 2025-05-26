#include "mapscene.h"
#include "itemmanager.h"
#include "creaturemanager.h"
#include "spritemanager.h"
#include "clientversion.h"

#include <QDebug>
#include <QGraphicsLineItem>
#include <QGraphicsView>


MapScene::MapScene(QObject* parent) :
    QGraphicsScene(parent),
    currentMap(nullptr),
    currentLayer(static_cast<int>(Layer::Type::Ground)),
    showGrid(false),
    showCollisions(false),
    selectionRectItem(new MapSelectionItem(QRect(), this))
{
    selectionRectItem->setVisible(false);
    addItem(selectionRectItem);
}

MapScene::~MapScene()
{
    // currentMap is not owned.
}

void MapScene::setMap(Map* map)
{
    if (currentMap == map) return;

    if (currentMap) {
        // Disconnect old map signals.
    }

    currentMap = map;
    clearAllTileItems();

    if (currentMap) {
        setSceneRect(0, 0, currentMap->getSize().width() * MapTileItem::TilePixelSize,
                     currentMap->getSize().height() * MapTileItem::TilePixelSize);
    } else {
        setSceneRect(0, 0, 0, 0);
    }
    update();
}

void MapScene::updateTile(const QPoint& position)
{
    if (!currentMap) return;

    removeTileItem(position); // Remove existing.

    // A tile is now considered "present" if the ground layer tile data exists OR if any item at ANY visible layer exists for this X,Y.
    // This allows background to draw an "empty" grid slot and then MapTileItem adds visual content only when non-empty.
    bool hasContent = false;
    for (int z_layer = 0; z_layer < Map::LayerCount; ++z_layer) {
        Layer* layerObj = currentMap->getLayer(static_cast<Layer::Type>(z_layer));
        if (layerObj && layerObj->isVisible()) { // Check only visible layers.
            Tile* tileDataAtLayer = currentMap->getTile(position.x(), position.y(), z_layer);
            if (tileDataAtLayer && !tileDataAtLayer->isEmpty()) {
                hasContent = true;
                break;
            }
        }
    }
    
    if (hasContent) { // Only create MapTileItem if there's actual visual content on it at any visible layer.
        createTileItem(position);
    }
    update();
}

void MapScene::updateVisibleTiles(const QRectF& visibleRect)
{
    if (!currentMap) {
        clearAllTileItems();
        return;
    }

    int pad = 2; 
    int minX = qMax(0, static_cast<int>(floor(visibleRect.left() / MapTileItem::TilePixelSize)) - pad);
    int minY = qMax(0, static_cast<int>(floor(visibleRect.top() / MapTileItem::TilePixelSize)) - pad);
    int maxX = qMin(currentMap->getSize().width() - 1, static_cast<int>(ceil(visibleRect.right() / MapTileItem::TilePixelSize)) + pad - 1);
    int maxY = qMin(currentMap->getSize().height() - 1, static_cast<int>(ceil(visibleRect.bottom() / MapTileItem::TilePixelSize)) + pad - 1);

    QSet<QPoint> newVisibleSet;
    for (int y = minY; y <= maxY; ++y) {
        for (int x = minX; x <= maxX; ++x) {
            newVisibleSet.insert(QPoint(x, y));
        }
    }

    QSet<QPoint> toRemove = visibleTilePositions - newVisibleSet;
    for (const QPoint& pos : toRemove) {
        removeTileItem(pos);
    }

    QSet<QPoint> toAdd = newVisibleSet - visibleTilePositions;
    for (const QPoint& pos : toAdd) {
        // A tile is now considered "present" if its ground layer tile data exists OR if any item at ANY visible layer exists.
        // `createTileItem` itself now checks for this.
        createTileItem(pos); // `createTileItem` will decide if there's visible content and create it.
    }
    
    visibleTilePositions = newVisibleSet;

    if (showGrid) {
        createGridLines(visibleRect);
    } else {
        foreach (QGraphicsLineItem* line, gridLines.values()) {
            removeItem(line);
            delete line;
        }
        gridLines.clear();
    }
    update();
}

void MapScene::setCurrentLayer(int layer)
{
    if (currentLayer == layer) return;
    currentLayer = layer;
    
    for (MapTileItem* item : tileItems.values()) {
        item->setActiveLayer(currentLayer);
        item->update();
    }
    update();
}

void MapScene::setShowGrid(bool show)
{
    if (showGrid == show) return;
    showGrid = show;
    
    if (showGrid) {
        QGraphicsView* view = !views().isEmpty() ? views().first() : nullptr;
        if (view) {
            createGridLines(view->mapToScene(view->viewport()->rect()).boundingRect());
        } else {
            createGridLines(sceneRect());
        }
    } else {
        foreach (QGraphicsLineItem* line, gridLines.values()) {
            removeItem(line);
            delete line;
        }
        gridLines.clear();
    }
    update();
}

void MapScene::setShowCollisions(bool show)
{
    if (showCollisions == show) return;
    showCollisions = show;
    for (MapTileItem* item : tileItems.values()) {
        item->setShowCollisions(show);
        item->update();
    }
    update();
}

void MapScene::clearSelection()
{
    if (selectionRectItem) {
        selectionRectItem->setVisible(false);
    }
    for (MapTileItem* item : tileItems.values()) {
        if (item->isSelected()) {
            item->setSelected(false);
            item->update();
        }
    }
    if (currentMap) {
        currentMap->clearSelection();
    }
    update();
}

void MapScene::selectTile(const QPoint& position)
{
    if (!currentMap || !selectionRectItem) return;

    selectionRectItem->setRect(QRect(position.x(), position.y(), 1, 1));
    selectionRectItem->setVisible(true);
    selectionRectItem->update();

    for (MapTileItem* item : tileItems.values()) {
        item->setSelected(item->getTile()->getPosition() == position);
        item->update();
    }
    update();
}

void MapScene::selectTiles(const QRect& rect)
{
    if (!currentMap || !selectionRectItem) return;

    selectionRectItem->setRect(rect);
    selectionRectItem->setVisible(true);
    selectionRectItem->update();

    for (MapTileItem* item : tileItems.values()) {
        item->setSelected(rect.contains(item->getTile()->getPosition()));
        item->update();
    }
    update();
}

void MapScene::createGridLines(const QRectF& visibleRect)
{
    foreach (QGraphicsLineItem* line, gridLines.values()) {
        removeItem(line);
        delete line;
    }
    gridLines.clear();

    if (!currentMap || !showGrid) return;

    qreal left = visibleRect.left();
    qreal right = visibleRect.right();
    qreal top = visibleRect.top();
    qreal bottom = visibleRect.bottom();

    left = floor(left / MapTileItem::TilePixelSize) * MapTileItem::TilePixelSize;
    top = floor(top / MapTileItem::TilePixelSize) * MapTileItem::TilePixelSize;
    right = ceil(right / MapTileItem::TilePixelSize) * MapTileItem::TilePixelSize;
    bottom = ceil(bottom / MapTileItem::TilePixelSize) * MapTileItem::TilePixelSize;

    QPen gridPen(Qt::lightGray, 1);
    gridPen.setCosmetic(true);

    // Vertical lines
    for (qreal x = left; x <= right + MapTileItem::TilePixelSize / 2.0; x += MapTileItem::TilePixelSize) {
        if (x < 0 || x > currentMap->getSize().width() * MapTileItem::TilePixelSize) continue;
        QGraphicsLineItem* line = new QGraphicsLineItem(x, top, x, bottom);
        line->setPen(gridPen);
        line->setZValue(999);
        addItem(line);
        gridLines.insert(x, line);
    }

    // Horizontal lines
    for (qreal y = top; y <= bottom + MapTileItem::TilePixelSize / 2.0; y += MapTileItem::TilePixelSize) {
        if (y < 0 || y > currentMap->getSize().height() * MapTileItem::TilePixelSize) continue;
        QGraphicsLineItem* line = new QGraphicsLineItem(left, y, right, y);
        line->setPen(gridPen);
        line->setZValue(999);
        addItem(line);
        gridLines.insert(y, line);
    }
}

void MapScene::updateGridLines()
{
    QGraphicsView* view = !views().isEmpty() ? views().first() : nullptr;
    if (view) {
        createGridLines(view->mapToScene(view->viewport()->rect()).boundingRect());
    } else {
        createGridLines(sceneRect());
    }
}

void MapScene::createTileItem(const QPoint& position)
{
    if (!currentMap || tileItems.contains(position)) return;

    // Check if there is any visible content at this position.
    bool hasVisibleContent = false;
    for (int z_layer = 0; z_layer < Map::LayerCount; ++z_layer) {
        Layer* layerObj = currentMap->getLayer(static_cast<Layer::Type>(z_layer));
        if (layerObj && layerObj->isVisible()) {
            Tile* tileDataAtLayer = currentMap->getTile(position.x(), position.y(), z_layer);
            if (tileDataAtLayer && !tileDataAtLayer->isEmpty()) {
                hasVisibleContent = true;
                break;
            }
        }
    }

    // Only create a `MapTileItem` if the tile has actual visual content on any visible layer.
    // If we were to display `empty` tiles (e.g. gray square, to show presence of map space),
    // they should be drawn in `drawBackground`.
    if (hasVisibleContent) {
        // Pass the actual `Tile` data for `Layer::Ground` to `MapTileItem` for overall `Tile` data representation.
        // The `MapTileItem`'s `updateCache` then uses `mapModel` and `activeLayer` to decide which content to draw.
        Tile* groundTileData = currentMap->getTile(position.x(), position.y(), static_cast<Layer::Type>(Layer::Type::Ground));
        
        MapTileItem* item = new MapTileItem(groundTileData, currentMap, currentLayer, this);
        item->setPos(position.x() * MapTileItem::TilePixelSize, position.y() * MapTileItem::TilePixelSize);
        item->setShowCollisions(showCollisions);
        
        item->setFlags(QGraphicsItem::ItemSendsGeometryChanges | QGraphicsItem::ItemIsSelectable); 

        addItem(item);
        tileItems.insert(position, item);
    }
}

void MapScene::removeTileItem(const QPoint& position)
{
    MapTileItem* item = tileItems.value(position, nullptr);
    if (item) {
        removeItem(item);
        tileItems.remove(position);
        delete item;
    }
}

void MapScene::clearAllTileItems()
{
    foreach (MapTileItem* item, tileItems.values()) {
        removeItem(item);
        delete item;
    }
    tileItems.clear();
    visibleTilePositions.clear();

    foreach (QGraphicsLineItem* line, gridLines.values()) {
        removeItem(line);
        delete line;
    }
    gridLines.clear();

    if (selectionRectItem) {
        selectionRectItem->setVisible(false);
        selectionRectItem->setRect(QRectF());
    }
    update();
}

void MapScene::drawBackground(QPainter* painter, const QRectF& rect)
{
    // Draw the solid black background or a grid of dark gray tiles
    // This provides the default "empty map" look below all `MapTileItem`s.
    painter->fillRect(rect, Qt::darkGray); // Consistent with MapTileItem's fallback.
    
    // You could also draw a detailed checkerboard background here, similar to what a basic map editor might do for empty spaces.
    // Example (very simplified checkerboard):
    // const int TILE_SIZE = MapTileItem::TilePixelSize;
    // QRectF adjustedRect = rect.normalized();
    // for (qreal x = adjustedRect.left(); x < adjustedRect.right(); x += TILE_SIZE) {
    //     for (qreal y = adjustedRect.top(); y < adjustedRect.bottom(); y += TILE_SIZE) {
    //         if (fmod(floor(x / TILE_SIZE) + floor(y / TILE_SIZE), 2.0) == 0) {
    //             painter->fillRect(x, y, TILE_SIZE, TILE_SIZE, QColor(60, 60, 60)); // Darker gray
    //         } else {
    //             painter->fillRect(x, y, TILE_SIZE, TILE_SIZE, QColor(70, 70, 70)); // Lighter gray
    //         }
    //     }
    // }

    // No need to call base class as we want full custom background.
}


// --- MapTileItem Implementation (remains as in previous delivery, with subtle clarifications in comments) ---
// Using ItemProperty enum from Source/src/item.h for type-checking.
// This block ensures the correct definition is available here if not in main.h.
#ifndef ITEMPROPERTY_IS_ROTATABLE_DEFINED
    #define ITEMPROPERTY_IS_ROTATABLE_DEFINED
    enum ItemProperty {
        IsGround = 1, IsWall = 2, IsDoor = 3, IsTeleport = 4, IsBlocking = 5,
        IsWalkable = 6, IsStackable = 7, IsContainer = 8, IsFluidContainer = 9,
        IsReadable = 10, IsWriteable = 11, IsChargeable = 12, IsAlwaysOnTop = 13,
        IsAlwaysOnBottom = 14, IsMagicField = 15, IsNotMoveable = 16,
        IsPickupable = 17, IsHangable = 18, IsRoteable = 19, // IsRotatable is more common name
        IsRotatable = 19, IsHasCharges = 20, IsBorder = 21, IsOptionalBorder = 22,
        IsHouse = 23, IsHouseExit = 24, IsTownExit = 25, IsWaypoint = 26,
        IsFlag = 27, IsStairs = 28, IsLadder = 29, IsDoodad = 30
    };
#endif

MapTileItem::MapTileItem(Tile* tileData, Map* mapModel, int activeLayer, QObject* parent) :
    QGraphicsRectItem(0, 0, TilePixelSize, TilePixelSize, qgraphicsitem_cast<QGraphicsItem*>(parent)),
    currentTileData(tileData),
    mapModel(mapModel),
    activeLayer(activeLayer),
    isTileSelected(false),
    showCollisions(false),
    cacheNeedsUpdate(true)
{
    setAcceptHoverEvents(true);
    setCacheMode(QGraphicsItem::DeviceCoordinateCache);
}

MapTileItem::~MapTileItem() {
    // currentTileData and mapModel are not owned.
}

QRectF MapTileItem::boundingRect() const { return QRectF(0, 0, TilePixelSize, TilePixelSize); }
QPainterPath MapTileItem::shape() const {
    QPainterPath path;
    path.addRect(boundingRect());
    return path;
}

void MapTileItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option); Q_UNUSED(widget);
    if (cacheNeedsUpdate || cachedPixmap.isNull()) {
        updateCache();
        cacheNeedsUpdate = false;
    }
    if (!cachedPixmap.isNull()) {
        painter->drawPixmap(0, 0, cachedPixmap);
    } else {
        painter->fillRect(boundingRect(), Qt::darkGray);
    }
    if (isTileSelected) {
        painter->setPen(QPen(QColor(255, 255, 0), 2));
        painter->setBrush(QColor(255, 255, 0, 80));
        painter->drawRect(boundingRect());
    }
    if (showCollisions && currentTileData && currentTileData->isBlocking()) {
        painter->setPen(QPen(Qt::red, 2));
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(boundingRect());
    }
}

void MapTileItem::setTile(Tile* tileData) { currentTileData = tileData; cacheNeedsUpdate = true; update(); }
void MapTileItem::setActiveLayer(int layer) {
    if (activeLayer != layer) { activeLayer = layer; cacheNeedsUpdate = true; update(); }
}
void MapTileItem::setSelected(bool selected) {
    if (isTileSelected != selected) { isTileSelected = selected; update(); }
}
void MapTileItem::setShowCollisions(bool show) {
    if (showCollisions != show) { showCollisions = show; update(); }
}

void MapTileItem::updateCache() {
    cachedPixmap = QPixmap(TilePixelSize, TilePixelSize);
    cachedPixmap.fill(Qt::transparent);

    QPainter painter(&cachedPixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);

    if (!currentTileData || !mapModel) {
        painter->fillRect(boundingRect(), Qt::darkGray);
        painter->end();
        return;
    }
    
    // Draw contents of the tile, respecting layer visibility set in mapModel.
    // Order matters for Z-stacking from bottom (Ground) to top (Topmost).
    
    // Pass 1: Draw Ground Layer always if visible.
    Layer* groundLayer = mapModel->getLayer(Layer::Type::Ground);
    if (groundLayer && groundLayer->isVisible()) {
        Tile* groundTile = mapModel->getTile(currentTileData->getX(), currentTileData->getY(), static_cast<Layer::Type>(Layer::Type::Ground));
        if (groundTile) {
            // Tile::draw handles drawing `Item` and `Creature` onto painter for its local view.
            groundTile->draw(painter, QPointF(0, 0), 1.0, false); 
        }
    }

    // Pass 2: Draw other layers from bottom to top based on Item properties and layer visibility.
    for (int z = 1; z < Map::LayerCount; ++z) { 
        Layer::Type currentDrawLayerType = static_cast<Layer::Type>(z);
        Layer* layerObj = mapModel->getLayer(currentDrawLayerType);

        if (!layerObj || !layerObj->isVisible()) { continue; } // Skip hidden layers.
        
        Tile* tileDataAtLayer = mapModel->getTile(currentTileData->getX(), currentTileData->getY(), z);
        if (tileDataAtLayer && !tileDataAtLayer->isEmpty()) { 
             tileDataAtLayer->draw(painter, QPointF(0, 0), 1.0, false);
        }
    }
    
    painter.end();
}

// MapItemItem Implementation (retains as in previous delivery)
MapItemItem::MapItemItem(Item* item, QGraphicsItem* parent) : QGraphicsPixmapItem(parent), currentItem(item) {
    if (currentItem) setPixmap(ItemManager::getInstance().getSprite(currentItem->getId()));
    setZValue(50);
}
void MapItemItem::setItem(Item* item) { currentItem = item; if (currentItem) setPixmap(ItemManager::getInstance().getSprite(currentItem->getId())); else setPixmap(QPixmap()); update(); }
void MapItemItem::setSelected(bool selected) { Q_UNUSED(selected); }

// MapCreatureItem Implementation (retains as in previous delivery)
MapCreatureItem::MapCreatureItem(Creature* creature, QGraphicsItem* parent) : QGraphicsPixmapItem(parent), currentCreature(creature) {
    if (currentCreature) setPixmap(currentCreature->getSprite());
    setZValue(75);
}
void MapCreatureItem::setCreature(Creature* creature) { currentCreature = creature; if (currentCreature) setPixmap(currentCreature->getSprite()); else setPixmap(QPixmap()); update(); }
void MapCreatureItem::setSelected(bool selected) { Q_UNUSED(selected); }

// MapSelectionItem Implementation (retains as in previous delivery)
MapSelectionItem::MapSelectionItem(const QRect& rect, QGraphicsItem* parent) : QGraphicsRectItem(rect.x() * MapTileItem::TilePixelSize, rect.y() * MapTileItem::TilePixelSize, rect.width() * MapTileItem::TilePixelSize, rect.height() * MapTileItem::TilePixelSize, parent), currentRect(rect) {
    setZValue(1000);
    setPen(QPen(QColor(255, 255, 0), 2));
    setBrush(QColor(255, 255, 0, 50));
    setOpacity(0.8);
    setVisible(false);
}
void MapSelectionItem::setRect(const QRect& rect) { currentRect = rect; QGraphicsRectItem::setRect(QRectF(rect.x() * MapTileItem::TilePixelSize, rect.y() * MapTileItem::TilePixelSize, rect.width() * MapTileItem::TilePixelSize, rect.height() * MapTileItem::TilePixelSize)); update(); }
void MapSelectionItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    Q_UNUSED(option); Q_UNUSED(widget);
    painter->setPen(pen());
    painter->setBrush(brush());
    painter->drawRect(rect());
}