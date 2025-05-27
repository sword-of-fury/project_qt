#include "tile.h"
#include "map.h" // Needed for Map::getLayer in Tile::draw for visibility, and Map::LayerCount for iteration
#include "itemmanager.h" // For getting item properties (ItemType, draw offsets etc) and sprites
#include "spritemanager.h" // For raw sprite drawing (GameSprite, etc) and creature sprites.
#include "creaturemanager.h" // For creature details
#include <QDebug> // For qDebug output

// Constructor with QPoint
Tile::Tile(const QPoint& position, QObject* parent)
    : QObject(parent),
      position(position),
      color(Qt::darkGray), // Default background color
      tileState(TILESTATE_NONE),
      mapFlags(TILE_FLAG_NONE), // Initialize with NONE
      statFlags(0), // Initialize statFlags
      house_id(0),
      houseExits(nullptr), // Initialize houseExits
      hasCollisionValue(false)
{
    // Items and creatures vectors are default-constructed empty
    // qDebug() << "Tile created at" << position;
}

// Constructor with int x, y, z (position)
Tile::Tile(int x, int y, int z, QObject* parent)
    : Tile(QPoint(x, y), parent) // Delegate to QPoint constructor
{
    // mapFlags, statFlags, houseExits are initialized by the delegated constructor
    position.setZ(z); // Set the Z/layer
    // qDebug() << "Tile created at" << position.x() << "," << position.y() << "," << position.z();
}

// Destructor to clean up houseExits
Tile::~Tile() {
    delete houseExits;
    houseExits = nullptr;
}

// Tile(TileLocation& loc, QObject* parent) from Source, adaptation will need proper Qt struct equivalent for TileLocation
/*
Tile::Tile(TileLocation& loc, QObject* parent)
    : Tile(loc.getX(), loc.getY(), loc.getZ(), parent)
{
    // Transfer data from TileLocation, if it holds relevant data for `Tile` class properties.
    // e.g., if TileLocation holds map flags or other properties directly.
    // this->setMapFlags(loc.getMapFlags());
}
*/

void Tile::setColor(const QColor& newColor)
{
    if (color != newColor) {
        color = newColor;
        emit colorChanged(color);
        emit changed();
    }
}

// Add an item to this tile at a specific layer.
bool Tile::addItem(Item* item, Layer::Type layer)
{
    if (!item) return false;

    // Based on original RME's Item stacking and layer logic, it's complex.
    // Here we're using a single `items` vector in `Tile` (as in Source/tile.h)
    // and assuming its draw order or position in vector reflects Z-order on tile.

    // A simpler approach for the Qt port in Tile: add the item directly,
    // and let `Tile::draw()` handle complex z-ordering for rendering,
    // which then needs to know the true `Layer::Type` of each item for sorting/drawing.
    // This assumes Item knows its own layer properties (e.g. IsGroundTile, IsOnTop).

    // For now, assume this function adds to the correct internal representation that `draw()` can interpret.
    items.append(*item); // Appends a copy (as `items` is `QVector<Item>`)
    emit itemsChanged();
    emit changed();
    return true;
}

bool Tile::removeItem(Item* item, Layer::Type layer)
{
    if (!item) return false;
    bool removed = false;
    for (int i = 0; i < items.size(); ++i) {
        if (items.at(i).getId() == item->getId()) { // Simple ID match for now
            items.removeAt(i);
            removed = true;
            break;
        }
    }
    if (removed) {
        emit itemsChanged();
        emit changed();
    }
    return removed;
}

// Gets all items belonging to a specific layer. (from Source/src/tile.h)
QVector<Item*> Tile::getItemsByLayer(Layer::Type layer) const
{
    QVector<Item*> layerItems;
    for (const Item& item : items) {
        // Implement logic to determine if an Item belongs to a specific Layer::Type.
        // This is based on Item's properties (IsGroundTile, IsWall, etc.) and `ItemProperty` enum.
        if (getItemRenderLayer(item) == layer) {
            layerItems.append(const_cast<Item*>(&item));
        }
    }
    return layerItems;
}

bool Tile::hasItem(Item* item, Layer::Type layer) const
{
    Q_UNUSED(layer); // Layer argument may be for checking specific item instances
    for (const Item& existingItem : items) {
        if (existingItem.getId() == item->getId()) { // Simple ID match for now
            return true;
        }
    }
    return false;
}

