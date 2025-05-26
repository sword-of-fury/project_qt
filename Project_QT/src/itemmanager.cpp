#include "itemmanager.h"
#include "item.h"
#include "spritemanager.h" // Added include
#include <QDebug>
#include <QFile>
#include <QPixmap>
#include <QDir>
#include <QImage>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

ItemManager* ItemManager::instance = nullptr;

ItemManager& ItemManager::getInstance()
{
    if (!instance) {
        instance = new ItemManager();
    }
    return *instance;
}

ItemManager::ItemManager(QObject* parent)
    : QObject(parent)
{
}

ItemManager::~ItemManager()
{
    if (instance == this) {
        instance = nullptr;
    }
    
    // Delete all items
    qDeleteAll(m_itemsById);
    m_itemsById.clear();
    m_itemsByName.clear();
    // m_sprites.clear(); // m_sprites is removed
    m_itemProperties.clear(); // Clear the item properties map
}

Item* ItemManager::getItemById(int id) const
{
    return m_itemsById.value(id, nullptr);
}

Item* ItemManager::getItemByName(const QString& name) const
{
    return m_itemsByName.value(name, nullptr);
}

QList<Item*> ItemManager::getAllItems() const
{
    return m_itemsById.values();
}

int ItemManager::getItemCount() const
{
    return m_itemsById.count();
}

Item* ItemManager::createItem(int id, const QString& name, const QPixmap& sprite)
{
    // Check if item already exists
    if (m_itemsById.contains(id)) {
        qWarning() << "Item with ID" << id << "already exists";
        return m_itemsById[id];
    }
    
    // Get item properties
    const ItemProperties* properties = m_itemProperties.value(id, nullptr);
    if (!properties) {
        qWarning() << "Item properties not found for ID" << id;
        // Optionally create a basic item even without properties
        Item* item = new Item(id, name);
        // Set sprite if provided
        if (!sprite.isNull()) {
            item->setIcon(sprite);
        }
         // Add to maps
        m_itemsById[id] = item;
        m_itemsByName[name] = item;
        emit itemAdded(item);
        return item;
    }

    // Create new item with properties
    Item* item = new Item(id, properties);
    
    // Set sprite if provided (this might be redundant if spriteId is in properties)
    if (!sprite.isNull()) {
        item->setIcon(sprite);
    }
    
    // Add to maps
    m_itemsById[id] = item;
    m_itemsByName[name] = item;
    
    // Emit signal
    emit itemAdded(item);
    
    return item;
}

bool ItemManager::removeItem(int id)
{
    Item* item = getItemById(id);
    if (item) {
        return removeItem(item);
    }
    return false;
}

bool ItemManager::removeItem(Item* item)
{
    if (!item) {
        return false;
    }
    
    int id = item->getId();
    QString name = item->getName();
    
    // Remove from maps
    m_itemsById.remove(id);
    m_itemsByName.remove(name);
    
    // Emit signal
    emit itemRemoved(id);
    
    // Delete item
    delete item;
    
    return true;
}

bool ItemManager::loadItems(const QString& filename)
{
    // Check file extension
    if (filename.endsWith(".json", Qt::CaseInsensitive)) {
        return loadItemsFromJson(filename);
    }
    
    qWarning() << "Unsupported item file format:" << filename;
    emit error(tr("Unsupported item file format: %1").arg(filename));
    return false;
}

bool ItemManager::saveItems(const QString& filename)
{
    // Check file extension
    if (filename.endsWith(".json", Qt::CaseInsensitive)) {
        return saveItemsToJson(filename);
    }
    
    qWarning() << "Unsupported item file format:" << filename;
    emit error(tr("Unsupported item file format: %1").arg(filename));
    return false;
}

