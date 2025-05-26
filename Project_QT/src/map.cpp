#include "map.h"
#include "tile.h"
#include "item.h"
#include "layer.h"
#include "bordersystem.h"
#include "otbmfile.h"
#include "itemmanager.h" // For ItemManager (used in cleanDuplicateItems for properties).
#include "spawn.h"       // Added for Spawn class integration

#include <QDebug>
#include <QFileInfo>
#include <QVector>
#include <algorithm> // For std::sort, std::remove
#include <functional> // For std::function

Map* Map::s_instance = nullptr;

Map& Map::getInstance() {
    if (!s_instance) {
        s_instance = new Map();
        qAddPostRoutine([](){ delete s_instance; s_instance = nullptr; });
    }
    return *s_instance;
}

Map::Map(QObject* parent) :
    QObject(parent),
    size(0, 0),
    modified(false),
    unnamed(true),
    selectionRect(0, 0, 0, 0),
    multiSelectionMode(false),
    currentLayer(static_cast<int>(Layer::Type::Ground)),
    borderSystem(new BorderSystem(this)) // BorderSystem is owned by Map
{
    // Initialize m_version to a default (e.g., OTBM_4 and a common client version)
    m_version.otbm_version = MapVersion::MAP_OTBM_4; // Default to a modern version
    m_version.client_version_major = 10; // Example default client version
    m_version.client_version_minor = 98;
    m_version.client_version_patch = 0;
    // m_warnings, m_error, m_spawns are default constructed.

    layers.reserve(Map::LayerCount);
    for (int i = 0; i < Map::LayerCount; ++i) {
        Layer* newLayer = new Layer(static_cast<Layer::Type>(i), this);
        layers.append(newLayer);
    }
    borderSystem->setMap(this);
}

Map::~Map() {
    clear();
}

void Map::clear() {
    for (int x = 0; x < tiles.size(); ++x) {
        for (int y = 0; y < tiles[x].size(); ++y) {
            for (int z = 0; z < tiles[x][y].size(); ++z) {
                delete tiles[x][y][z]; // Delete Tile objects
                tiles[x][y][z] = nullptr;
            }
        }
    }
    tiles.clear();
    setSize(QSize(0,0));
    clearSelection();
    setModified(false);
    unnamed = true;
    name = QString();
    filename = QString();
    description = QString();
    spawnFile = QString();
    houseFile = QString();
    waypoints.clear();
    towns.clear();
    houses.clear();

    // Clear spawns (assuming Map owns Spawn objects, which it will eventually)
    // For now, just clearing the list as Spawn definition is pending.
    qDeleteAll(m_spawns); // Map owns Spawn objects
    m_spawns.clear();

    // Reset version to a default state
    m_version.otbm_version = MapVersion::MAP_OTBM_4;
    m_version.client_version_major = 10;
    m_version.client_version_minor = 98;
    m_version.client_version_patch = 0;
    
    clearWarnings();
    clearError();

    for (Layer* layer : layers) {
        layer->setVisible(true);
        layer->setLocked(false);
    }
    emit mapChanged();
}

void Map::ensureTilesExist() {
    tiles.resize(size.width());
    for (int x = 0; x < size.width(); ++x) {
        tiles[x].resize(size.height());
        for (int y = 0; y < size.height(); ++y) {
            tiles[x][y].resize(Map::LayerCount);
            for (int z = 0; z < Map::LayerCount; ++z) {
                if (tiles[x][y][z] == nullptr) {
                    tiles[x][y][z] = new Tile(x, y, z, this);
                }
            }
        }
    }
}

void Map::setSize(const QSize& newSize) {
    if (size == newSize) return;

    size = newSize;
    ensureTilesExist();
    setModified(true);
    emit mapChanged();
}

Tile* Map::getTile(int x, int y, int z) {
    if (x < 0 || x >= size.width() || y < 0 || y >= size.height() || z < 0 || z >= Map::LayerCount) {
        return nullptr;
    }
    return tiles[x][y][z];
}

const Tile* Map::getTile(int x, int y, int z) const {
    if (x < 0 || x >= size.width() || y < 0 || y >= size.height() || z < 0 || z >= Map::LayerCount) {
        return nullptr;
    }
    return tiles[x][y][z];
}