void Tile::clearLayer(Layer::Type layer)
{
    // Clears all items belonging to a specific layer. (from Source/src/tile.h)
    // Needs to iterate `items` and filter by layer type.
    QVector<Item> remainingItems;
    for (const Item& item : items) {
        if (getItemRenderLayer(item) != layer) {
            remainingItems.append(item);
        }
    }
    if (remainingItems.size() != items.size()) { // Only if items were removed
        items = remainingItems;
        emit itemsChanged();
        emit changed();
    }
}

// Creature management methods
void Tile::addCreature(Creature* creature)
{
    if (creature) {
        creatures.append(creature); // Append pointer (ownership model to be clarified elsewhere)
        emit creatureAdded(creature, position);
        emit changed();
    }
}

void Tile::removeCreature(Creature* creature)
{
    if (creature) {
        creatures.removeOne(creature);
        emit creatureRemoved(creature, position);
        emit changed();
    }
}

QVector<Creature*> Tile::getCreatures() const
{
    return creatures;
}

void Tile::clearCreatures()
{
    if (!creatures.isEmpty()) {
        // Assume creature pointers are managed elsewhere (e.g., CreatureManager).
        // If Tile had ownership, we would delete here.
        creatures.clear();
        emit creaturesCleared(position);
        emit changed();
    }
}

void Tile::addItem(const Item& item)
{
    // Adds a COPY of the item for local tile storage (as in original's `items` vector).
    // Note: The original RME's `addItem` in `tile.cpp` has complex logic for ground items and Z-ordering.
    // We will place all items directly into `items` vector. Layer logic for Z-ordering handled in `draw`.
    items.append(item);
    emit itemsChanged();
    emit changed();
}

void Tile::removeItem(const Item& item)
{
    // Removes an item matching the provided `item` (by ID/properties) from the internal list.
    bool removed = false;
    for (int i = 0; i < items.size(); ++i) {
        if (items.at(i).getId() == item.getId()) { // Simple ID match, refine as needed for complex items.
            items.removeAt(i);
            removed = true;
            break;
        }
    }
    if (removed) {
        emit itemsChanged();
        emit changed();
    }
}

void Tile::clearItems()
{
    if (!items.isEmpty()) {
        items.clear();
        emit itemsChanged();
        emit changed();
    }
}