bool ItemManager::loadItemsFromJson(const QString& filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Cannot open item file:" << filename;
        emit error(tr("Cannot open item file: %1").arg(filename));
        return false;
    }
    
    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    
    if (doc.isNull() || !doc.isArray()) {
        qWarning() << "Invalid JSON format in item file:" << filename;
        emit error(tr("Invalid JSON format in item file: %1").arg(filename));
        file.close();
        return false;
    }
    
    QJsonArray itemsArray = doc.array();
    
    // Clear existing items
    qDeleteAll(m_itemsById);
    m_itemsById.clear();
    m_itemsByName.clear();
    
    // Load items from JSON
    for (const QJsonValue& value : itemsArray) {
        if (!value.isObject()) {
            continue;
        }
        
        QJsonObject obj = value.toObject();
        
        int id = obj["id"].toInt();
        QString name = obj["name"].toString();
        QString type = obj["type"].toString();
        bool blocking = obj["blocking"].toBool(false);
        bool walkable = obj["walkable"].toBool(true);
        bool collision = obj["collision"].toBool(false);
        
        // Create item properties (if not loading from DAT)
        // If loading from DAT, this part might be different
        ItemProperties props;
        props.id = id;
        props.name = name;
        props.blocking = blocking;
        props.walkable = walkable;
        props.collidable = collision;
        // Initialize other properties from JSON if available
        props.spriteId = obj["spriteId"].toInt(0); // Assuming spriteId is in JSON
        props.flags = obj["flags"].toUInt(0); // Assuming flags are in JSON
        props.weight = obj["weight"].toInt(0); // Assuming weight is in JSON
        props.speed = obj["speed"].toInt(0); // Assuming speed is in JSON
        props.lightLevel = obj["lightLevel"].toInt(0); // Assuming lightLevel is in JSON
        props.lightColor = obj["lightColor"].toInt(0); // Assuming lightColor is in JSON
        props.wareId = obj["wareId"].toInt(0); // Assuming wareId is in JSON
        props.alwaysOnTop = obj["alwaysOnTop"].toInt(0); // Assuming alwaysOnTop is in JSON
        props.alwaysOnTopOrder = obj["alwaysOnTopOrder"].toInt(0); // Assuming alwaysOnTopOrder is in JSON
        props.drawHeight = obj["drawHeight"].toInt(0); // Assuming drawHeight is in JSON
        props.drawOffsetX = obj["drawOffsetX"].toInt(0); // Assuming drawOffsetX is in JSON
        props.drawOffsetY = obj["drawOffsetY"].toInt(0); // Assuming drawOffsetY is in JSON
        props.frames = obj["frames"].toInt(1); // Assuming frames are in JSON, default to 1

        // Load attributes
        if (obj.contains("attributes") && obj["attributes"].isObject()) {
            QJsonObject attrs = obj["attributes"].toObject();
            for (auto it = attrs.begin(); it != attrs.end(); ++it) {
                props.attributes[it.key()] = it.value().toVariant();
            }
        }
        m_itemProperties[id] = props; // Store properties

        // Create item (using the new constructor)
        Item* item = createItem(id, name); // createItem now handles properties lookup

        // Set properties (redundant if createItem uses properties, but keep for now)
        item->setType(type);
        // item->setBlocking(blocking); // Handled by createItem
        // item->setWalkable(walkable); // Handled by createItem
        // item->setCollision(collision); // Handled by createItem

        // Load attributes (redundant if createItem uses properties, but keep for now)
        // if (obj.contains("attributes") && obj["attributes"].isObject()) {
        //     QJsonObject attrs = obj["attributes"].toObject();
        //     for (auto it = attrs.begin(); it != attrs.end(); ++it) {
        //         item->setAttribute(it.key(), it.value().toVariant());
        //     }
        // }
    }
    
    file.close();
    emit itemsLoaded();
    return true;
}

bool ItemManager::saveItemsToJson(const QString& filename)
{
    QJsonArray itemsArray;
    
    // Convert items to JSON
    for (Item* item : m_itemsById.values()) {
        QJsonObject obj;
        obj["id"] = item->getId();
        obj["name"] = item->getName();
        obj["type"] = item->getType();
        obj["blocking"] = item->isBlocking();
        obj["walkable"] = item->isWalkable();
        obj["collision"] = item->isCollidable();
        
        // Save attributes
        QJsonObject attrs;
        for (const QString& key : item->getAttributes().keys()) {
            attrs[key] = QJsonValue::fromVariant(item->getAttribute(key));
        }
        
        if (!attrs.isEmpty()) {
            obj["attributes"] = attrs;
        }
        
        itemsArray.append(obj);
    }
    
    QJsonDocument doc(itemsArray);
    QByteArray data = doc.toJson(QJsonDocument::Indented);
    
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Cannot open file for writing:" << filename;
        emit error(tr("Cannot open file for writing: %1").arg(filename));
        return false;
    }
    
    file.write(data);
    file.close();
    return true;
}

