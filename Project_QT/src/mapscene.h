#ifndef MAPSCENE_H
#define MAPSCENE_H

#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QPainter>
#include <QSet>
#include <QMap>
#include <QCache>
#include <QRectF>

#include "map.h"
#include "tile.h"
#include "item.h"
#include "creature.h"
#include "spritemanager.h"

class MapTileItem;
class MapSelectionItem;


class MapScene : public QGraphicsScene
{
    Q_OBJECT
public:
    explicit MapScene(QObject* parent = nullptr);
    virtual ~MapScene();

    void setMap(Map* map);
    Map* getMap() const { return currentMap; }

    void updateTile(const QPoint& position);
    void updateVisibleTiles(const QRectF& visibleRect);

    void setCurrentLayer(int layer);
    int getCurrentLayer() const { return currentLayer; }

    void setShowGrid(bool show);
    bool getShowGrid() const { return showGrid; }

    void setShowCollisions(bool show);
    bool getShowCollisions() const { return showCollisions; }

    void clearSelection();
    void selectTile(const QPoint& position);
    void selectTiles(const QRect& rect);

    void createGridLines(const QRectF& rect);
    void updateGridLines();

protected:
    // Override drawBackground to draw a consistent base under all items
    void drawBackground(QPainter* painter, const QRectF& rect) override;


private:
    Map* currentMap;
    int currentLayer;
    bool showGrid;
    bool showCollisions;

    QMap<QPoint, MapTileItem*> tileItems; 
    QSet<QPoint> visibleTilePositions;

    QMap<qreal, QGraphicsLineItem*> gridLines;
    MapSelectionItem* selectionRectItem;

    void createTileItem(const QPoint& position);
    void removeTileItem(const QPoint& position);
    void clearAllTileItems();
};

// MapTileItem, MapItemItem, MapCreatureItem, MapSelectionItem definitions as in previous commits
// ... (omitted for brevity, assume content from previous full file is here) ...
// (The actual content below is from the prior mapscene.h post.)
class MapTileItem : public QGraphicsRectItem
{
public:
    enum { Type = QGraphicsItem::UserType + 1 };
    explicit MapTileItem(Tile* tileData, Map* mapModel, int activeLayer, QObject* parent = nullptr);
    virtual ~MapTileItem();
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;
    QPainterPath shape() const override;
    void setTile(Tile* tileData);
    Tile* getTile() const { return currentTileData; }
    void setActiveLayer(int layer);
    int getActiveLayer() const { return activeLayer; }
    void setSelected(bool selected);
    bool isSelected() const { return isTileSelected; }
    void setShowCollisions(bool show);
    bool getShowCollisions() const { return showCollisions; }
    void updateCache();
    int type() const override { return Type; }
    static const int TilePixelSize = 32;

private:
    Tile* currentTileData;
    Map* mapModel;
    int activeLayer;
    bool isTileSelected;
    bool showCollisions;
    QPixmap cachedPixmap;
    bool cacheNeedsUpdate;
    void drawTileContentToCache(QPainter& painter);
};

class MapItemItem : public QGraphicsPixmapItem
{
public:
    enum { Type = QGraphicsItem::UserType + 2 };
    explicit MapItemItem(Item* item, QGraphicsItem* parent = nullptr);
    void setItem(Item* item);
    Item* getItem() const { return currentItem; }
    void setSelected(bool selected);

private:
    Item* currentItem;
};

class MapCreatureItem : public QGraphicsPixmapItem
{
public:
    enum { Type = QGraphicsItem::UserType + 3 };
    explicit MapCreatureItem(Creature* creature, QGraphicsItem* parent = nullptr);
    void setCreature(Creature* creature);
    Creature* getCreature() const { return currentCreature; }
    void setSelected(bool selected);

private:
    Creature* currentCreature;
};

class MapSelectionItem : public QGraphicsRectItem
{
public:
    enum { Type = QGraphicsItem::UserType + 4 };
    explicit MapSelectionItem(const QRect& rect = QRect(), QGraphicsItem* parent = nullptr);
    void setRect(const QRect& rect);
    QRect getRect() const { return currentRect; }
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) override;

private:
    QRect currentRect;
};