void Tile::draw(QPainter& painter, const QPointF& offset, double zoom, bool showCollisions) const
{
    // This function acts as the composite renderer for the tile,
    // drawing all items and creatures that belong to it, respecting layer visibility.
    // This is the direct migration of drawing logic from `Source/src/map_drawer.cpp::DrawTile()`.

    // painter coordinates are local to this tile (0,0) to (TilePixelSize, TilePixelSize).
    // offset parameter could be used if painting onto a larger canvas relative to a start.
    // For `MapTileItem::paint` calling, `offset` will typically be (0,0).
    Q_UNUSED(offset); 
    Q_UNUSED(zoom); // Zoom already applied by QGraphicsView or MapTileItem's pixmap.
    Q_UNUSED(showCollisions); // Collision handled by MapTileItem::paint.

    // Get a pointer to the main Map object from the `Map` singleton or by passing to `MapTileItem`.
    // It's already passed via MapTileItem, `Map::getInstance()` is simpler here.
    Map* mapInstance = &Map::getInstance();
    if (!mapInstance) return;

    // Store the order in which items should be drawn based on their layer properties.
    // The original `MapDrawer::DrawTile` drew from layer 0 up to 15.
    // We recreate that here, ensuring visibility check.
    // `items` and `creatures` store items from potentially ALL layers on this specific tile.

    // A list of items to draw in correct Z-order for this tile.
    QVector<const Item*> itemsToDraw;
    QVector<const Creature*> creaturesToDraw; // Creatures usually above items.

    // Categorize items by their visual layer, considering visibility from `Map` singleton.
    for (int z = 0; z < Map::LayerCount; ++z) { // Iterate all possible layers (0-15).
        Layer::Type currentLayerType = static_cast<Layer::Type>(z);
        Layer* layerObj = mapInstance->getLayer(currentLayerType);

        if (!layerObj || !layerObj->isVisible()) { // If layer is not visible, skip drawing its content.
            continue;
        }
        
        // Find items that logically belong to this Z-layer on the current tile.
        // ItemType (from ItemManager) would define the true layer (e.g., ground item, wall, effect).
        // This is where `Tile::getItemsByLayer(currentLayerType)` becomes crucial, or iterate all and filter.
        for (const Item& item : items) { // This `items` is the flat list of all items on this specific X,Y tile.
            // Check if item's `actual` drawing layer matches `currentLayerType`.
            // Use `getItemRenderLayer` which maps ItemProperties to a conceptual layer type.
            if (getItemRenderLayer(item) == currentLayerType) {
                 itemsToDraw.append(&item);
            }
        }
        // Assuming creatures also have a 'layer' type for rendering order (usually always high).
        // Or handle them after all items are drawn. For now, place creatures here or as a final pass.
        // Original Tibia rendering generally drew ground first, then items (bottom to top), then creatures.
    }
    // Now, iterate `itemsToDraw` in desired order and `creatures`.
    // The actual drawing call for each item (Item::draw) will perform the blitting.

    QVector<const Item*> groundItems;
    QVector<const Item*> normalItems;
    QVector<const Item*> topItems;

    for (const Item& item : this->items) {
        if (item.isGroundTile()) {
            groundItems.append(&item);
        } else if (item.isAlwaysOnTop()) {
            topItems.append(&item);
        } else {
            normalItems.append(&item);
        }
    }

    // Perform Drawing Passes:
    // 1. Ground Items
    // These items conceptually belong to the Ground layer or similar bottom-most visible layer.
    // The getItemRenderLayer can help determine which specific Layer::Type they map to for visibility.
    if (mapInstance->getLayer(Layer::Type::Ground)->isVisible()) { // Check general Ground layer visibility
        for (const Item* item : groundItems) {
            // Further check if this specific item's conceptual layer is visible, if needed.
            // For ground items, Layer::Type::Ground is the primary check.
            item->draw(painter, QPoint(0,0), 1.0);
        }
    }
    
    // 2. Normal Items
    // These items typically belong to Objects, Items, Walls, Effects layers.
    // The order within normalItems is preserved from this->items for now.
    // A sort by a 'topOrder' property would happen here if implemented.
    for (const Item* item : normalItems) {
        Layer::Type itemLayerType = getItemRenderLayer(*item); // Determine the item's conceptual layer
        Layer* itemActualLayer = mapInstance->getLayer(itemLayerType);
        if (itemActualLayer && itemActualLayer->isVisible()) {
            item->draw(painter, QPoint(0,0), 1.0);
        }
    }

    // 3. Top Items
    // These items are drawn above normal items.
    // They might belong to various conceptual layers (e.g. Effects, Top)
    for (const Item* item : topItems) {
        Layer::Type itemLayerType = getItemRenderLayer(*item);
        Layer* itemActualLayer = mapInstance->getLayer(itemLayerType);
        if (itemActualLayer && itemActualLayer->isVisible()) {
             item->draw(painter, QPoint(0,0), 1.0);
        }
    }
    
    // 4. Draw Creatures
    Layer* creaturesLayer = mapInstance->getLayer(Layer::Type::Creatures);
    if (creaturesLayer && creaturesLayer->isVisible() && !creatures.isEmpty()) {
        for (const Creature* creature : this->creatures) {
            if (creature) { // Ensure pointer is valid.
                // Creature::draw(painter, ...) would be ideal if creatures have complex drawing.
                // For now, just drawing their sprite:
                QPixmap creatureSprite = creature->getSprite(); 
                if (!creatureSprite.isNull()) {
                    // Assuming creature sprites are drawn at tile origin (0,0) like items.
                    // Offsets would be handled inside creature->getSprite() or a creature->draw() method.
                    painter.drawPixmap(0, 0, creatureSprite); 
                }
            }
        }
    }

    // Note: Collision indicators (`showCollisions`) are handled by MapTileItem::paint.
}

