#include "otbm.h"
#include <QDebug>

OTBMFile::OTBMFile() : version(OTBM_4), width(0), height(0) {
}

OTBMFile::~OTBMFile() {
    file.close();
}

bool OTBMFile::load(const QString& filePath) {
    file.setFileName(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Failed to open file for reading:" << file.errorString();
        return false;
    }

    // Read OTBM signature
    quint32 signature;
    if (!file.readU32(signature) || signature != OTBM_SIGNATURE) {
        qDebug() << "Invalid OTBM signature.";
        file.close();
        return false;
    }

    // Read OTBM version
    quint32 versionNum;
    if (!file.readU32(versionNum)) {
        qDebug() << "Failed to read OTBM version.";
        file.close();
        return false;
    }
    version = static_cast<OTBMVersion>(versionNum);

    // Read map data
    if (!readMapData()) {
        file.close();
        return false;
    }

    file.close();
    return true;
}

bool OTBMFile::save(const QString& filePath) {
    file.setFileName(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "Failed to open file for writing:" << file.errorString();
        return false;
    }

    // Write OTBM header
    if (!file.writeU32(OTBM_SIGNATURE) || !file.writeU32(OTBM_VERSION)) {
        file.close();
        return false;
    }

    // Write map data
    if (!writeMapData()) {
        file.close();
        return false;
    }

    file.close();
    return true;
}

bool OTBMFile::writeHouses() {
    // Write houses node
    if (!houses.isEmpty()) {
        if (!file.writeU8(OTBM_HOUSES)) {
            return false;
        }

        // Write each house
        for (auto it = houses.begin(); it != houses.end(); ++it) {
            if (!file.writeU8(OTBM_HOUSE) || !file.writeU32(it.key()) || !file.writeString(it.value().name) || !file.writePosition(it.value().position)) {
                return false;
            }
        }

        // End of houses node
        if (!file.writeU8(0xFF)) {
            return false;
        }
    }

    return true;
}

bool OTBMFile::writeHeader() {
    if (!file.writeU32(version)) {
        return false;
    }

    if (!file.writeU16(width) || !file.writeU16(height)) {
        return false;
    }

    // Zapisujemy wersję items.otb (domyślnie 1.0)
    if (!file.writeU32(1) || !file.writeU32(0)) {
        return false;
    }

    return true;
}

bool OTBMFile::readHeader() {
    quint32 versionNum;
    if (!file.readU32(versionNum)) {
        return false;
    }
    version = static_cast<OTBMVersion>(versionNum);

    if (!file.readU16(width) || !file.readU16(height)) {
        return false;
    }

    // Pomijamy wersję items.otb
    quint32 majorVersion, minorVersion;
    if (!file.readU32(majorVersion) || !file.readU32(minorVersion)) {
        return false;
    }

    return true;
}

bool OTBMFile::writeMapData() {
    // Write map data node
    if (!file.writeU8(OTBM_MAP_DATA)) {
        return false;
    }

    // Write map dimensions (width, height, depth)
    // Assuming Map::getInstance().getSize() returns a QSize with width and height
    // Assuming depth is always 16 for OTBM
    if (!file.writeU16(Map::getInstance().getSize().width()) ||
        !file.writeU16(Map::getInstance().getSize().height()) ||
        !file.writeU8(Map::LayerCount)) { // Depth is always 16 in OTBM
        qDebug() << "Failed to write map dimensions";
        return false;
    }

    // Write map attributes (description, spawn file, house file)
    if (!description.isEmpty()) {
        if (!file.writeU8(OTBM_ATTR_DESCRIPTION) || !file.writeString(description)) {
            return false;
        }
    }

    if (!spawnFile.isEmpty()) {
        if (!file.writeU8(OTBM_ATTR_EXT_SPAWN_FILE) || !file.writeString(spawnFile)) {
            return false;
        }
    }

    if (!houseFile.isEmpty()) {
        if (!file.writeU8(OTBM_ATTR_EXT_HOUSE_FILE) || !file.writeString(houseFile)) {
            return false;
        }
    }

    // Write tile areas
    if (!writeTileArea()) {
        return false;
    }

    // Write towns
    if (!writeTowns()) {
        return false;
    }

    // Write waypoints
    if (!writeWaypoints()) {
        return false;
    }

    // Write houses
    if (!writeHouses()) {
        return false;
    }

    // End of map data node
    if (!file.writeU8(0xFF)) {
        return false;
    }

    return true;
}