// [REMOVED] bool ItemManager::loadSprites(const QString& directory)
// This function, which loaded individual image files (PNG, JPG) into m_sprites, is removed.
// SpriteManager should be responsible for all sprite/image loading.
// If specific item icons need to be loaded this way, SpriteManager should offer a similar
// function, or ItemManager should request SpriteManager to load them.

// [REMOVED] QPixmap ItemManager::getSprite(int id) const
// The old implementation using m_sprites is removed.
// This function will be re-implemented later to fetch QPixmap from SpriteManager
// based on item's sprite ID from m_itemProperties.
QPixmap ItemManager::getSprite(int id) const
{
    // Placeholder implementation, will be updated.
    // Example:
    // if (m_itemProperties.contains(id)) {
    //    int spriteIdToFetch = m_itemProperties.value(id).spriteId;
    //    return SpriteManager::getInstance()->getSpriteAsPixmap(spriteIdToFetch); // Assuming such a method
    // }
    return QPixmap();
}


// [REMOVED] QPixmap ItemManager::loadSpriteFromFile(const QString& filename)
// This was a helper for the removed loadSprites(directory).

// [ BEGIN REMOVED Tibia sprite loading function implementations ]
// Implementations of:
// - bool ItemManager::loadSprites(const QString& sprPath, const QString& datPath)
// - bool ItemManager::loadSpritesFromDirectory(const QString& directory)
// - bool ItemManager::readTibiaSprHeader(...)
// - bool ItemManager::readTibiaDatHeader(QDataStream& in)
// - bool ItemManager::readDatItem(QDataStream& in, DatItem& item)
// - QImage ItemManager::convertSpriteDataToImage(...)
// - bool ItemManager::loadTibiaSpr(...)
// - QImage ItemManager::decodeSprite(...)
// - QImage ItemManager::decodeSpriteRLE(...)
// - QImage ItemManager::decodeSpritePNG(...)
// - static QString readDatString(QDataStream& in)
// are removed from this file as this functionality is now in SpriteManager
// or will be handled differently.
// [ END REMOVED Tibia sprite loading function implementations ]


// getItemSprite, getItemGameSprite will be modified later to use m_itemProperties and SpriteManager
Sprite* ItemManager::getItemSprite(int id) const
{
    // Use m_itemProperties to get spriteId and fetch from SpriteManager
    if (m_itemProperties.contains(id)) {
        int spriteIdToFetch = m_itemProperties.value(id).spriteId;
        return SpriteManager::getInstance()->getSprite(spriteIdToFetch);
    }
    return nullptr;
}

GameSprite* ItemManager::getItemGameSprite(int id) const
{
    // Use m_itemProperties to get spriteId and fetch GameSprite from SpriteManager
     if (m_itemProperties.contains(id)) {
        int spriteIdToFetch = m_itemProperties.value(id).spriteId;
        return SpriteManager::getInstance()->getSprite(spriteIdToFetch); // Should be getSprite, not getCreatureSprite
    }
    return nullptr;
}

QString ItemManager::getItemName(int id) const
{
    if (items.contains(id)) {
        return items[id].name;
    }
    return QString();
}

bool ItemManager::isItemStackable(int id) const
{
    if (items.contains(id)) {
        return items[id].stackable;
    }
    return false;
}

bool ItemManager::isItemContainer(int id) const
{
    if (items.contains(id)) {
        return items[id].container;
    }
    return false;
}

bool ItemManager::isItemFluidContainer(int id) const
{
    if (items.contains(id)) {
        return items[id].fluidContainer;
    }
    return false;
}