Tile* Map::getTile(const QPoint& pos, Layer::Type layerType) {
    return getTile(pos.x(), pos.y(), static_cast<int>(layerType));
}

const Tile* Map::getTile(const QPoint& pos, Layer::Type layerType) const {
    return getTile(pos.x(), pos.y(), static_cast<int>(layerType));
}

void Map::addItem(int x, int y, Layer::Type layer, const Item& item) {
    Tile* tile = getTile(x, y, static_cast<int>(layer));
    if (tile) {
        tile->addItem(item);
        setModified(true);
        emit tileChanged(QPoint(x,y));
        borderSystem->applyBorders(QPoint(x,y), static_cast<int>(layer));
    }
}

void Map::removeItem(int x, int y, Layer::Type layer, const Item& item) {
    Tile* tile = getTile(x, y, static_cast<int>(layer));
    if (tile) {
        tile->removeItem(item);
        setModified(true);
        emit tileChanged(QPoint(x,y));
        borderSystem->applyBorders(QPoint(x,y), static_cast<int>(layer));
    }
}

void Map::clearLayer(int x, int y, Layer::Type layer) {
    Tile* tile = getTile(x, y, static_cast<int>(layer));
    if (tile) {
        tile->clearLayer(layer);
        setModified(true);
        emit tileChanged(QPoint(x,y));
        borderSystem->applyBorders(QPoint(x,y), static_cast<int>(layer));
    }
}

QList<Item> Map::getItems(int x, int y, Layer::Type layer) const {
    const Tile* tile = getTile(x, y, static_cast<int>(layer));
    if (tile) {
        return QList<Item>(tile->getItems().begin(), tile->getItems().end());
    }
    return QList<Item>();
}

Layer* Map::getLayer(Layer::Type type) {
    if (static_cast<int>(type) >= 0 && static_cast<int>(type) < layers.size()) {
        return layers[static_cast<int>(type)];
    }
    return nullptr;
}

const Layer* Map::getLayer(Layer::Type type) const {
    if (static_cast<int>(type) >= 0 && static_cast<int>(type) < layers.size()) {
        return layers[static_cast<int>(type)];
    }
    return nullptr;
}

void Map::setLayerVisible(Layer::Type type, bool visible) {
    Layer* layer = getLayer(type);
    if (layer) {
        layer->setVisible(visible);
    }
}

void Map::setLayerLocked(Layer::Type type, bool locked) {
    Layer* layer = getLayer(type);
    if (layer) {
        layer->setLocked(locked);
    }
}

bool Map::isLayerVisible(Layer::Type type) const {
    const Layer* layer = getLayer(type);
    return layer ? layer->isVisible() : false;
}

void Map::setModified(bool value) {
    if (modified != value) {
        modified = value;
        emit mapChanged();
    }
}

bool Map::convert(MapVersion to, bool showdialog) {
    Q_UNUSED(to);
    Q_UNUSED(showdialog);
    qDebug() << "Map::convert triggered. Actual conversion logic not implemented.";
    return true;
}

bool Map::loadFromFile(const QString& filename) {
    OTBMFile loader;
    connect(&loader, &OTBMFile::loadProgress, this, &Map::loadProgress);
    // Capture errors from loader into m_error
    connect(&loader, &OTBMFile::error, this, [this](const QString& msg){ 
        qDebug() << "Map load error:" << msg; 
        setError(msg); // Store the error message
    });
    
    clearError(); // Clear previous errors before loading
    clearWarnings(); // Clear previous warnings

    // Pass this map instance so the loader can fill it directly
    bool success = loader.load(filename, this);
    
    if (success) {
        // Assuming OTBMFile has methods to get version and warnings after load
        // For now, these are placeholders until OTBMFile is updated
        // MapVersion loadedVersion = loader.getMapVersion(); // Placeholder
        // setVersion(loadedVersion); // Placeholder
        
        // QStringList loadedWarnings = loader.getWarnings(); // Placeholder
        // for(const QString& warning : loadedWarnings) { // Placeholder
        //     addWarning(warning); // Placeholder
        // }

        qDebug() << "Map loaded from" << filename;
        setName(QFileInfo(filename).baseName().toStdString());
        setModified(false);
        unnamed = false;
        this->filename = filename;
        this->setSize(QSize(loader.getWidth(), loader.getHeight()));
        this->setDescription(loader.getDescription());
        this->setSpawnFile(loader.getSpawnFile());
        this->setHouseFile(loader.getHouseFile());
        this->setTowns(loader.getTowns());
        this->setHouses(loader.getHouses());
        this->setWaypoints(loader.getWaypoints());

        ensureTilesExist(); // Creates/ensures all 16 layers of tiles.
        // OTBMFile's `readTile` typically calls `Map::addItem` implicitly,
        // which now populates the `tiles` vector using the layer-aware `Tile::addItem`.

        emit mapChanged();
    } else {
        qDebug() << "Failed to load map from" << filename;
        // m_error should already be set by the loader via the connected signal
    }
    return success;
}