bool OTBMFile::writeWaypoints() {
    // Write waypoints node
    if (!waypoints.isEmpty()) {
        if (!file.writeU8(OTBM_WAYPOINTS)) {
            return false;
        }

        // Write each waypoint
        for (auto it = waypoints.begin(); it != waypoints.end(); ++it) {
            if (!file.writeU8(OTBM_WAYPOINT) || !file.writeString(it.key()) || !file.writePosition(it.value())) {
                return false;
            }
        }

        // End of waypoints node
        if (!file.writeU8(0xFF)) {
            return false;
        }
    }

    return true;
}

bool OTBMFile::writeTowns() {
    // Write towns node
    if (!towns.isEmpty()) {
        if (!file.writeU8(OTBM_TOWNS)) {
            return false;
        }

        // Write each town
        for (auto it = towns.begin(); it != towns.end(); ++it) {
            if (!file.writeU8(OTBM_TOWN) || !file.writeU32(it.key()) || !file.writeString(it.value())) {
                return false;
            }
        }

        // End of towns node
        if (!file.writeU8(0xFF)) {
            return false;
        }
    }

    return true;
}

bool OTBMFile::writeTile(const Tile& tile, quint8 relativeX, quint8 relativeY) {
    // Write relative X and Y coordinates
    if (!file.writeU8(relativeX) || !file.writeU8(relativeY)) {
        return false;
    }

    // Write tile properties (if any) - TODO

    // Iterate through items on the tile and write each one
    // Assuming Tile::getItems() returns a QList<Item*>
    const QList<Item*>& items = tile.getItems();
    for (const Item* item : items) {
        if (!file.writeU8(OTBM_ITEM)) { // Start item node
            return false;
        }
        if (!writeItem(*item)) { // Write the item data
            return false;
        }
    }

    // End of tile node
    if (!file.writeU8(0xFF)) {
        return false;
    }

    return true;
}

bool OTBMFile::writeTileArea() {
    // Iterate through 8x8x16 blocks of the map
    for (quint16 z = 0; z < Map::LayerCount; ++z) {
        for (quint16 baseY = 0; baseY < Map::getInstance().getSize().height(); baseY += 8) {
            for (quint16 baseX = 0; baseX < Map::getInstance().getSize().width(); baseX += 8) {
                bool areaHasTiles = false;
                // Check if this 8x8x16 area contains any tiles with items
                for (quint16 y = 0; y < 8; ++y) {
                    for (quint16 x = 0; x < 8; ++x) {
                        quint16 tileX = baseX + x;
                        quint16 tileY = baseY + y;
                        if (tileX < Map::getInstance().getSize().width() && tileY < Map::getInstance().getSize().height()) {
                            // Assuming getTile method in Map returns a Tile object or nullptr
                            Tile* tile = Map::getInstance().getTile(QPoint(tileX, tileY), z); // Assuming getTile takes z
                            if (tile && !tile->getItems().isEmpty()) { // Assuming getItems() returns a list of items
                                areaHasTiles = true;
                                break;
                            }
                        }
                    }
                    if (areaHasTiles) break;
                }
                if (!areaHasTiles) continue; // Skip empty areas

                // Write the tile area node
                if (!file.writeU8(OTBM_TILE_AREA)) {
                    return false;
                }
                if (!file.writeU16(baseX) || !file.writeU16(baseY) || !file.writeU8(z)) {
                    return false;
                }

                // Write tiles within this area
                for (quint8 y = 0; y < 8; ++y) { // Use quint8 for relative coordinates
                    for (quint8 x = 0; x < 8; ++x) { // Use quint8 for relative coordinates
                        quint16 tileX = baseX + x;
                        quint16 tileY = baseY + y;
                        if (tileX < Map::getInstance().getSize().width() && tileY < Map::getInstance().getSize().height()) {
                            Tile* tile = Map::getInstance().getTile(QPoint(tileX, tileY), z); // Assuming getTile takes z
                            if (tile && !tile->getItems().isEmpty()) {
                                if (!file.writeU8(OTBM_TILE)) { // Start tile node
                                    return false;
                                }
                                if (!writeTile(*tile, x, y)) { // Pass relative x and y
                                    return false;
                                }
                            }
                        }
                    }
                }

                // End of tile area node
                if (!file.writeU8(0xFF)) {
                    return false;
                }
            }
        }
    }

    return true;
}