bool ItemManager::saveItems(const QString& filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Cannot open file for writing:" << filename;
        return false;
    }

    QDataStream out(&file);
    out.setByteOrder(QDataStream::LittleEndian);

    // Write OTB header
    out << quint32(0x4F54424D); // 'OTBM' signature
    out << quint32(1); // Version

    // Write items
    for (const auto& item : items) {
        out << item.getId();
        out << item.getName();
        out << item.isStackable();
        out << item.isContainer();
        out << item.isFluidContainer();
        out << item.isBlocking();
        out << item.isWalkable();
        out << item.isCollidable();
        out << item.getWeight();
        out << item.getSpeed();
    }

    file.close();
    return true;
}

QPixmap ItemManager::getItemIcon(int id) const
{
    if (icons.contains(id)) {
        return icons[id];
    }
    return QPixmap();
}

bool ItemManager::hasCollision(int id) const
{
    if (items.contains(id)) {
        return items[id]["collision"].toBool();
    }
    return false;
}

bool ItemManager::isWalkable(int id) const
{
    if (items.contains(id)) {
        return items[id]["walkable"].toBool();
    }
    return true;
}

bool ItemManager::isBlocking(int id) const
{
    if (items.contains(id)) {
        return items[id]["blocking"].toBool();
    }
    return false;
}

// Loads item definitions from Tibia.dat. Sprites are NOT loaded by this function.
// SpriteManager should be loaded with sprites first via SpriteManager::loadSprites.
// This function populates ItemManager::m_itemProperties map.
bool ItemManager::loadTibiaDat(const QString& filename) {
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Cannot open tibia.dat file:" << filename;
        emit error(tr("Cannot open .dat file: %1").arg(filename));
        return false;
    }

    QDataStream in(&file);
    in.setByteOrder(QDataStream::LittleEndian);
    in.setVersion(QDataStream::Qt_5_0); // As used previously

    // Use the internal helper to read the .dat header into m_datHeader
    if (!readTibiaDatHeaderInternal(in, m_datHeader)) {
        qWarning() << "Error reading tibia.dat header in ItemManager";
        emit error(tr("Error reading .dat header: %1").arg(filename));
        file.close();
        return false;
    }

    qInfo() << "Loading" << m_datHeader.items << "item definitions from" << filename;
    
    m_itemProperties.clear(); // Clear previous item definitions

    for (quint32 i = 0; i < m_datHeader.items; ++i) {
        DatItem datItem; // Use ItemManager's own DatItem struct (defined in itemmanager.h)
        if (!readDatItemInternal(in, datItem)) { // Use ItemManager's internal .dat item reader
            qWarning() << "Error reading .dat item definition at index" << i;
            if (in.status() != QDataStream::Ok) {
                 qCritical() << "Stream error in .dat file at item index << " << i << ". Aborting.";
                 file.close();
                 m_itemProperties.clear(); // Don't keep partial data
                 return false;
            }
            continue; // Skip this item if recoverable error
        }

        ItemProperties props;
        props.id = datItem.id; 
        props.name = datItem.name;
        props.spriteId = datItem.spriteId; // This ID will be used to get sprite from SpriteManager
        
        // Populate flags and other properties from datItem
        props.stackable = (datItem.flags & 0x01) != 0; 
        props.container = (datItem.flags & 0x02) != 0; 
        props.fluidContainer = (datItem.flags & 0x04) != 0; // Example flag, adjust if needed
        // Add more flag interpretations as necessary
        
        props.blocking = datItem.blocking != 0;
        props.walkable = datItem.walkable != 0; 
        props.collidable = datItem.collidable != 0;
        props.weight = datItem.weight;
        props.speed = datItem.speed;
        // Copy other relevant fields from datItem to props.attributes if needed

        m_itemProperties[props.id] = props; // Store properties, keyed by client ID from .dat
    }

    file.close();
    qInfo() << "Successfully processed" << m_itemProperties.size() << "item definitions from" << filename;
    emit itemsLoaded(); // Item definitions are loaded
    return true;
}