bool Map::saveToFile(const QString& filename) const {
    OTBMFile saver;
    // OTBMFile::save does not have direct access to internal map tiles.
    // The saver must receive all tiles for writing.
    // The `OTBMFile::save(filename, const Map* mapInstance)` must iterate map and write all its tiles and items.
    
    connect(&saver, &OTBMFile::saveProgress, this, &Map::saveProgress);
    connect(&saver, &OTBMFile::error, this, [this](const QString& msg){ qDebug() << "Map save error:" << msg; });
    
    // Passes a const pointer to this Map instance for the saver to read.
    bool success = saver.save(filename, this);

    if (success) {
        qDebug() << "Map saved to" << filename;
    } else {
        qDebug() << "Failed to save map to" << filename;
    }
    return success;
}

bool Map::importFromOTBM(const QString& filename) { Q_UNUSED(filename); return false; /* Placeholder */ }
bool Map::exportToOTBM(const QString& filename) { Q_UNUSED(filename); return false; /* Placeholder */ }
bool Map::importFromJSON(const QString& filename) { Q_UNUSED(filename); return false; /* Placeholder */ }
bool Map::exportToJSON(const QString& filename) { Q_UNUSED(filename); return false; /* Placeholder */ }
bool Map::importFromXML(const QString& filename) { Q_UNUSED(filename); return false; /* Placeholder */ }
bool Map::exportToXML(const QString& filename) { Q_UNUSED(filename); return false; /* Placeholder */ }

void Map::setDescription(const QString& desc) { description = desc; setModified(true); }
void Map::setSpawnFile(const QString& file) { spawnFile = file; setModified(true); }
void Map::setHouseFile(const QString& file) { houseFile = file; setModified(true); }
void Map::setTowns(const QMap<quint32, QString>& t) { towns = t; setModified(true); }
void Map::setHouses(const QMap<quint32, HouseInfo>& h) { houses = h; setModified(true); }
void Map::setWaypoints(const QMap<QString, Position>& w) { waypoints = w; setModified(true); }

void Map::setSelection(const QRect& newSelection) {
    if (selectionRect == newSelection) return;
    selectionRect = newSelection;
    selectedPositions.clear(); // Clear individual selection for rect.
    for (int x = selectionRect.left(); x < selectionRect.left() + selectionRect.width(); ++x) {
        for (int y = selectionRect.top(); y < selectionRect.top() + selectionRect.height(); ++y) {
            // Check bounds to ensure points are actually within the map dimensions.
            if (x >= 0 && x < size.width() && y >= 0 && y < size.height()) {
                 selectedPositions.insert(QPoint(x, y)); // Use QPoint for selected positions set.
            }
        }
    }
    emit selectionChanged(selectionRect);
}

void Map::addToSelection(const QRect& rect) {
    for (int x = rect.left(); x < rect.left() + rect.width(); ++x) {
        for (int y = rect.top(); y < rect.top() + rect.height(); ++y) {
            if (x >= 0 && x < size.width() && y >= 0 && y < size.height()) {
                selectedPositions.insert(QPoint(x, y));
            }
        }
    }
    if (selectionRect.isNull() || selectionRect.isEmpty()) {
        selectionRect = rect;
    } else {
        selectionRect = selectionRect.united(rect);
    }
    emit selectionChanged(selectionRect);
}