Layer::Type Tile::getItemRenderLayer(const Item& item) const {
    // This is a crucial helper for Z-ordering in `Tile::draw` (MapTileItem::updateCache).
    // It maps Item properties from Source/src/item.h to a conceptual `Layer::Type`.
    // It needs to be kept in sync with the `MapDrawer::DrawTile` logic (specifically item filtering logic by draw layer).
    // ItemProperty values correspond to flags/attributes within Item.
    if (item.isGroundTile()) return Layer::Type::Ground; // Updated to use direct method
    // Assuming ItemProperty enum is still used for other properties if direct methods aren't available for all.
    if (item.hasProperty(ItemProperty::IsGroundDetail)) return Layer::Type::GroundDetail; 
    if (item.hasProperty(ItemProperty::IsWall) || item.hasProperty(ItemProperty::IsDoor)) return Layer::Type::Walls;
    if (item.hasProperty(ItemProperty::IsEffect)) return Layer::Type::Effects; 
    if (item.isAlwaysOnTop()) return Layer::Type::Top; // Updated to use direct method

    // Default for items that are not specifically ground, wall, effect, or always on top.
    // These would typically be general objects, containers, stackable items, etc.
    if (item.hasProperty(ItemProperty::IsContainer) || item.hasProperty(ItemProperty::IsStackable)) return Layer::Type::Objects; 
    
    // Fallback layer type for items not matching above categories.
    // Layer::Type::Items could be a generic layer for miscellaneous items.
    return Layer::Type::Items; 
}


// --- Tile Properties Implementation ---
// Most of these are directly from Source/src/tile.cpp

void Tile::setCollision(bool hasCollision) {
    if (hasCollisionValue != hasCollision) {
        hasCollisionValue = hasCollision;
        emit changed(); // Or a specific collisionChanged signal
    }
}

bool Tile::isBlocking() const {
    // A tile is blocking if any item on it is blocking, or if its explicit collision flag is set.
    if (hasCollisionValue) return true; // Direct tile flag.
    for (const Item& item : items) {
        if (item.isBlocking()) return true; // Item-based blocking.
    }
    // Also consider creatures here if they block movement.
    // For now, based on Source, creatures do not necessarily block unless they're specific items.
    return false;
}

bool Tile::isWalkable() const {
    // A tile is walkable if no item on it makes it un-walkable, and no collision flag.
    // This is inverted logic: `isWalkable = !hasCollision && !anyItemIsBlockingMovement`.
    if (hasCollisionValue) return false; // Explicit collision.
    for (const Item& item : items) {
        if (!item.isWalkable()) return false; // If any item is not walkable.
    }
    return true;
}

void Tile::selectGround() { /* Original has specific select/deselect ground, keep here for signature matching. */ select(); }
void Tile::deselectGround() { /* Original has specific select/deselect ground, keep here for signature matching. */ deselect(); }
bool Tile::hasUniqueItem() const {
    // Check if any item on the tile has a unique ID (actionId, uniqueId, text).
    for (const Item& item : items) {
        if (item.getUniqueID() != 0 || item.getActionID() != 0 || !item.getText().isEmpty()) {
            return true;
        }
    }
    return false;
}
QVector<Item> Tile::popSelectedItems(bool ignoreTileSelected) {
    QVector<Item> popped;
    if (isTileSelected || ignoreTileSelected) { // Check if tile is selected or forced
        // Find selected items in `items` list and move to popped.
        QVector<Item> remaining;
        for (const Item& item : items) {
            // Assumed item has an isSelected() state; otherwise, means ALL items on a selected tile.
            // If the map has specific selected items on the tile, pick those.
            // For now, if the tile is selected, pop all items for this simplified view.
            popped.append(item);
        }
        items.clear();
        emit itemsChanged(); emit changed();
    }
    return popped;
}
QVector<Item> Tile::getSelectedItems(bool unzoomed) {
    QVector<Item> selectedItems;
    Q_UNUSED(unzoomed); // For potential scaling.
    if (isTileSelected) {
        selectedItems = items; // If tile is selected, all its items are selected.
    }
    return selectedItems;
}
Item* Tile::getTopSelectedItem() {
    if (!items.isEmpty() && isTileSelected) { // Assume topmost if tile is selected.
        return &items.last(); // Qt QVector last item (last drawn = topmost).
    }
    return nullptr;
}
uint8_t Tile::getMiniMapColor() const {
    // Get minimap color. Prioritize non-transparent items (e.g. ground).
    // Original Tibia rendering chooses topmost visible opaque item for minimap color.
    // Requires ItemManager::getMiniMapColor for the item ID.
    // Iterate in reverse to get topmost item first.
    for (int i = items.size() - 1; i >= 0; --i) {
        const Item& item = items.at(i);
        // TODO: Need to check if item is visible (e.g. not an effect that shouldn't be on minimap, or based on layer visibility)
        // This check might be too simple if ItemManager/SpriteManager isn't fully robust yet.
        if (ItemManager::getInstance().hasSprite(item.getId())) { 
            // Assuming getSpriteManager() is a valid way to access SpriteManager from ItemManager
            // This part needs to align with how ItemManager and SpriteManager are structured.
            // If SpriteManager is a direct singleton: SpriteManager::getInstance().getMiniMapColor(item.getId());
            // For now, let's assume ItemManager provides a way or SpriteManager is accessible.
            // This is a placeholder call, actual structure of SpriteManager access might differ.
            // uint8_t color = ItemManager::getInstance().getSpriteManager().getMiniMapColor(item.getId());

            // Placeholder: Using a dummy call to SpriteManager if available, or just a default
            // This requires SpriteManager instance and its method. For now, using a simplified check.
            const SpriteManager& sm = ItemManager::getInstance().getSpriteManager(); // Assuming this getter exists
            uint8_t color = sm.getMiniMapColor(item.getId());

            if (color != 0) return color; // If not fully transparent (0), return it.
        }
    }
    return 0; // Default transparent if nothing found.
}