// Static helper function to read a string from a QDataStream, specific to ItemManager's .dat parsing
QString ItemManager::readDatStringInternal(QDataStream& in) {
    quint16 length;
    in >> length;
    if (in.status() != QDataStream::Ok) {
        qWarning() << "readDatStringInternal: Failed to read string length.";
        return QString();
    }
    if (length == 0) {
        return QString();
    }
    if (length > 2048) { // Sanity check for very long strings
        qWarning() << "readDatStringInternal: String length" << length << "exceeds sanity limit. Skipping.";
        if (!in.device() || !in.device()->isSequential()) { // Check if device supports skipping
            in.skipRawData(length); 
        } else {
            // For sequential devices, must read to skip. Limit read to avoid OOM.
            QByteArray dummy(qMin(length, (quint16)2048), Qt::Uninitialized);
            in.readRawData(dummy.data(), dummy.size());
            if (length > 2048) in.skipRawData(length - 2048); // Skip remaining if any
        }
        return QString(); 
    }
    QByteArray data(length, Qt::Uninitialized);
    int bytesRead = in.readRawData(data.data(), length);
    if (bytesRead != length) {
        qWarning() << "readDatStringInternal: Failed to read string data, expected" << length << "got" << bytesRead;
        return QString(); 
    }
    return QString::fromLatin1(data);
}

// Internal helper to read .dat header, populates the passed DatHeader struct
bool ItemManager::readTibiaDatHeaderInternal(QDataStream& in, DatHeader& header) {
    quint32 signature;
    in >> signature;
    if (in.status() != QDataStream::Ok) {
        qWarning() << "Failed to read .dat signature"; return false;
    }
    header.version = signature; 
    
    in >> header.items >> header.outfits >> header.effects >> header.missiles;
    if (in.status() != QDataStream::Ok) {
        qWarning() << "Failed to read .dat header counts (items, outfits, effects, missiles)."; 
        return false;
    }

    // Increased sanity checks, adjust if necessary for specific .dat versions
    if (header.items > 1000000 || header.outfits > 50000 || header.effects > 50000 || header.missiles > 50000) {
        qWarning() << "Suspiciously high counts in .dat header:"
                   << "Items:" << header.items
                   << "Outfits:" << header.outfits
                   << "Effects:" << header.effects
                   << "Missiles:" << header.missiles;
        // Consider if this should be a fatal error for the specific .dat version being parsed
    }
    return true;
}

// Internal helper to read a single item's data from .dat stream
bool ItemManager::readDatItemInternal(QDataStream& in, DatItem& item) {
    in >> item.id;
    if (in.status() != QDataStream::Ok) { qWarning() << "Failed to read DatItem id"; return false; }

    item.name = ItemManager::readDatStringInternal(in); 
    // Check stream status after reading string, as readDatStringInternal might return QString() on error but not set stream error for all cases
    if (item.name.isNull() && (in.status() != QDataStream::Ok || length > 0) ) { // length condition from readDatStringInternal if it were passed or accessible
        qWarning() << "Error reading or processing item name for ID:" << item.id;
        // Decide if this is fatal. If name is critical and read failed, return false.
        // If an empty name is permissible after a valid zero-length read, this check needs adjustment.
        // For now, assume a null string after trying to read a positive length is an error.
        // This depends on readDatStringInternal's exact error reporting.
        // A robust way is for readDatStringInternal to throw or return a special error indicator.
    }
    
    in >> item.spriteId >> item.flags >> item.weight >> item.speed >> item.lightLevel 
       >> item.lightColor >> item.wareId >> item.alwaysOnTop >> item.alwaysOnTopOrder 
       >> item.blocking >> item.walkable >> item.collidable;
    
    if (in.status() != QDataStream::Ok) { 
        qWarning() << "Failed to read one or more properties for DatItem ID:" << item.id << "after name read.";
        return false; 
    }
    return true;
}

// The following functions are OTB-related or ambiguous legacy functions.
// They are removed or will be handled if OTB processing is confirmed as a requirement.
// bool ItemManager::readTibiaDatHeader(QDataStream& in) { ... } // Ambiguous, removed. Internal version is used for .dat
// bool ItemManager::readTibiaSprHeader(QDataStream& in) { ... } // Ambiguous, removed. SpriteManager handles .spr headers.
// bool ItemManager::readDatItem(QDataStream& in, DatItem& item) { ... } // Ambiguous, removed. Internal version is used for .dat
// bool ItemManager::readSpriteData(QDataStream& in, SpriteData& sprite) { ... } // Obsolete.