void Map::removeFromSelection(const QRect& rect) {
    for (int x = rect.left(); x < rect.left() + rect.width(); ++x) {
        for (int y = rect.top(); y < rect.top() + rect.height(); ++y) {
            if (x >= 0 && x < size.width() && y >= 0 && y < size.height()) {
                selectedPositions.remove(QPoint(x, y));
            }
        }
    }
    if (selectedPositions.isEmpty()) {
        selectionRect = QRect();
    } else {
        // To maintain `selectionRect` as a bounding box of possibly fragmented `selectedPositions`
        // we'd need to recompute its bounds (min x,y to max x,y of all `selectedPositions`).
        // For simplicity for now, it's just `currentRect` and may not reflect fragmentation exactly.
        int minX=size.width(), minY=size.height(), maxX=0, maxY=0;
        bool first = true;
        for(const QPoint& p : selectedPositions) {
            if (first) {
                minX=p.x(); minY=p.y(); maxX=p.x(); maxY=p.y();
                first=false;
            } else {
                minX = qMin(minX, p.x()); minY = qMin(minY, p.y());
                maxX = qMax(maxX, p.x()); maxY = qMax(maxY, p.y());
            }
        }
        selectionRect = QRect(minX, minY, maxX - minX + 1, maxY - minY + 1);
    }
    emit selectionChanged(selectionRect);
}

void Map::toggleSelectionAt(const QPoint& point) {
    if (selectedPositions.contains(point)) {
        selectedPositions.remove(point);
    } else {
        selectedPositions.insert(point);
    }
    if (selectedPositions.isEmpty()) {
        selectionRect = QRect();
    } else {
        int minX=size.width(), minY=size.height(), maxX=0, maxY=0;
        bool first = true;
        for(const QPoint& p : selectedPositions) {
            if (first) {
                minX=p.x(); minY=p.y(); maxX=p.x(); maxY=p.y();
                first=false;
            } else {
                minX = qMin(minX, p.x()); minY = qMin(minY, p.y());
                maxX = qMax(maxX, p.x()); maxY = qMax(maxY, p.y());
            }
        }
        selectionRect = QRect(minX, minY, maxX - minX + 1, maxY - minY + 1);
    }
    emit selectionChanged(selectionRect);
}

bool Map::isSelected(const QPoint& point) const {
    return selectedPositions.contains(point);
}

void Map::clearSelection() {
    selectionRect = QRect();
    selectedPositions.clear();
    emit selectionChanged(selectionRect);
}

QVector<Tile*> Map::getSelectedTiles() const {
    QVector<Tile*> selectedTiles;
    for (const QPoint& pos : selectedPositions) {
        Tile* tile = getTile(pos.x(), pos.y(), currentLayer); // At current active layer.
        if (tile) selectedTiles.append(tile);
    }
    return selectedTiles;
}

// Selection Transformation: Placeholder implementations, actual logic will be done through Commands
// These functions in original RME typically would trigger the actual map data manipulation through Editor/Actions.
// They modify the `Map` instance's tile data (not just the selection rect itself).

void Map::moveSelection(const QPoint& offset, bool copy) {
    qDebug() << "Map: Move selection by" << offset << ", copy:" << copy << ". (Stub).";
    // This implies creating AddItemCommands/RemoveItemCommands for affected tiles.
    // Update selectedPositions set as well to reflect new positions.
    // selectionRect also updates.
    // Example: This should use a MapCommand macro like the original `Editor::moveSelection`.
    setModified(true);
    // After moving, update selection. For now: just clear for visual indication of move.
    clearSelection();
    emit mapChanged(); // Trigger full redraw.
}

void Map::rotateSelection(int degrees) {
    qDebug() << "Map: Rotate selection by" << degrees << "degrees. (Stub).";
    // Similar to move, actual tile/item manipulation.
    // Rotate every `Item` on selected tiles that is rotatable, then update tile visuals.
    setModified(true);
    // After rotation, clear selection for visual simplicity of action.
    clearSelection();
    emit mapChanged();
}

void Map::flipSelectionHorizontally() {
    qDebug() << "Map: Flip selection horizontally. (Stub).";
    setModified(true);
    clearSelection();
    emit mapChanged();
}

