#ifndef ITEMMANAGER_H
#define ITEMMANAGER_H

#include <QObject>
#include <QMap>
#include <QPixmap>
#include <QString>
#include <QList>
#include <QFile>
#include <QDataStream>
#include <QImage>
#include <QDebug>
#include <QDir>
#include "spritemanager.h" // Added include

class Item;

class SpriteManager; // Forward declaration

class ItemManager : public QObject
{
    Q_OBJECT

public:
    static ItemManager& getInstance();
    ~ItemManager();

    // Item access methods
    Item* getItemById(int id) const;
    Item* getItemByName(const QString& name) const;
    QList<Item*> getAllItems() const;
    int getItemCount() const;
    
    // Item loading and saving
    bool loadItems(const QString& filename);
    bool saveItems(const QString& filename);
    
    // Item creation and management
    Item* createItem(int id, const QString& name, const QPixmap& sprite = QPixmap()); // Sprite here is for icon on Item obj
    bool removeItem(int id);
    bool removeItem(Item* item);
    
    // Sprite management is now primarily handled by SpriteManager.
    // ItemManager will get sprites from SpriteManager based on sprite IDs in item definitions.
    // QPixmap getSprite(int id) const; // Will be re-added, modified to use SpriteManager

    // Method to load item definitions from Tibia .dat files
    bool loadTibiaDat(const QString& filename);
    // bool loadItemsOtb(const QString& filename); // If OTB support is needed

signals:
    void itemsLoaded();
    // void spritesLoaded(); // Removed, SpriteManager handles this
    void itemAdded(Item* item);
    void itemRemoved(int id);
    void error(const QString& message);

private:
    explicit ItemManager(QObject* parent = nullptr);
    ItemManager(const ItemManager&) = delete;
    ItemManager& operator=(const ItemManager&) = delete;
    
    // Helper methods
    bool loadItemsFromJson(const QString& filename);
    bool saveItemsToJson(const QString& filename);
    // QPixmap loadSpriteFromFile(const QString& filename); // Removed, was helper for removed loadSprites

    // Tibia-specific data structures for parsing .dat files within ItemManager
    struct DatHeader {
        quint32 version;
        quint32 items;
        quint32 outfits;
        quint32 effects;
        quint32 missiles;
    };
    DatHeader m_datHeader; // Member to store .dat header info

    struct DatItem { // Structure for .dat item properties
        quint16 id;       // Client item ID
        QString name;
        quint16 spriteId; // ID used by SpriteManager
        quint8 flags;
        quint8 weight;
        quint8 speed;
        quint8 lightLevel;
        quint8 lightColor;
        quint16 wareId;
        quint8 alwaysOnTop;
        quint8 alwaysOnTopOrder;
        quint8 blocking;
        quint8 walkable;
        quint8 collidable;
    };

    // Internal helpers for .dat parsing (declarations if not static in .cpp)
    bool readTibiaDatHeaderInternal(QDataStream& in, DatHeader& header);
    bool readDatItemInternal(QDataStream& in, DatItem& item);
    static QString readDatStringInternal(QDataStream& in);


    struct ItemProperties {
        quint16 id;
        QString name;
        quint16 spriteId;   // Sprite ID to fetch from SpriteManager
        quint32 flags; // Combined flags from DAT
        bool stackable;
        bool container;
        bool fluidContainer;
        bool blocking;
        bool walkable;
        bool collidable;
        int weight;
        int speed;
        quint8 lightLevel;
        quint8 lightColor;
        quint16 wareId;
        quint8 alwaysOnTop;
        quint8 alwaysOnTopOrder;
        // Add draw properties from GameSprite if needed here, or access via GameSprite*
        uint16_t drawHeight; // Example: from GameSprite
        uint16_t drawOffsetX; // Example: from GameSprite
        uint16_t drawOffsetY; // Example: from GameSprite
        uint8_t frames; // Example: from GameSprite animation

        QMap<QString, QVariant> attributes; // For JSON items
    };
    QMap<quint16, ItemProperties> m_itemProperties; // Stores all item definitions

    // OTB related structures & map (if OTB loading is handled by ItemManager)
    struct OtbItem { // Example, if OTB is handled
        quint16 serverId;
        quint16 clientId; 
        quint8 flags;
        quint16 otbAttributes; 
    };
    QMap<int, int> m_clientToServerMap; 
    QMap<int, int> m_serverToClientMap;

    // Data storage for actual Item objects
    QMap<int, Item*> m_itemsById;
    QMap<QString, Item*> m_itemsByName;
    // QMap<int, QPixmap> m_sprites; // Removed
    
    static ItemManager* instance;

    // Load item properties from .dat file
    bool loadItemPropertiesFromDat(const QString& datPath);

    // getItemSprite, getItemGameSprite will be modified later to use m_itemProperties and SpriteManager
    Sprite* getItemSprite(int id) const;
    GameSprite* getItemGameSprite(int id) const;

protected:
    // ...existing code...
};

#endif // ITEMMANAGER_H