bool ItemManager::readItemsOtbHeader(QDataStream& in) {
    // This function seems specific to OTB. Ensure 'otbHeader' member or local struct is used.
    // For now, assuming it's valid if OTB loading is kept.
    // Example:
    // quint32 version, itemCount;
    // in >> version >> itemCount;
    // if (in.status() != QDataStream::Ok) { qWarning("Failed to read OTB header"); return false; }
    // Store version and itemCount in appropriate members if needed.
    qWarning("ItemManager::readItemsOtbHeader called - verify OTB header members and parsing logic.");
    return true; 
}

bool ItemManager::readOtbItem(QDataStream& in, OtbItem& item) // Assumes OtbItem is defined in itemmanager.h

    QDataStream in(&file);
    in.setByteOrder(QDataStream::LittleEndian);

    // Read OTB header
    char header[4];
    in.readRawData(header, 4);
    if (strncmp(header, "OTBI", 4) != 0) {
        qWarning() << "Invalid OTB file format: missing 'OTBI' header";
        file.close();
        return false;
    }

    // Read version
    quint32 version;
    in >> version;
    qDebug() << "Loading OTB Version:" << version;

    // Read item count
    quint32 itemCount;
    in >> itemCount;
    
    if (itemCount > 100000) { // Sanity check
        qWarning() << "Suspicious item count in OTB file:" << itemCount;
        file.close();
        return false;
    }
    
    qDebug() << "Loading" << itemCount << "items from OTB";

    // Clear existing items
    items.clear();
    clientToServerMap.clear();
    serverToClientMap.clear();

    // Read items
    for (quint32 i = 0; i < itemCount; ++i) {
        OtbItem item;
        if (!readOtbItem(in, item)) {
            qWarning() << "Failed to read OTB item" << i;
            continue;
        }

        // Map client to server ID and vice versa
        clientToServerMap[item.clientId] = item.serverId;
        serverToClientMap[item.serverId] = item.clientId;

        // Create and initialize item properties
        ItemProperties props;
        props.id = item.serverId;
        props.name = QString("Item_%1").arg(item.serverId);
        
        // Set basic properties from flags
        props.stackable = (item.flags & 0x01) != 0;
        props.container = (item.attributes & 0x01) != 0;
        props.fluidContainer = (item.attributes & 0x02) != 0;
        
        // Set movement and blocking properties
        props.blocking = (item.attributes & 0x04) != 0;
        props.walkable = (item.attributes & 0x08) == 0; // Inverted logic: if flag is set, item is NOT walkable
        props.collidable = (item.attributes & 0x10) != 0;
        
        // Set default values for other properties
        // These could be loaded from additional attributes in a full implementation
        props.weight = 0;
        props.speed = 0;
        
        // Map client ID to sprite ID (this assumes client ID corresponds to sprite ID)
        props.spriteId = item.clientId;
        
        // Store the item
        items[item.serverId] = props;
        
        // Log some item details for debugging
        if (i < 10) { // Only log first 10 items to avoid spam
            qDebug() << "Loaded item:" << props.id 
                     << "Name:" << props.name 
                     << "Sprite ID:" << props.spriteId
                     << "Stackable:" << props.stackable
                     << "Container:" << props.container;
        }
        
        
        // Update progress every 1000 items
        if (i % 1000 == 0) {
            qDebug() << "Loaded" << i << "of" << itemCount << "items";
        }
    }

    file.close();
    qDebug() << "Successfully loaded" << items.size() << "items from OTB file";
    return !items.isEmpty();
}

bool ItemManager::readTibiaDatHeader(QDataStream& in) {
    in >> datHeader.version >> datHeader.items >> datHeader.sprites;
    return true;
}

bool ItemManager::readTibiaSprHeader(QDataStream& in) {
    in >> sprHeader.version >> sprHeader.sprites;
    return true;
}

bool ItemManager::readItemsOtbHeader(QDataStream& in) {
    in >> otbHeader.version >> otbHeader.items;
    return true;
}