void Map::flipSelectionVertically() {
    qDebug() << "Map: Flip selection vertically. (Stub).";
    setModified(true);
    clearSelection();
    emit mapChanged();
}

uint32_t Map::cleanDuplicateItems(const std::vector<std::pair<uint16_t, uint16_t>>& ranges, const PropertyFlags& flags) {
    uint32_t duplicates_removed = 0;

    auto isInRanges = [&](uint16_t id) -> bool {
        if (ranges.empty()) {
            return true; // Process all items if ranges is empty
        }
        for (const auto& range : ranges) {
            if (id >= range.first && id <= range.second) {
                return true;
            }
        }
        return false;
    };

    ItemManager& itemManager = ItemManager::getInstance();

    auto compareItems = [&](const Item& item1, const Item& item2) -> bool {
        if (item1.getId() != item2.getId()) {
            return false;
        }

        // Note: The Item class currently doesn't directly expose all these properties.
        // We'll use hasProperty with assumptions for ItemProperty enum values.
        // This section will require ItemProperty enum to be comprehensive or Item class to have direct getters.

        // Example for unpassable (assuming ItemProperty::IsBlocking implies unpassable)
        if (!flags.ignore_unpassable && (item1.hasProperty(ItemProperty::IsBlocking) != item2.hasProperty(ItemProperty::IsBlocking))) {
            return false;
        }
        // Example for unmovable (assuming ItemProperty::IsMovable)
        if (!flags.ignore_unmovable && (item1.hasProperty(ItemProperty::IsMovable) != item2.hasProperty(ItemProperty::IsMovable))) {
            return false;
        }
        if (!flags.ignore_block_missiles && (item1.hasProperty(ItemProperty::BlockMissiles) != item2.hasProperty(ItemProperty::BlockMissiles))) {
            return false;
        }
        if (!flags.ignore_block_pathfinder && (item1.hasProperty(ItemProperty::BlockPathfinder) != item2.hasProperty(ItemProperty::BlockPathfinder))) {
            return false;
        }
        if (!flags.ignore_readable && (item1.hasProperty(ItemProperty::IsReadable) != item2.hasProperty(ItemProperty::IsReadable))) {
            return false;
        }
        if (!flags.ignore_writeable && (item1.hasProperty(ItemProperty::IsWritable) != item2.hasProperty(ItemProperty::IsWritable))) {
            return false;
        }
        if (!flags.ignore_pickupable && (item1.hasProperty(ItemProperty::IsPickupable) != item2.hasProperty(ItemProperty::IsPickupable))) {
            return false;
        }
        if (!flags.ignore_stackable && (item1.hasProperty(ItemProperty::IsStackable) != item2.hasProperty(ItemProperty::IsStackable))) {
            return false; // This is tricky; true duplicates of stackables are usually merged. This flag might mean something else.
        }
        if (!flags.ignore_rotatable && (item1.hasProperty(ItemProperty::IsRotatable) != item2.hasProperty(ItemProperty::IsRotatable))) {
            return false;
        }
        if (!flags.ignore_hangable && (item1.hasProperty(ItemProperty::IsHangable) != item2.hasProperty(ItemProperty::IsHangable))) {
            return false;
        }
        // Hooks might need specific handling if they depend on orientation, not just presence.
        if (!flags.ignore_hook_east && (item1.hasProperty(ItemProperty::HookEast) != item2.hasProperty(ItemProperty::HookEast))) {
            return false;
        }
        if (!flags.ignore_hook_south && (item1.hasProperty(ItemProperty::HookSouth) != item2.hasProperty(ItemProperty::HookSouth))) {
            return false;
        }
        // Elevation might be a direct attribute of the Item, e.g., item1.getElevation()
        // Assuming Item::getElevation() exists or is mapped via a property
        if (!flags.ignore_elevation && (item1.hasProperty(ItemProperty::HasElevation) != item2.hasProperty(ItemProperty::HasElevation) || 
                                        (item1.hasProperty(ItemProperty::HasElevation) && item1.getElevation() != item2.getElevation()) )) {
            // This check assumes getElevation() is valid only if HasElevation is true.
            // And that ItemProperty::HasElevation exists.
            return false;
        }

        // If we made it here, items are considered duplicates based on the flags.
        return true;
    };
    
    if (tiles.isEmpty() || size.width() == 0 || size.height() == 0) {
        return 0;
    }

    for (int x = 0; x < size.width(); ++x) {
        if (x >= tiles.size()) continue;
        for (int y = 0; y < size.height(); ++y) {
            if (y >= tiles[x].size()) continue;
            for (int z = 0; z < Map::LayerCount; ++z) {
                if (z >= tiles[x][y].size()) continue;

                Tile* currentTile = tiles[x][y][z];
                if (currentTile) {
                    uint32_t count_on_tile = currentTile->cleanDuplicateItems(isInRanges, compareItems);
                    if (count_on_tile > 0) {
                        duplicates_removed += count_on_tile;
                        setModified(true);
                        emit tileChanged(currentTile->getPosition());
                    }
                }
            }
        }
    }

    if (duplicates_removed > 0) {
        qDebug() << "Removed" << duplicates_removed << "duplicate items from the map.";
    }

    return duplicates_removed;
}

