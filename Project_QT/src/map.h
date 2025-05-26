#ifndef MAP_H
#define MAP_H

#include <QObject>
#include <QPoint>
#include <QSize>
#include <QVector>
#include <QMap>
#include <QString>
#include <QRect>
#include <QSet>

#include <vector>

#include "tile.h"
#include "item.h"
#include "layer.h"
#include "bordersystem.h"

// Forward declarations for specific structures
struct Position {
    int x, y, z;
    Position(int _x = 0, int _y = 0, int _z = 0) : x(_x), y(_y), z(_z) {}
    bool operator==(const Position& other) const { return x == other.x && y == other.y && z == other.z; }
    // Define hashCode and equals for QSet<Position> using QPoint
    // QPoint has built-in hash and equality, so often `QPoint` is used instead of `Position` in QSet
    // If sticking to `Position`, you'll need `qHash(const Position&)` and `operator==`.
    // For now, conversion to QPoint for set will work or define `qHash` if this struct is used.
    // Assuming implicit conversion or wrapper to QPoint for set functionality or a custom `qHash`.
    
    // Minimal for sorting:
    bool operator<(const Position& p) const {
        if (x != p.x) return x < p.x;
        if (y != p.y) return y < p.y;
        return z < p.z;
    }
};

// Define HouseInfo struct (from Source/src/house.h)
struct HouseInfo {
    uint32_t id;
    QString name;
    Position position;
};

// Structures needed for Map::cleanDuplicateItems from Source/map.h
struct PropertyFlags {
    bool match_uid;
    bool match_actionid;
    bool match_text;
    bool match_attribute;
    bool match_name; // From Source/map.h (used for item name matching)
    // Add any other flags from original Source/map.h PropertyFlags
};

class Map : public QObject
{
    Q_OBJECT
public:
    static Map& getInstance();

    explicit Map(QObject* parent = nullptr);
    virtual ~Map();

    void clear();

    void setSize(const QSize& size);
    QSize getSize() const { return size; }
    static const int LayerCount = 16;

    Tile* getTile(int x, int y, int z);
    const Tile* getTile(int x, int y, int z) const;
    Tile* getTile(const QPoint& pos, Layer::Type layerType);
    const Tile* getTile(const QPoint& pos, Layer::Type layerType) const;

    void addItem(int x, int y, Layer::Type layer, const Item& item);
    void removeItem(int x, int y, Layer::Type layer, const Item& item);
    void clearLayer(int x, int y, Layer::Type layer);
    QList<Item> getItems(int x, int y, Layer::Type layer) const;

    Layer* getLayer(Layer::Type type);
    const Layer* getLayer(Layer::Type type) const;
    void setLayerVisible(Layer::Type type, bool visible);
    void setLayerLocked(Layer::Type type, bool locked);
    bool isLayerVisible(Layer::Type type) const;
    
    bool isModified() const { return modified; }
    void setModified(bool value);

    bool convert(MapVersion to, bool showdialog);

    bool loadFromFile(const QString& filename);
    bool saveToFile(const QString& filename);
    
signals:
    void loadProgress(int progress);
    void saveProgress(int progress);
    void tileChanged(const QPoint& position);
    void mapChanged();
    void selectionChanged(const QRect& selectionRect);

    bool importFromOTBM(const QString& filename);
    bool exportToOTBM(const QString& filename);
    bool importFromJSON(const QString& filename);
    bool exportToJSON(const QString& filename);
    bool importFromXML(const QString& filename);
    bool exportToXML(const QString& filename);

    void setDescription(const QString& description);
    QString getDescription() const { return description; }
    void setSpawnFile(const QString& spawnFile);
    QString getSpawnFile() const { return spawnFile; }
    void setHouseFile(const QString& houseFile);
    QString getHouseFile() const { return houseFile; }
    void setTowns(const QMap<quint32, QString>& towns);
    const QMap<quint32, QString>& getTowns() const { return towns; }
    void setHouses(const QMap<quint32, HouseInfo>& houses);
    const QMap<quint32, HouseInfo>& getHouses() const { return houses; }
    void setWaypoints(const QMap<QString, Position>& waypoints);
    const QMap<QString, Position>& getWaypoints() const { return waypoints; }
    
    void setSelection(const QRect& selection);
    QRect getSelection() const { return selectionRect; }
    void addToSelection(const QRect& rect);
    void removeFromSelection(const QRect& rect);
    void toggleSelectionAt(const QPoint& point);
    bool isSelected(const QPoint& point) const;
    void clearSelection();
    QVector<Tile*> getSelectedTiles() const;

    void moveSelection(const QPoint& offset, bool copy = false);
    void rotateSelection(int degrees);
    void flipSelectionHorizontally();
    void flipSelectionVertically();

    // From Source/map.h: Map cleanup, needs PropertyFlags
    uint32_t cleanDuplicateItems(const std::vector<std::pair<uint16_t, uint16_t>>& ranges, const PropertyFlags& flags);
    void cleanInvalidTiles(bool showdialog);
    void convertHouseTiles(uint32_t fromId, uint32_t toId);
    
    bool addSpawn(Tile* spawnTile);
    void removeSpawn(Tile* spawnTile);
    void removeSpawn(const Position& position);

    int getCurrentLayer() const { return currentLayer; }
    void setCurrentLayer(int layer);
    
    std::string getName() const { return name.toStdString(); }
    void setName(const std::string& n);
    bool hasFile() const;
    bool getLoaderError() const { return false; }
    bool hasError() const { return false; }
    const wxArrayString& getWarnings() const;

    BorderSystem* getBorderSystem() const { return borderSystem; }
    
private:
    static Map* s_instance; // The singleton instance
    explicit Map(QObject* parent = nullptr);

    QVector<QVector<QVector<Tile*>>> tiles; // tiles[x][y][z]
    QSize size;

    QVector<Layer*> layers; // Managed layers

    QString name;
    QString filename;
    QString description;
    QString spawnFile;
    QString houseFile;
    QString waypointFile;
    QMap<quint32, QString> towns;
    QMap<quint32, HouseInfo> houses;
    QMap<QString, Position> waypoints;

    bool modified;
    bool unnamed;

    QRect selectionRect;
    QSet<QPoint> selectedPositions;
    bool multiSelectionMode;

    int currentLayer;

    BorderSystem* borderSystem;

    void ensureTilesExist();
};

#endif // MAP_H