bool ItemManager::readDatItem(QDataStream& in, DatItem& item) {
    in >> item.id >> item.spriteId >> item.flags;
    
    // Wczytaj nazwę
    quint8 nameLength;
    in >> nameLength;
    QByteArray nameData(nameLength, 0);
    in.readRawData(nameData.data(), nameLength);
    item.name = QString::fromLatin1(nameData);

    // Wczytaj dodatkowe właściwości
    in >> item.weight >> item.speed;
    in >> item.lightLevel >> item.lightColor;
    in >> item.wareId;
    in >> item.alwaysOnTop >> item.alwaysOnTopOrder;
    in >> item.blocking >> item.walkable >> item.collidable;

    return true;
}

bool ItemManager::readSpriteData(QDataStream& in, SpriteData& sprite) {
    in >> sprite.offset >> sprite.size;
    
    // Zapisz pozycję
    qint64 pos = in.device()->pos();
    
    // Przejdź do danych sprite'a
    in.device()->seek(sprite.offset);
    
    // Wczytaj dane
    sprite.data.resize(sprite.size);
    in.readRawData(sprite.data.data(), sprite.size);
    
    // Wróć do poprzedniej pozycji
    in.device()->seek(pos);
    
    return true;
}

bool ItemManager::readOtbItem(QDataStream& in, OtbItem& item)
{
    // Read item group
    quint8 group;
    in >> group;
    
    // Read server ID
    in >> item.serverId;
    if (in.status() != QDataStream::Ok) {
        qWarning() << "Error reading server ID";
        return false;
    }
    
    // Read client ID
    in >> item.clientId;
    if (in.status() != QDataStream::Ok) {
        qWarning() << "Error reading client ID for server ID:" << item.serverId;
        return false;
    }
    
    // Read flags
    in >> item.flags;
    
    // Read attributes
    in >> item.attributes;
    
    // Skip the rest of the item data for now
    // In a full implementation, you would read additional attributes here
    // based on the flags and attributes
    
    if (in.status() != QDataStream::Ok) {
        qWarning() << "Error reading item data for server ID:" << item.serverId;
        return false;
    }
    
    return true;
}

QImage ItemManager::decodeSprite(const SpriteData& spriteData) {
    // Sprawdź format sprite'a na podstawie wersji
    if (sprHeader.version >= 0x0200) {
        // Nowy format (PNG)
        return decodeSpritePNG(spriteData.data);
    } else {
        // Stary format (RLE)
        return decodeSpriteRLE(spriteData.data);
    }
}

QImage ItemManager::decodeSpriteRLE(const QByteArray& data) {
    QImage image(32, 32, QImage::Format_ARGB32);
    image.fill(Qt::transparent);

    QDataStream stream(data);
    stream.setByteOrder(QDataStream::LittleEndian);

    for (int y = 0; y < 32; ++y) {
        for (int x = 0; x < 32; ++x) {
            quint8 pixel;
            stream >> pixel;

            if (pixel == 0) {
                // Przezroczysty piksel
                image.setPixel(x, y, qRgba(0, 0, 0, 0));
            } else {
                // Kolorowy piksel
                quint8 r, g, b;
                stream >> r >> g >> b;
                image.setPixel(x, y, qRgba(r, g, b, 255));
            }
        }
    }

    return image;
}

QImage ItemManager::decodeSpritePNG(const QByteArray& data) {
    QImage image;
    image.loadFromData(data, "PNG");
    return image;
}

int ItemManager::getServerId(int clientId) const {
    return clientToServerMap.value(clientId, -1);
}

int ItemManager::getClientId(int serverId) const {
    return serverToClientMap.value(serverId, -1);
}

bool ItemManager::isItemBlocking(int id) const {
    if (items.contains(id)) {
        return items[id].blocking;
    }
    return false;
}

bool ItemManager::isItemWalkable(int id) const {
    if (items.contains(id)) {
        return items[id].walkable;
    }
    return false;
}

bool ItemManager::isItemCollidable(int id) const {
    if (items.contains(id)) {
        return items[id].collidable;
    }
    return false;
}

int ItemManager::getItemWeight(int id) const {
    if (items.contains(id)) {
        return items[id].weight;
    }
    return 0;
}

int ItemManager::getItemSpeed(int id) const {
    if (items.contains(id)) {
        return items[id].speed;
    }
    return 0;
}

