#ifndef OTB_H
#define OTB_H

#include "binaryfile.h"
#include <QMap>
#include <QString>

// Atrybuty przedmiotów
enum OTBItemAttribute {
    OTB_ATTR_NONE = 0,
    OTB_ATTR_FIRST = 1,
    OTB_ATTR_SERVERID = OTB_ATTR_FIRST,
    OTB_ATTR_CLIENTID,
    OTB_ATTR_SPEED,
    OTB_ATTR_SLOT,
    OTB_ATTR_MAXITEMS,
    OTB_ATTR_WEIGHT,
    OTB_ATTR_WEAPON,
    OTB_ATTR_AMU,
    OTB_ATTR_ARMOR,
    OTB_ATTR_MAGLEVEL,
    OTB_ATTR_MAGFIELDTYPE,
    OTB_ATTR_WRITEABLE,
    OTB_ATTR_ROTATETO,
    OTB_ATTR_DECAY,
    OTB_ATTR_SPRITEHASH,
    OTB_ATTR_MINIMAPCOLOR,
    OTB_ATTR_07,
    OTB_ATTR_08,
    OTB_ATTR_LIGHT,
    OTB_ATTR_DECAY2,
    OTB_ATTR_WEAPON2,
    OTB_ATTR_AMU2,
    OTB_ATTR_ARMOR2,
    OTB_ATTR_WRITEABLE2,
    OTB_ATTR_LIGHT2,
    OTB_ATTR_TOPORDER,
    OTB_ATTR_WRITEABLE3,
    OTB_ATTR_WAREID,
    OTB_ATTR_LAST = OTB_ATTR_WAREID
};

// Typy przedmiotów
enum OTBItemType {
    OTB_ITEM_TYPE_NONE = 0,
    OTB_ITEM_TYPE_DEPOT = 1,
    OTB_ITEM_TYPE_MAILBOX = 2,
    OTB_ITEM_TYPE_TRASHHOLDER = 3,
    OTB_ITEM_TYPE_CONTAINER = 4,
    OTB_ITEM_TYPE_DOOR = 5,
    OTB_ITEM_TYPE_MAGICWALL = 6,
    OTB_ITEM_TYPE_TELEPORT = 7,
    OTB_ITEM_TYPE_BED = 8,
    OTB_ITEM_TYPE_KEY = 9,
    OTB_ITEM_TYPE_RUNE = 10,
    OTB_ITEM_TYPE_LAST = OTB_ITEM_TYPE_RUNE
};

// Flagi przedmiotów
enum OTBItemFlag {
    OTB_ITEM_FLAG_NONE = 0,
    OTB_ITEM_FLAG_BLOCK_SOLID = 1 << 0,
    OTB_ITEM_FLAG_BLOCK_PROJECTILE = 1 << 1,
    OTB_ITEM_FLAG_BLOCK_PATHFIND = 1 << 2,
    OTB_ITEM_FLAG_HAS_HEIGHT = 1 << 3,
    OTB_ITEM_FLAG_USEABLE = 1 << 4,
    OTB_ITEM_FLAG_PICKUPABLE = 1 << 5,
    OTB_ITEM_FLAG_MOVEABLE = 1 << 6,
    OTB_ITEM_FLAG_STACKABLE = 1 << 7,
    OTB_ITEM_FLAG_FLOORCHANGEDOWN = 1 << 8,
    OTB_ITEM_FLAG_FLOORCHANGENORTH = 1 << 9,
    OTB_ITEM_FLAG_FLOORCHANGEEAST = 1 << 10,
    OTB_ITEM_FLAG_FLOORCHANGESOUTH = 1 << 11,
    OTB_ITEM_FLAG_FLOORCHANGEWEST = 1 << 12,
    OTB_ITEM_FLAG_ALWAYSONTOP = 1 << 13,
    OTB_ITEM_FLAG_READABLE = 1 << 14,
    OTB_ITEM_FLAG_ROTATABLE = 1 << 15,
    OTB_ITEM_FLAG_HANGABLE = 1 << 16,
    OTB_ITEM_FLAG_VERTICAL = 1 << 17,
    OTB_ITEM_FLAG_HORIZONTAL = 1 << 18,
    OTB_ITEM_FLAG_CANNOTDECAY = 1 << 19,
    OTB_ITEM_FLAG_ALLOWDISTREAD = 1 << 20,
    OTB_ITEM_FLAG_UNUSED = 1 << 21,
    OTB_ITEM_FLAG_CLIENTCHARGES = 1 << 22,
    OTB_ITEM_FLAG_LOOKTHROUGH = 1 << 23,
    OTB_ITEM_FLAG_ANIMATION = 1 << 24,
    OTB_ITEM_FLAG_FULLTILE = 1 << 25,
    OTB_ITEM_FLAG_FORCEUSE = 1 << 26
};

struct OTBItem {
    quint16 serverId;
    quint16 clientId;
    quint8 type;
    quint32 flags;
    QMap<OTBItemAttribute, QVariant> attributes;
};

class OTBFile {
public:
    OTBFile();
    ~OTBFile();

    bool load(const QString& filename);
    bool save(const QString& filename);

    // Gettery
    quint32 getMajorVersion() const { return majorVersion; }
    quint32 getMinorVersion() const { return minorVersion; }
    QMap<quint16, OTBItem> getItems() const { return items; }

    // Settery
    void setMajorVersion(quint32 version) { majorVersion = version; }
    void setMinorVersion(quint32 version) { minorVersion = version; }
    void setItems(const QMap<quint16, OTBItem>& i) { items = i; }

private:
    bool readHeader();
    bool writeHeader();
    bool readItem(OTBItem& item);
    bool writeItem(const OTBItem& item);

    BinaryFile file;
    quint32 majorVersion;
    quint32 minorVersion;
    QMap<quint16, OTBItem> items;
};

#endif // OTB_H 