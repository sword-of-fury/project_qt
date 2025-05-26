#ifndef OTBM_H
#define OTBM_H

#include "binaryfile.h"
#include <QMap>
#include <QVector>
#include <QString>

// Wersje formatu OTBM
enum OTBMVersion {
    OTBM_1 = 1,
    OTBM_2 = 2,
    OTBM_3 = 3,
    OTBM_4 = 4
};

// Typy węzłów OTBM
enum OTBMNodeType {
    OTBM_ROOTV1 = 1,
    OTBM_MAP_DATA = 2,
    OTBM_ITEM_DEF = 3,
    OTBM_TILE_AREA = 4,
    OTBM_TILE = 5,
    OTBM_ITEM = 6,
    OTBM_TILE_SQUARE = 7,
    OTBM_TILE_REF = 8,
    OTBM_SPAWNS = 9,
    OTBM_SPAWN_AREA = 10,
    OTBM_MONSTER = 11,
    OTBM_TOWNS = 12,
    OTBM_TOWN = 13,
    OTBM_HOUSETILE = 14,
    OTBM_WAYPOINTS = 15,
    OTBM_WAYPOINT = 16
};

// Atrybuty OTBM
enum OTBMItemAttribute {
    OTBM_ATTR_DESCRIPTION = 1,
    OTBM_ATTR_EXT_FILE = 2,
    OTBM_ATTR_TILE_FLAGS = 3,
    OTBM_ATTR_ACTION_ID = 4,
    OTBM_ATTR_UNIQUE_ID = 5,
    OTBM_ATTR_TEXT = 6,
    OTBM_ATTR_DESC = 7,
    OTBM_ATTR_TELE_DEST = 8,
    OTBM_ATTR_ITEM = 9,
    OTBM_ATTR_DEPOT_ID = 10,
    OTBM_ATTR_EXT_SPAWN_FILE = 11,
    OTBM_ATTR_RUNE_CHARGES = 12,
    OTBM_ATTR_EXT_HOUSE_FILE = 13,
    OTBM_ATTR_HOUSEDOORID = 14,
    OTBM_ATTR_COUNT = 15,
    OTBM_ATTR_DURATION = 16,
    OTBM_ATTR_DECAYING_STATE = 17,
    OTBM_ATTR_WRITTENDATE = 18,
    OTBM_ATTR_WRITTENBY = 19,
    OTBM_ATTR_SLEEPERGUID = 20,
    OTBM_ATTR_SLEEPSTART = 21,
    OTBM_ATTR_CHARGES = 22
};

// Struktury danych OTBM
#pragma pack(push, 1)
struct OTBMTeleportDest {
    quint16 x;
    quint16 y;
    quint8 z;
};

struct OTBMHouseTile {
    quint8 x;
    quint8 y;
    quint32 houseId;
};

struct OTBMTownTemple {
    quint16 x;
    quint16 y;
    quint8 z;
};
#pragma pack(pop)

class OTBMFile {
public:
    OTBMFile();
    ~OTBMFile();

    bool load(const QString& filename);
    bool save(const QString& filename);

    // Gettery
    quint16 getWidth() const { return width; }
    quint16 getHeight() const { return height; }
    OTBMVersion getVersion() const { return version; }
    QString getDescription() const { return description; }
    QString getSpawnFile() const { return spawnFile; }
    QString getHouseFile() const { return houseFile; }

    // Settery
    void setWidth(quint16 w) { width = w; }
    void setHeight(quint16 h) { height = h; }
    void setVersion(OTBMVersion v) { version = v; }
    void setDescription(const QString& desc) { description = desc; }
    void setSpawnFile(const QString& file) { spawnFile = file; }
    void setHouseFile(const QString& file) { houseFile = file; }

private:
    bool readHeader();
    bool writeHeader();
    bool readMapData();
    bool writeMapData();
    bool readTileArea();
    bool writeTileArea();
    bool readTile();
    bool writeTile();
    bool readItem();
    bool writeItem();

    BinaryFile file;
    OTBMVersion version;
    quint16 width;
    quint16 height;
    QString description;
    QString spawnFile;
    QString houseFile;
    QMap<quint32, QString> towns;
    QVector<OTBMHouseTile> houses;
};

#endif // OTBM_H 