bool Tile::hasGround() const {
    // Check if a ground item is present on the tile.
    for (const Item& item : items) {
        if (item.hasProperty(static_cast<ItemProperty>(ItemProperty::IsGroundTile))) {
            return true;
        }
    }
    return false;
}
bool Tile::hasBorders() const {
    // Check if the tile contains border items.
    for (const Item& item : items) {
        if (item.hasProperty(static_cast<ItemProperty>(ItemProperty::IsBorder))) {
            return true;
        }
    }
    return false;
}
GroundBrush* Tile::getGroundBrush() const {
    // Return a ground brush if tile has one. Need access to BrushManager / GroundBrush.
    return nullptr; // Placeholder, as specific brush types aren't fully migrated yet.
}

void Tile::cleanBorders() {
    // Clears all border items from the tile.
    QVector<Item> remainingItems;
    for (const Item& item : items) {
        if (!item.hasProperty(static_cast<ItemProperty>(ItemProperty::IsBorder))) {
            remainingItems.append(item);
        }
    }
    items = remainingItems;
    emit itemsChanged(); emit changed();
}

void Tile::addBorderItem(Item* item) {
    // Adds a border item. This typically places them at a specific z-order.
    // Original has `addItem` handling this `IsBorder` at the bottom (top order = 0).
    // `addItem` now appends to `items` vector, so for specific z-order, it needs sorting.
    addItem(*item);
}
void Tile::borderize(Map* mapInstance) {
    // Trigger the automagic border system. Requires BorderSystem class.
    if (mapInstance && mapInstance->getBorderSystem()) { // Access the Map's BorderSystem.
        mapInstance->getBorderSystem()->applyBorders(position, getZ());
    }
    // `mapInstance->getBorderSystem()->applyBorders` internally calls `updateTile`
}

bool Tile::hasWall() const {
    for (const Item& item : items) {
        if (item.hasProperty(static_cast<ItemProperty>(ItemProperty::IsWall))) return true;
    }
    return false;
}
Item* Tile::getWall() const {
    for (const Item& item : items) {
        if (item.hasProperty(static_cast<ItemProperty>(ItemProperty::IsWall))) return const_cast<Item*>(&item);
    }
    return nullptr;
}
void Tile::cleanWalls(WallBrush* wb) {
    // Clears all wall items.
    QVector<Item> remainingItems;
    for (const Item& item : items) {
        if (!item.hasProperty(static_cast<ItemProperty>(ItemProperty::IsWall))) {
            remainingItems.append(item);
        } else if (wb && item.hasProperty(static_cast<ItemProperty>(ItemProperty::IsWall))) { // Check against specific wall brush if provided.
            // Add specific logic here for matching `WallBrush` based on `wb` if item related to it.
            // For now, if `wb` exists, remove all wall items if matching its criteria.
            // Simplified: Assume `wb` filter will be implemented for `WallBrush::hasWall(Item* item)`.
            if (!wb->hasWall(const_cast<Item*>(&item))) { // If this wall item does NOT belong to the brush `wb`
                remainingItems.append(item); // Keep it.
            }
        }
    }
    items = remainingItems;
    emit itemsChanged(); emit changed();
}
void Tile::cleanWalls(bool dontdelete) { Q_UNUSED(dontdelete); cleanWalls(nullptr); } // Overload from Source/tile.cpp
void Tile::addWallItem(Item* item) { addItem(*item); } // Similar to addBorderItem.
void Tile::wallize(Map* mapInstance) { Q_UNUSED(mapInstance); /* Placeholder, triggers wall automagic. */ }
void Tile::hasTable() const { /* Impl */ }
void Tile::getTable() const { /* Impl */ }
void Tile::hasCarpet() const { /* Impl */ }
void Tile::getCarpet() const { /* Impl */ }
void Tile::cleanTables(bool dontdelete) { Q_UNUSED(dontdelete); /* Impl */ }
void Tile::tableize(Map* mapInstance) { Q_UNUSED(mapInstance); /* Stub */ }
void Tile::carpetize(Map* mapInstance) { Q_UNUSED(mapInstance); /* Stub */ }

