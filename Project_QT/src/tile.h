#ifndef TILE_H
#define TILE_H

#include <QObject>
#include <QPoint>
#include <QColor>
#include <QVector> // For dynamic arrays of items/creatures
#include <QSet> // For zones

// Forward declarations to avoid circular includes and large headers
class Item; // Defined in item.h
class Creature; // Defined in creature.h
class Map; // Defined in map.h
class GroundBrush; // Defined in normalbrush.h or new groundbrush.h
class WallBrush; // Defined in new wallbrush.h
class House; // Defined in house.h
// Custom list type for House Exits - derived from Source/house.h
typedef std::vector<uint32_t> HouseExitList;

// Make sure Layer::Type is visible (defined in layer.h)
#include "layer.h"
#include "item.h" // Needed here for ItemProperty enum

/**
 * @brief The Tile class represents a single tile on the map.
 * It holds items, creatures, and other properties.
 */
class Tile : public QObject
{
    Q_OBJECT
public:
    // TILESTATE flags directly from Source/src/tile.h
    enum TileStateFlag {
        TILESTATE_NONE = 0x0000,
        TILESTATE_MODIFIED = 0x0001,
        TILESTATE_DEPRECATED = 0x0002, // Reserved
        TILESTATE_SELECTED = 0x0004,
        TILESTATE_OP_BORDER = 0x0008, // If this is true, gravel will be placed on the tile!
        // ... more flags from original tile.h will be added as features are migrated
    };

    explicit Tile(const QPoint& position, QObject* parent = nullptr);
    Tile(int x, int y, int z, QObject* parent = nullptr);
    // If Tile(TileLocation& loc) is still used from original, adapt to Qt's position/struct
    // Tile(TileLocation& loc, QObject* parent = nullptr);

    // Position of the tile on the map
    QPoint getPosition() const { return position; }
    void setPosition(const QPoint& pos) { position = pos; }
    int getX() const { return position.x(); }
    int getY() const { return position.y(); }
    int getZ() const { return position.z(); } // Z usually refers to layer in this context

    // Color of the tile (used for rendering background or properties)
    QColor getColor() const { return color; }
    void setColor(const QColor& newColor);

    // Adding/removing/getting items and creatures from this specific tile
    // Ownership is managed by the Map class or specific item/creature managers
    bool addItem(Item* item, Layer::Type layer); // Takes ownership or shares ptr? Map will decide.
    bool removeItem(Item* item, Layer::Type layer);
    QVector<Item*> getItemsByLayer(Layer::Type layer) const; // Returns pointers, not copies, based on Item properties/flags

    bool hasItem(Item* item, Layer::Type layer) const;
    void clearLayer(Layer::Type layer);

    // Creature management
    void addCreature(Creature* creature); // Takes ownership if it makes sense here
    void removeCreature(Creature* creature);
    QVector<Creature*> getCreatures() const; // Returns pointers to creatures on this tile
    void clearCreatures();

    // Raw item management (current items vector in Project_QT, from Source Tile)
    void addItem(const Item& item); // Adds a COPY of the item for local tile storage (as in original)
    void removeItem(const Item& item);
    void clearItems();
    const QVector<Item>& getItems() const { return items; } // Returns a const ref to the internal list of items

    // Drawing the tile content for a QPainter (replacing SFML drawing logic)
    // It's the tile's responsibility to draw all items and creatures that belong to it.
    void draw(QPainter& painter, const QPointF& offset, double zoom, bool showCollisions = false) const;

    // Tile properties and flags (from Source/src/tile.h, adapted)
    bool isModified() const { return (tileState & TILESTATE_MODIFIED) != 0; }
    void modify() { tileState |= TILESTATE_MODIFIED; }
    void unmodify() { tileState &= ~TILESTATE_MODIFIED; }

    bool isEmpty() const { return items.isEmpty() && creatures.isEmpty(); }
    int size() const { return items.size() + creatures.size(); }

    // Collision & Walkability (from Source/src/tile.h, consolidated)
    bool isBlocking() const;
    bool isWalkable() const;
    bool getCollision() const { return hasCollisionValue; } // From source, separate from Item::isBlocking()
    void setCollision(bool hasCollision);

    // Selection status
    void select() { tileState |= TILESTATE_SELECTED; }
    void deselect() { tileState &= ~TILESTATE_SELECTED; }
    bool isSelected() const { return (tileState & TILESTATE_SELECTED) != 0; }
    void selectGround(); // Original has distinct method for ground selection
    void deselectGround();

    // Unique Items and Specific Content (from Source/src/tile.h)
    bool hasUniqueItem() const; // Check if the tile contains an item with a unique ID
    QVector<Item> popSelectedItems(bool ignoreTileSelected = false);
    QVector<Item> getSelectedItems(bool unzoomed = false);
    Item* getTopSelectedItem(); // Get topmost selected item