void Map::cleanInvalidTiles(bool showdialog) {
    uint32_t total_removed_count = 0;
    bool map_was_modified = false;

    if (tiles.isEmpty() || size.width() == 0 || size.height() == 0) {
        if (showdialog) {
            qDebug() << "Map is empty. No tiles to clean.";
        }
        return; // Nothing to do if map is empty
    }

    for (int x = 0; x < size.width(); ++x) {
        if (x >= tiles.size()) continue; // Should not happen with ensureTilesExist
        for (int y = 0; y < size.height(); ++y) {
            if (y >= tiles[x].size()) continue; // Should not happen
            for (int z = 0; z < Map::LayerCount; ++z) { // Iterate through all layers (floors)
                if (z >= tiles[x][y].size()) continue; // Should not happen

                Tile* tile = tiles[x][y][z];
                if (tile) {
                    uint32_t removed_on_tile = tile->cleanInvalidItems();
                    if (removed_on_tile > 0) {
                        total_removed_count += removed_on_tile;
                        map_was_modified = true;
                        // Emit tileChanged for the specific tile's position (x,y,z)
                        // The tile itself emits changed() which might be sufficient if MapView listens to that.
                        // For directness, Map can also signal.
                        emit tileChanged(tile->getPosition()); // tile->getPosition() includes x,y,z
                    }
                }
            }
        }
    }

    if (map_was_modified) {
        setModified(true); // Set the map as modified
    }

    if (showdialog) {
        if (total_removed_count > 0) {
            qDebug() << "Removed" << total_removed_count << "invalid items from the map.";
        } else {
            qDebug() << "No invalid items found on the map.";
        }
    }
    // The requirement was to return uint32_t, but the function signature is void.
    // For now, sticking to void and using qDebug as per refined instructions.
    // If it needs to return, the signature in map.h and map.cpp must be changed.
    // For this task, I will keep it void and use qDebug.
}

void Map::convertHouseTiles(uint32_t fromId, uint32_t toId) {
    if (fromId == 0 && toId == 0) { // Converting from unassigned to unassigned makes no sense.
        qDebug() << "Map::convertHouseTiles: fromId and toId cannot both be 0.";
        return;
    }
    if (fromId == toId) { // Converting to the same ID makes no sense.
        qDebug() << "Map::convertHouseTiles: fromId and toId are the same.";
        return;
    }

    uint32_t tiles_affected = 0;

    if (tiles.isEmpty() || size.width() == 0 || size.height() == 0) {
        qDebug() << "Map is empty. No house tiles to convert.";
        return;
    }

    for (int x = 0; x < size.width(); ++x) {
        if (x >= tiles.size()) continue; 
        for (int y = 0; y < size.height(); ++y) {
            if (y >= tiles[x].size()) continue; 
            for (int z = 0; z < Map::LayerCount; ++z) {
                if (z >= tiles[x][y].size()) continue;

                Tile* currentTile = tiles[x][y][z]; // Direct access for efficiency
                if (!currentTile) {
                    continue;
                }

                // Original logic: "if fromID is 0, all unassigned house tiles are assigned to toID"
                // "if fromID is not 0, only tiles with houseId == fromID are changed"
                bool should_convert = false;
                if (fromId == 0) { // Convert unassigned tiles
                    if (currentTile->getHouseID() == 0) {
                        should_convert = true;
                    }
                } else { // Convert tiles with specific fromId
                    if (currentTile->getHouseID() == fromId) {
                        should_convert = true;
                    }
                }
                
                if (should_convert) {
                    currentTile->setHouseID(toId); // This should internally emit Tile::changed()
                    tiles_affected++;
                    // Tile::setHouseID should mark the tile as modified and emit a signal.
                    // The map itself also needs to be marked as modified.
                    // Emitting tileChanged for the specific tile's position
                    emit tileChanged(currentTile->getPosition()); 
                }
            }
        }
    }

    if (tiles_affected > 0) {
        setModified(true); // Set the map as modified if any tile was changed
        qDebug() << "Converted" << tiles_affected << "house tiles from ID" << fromId << "to ID" << toId;
    } else {
        qDebug() << "No house tiles found matching ID" << fromId << "to convert to ID" << toId;
    }
}