bool Tile::hasTable() const {
    for (const Item& item : items) {
        if (item.hasProperty(ItemProperty::IsTable)) { // Assuming ItemProperty::IsTable exists
            return true;
        }
    }
    return false;
}

Item* Tile::getTable() const {
    for (const Item& item : items) {
        if (item.hasProperty(ItemProperty::IsTable)) {
            return const_cast<Item*>(&item); // Return pointer to item in vector
        }
    }
    return nullptr;
}

bool Tile::hasCarpet() const {
    for (const Item& item : items) {
        if (item.hasProperty(ItemProperty::IsCarpet)) { // Assuming ItemProperty::IsCarpet exists
            return true;
        }
    }
    return false;
}

Item* Tile::getCarpet() const {
    for (const Item& item : items) {
        if (item.hasProperty(ItemProperty::IsCarpet)) {
            return const_cast<Item*>(&item);
        }
    }
    return nullptr;
}


bool Tile::hasOptionalBorder() const {
    for (const Item& item : items) {
        if (item.hasProperty(static_cast<ItemProperty>(ItemProperty::IsOptionalBorder))) return true;
    }
    return false;
}
void Tile::setOptionalBorder(bool b) {
    if (b) {
        // Find existing optional border and if none, create one if brush available.
    } else {
        // Remove existing optional border.
        QVector<Item> remaining;
        for (const Item& item : items) {
            if (!item.hasProperty(static_cast<ItemProperty>(ItemProperty::IsOptionalBorder))) {
                remaining.append(item);
            }
        }
        items = remaining;
    }
    emit itemsChanged(); emit changed();
}


bool Tile::isHouseTile() const { /* Impl based on house_id */ return house_id != 0; }
uint32_t Tile::getHouseID() const { return house_id; }
void Tile::setHouseID(uint32_t newHouseId) {
    if (house_id != newHouseId) {
        house_id = newHouseId;
        emit changed();
    }
}
void Tile::addHouseExit(House* h) { /* Impl. Requires HouseExitList/House class. */ }
void Tile::removeHouseExit(House* h) { /* Impl. */ }
bool Tile::isHouseExit() const { return houseExits && !houseExits->empty(); }
bool Tile::isTownExit(Map& map) const { /* Impl. Needs Map.towns and Tile.hasProperty(IsTownExit)*/ return false; }
const HouseExitList* Tile::getHouseExits() const { return houseExits; }
HouseExitList* Tile::getHouseExits() { return houseExits; }
bool Tile::hasHouseExit(uint32_t exit) const { /* Impl. */ return false; }
void Tile::setHouse(House* house) { /* Impl. Link Tile to House object */ }
House* Tile::getHouse() const { /* Impl. */ return nullptr; }

void Tile::addZoneId(uint16_t _zoneId) { if (!zoneIds.contains(_zoneId)) { zoneIds.push_back(_zoneId); emit changed(); } }
void Tile::removeZoneId(uint16_t _zoneId) { zoneIds.erase(std::remove(zoneIds.begin(), zoneIds.end(), _zoneId), zoneIds.end()); emit changed(); }
void Tile::clearZoneId() { if (!zoneIds.empty()) { zoneIds.clear(); emit changed(); } }
void Tile::setZoneIds(Tile* tile) { if (tile) { zoneIds = tile->getZoneIds(); emit changed(); } }
const std::vector<uint16_t>& Tile::getZoneIds() const { return zoneIds; }
uint16_t Tile::getZoneId() const { return zoneIds.empty() ? 0 : zoneIds.front(); }

void Tile::setMapFlags(uint16_t _flags) { mapFlags = _flags; emit changed(); }
void Tile::unsetMapFlags(uint16_t _flags) { mapFlags &= ~_flags; emit changed(); }
uint16_t Tile::getMapFlags() const { return mapFlags; }

