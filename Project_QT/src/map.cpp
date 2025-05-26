#include "map.h"
#include "tile.h"
#include "item.h"
#include "layer.h"
#include "bordersystem.h"
#include "otbmfile.h"
#include "itemmanager.h" // For ItemManager (used in cleanDuplicateItems for properties).

#include <QDebug>
#include <QFileInfo>
#include <QVector>
#include <algorithm> // For std::sort, std::remove

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
    connect(&loader, &OTBMFile::error, this, [this](const QString& msg){ qDebug() << "Map load error:" << msg; });
    
    // Pass this map instance so the loader can fill it directly
    bool success = loader.load(filename, this);
    
    if (success) {
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
    qDebug() << "Map: Clean duplicate items. (Stub).";
    Q_UNUSED(ranges); Q_UNUSED(flags);
    return 0; // No items removed.
}

void Map::cleanInvalidTiles(bool showdialog) { Q_UNUSED(showdialog); /* Impl */ }
void Map::convertHouseTiles(uint32_t fromId, uint32_t toId) { Q_UNUSED(fromId); Q_UNUSED(toId); /* Impl */ }

bool Map::addSpawn(Tile* spawnTile) { Q_UNUSED(spawnTile); return false; /* Impl */ }
void Map::removeSpawn(Tile* spawnTile) { Q_UNUSED(spawnTile); /* Impl */ }
void Map::removeSpawn(const Position& position) { Q_UNUSED(position); /* Impl */ }

void Map::setCurrentLayer(int layer) {
    if (currentLayer == layer) return;
    currentLayer = layer;
    // Note: MapView/LayerWidget trigger this, then MapView propagates to MapScene.
    // This updates the *Map Model's* internal concept of current layer.
}

bool Map::hasFile() const { return !filename.isEmpty(); }

// Assumed existence from Source `map.h` (used by `ItemManager` or similar logic in Tibia)
const wxArrayString& Map::getWarnings() const { static wxArrayString s_empty; return s_empty; }

void Map::setName(const std::string& n) { name = QString::fromStdString(n); }