bool OTBMFile::readTile(quint16 baseX, quint16 baseY, quint8 baseZ) { // Added parameters
    quint8 x, y;
    if (!file.readU8(x) || !file.readU8(y)) {
        return false;
    }

    quint16 tileX = baseX + x;
    quint16 tileY = baseY + y;
    quint8 tileZ = baseZ;

    // TODO: Create or get the Tile object at (tileX, tileY, tileZ)
    // For now, just read items within this tile node

    quint8 nodeType;
    while (file.peekU8(nodeType) && nodeType != 0xFF) { // Read until end of node
        if (nodeType == OTBM_ITEM) {
            if (!readItem(tileX, tileY, tileZ)) { // Pass tile coordinates to readItem
                return false;
            }
        } else {
            qDebug() << "Nieznany typ węzła w kafelku:" << nodeType;
            return false;
        }
        if (!file.readU8(nodeType)) return false; // Consume the node type byte
    }

    return true;
}

bool OTBMFile::readItem(quint16 tileX, quint16 tileY, quint8 tileZ) {
    quint16 itemId;
    if (!file.readU16(itemId)) {
        return false;
    }

    // Create an Item object with itemId
    Item item(itemId);

    quint8 nodeType;
    while (file.peekU8(nodeType) && nodeType != 0xFF) { // Read until end of node
        quint8 attribute;
        if (!file.readU8(attribute)) return false; // Consume the attribute type byte
        quint16 size;
        if (!file.readU16(size)) return false;

        switch (attribute) {
            case OTBM_ATTR_COUNT:
            {
                quint8 count;
                if (!file.readU8(count)) return false;
                item.setAttribute("count", count);
                break;
            }
            case OTBM_ATTR_ACTION_ID:
            {
                quint16 actionId;
                if (!file.readU16(actionId)) return false;
                item.setAttribute("actionid", actionId);
                break;
            }
            case OTBM_ATTR_UNIQUE_ID:
            {
                quint16 uniqueId;
                if (!file.readU16(uniqueId)) return false;
                item.setAttribute("uid", uniqueId);
                break;
            }
            case OTBM_ATTR_TEXT:
            {
                QString text;
                if (!file.readString(text)) return false;
                item.setAttribute("text", text);
                break;
            }
            case OTBM_ATTR_DESC:
            {
                QString desc;
                if (!file.readString(desc)) return false;
                item.setAttribute("description", desc);
                break;
            }
            case OTBM_ATTR_TELE_DEST:
            {
                OTBMTeleportDest dest;
                if (!file.readU16(dest.x) || !file.readU16(dest.y) || !file.readU8(dest.z)) return false;
                item.setAttribute("teleport_dest_x", dest.x);
                item.setAttribute("teleport_dest_y", dest.y);
                item.setAttribute("teleport_dest_z", dest.z);
                break;
            }
            case OTBM_ATTR_DEPOT_ID:
            {
                quint16 depotId;
                if (!file.readU16(depotId)) return false;
                item.setAttribute("depot_id", depotId);
                break;
            }
            case OTBM_ATTR_RUNE_CHARGES:
            {
                quint16 charges;
                if (!file.readU16(charges)) return false;
                item.setAttribute("rune_charges", charges);
                break;
            }
            case OTBM_ATTR_HOUSEDOORID:
            {
                quint32 houseDoorId;
                if (!file.readU32(houseDoorId)) return false;
                item.setAttribute("house_door_id", houseDoorId);
                break;
            }
            case OTBM_ATTR_COUNT:
            {
                quint8 count;
                if (!file.readU8(count)) return false;
                item.setAttribute("count", count);
                break;
            }
            case OTBM_ATTR_DURATION:
            {
                quint32 duration;
                if (!file.readU32(duration)) return false;
                item.setAttribute("duration", duration);
                break;
            }
            case OTBM_ATTR_DECAYING_STATE:
            {
                quint8 state;
                if (!file.readU8(state)) return false;
                item.setAttribute("decaying_state", state);
                break;
            }
            case OTBM_ATTR_WRITTENDATE:
            {
                quint32 date;
                if (!file.readU32(date)) return false;
                item.setAttribute("written_date", date);
                break;
            }
            case OTBM_ATTR_WRITTENBY:
            {
                QString writtenBy;
                if (!file.readString(writtenBy)) return false;
                item.setAttribute("written_by", writtenBy);
                break;
            }
            case OTBM_ATTR_SLEEPERGUID:
            {
                quint32 guid;
                if (!file.readU32(guid)) return false;
                item.setAttribute("sleeper_guid", guid);
                break;
            }
            case OTBM_ATTR_SLEEPSTART:
            {
                quint32 start;
                if (!file.readU32(start)) return false;
                item.setAttribute("sleep_start", start);
                break;
            }
            case OTBM_ATTR_CHARGES:
            {
                quint16 charges;
                if (!file.readU16(charges)) return false;
                item.setAttribute("charges", charges);
                break;
            }
            default:
                qDebug() << "Nieznany atrybut przedmiotu:" << attribute;
                // Skip unknown attribute data
                if (!file.skip(size)) return false;
                break;
        }
         if (!file.readU8(nodeType)) return false; // Consume the node type byte
    }

    // Add the created item to the Map at (tileX, tileY, tileZ)
    // Need to determine the correct layer for the item based on its type/properties.
    // For now, adding to a default layer (e.g., Layer::Object)
    // Map::getInstance().addItem(tileX, tileY, Layer::Object, item);

    // Determine the layer based on item properties (requires ItemManager)
    Layer::Type layer = Layer::Object; // Default to Object layer
    ItemProperties props = ItemManager::getInstance()->getItem(itemId);

    // Basic layer determination based on common item properties
    if (props.isGround()) { // Assuming isGround() method exists or can be derived
        layer = Layer::Ground;
    } else if (props.isBorder()) { // Assuming isBorder() method exists or can be derived
        layer = Layer::Border;
    } else if (props.isWall()) { // Assuming isWall() method exists or can be derived
        layer = Layer::Wall;
    } else if (props.isCreature()) { // Assuming isCreature() method exists or can be derived
        layer = Layer::Creature;
    } else if (props.isBlocking()) {
        layer = Layer::Wall; // Or WallDetail, depending on specific item
    } else if (props.isWalkable()) {
        layer = Layer::GroundDetail; // Or ObjectDetail
    }
    // Add more specific checks if needed based on item ID ranges or other properties

    Map::getInstance().addItem(tileX, tileY, layer, item);

    return true;
}