void Tile::setStatFlags(uint16_t _flags) { statFlags = _flags; emit changed(); }
void Tile::unsetStatFlags(uint16_t _flags) { statFlags &= ~_flags; emit changed(); }
uint16_t Tile::getStatFlags() const { return statFlags; } // Currently not used for much.

// --- Simple/Stubbed Methods ---
bool Tile::isBlockingCreature() const {
    // For now, assume creatures are not blocked by default tile properties,
    // only by specific items that might have IsBlockingCreature property.
    // Or if the tile itself has a general "block creature" flag (not currently implemented).
    for (const Item& item : items) {
        if (item.hasProperty(ItemProperty::IsBlockingCreature)) { // Assuming ItemProperty::IsBlockingCreature
            return true;
        }
    }
    return false;
}

bool Tile::isStairs() const {
    for (const Item& item : items) {
        if (item.hasProperty(ItemProperty::IsStairs)) return true; // Assumes ItemProperty::IsStairs exists
    }
    return false;
}

bool Tile::isLadder() const {
    for (const Item& item : items) {
        if (item.hasProperty(ItemProperty::IsLadder)) return true; // Assumes ItemProperty::IsLadder exists
    }
    return false;
}

int Tile::getMovementCost() const {
    // For now, return a default. Later, this could be based on ground item properties.
    // Or if Tile itself stores a movement cost modifier.
    return 1; // Default cost
}

void Tile::setMovementCost(int cost) {
    // Currently, Tile doesn't have a member to store this.
    // This can be a stub or log a warning if not implemented.
    Q_UNUSED(cost);
    // qDebug() << "Tile::setMovementCost not implemented yet.";
}

bool Tile::isProtectionZone() const {
    return (mapFlags & TILE_FLAG_PROTECTIONZONE) != 0;
}

void Tile::setProtectionZone(bool isPZ) {
    bool changed_flag = false;
    if (isPZ) {
        if (!(mapFlags & TILE_FLAG_PROTECTIONZONE)) {
            mapFlags |= TILE_FLAG_PROTECTIONZONE;
            changed_flag = true;
        }
    } else {
        if (mapFlags & TILE_FLAG_PROTECTIONZONE) {
            mapFlags &= ~TILE_FLAG_PROTECTIONZONE;
            changed_flag = true;
        }
    }
    if (changed_flag) {
        emit changed(); // Emit general tile changed signal
        modify(); // Mark tile as modified for saving
    }
}

bool Tile::isNoLogout() const {
    return (mapFlags & TILE_FLAG_NOLOGOUT) != 0;
}

void Tile::setNoLogout(bool noLogout) {
    bool changed_flag = false;
    if (noLogout) {
        if (!(mapFlags & TILE_FLAG_NOLOGOUT)) {
            mapFlags |= TILE_FLAG_NOLOGOUT;
            changed_flag = true;
        }
    } else {
        if (mapFlags & TILE_FLAG_NOLOGOUT) {
            mapFlags &= ~TILE_FLAG_NOLOGOUT;
            changed_flag = true;
        }
    }
    if (changed_flag) {
        emit changed();
        modify();
    }
}

// Stubs for other property editor methods
bool Tile::isNoMoveItems() const { return false; /* Stub */ }
void Tile::setNoMoveItems(bool noMove) { Q_UNUSED(noMove); /* Stub */ }
bool Tile::isNoMoveCreatures() const { return false; /* Stub */ }
void Tile::setNoMoveCreatures(bool noMove) { Q_UNUSED(noMove); /* Stub */ }
bool Tile::isNoSpawn() const { return false; /* Stub */ }
void Tile::setNoSpawn(bool noSpawn) { Q_UNUSED(noSpawn); /* Stub */ }
bool Tile::hasCollision() const { return getCollision(); /* Stub, or specific logic if different from getCollision */ }