bool ItemManager::loadItemPropertiesFromDat(const QString& datPath)
{
    QFile file(datPath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Cannot open item properties file:" << datPath;
        emit error(tr("Cannot open item properties file: %1").arg(datPath));
        return false;
    }

    QDataStream in(&file);
    in.setByteOrder(QDataStream::LittleEndian);

    // Read DAT header (assuming a specific structure based on Tibia DAT)
    SpriteManager::DatHeader datHeader; // Use the struct from SpriteManager
    if (!SpriteManager::getInstance()->readTibiaDatHeader(in)) { // Assuming SpriteManager has a public method for this
         qWarning() << "Failed to read DAT header from" << datPath;
         emit error(tr("Failed to read DAT header from %1").arg(datPath));
         file.close();
         return false;
    }
    // The readTibiaDatHeader in SpriteManager should populate its internal datHeader
    // We might need to access it or pass it here.
    // For now, let's assume we can read item count directly or get it from SpriteManager
    quint32 itemCount = SpriteManager::getInstance()->getItemSpriteMaxID(); // Assuming this gives the count from the loaded DAT

    // Clear existing properties
    m_itemProperties.clear();

    // Read item properties
    for (quint16 i = 0; i < itemCount; ++i) {
        SpriteManager::DatItem datItem; // Use the struct from SpriteManager
        if (!SpriteManager::getInstance()->readDatItem(in, datItem)) { // Assuming SpriteManager has a public method for this
            qWarning() << "Failed to read item data for item index" << i << " from" << datPath;
            emit error(tr("Failed to read item data for item index %1 from %2").arg(i).arg(datPath));
            // Continue loading other items or return false?
            // For now, let's continue but log the error.
            continue;
        }

        // Map DatItem to ItemProperties
        ItemProperties props;
        props.id = datItem.id; // Assuming DatItem has an ID field
        props.name = datItem.name; // Assuming DatItem has a name field
        props.spriteId = datItem.spriteId; // Assuming DatItem has a spriteId field
        props.flags = datItem.flags; // Assuming DatItem has a flags field
        props.stackable = (datItem.flags & Item::ItemProperty::IsStackable); // Map flags
        props.container = (datItem.flags & Item::ItemProperty::IsContainer); // Map flags
        props.fluidContainer = (datItem.flags & Item::ItemProperty::IsFluidContainer); // Map flags
        props.blocking = (datItem.flags & Item::ItemProperty::IsBlockingPath); // Map flags
        props.walkable = (datItem.flags & Item::ItemProperty::IsGroundTile); // Map flags (assuming walkable is related to ground tile flag)
        props.collidable = (datItem.flags & Item::ItemProperty::IsBlockingPath); // Map flags (assuming collidable is related to blocking path)
        props.weight = datItem.weight; // Assuming DatItem has a weight field
        props.speed = datItem.speed; // Assuming DatItem has a speed field
        props.lightLevel = datItem.lightLevel; // Assuming DatItem has light fields
        props.lightColor = datItem.lightColor; // Assuming DatItem has light fields
        props.wareId = datItem.wareId; // Assuming DatItem has wareId
        props.alwaysOnTop = datItem.alwaysOnTop; // Assuming DatItem has alwaysOnTop
        props.alwaysOnTopOrder = datItem.alwaysOnTopOrder; // Assuming DatItem has alwaysOnTopOrder

        // Get draw properties from GameSprite (requires sprites to be loaded first)
        GameSprite* gameSprite = SpriteManager::getInstance()->getSprite(props.spriteId); // Use getSprite
        if (gameSprite) {
            props.drawHeight = gameSprite->getDrawHeight();
            QPair<int, int> offset = gameSprite->getDrawOffset();
            props.drawOffsetX = offset.first;
            props.drawOffsetY = offset.second;
            props.frames = gameSprite->frames; // Get frames for animation
        } else {
            props.drawHeight = 0;
            props.drawOffsetX = 0;
            props.drawOffsetY = 0;
            props.frames = 1; // Default to 1 frame if sprite not found
        }

        // Attributes from DAT are not typically stored in a map like JSON attributes
        // If there are specific DAT attributes, they should be added as fields to ItemProperties

        m_itemProperties[props.id] = props; // Store properties by item ID
    }

    file.close();
    qDebug() << "Loaded" << m_itemProperties.count() << "item properties from" << datPath;
    return true;
}