bool OTBMFile::writeItem(const Item& item) {
    if (!file.writeU16(item.getId())) {
        return false;
    }

    // Write item attributes
    QMap<QString, QVariant> attributes = item.getAttributes(); // Assuming getAttributes() method exists in Item class

    if (attributes.contains("count")) {
        if (!file.writeU8(OTBM_ATTR_COUNT) || !file.writeU16(1) || !file.writeU8(attributes.value("count").toUInt())) return false;
    }
    if (attributes.contains("actionid")) {
        if (!file.writeU8(OTBM_ATTR_ACTION_ID) || !file.writeU16(2) || !file.writeU16(attributes.value("actionid").toUInt())) return false;
    }
    if (attributes.contains("uid")) {
        if (!file.writeU8(OTBM_ATTR_UNIQUE_ID) || !file.writeU16(2) || !file.writeU16(attributes.value("uid").toUInt())) return false;
    }
    if (attributes.contains("text")) {
        QString text = attributes.value("text").toString();
        if (!file.writeU8(OTBM_ATTR_TEXT) || !file.writeU16(text.length()) || !file.writeString(text)) return false;
    }
    if (attributes.contains("description")) {
        QString desc = attributes.value("description").toString();
        if (!file.writeU8(OTBM_ATTR_DESC) || !file.writeU16(desc.length()) || !file.writeString(desc)) return false;
    }
    if (attributes.contains("teleport_dest_x") && attributes.contains("teleport_dest_y") && attributes.contains("teleport_dest_z")) {
        if (!file.writeU8(OTBM_ATTR_TELE_DEST) || !file.writeU16(5) ||
            !file.writeU16(attributes.value("teleport_dest_x").toUInt()) ||
            !file.writeU16(attributes.value("teleport_dest_y").toUInt()) ||
            !file.writeU8(attributes.value("teleport_dest_z").toUInt())) return false;
    }
    if (attributes.contains("depot_id")) {
        if (!file.writeU8(OTBM_ATTR_DEPOT_ID) || !file.writeU16(2) || !file.writeU16(attributes.value("depot_id").toUInt())) return false;
    }
    if (attributes.contains("rune_charges")) {
        if (!file.writeU8(OTBM_ATTR_RUNE_CHARGES) || !file.writeU16(2) || !file.writeU16(attributes.value("rune_charges").toUInt())) return false;
    }
    if (attributes.contains("house_door_id")) {
        if (!file.writeU8(OTBM_ATTR_HOUSEDOORID) || !file.writeU16(4) || !file.writeU32(attributes.value("house_door_id").toUInt())) return false;
    }
    if (attributes.contains("duration")) {
        if (!file.writeU8(OTBM_ATTR_DURATION) || !file.writeU16(4) || !file.writeU32(attributes.value("duration").toUInt())) return false;
    }
    if (attributes.contains("decaying_state")) {
        if (!file.writeU8(OTBM_ATTR_DECAYING_STATE) || !file.writeU16(1) || !file.writeU8(attributes.value("decaying_state").toUInt())) return false;
    }
    if (attributes.contains("written_date")) {
        if (!file.writeU8(OTBM_ATTR_WRITTENDATE) || !file.writeU16(4) || !file.writeU32(attributes.value("written_date").toUInt())) return false;
    }
    if (attributes.contains("written_by")) {
        QString writtenBy = attributes.value("written_by").toString();
        if (!file.writeU8(OTBM_ATTR_WRITTENBY) || !file.writeU16(writtenBy.length()) || !file.writeString(writtenBy)) return false;
    }
    if (attributes.contains("sleeper_guid")) {
        if (!file.writeU8(OTBM_ATTR_SLEEPERGUID) || !file.writeU16(4) || !file.writeU32(attributes.value("sleeper_guid").toUInt())) return false;
    }
    if (attributes.contains("sleep_start")) {
        if (!file.writeU8(OTBM_ATTR_SLEEPSTART) || !file.writeU16(4) || !file.writeU32(attributes.value("sleep_start").toUInt())) return false;
    }
    if (attributes.contains("charges")) {
        if (!file.writeU8(OTBM_ATTR_CHARGES) || !file.writeU16(2) || !file.writeU16(attributes.value("charges").toUInt())) return false;
    }

    // End of item node
    if (!file.writeU8(0xFF)) {
        return false;
    }

    return true;
}