// Helper: Get logical layer for rendering an item based on its properties.
Layer::Type Tile::getItemRenderLayer(const Item& item) const {
    // This mapping defines drawing order. Lower enum value = drawn first (bottom).
    // Original MapDrawer uses `ItemType` properties.
    if (item.hasProperty(ItemProperty::IsGroundTile)) return Layer::Type::Ground;
    // GroundDetail might be another explicit ItemProperty or category of small ground items.
    if (item.hasProperty(ItemProperty::IsWall)) return Layer::Type::Walls; // Walls might have a specific layer in 3D perspective.
    if (item.hasProperty(ItemProperty::IsDoor)) return Layer::Type::Walls; // Doors are usually drawn at wall level.
    if (item.hasProperty(ItemProperty::IsTeleport)) return Layer::Type::Objects;
    if (item.hasProperty(ItemProperty::IsMagicField)) return Layer::Type::Effects;
    if (item.hasProperty(ItemProperty::IsAlwaysOnTop)) return Layer::Type::Top;
    if (item.hasProperty(ItemProperty::IsGroundDetail)) return Layer::Type::GroundDetail; // Needs proper property and loading
    if (item.hasProperty(ItemProperty::IsHouse)) return Layer::Type::Objects; // Houses can be complex structures
    if (item.hasProperty(ItemProperty::IsHouseExit)) return Layer::Type::Objects; // House exits can be objects.
    if (item.hasProperty(ItemProperty::IsWaypoint)) return Layer::Type::Objects; // Waypoints are usually above objects.

    // Generic item placement if no specific layer flag is set on item.
    // Determine a sensible default based on `ItemProperty::IsContainer`, `ItemProperty::IsStackable`
    // or just the default `Objects` layer for regular items.
    return Layer::Type::Objects;
}

// Helper: checks an Item's internal properties (currently unused)
bool Tile::hasPropertyInternal(ItemProperty prop) const {
    Q_UNUSED(prop);
    return false;
}

uint32_t Tile::cleanInvalidItems() {
    uint32_t removed_count = 0;
    QVector<Item> validItems;
    validItems.reserve(items.size());

    ItemManager& itemManager = ItemManager::getInstance();

    for (const Item& item : items) {
        if (itemManager.getItemById(item.getId())) { // Check if item type is valid
            validItems.append(item);
        } else {
            removed_count++;
        }
    }

    if (removed_count > 0) {
        items = validItems; // Replace with the filtered list
        emit itemsChanged();
        emit changed(); // General tile changed signal
        // Consider if TILESTATE_MODIFIED needs to be set here
        // modify(); // If this function implies a user-driven modification that needs saving
    }

    return removed_count;
}

uint32_t Tile::cleanDuplicateItems(const std::function<bool(uint16_t)>& isInRanges,
                                   const std::function<bool(const Item&, const Item&)>& compareItemsFunc) {
    uint32_t removed_on_this_tile = 0;
    if (items.isEmpty()) {
        return 0;
    }

    QVector<Item> items_to_keep; // Items that are definitely not duplicates and match ranges.
    QVector<Item> final_items_on_tile; // All items that will remain (including those not in ranges).

    for (const Item& current_item_from_tile : items) {
        if (!isInRanges(current_item_from_tile.getId())) {
            final_items_on_tile.append(current_item_from_tile); // Keep if not in specified ranges for cleaning
            continue;
        }

        bool is_duplicate = false;
        for (const Item& kept_item : items_to_keep) {
            if (compareItemsFunc(current_item_from_tile, kept_item)) {
                is_duplicate = true;
                break;
            }
        }

        if (!is_duplicate) {
            items_to_keep.append(current_item_from_tile); // Add to list of unique items within the ranges
            final_items_on_tile.append(current_item_from_tile); // Add to final list for the tile
        } else {
            removed_on_this_tile++;
        }
    }

    if (removed_on_this_tile > 0) {
        items = final_items_on_tile; // Update the tile's items
        emit itemsChanged();
        emit changed();
        // modify(); // Potentially set TILESTATE_MODIFIED if this is a user-driven change needing save
    }

    return removed_on_this_tile;
}


// Comparison operators (from Source/src/tile.h)
// These are for sorting Tile pointers in collections.
bool tilePositionLessThan(const Tile* a, const Tile* b) {
    if (a->position.x() != b->position.x()) return a->position.x() < b->position.x();
    if (a->position.y() != b->position.y()) return a->position.y() < b->position.y();
    return a->position.z() < b->position.z();
}

bool tilePositionVisualLessThan(const Tile* a, const Tile* b) {
    // This is for visual sorting in drawing (e.g., isometric view depth sorting)
    // Not directly implemented for top-down view here, but important for 3D.
    // For Tibia's engine, lower Y means "closer to camera" and draws last if not same Z.
    // Also consider Item's 'topOrder'.
    if (a->position.z() != b->position.z()) return a->position.z() < b->position.z();
    if (a->position.y() != b->position.y()) return a->position.y() < b->position.y();
    return a->position.x() < b->position.x();
}