bool Map::addSpawn(Tile* spawnTile) {
    if (!spawnTile) {
        return false;
    }

    QPoint spawn_pos_qp = spawnTile->getPosition(); // QPoint from Tile

    // Check if a spawn already exists at this exact QPoint position
    for (const Spawn* existingSpawn : m_spawns) {
        if (existingSpawn->getPosition() == spawn_pos_qp) {
            qDebug() << "Spawn already exists at position:" << spawn_pos_qp;
            return false; // Or handle update logic if needed
        }
    }

    // Create a new Spawn object. Default radius, creature, interval from Spawn constructor
    Spawn* newSpawn = new Spawn(spawn_pos_qp);
    m_spawns.append(newSpawn);

    setModified(true);
    emit mapChanged(); // Or a more specific signal like spawnAdded(newSpawn)
    // Consider emitting a signal like: emit spawnAdded(newSpawn);
    qDebug() << "Spawn added at" << spawn_pos_qp;
    return true;
}

void Map::removeSpawn(Tile* spawnTile) {
    if (!spawnTile) {
        return;
    }
    QPoint tilePosQt = spawnTile->getPosition(); // QPoint from Tile
    // Convert QPoint to map.h::Position. Assuming Z from tile's layer.
    // Note: Spawn itself uses QPoint (x,y). Z is implicitly handled by the tile layer.
    // The map.h::Position includes Z. For comparing with Spawn's QPoint, we only use x,y.
    Position tilePosMap(tilePosQt.x(), tilePosQt.y(), spawnTile->getZ());
    removeSpawn(tilePosMap); // Call the Position-based removeSpawn
}

void Map::removeSpawn(const Position& position) { // position is map.h::Position {int x,y,z}
    for (int i = 0; i < m_spawns.size(); ++i) {
        Spawn* currentSpawn = m_spawns.at(i);
        // Spawn stores QPoint (x,y). Compare with map.h::Position's x and y. Z is ignored for now.
        if (currentSpawn->getPosition().x() == position.x && currentSpawn->getPosition().y() == position.y) {
            qDebug() << "Removing spawn at" << currentSpawn->getPosition();
            m_spawns.removeAt(i);
            delete currentSpawn;
            setModified(true);
            emit mapChanged(); // Or a more specific signal like spawnRemoved
            // Consider: emit spawnRemoved(position);
            return; // Assuming only one spawn per exact (x,y) position.
        }
    }
    qDebug() << "No spawn found at map.h::Position:" << position.x << "," << position.y << "," << position.z << "to remove.";
}


void Map::setCurrentLayer(int layer) {
    if (currentLayer == layer) return;
    currentLayer = layer;
    // Note: MapView/LayerWidget trigger this, then MapView propagates to MapScene.
    // This updates the *Map Model's* internal concept of current layer.
}

bool Map::hasFile() const { return !filename.isEmpty(); }

// getWarnings(), addWarning(), clearWarnings() are in map.h (inline or to be defined if complex)
// getError(), setError(), clearError() are in map.h (inline or to be defined if complex)
// getVersion() and setVersion() are in map.h (inline)

void Map::setName(const std::string& n) { name = QString::fromStdString(n); }