bool OTBMFile::readMapData() {
    quint8 nodeType;
    if (!file.readU8(nodeType) || nodeType != OTBM_MAP_DATA) {
        qDebug() << "Expected OTBM_MAP_DATA node, but got:" << nodeType;
        return false;
    }

    // Read map dimensions (width, height, depth)
    quint16 mapWidth, mapHeight;
    quint8 mapDepth; // Depth is always 16 in OTBM
    if (!file.readU16(mapWidth) || !file.readU16(mapHeight) || !file.readU8(mapDepth)) {
        qDebug() << "Failed to read map dimensions.";
        return false;
    }

    // Resize the map in Map class
    Map::getInstance().resize(mapWidth, mapHeight);

    // Read map attributes, tile areas, towns, houses, waypoints
    while (file.peekU8(nodeType) && nodeType != 0xFF) { // Read until end of node
        if (!file.readU8(nodeType)) return false; // Consume the node type byte

        switch (nodeType) {
            case OTBM_ATTR_DESCRIPTION:
                if (!file.readString(description)) return false;
                break;
            case OTBM_ATTR_EXT_SPAWN_FILE:
                if (!file.readString(spawnFile)) return false;
                break;
            case OTBM_ATTR_EXT_HOUSE_FILE:
                if (!file.readString(houseFile)) return false;
                break;
            case OTBM_TILE_AREA:
                if (!readTileArea()) return false;
                break;
            case OTBM_TOWNS:
                // Read towns
                while (file.peekU8(nodeType) && nodeType == OTBM_TOWN) {
                    if (!file.readU8(nodeType)) return false; // Consume OTBM_TOWN node type
                    quint32 townId;
                    QString townName;
                    OTBMTownTemple temple; // Temple position is not stored in OTBMFile, but read to consume bytes
                    if (!file.readU32(townId) || !file.readString(townName) ||
                        !file.readU16(temple.x) || !file.readU16(temple.y) || !file.readU8(temple.z)) {
                        qDebug() << "Failed to read town data.";
                        return false;
                    }
                    towns[townId] = townName; // Store town name
                }
                break;
            case OTBM_HOUSES:
                // Read houses
                while (file.peekU8(nodeType) && nodeType == OTBM_HOUSE) {
                    if (!file.readU8(nodeType)) return false; // Consume OTBM_HOUSE node type
                    quint32 houseId;
                    QString houseName;
                    Position housePosition;
                    if (!file.readU32(houseId) || !file.readString(houseName) || !file.readPosition(housePosition)) {
                        qDebug() << "Failed to read house data.";
                        return false;
                    }
                    houses[houseId] = {houseName, housePosition}; // Store house name and position
                }
                break;
            case OTBM_WAYPOINTS:
                // Read waypoints
                while (file.peekU8(nodeType) && nodeType == OTBM_WAYPOINT) {
                    if (!file.readU8(nodeType)) return false; // Consume OTBM_WAYPOINT node type
                    QString waypointName;
                    Position waypointPosition;
                    if (!file.readString(waypointName) || !file.readPosition(waypointPosition)) {
                        qDebug() << "Failed to read waypoint data.";
                        return false;
                    }
                    waypoints[waypointName] = waypointPosition; // Store waypoint name and position
                }
                break;
            default:
                qDebug() << "Unknown node type in map data:" << nodeType;
                // TODO: Skip unknown node instead of returning false
                return false;
        }
    }

    // Consume the end node byte (0xFF)
    if (nodeType != 0xFF) {
         qDebug() << "Expected end node (0xFF), but got:" << nodeType;
         return false;
    }

    return true;
}

bool OTBMFile::load(const QString& filePath) {
    file.setFileName(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Failed to open file for reading:" << file.errorString();
        return false;
    }

    // Read OTBM signature
    quint32 signature;
    if (!file.readU32(signature) || signature != OTBM_SIGNATURE) {
        qDebug() << "Invalid OTBM signature.";
        file.close();
        return false;
    }

    // Read OTBM version
    quint32 versionNum;
    if (!file.readU32(versionNum)) {
        qDebug() << "Failed to read OTBM version.";
        file.close();
        return false;
    }
    version = static_cast<OTBMVersion>(versionNum);

    // Read map data
    if (!readMapData()) {
        file.close();
        return false;
    }

    file.close();
    return true;
}