    // MiniMap color (from Source/src/tile.h)
    uint8_t getMiniMapColor() const;

    // Ground & Borders (from Source/src/tile.h & ground_brush.h, consolidated)
    bool hasGround() const;
    bool hasBorders() const;
    GroundBrush* getGroundBrush() const; // Get brush that provides the ground item

    // Automagic border & wall handling methods
    void cleanBorders();
    void addBorderItem(Item* item);
    void borderize(Map* mapInstance); // Map instance is needed to get neighbor tiles

    // Wall related
    bool hasWall() const;
    Item* getWall() const;
    void cleanWalls(WallBrush* wb = nullptr);
    void cleanWalls(bool dontdelete);
    void addWallItem(Item* item);
    void wallize(Map* mapInstance);

    // Tables & Carpets
    bool hasTable() const;
    Item* getTable() const;
    bool hasCarpet() const;
    Item* getCarpet() const;
    void cleanTables(bool dontdelete);
    void tableize(Map* mapInstance);
    void carpetize(Map* mapInstance);

    // Optional Border (e.g. gravel around mountains from Source/src/tile.h)
    bool hasOptionalBorder() const;
    void setOptionalBorder(bool b);

    // House related (from Source/src/house.h)
    bool isHouseTile() const;
    uint32_t getHouseID() const;
    void setHouseID(uint32_t newHouseId);
    void addHouseExit(House* h);
    void removeHouseExit(House* h);
    bool isHouseExit() const;
    bool isTownExit(Map& map) const;
    const HouseExitList* getHouseExits() const;
    HouseExitList* getHouseExits();
    bool hasHouseExit(uint32_t exit) const;
    void setHouse(House* house);
    House* getHouse() const;

    // Mapflags (PZ, PVPZONE etc. from Source/src/tile.h)
    void addZoneId(uint16_t _zoneId);
    void removeZoneId(uint16_t _zoneId);
    void clearZoneId();
    void setZoneIds(Tile* tile);
    const std::vector<uint16_t>& getZoneIds() const;
    uint16_t getZoneId() const; // Get primary Zone ID if any

    void setMapFlags(uint16_t _flags);
    void unsetMapFlags(uint16_t _flags);
    uint16_t getMapFlags() const;

    void setStatFlags(uint16_t _flags);
    void unsetStatFlags(uint16_t _flags);
    uint16_t getStatFlags() const;

    // Derived properties related to specific items on the tile
    bool isBlockingCreature() const;
    bool isStairs() const;
    bool isLadder() const;

    // Added from property editor usage
    int getMovementCost() const;
    void setMovementCost(int cost);
    bool isProtectionZone() const;
    void setProtectionZone(bool isPZ);
    bool isNoLogout() const;
    void setNoLogout(bool noLogout);
    bool isNoMoveItems() const;
    void setNoMoveItems(bool noMove);
    bool isNoMoveCreatures() const;
    void setNoMoveCreatures(bool noMove);
    bool isNoSpawn() const;
    void setNoSpawn(bool noSpawn);
    bool hasCollision() const; // If different from getCollision or for specific use by editor

signals:
    // Signals for UI/MapScene updates when content changes
    void creatureAdded(Creature* creature, const QPoint& position);
    void creatureRemoved(Creature* creature, const QPoint& position);
    void creaturesCleared(const QPoint& position);
    void itemsChanged(); // Generic signal for any item change on tile
    void colorChanged(const QColor& color); // If tile's base color changes
    void changed(); // General tile changed signal

private:
    QPoint position;
    QColor color;
    uint16_t tileState; // Bitfield for TILESTATE_ flags
    QVector<Item> items; // Local copy of items on this tile
    QVector<Creature*> creatures; // Pointers to creatures on this tile (ownership often outside Tile)
    uint32_t house_id;
    bool hasCollisionValue; // Consolidating collision determination
    HouseExitList* houseExits; // List of house exits (from Source/src/house.h)
    std::vector<uint16_t> zoneIds; // Zone IDs on this tile (from Source/src/tile.h)
    uint16_t mapFlags;
    uint16_t statFlags;

    // Helper for `Tile::draw` that determines Item's layer for drawing order based on `ItemProperty`
    // This is typically from `ItemType` information, e.g., `g_items.getItemType(item.getId()).isGroundTile()`.
    // It is `ItemProperty::IsGroundTile` in Item.
    Layer::Type getItemRenderLayer(const Item& item) const;

    // ItemProperty to check Item's internal properties (forwarded to Item methods)
    bool hasPropertyInternal(enum ItemProperty prop) const; // Helper, avoids direct includes if not needed

};

// Comparison operators (from Source/src/tile.h)
bool tilePositionLessThan(const Tile* a, const Tile* b);
bool tilePositionVisualLessThan(const Tile* a, const Tile* b);

#endif // TILE_H