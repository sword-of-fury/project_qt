#include "otbmfile.h"
#include <QDebug>

OTBMFile::OTBMFile(QObject *parent) : QObject(parent),
    mapVersion(MAP_OTBM_UNKNOWN)
{
}

OTBMFile::~OTBMFile() {
}

bool OTBMFile::load(const QString& filename) {
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        emit error("Nie można otworzyć pliku mapy");
        return false;
    }

    QDataStream stream(&file);
    stream.setVersion(QDataStream::Qt_5_0);

    // Wczytaj nagłówek
    if (!loadHeader(stream)) {
        emit error("Błąd wczytywania nagłówka mapy");
        return false;
    }

    // Sprawdź czy wersja mapy jest obsługiwana
    MapVersionInfo versionInfo = MapVersion::getMapVersionInfo(mapVersion);
    if (versionInfo.id == MAP_OTBM_UNKNOWN) {
        emit error("Nieobsługiwana wersja mapy");
        return false;
    }

    // Wczytaj elementy mapy
    if (versionInfo.supportsHouses && !loadHouses(stream)) {
        emit error("Błąd wczytywania domów");
        return false;
    }

    if (versionInfo.supportsSpawns && !loadSpawns(stream)) {
        emit error("Błąd wczytywania spawnów");
        return false;
    }

    if (versionInfo.supportsTeleports && !loadTeleports(stream)) {
        emit error("Błąd wczytywania teleportów");
        return false;
    }

    if (versionInfo.supportsWaypoints && !loadWaypoints(stream)) {
        emit error("Błąd wczytywania waypointów");
        return false;
    }

    if (versionInfo.supportsTowns && !loadTowns(stream)) {
        emit error("Błąd wczytywania miast");
        return false;
    }

    if (versionInfo.supportsZones && !loadZones(stream)) {
        emit error("Błąd wczytywania stref");
        return false;
    }

    if (versionInfo.supportsRegions && !loadRegions(stream)) {
        emit error("Błąd wczytywania regionów");
        return false;
    }

    emit mapLoaded();
    return true;
}

bool OTBMFile::save(const QString& filename) {
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly)) {
        emit error("Nie można zapisać pliku mapy");
        return false;
    }

    QDataStream stream(&file);
    stream.setVersion(QDataStream::Qt_5_0);

    // Zapisz nagłówek
    if (!saveHeader(stream)) {
        emit error("Błąd zapisywania nagłówka mapy");
        return false;
    }

    // Sprawdź czy wersja mapy jest obsługiwana
    MapVersionInfo versionInfo = MapVersion::getMapVersionInfo(mapVersion);
    if (versionInfo.id == MAP_OTBM_UNKNOWN) {
        emit error("Nieobsługiwana wersja mapy");
        return false;
    }

    // Zapisz elementy mapy
    if (versionInfo.supportsHouses && !saveHouses(stream)) {
        emit error("Błąd zapisywania domów");
        return false;
    }

    if (versionInfo.supportsSpawns && !saveSpawns(stream)) {
        emit error("Błąd zapisywania spawnów");
        return false;
    }

    if (versionInfo.supportsTeleports && !saveTeleports(stream)) {
        emit error("Błąd zapisywania teleportów");
        return false;
    }

    if (versionInfo.supportsWaypoints && !saveWaypoints(stream)) {
        emit error("Błąd zapisywania waypointów");
        return false;
    }

    if (versionInfo.supportsTowns && !saveTowns(stream)) {
        emit error("Błąd zapisywania miast");
        return false;
    }

    if (versionInfo.supportsZones && !saveZones(stream)) {
        emit error("Błąd zapisywania stref");
        return false;
    }

    if (versionInfo.supportsRegions && !saveRegions(stream)) {
        emit error("Błąd zapisywania regionów");
        return false;
    }

    emit mapSaved();
    return true;
}

bool OTBMFile::loadHeader(QDataStream& stream) {
    // Wczytaj magiczną liczbę
    quint32 magic;
    stream >> magic;
    if (magic != 0x4D42544F) { // "OTBM"
        return false;
    }

    // Wczytaj wersję
    quint32 version;
    stream >> version;
    mapVersion = static_cast<MapVersionID>(version);

    // Wczytaj rozmiar mapy
    quint32 width, height;
    stream >> width >> height;
    mapSize = QPoint(width, height);

    // Wczytaj opis
    stream >> mapDescription;

    return true;
}

bool OTBMFile::saveHeader(QDataStream& stream) {
    // Zapisz magiczną liczbę
    stream << quint32(0x4D42544F); // "OTBM"

    // Zapisz wersję
    stream << quint32(mapVersion);

    // Zapisz rozmiar mapy
    stream << quint32(mapSize.x()) << quint32(mapSize.y());

    // Zapisz opis
    stream << mapDescription;

    return true;
}

bool OTBMFile::loadHouses(QDataStream& stream) {
    houses.clear();
    quint32 count;
    stream >> count;

    for (quint32 i = 0; i < count; ++i) {
        House house;
        stream >> house.id;
        stream >> house.name;
        stream >> house.position;
        stream >> house.rent;
        stream >> house.size;
        stream >> house.isGuildHall;
        houses.append(house);
    }

    return true;
}

bool OTBMFile::saveHouses(QDataStream& stream) {
    stream << quint32(houses.size());

    for (const House& house : houses) {
        stream << house.id;
        stream << house.name;
        stream << house.position;
        stream << house.rent;
        stream << house.size;
        stream << house.isGuildHall;
    }

    return true;
}

bool OTBMFile::loadSpawns(QDataStream& stream) {
    spawns.clear();
    quint32 count;
    stream >> count;

    for (quint32 i = 0; i < count; ++i) {
        Spawn spawn;
        stream >> spawn.id;
        stream >> spawn.position;
        stream >> spawn.radius;
        
        quint32 creatureCount;
        stream >> creatureCount;
        for (quint32 j = 0; j < creatureCount; ++j) {
            quint32 creatureId;
            stream >> creatureId;
            spawn.creatures.append(creatureId);
        }
        
        spawns.append(spawn);
    }

    return true;
}

bool OTBMFile::saveSpawns(QDataStream& stream) {
    stream << quint32(spawns.size());

    for (const Spawn& spawn : spawns) {
        stream << spawn.id;
        stream << spawn.position;
        stream << spawn.radius;
        stream << quint32(spawn.creatures.size());
        
        for (quint32 creatureId : spawn.creatures) {
            stream << creatureId;
        }
    }

    return true;
}

bool OTBMFile::loadTeleports(QDataStream& stream) {
    teleports.clear();
    quint32 count;
    stream >> count;

    for (quint32 i = 0; i < count; ++i) {
        Teleport teleport;
        stream >> teleport.id;
        stream >> teleport.position;
        stream >> teleport.destination;
        stream >> teleport.description;
        teleports.append(teleport);
    }

    return true;
}

bool OTBMFile::saveTeleports(QDataStream& stream) {
    stream << quint32(teleports.size());

    for (const Teleport& teleport : teleports) {
        stream << teleport.id;
        stream << teleport.position;
        stream << teleport.destination;
        stream << teleport.description;
    }

    return true;
}

bool OTBMFile::loadWaypoints(QDataStream& stream) {
    waypoints.clear();
    quint32 count;
    stream >> count;

    for (quint32 i = 0; i < count; ++i) {
        Waypoint waypoint;
        stream >> waypoint.id;
        stream >> waypoint.position;
        stream >> waypoint.name;
        stream >> waypoint.description;
        waypoints.append(waypoint);
    }

    return true;
}

bool OTBMFile::saveWaypoints(QDataStream& stream) {
    stream << quint32(waypoints.size());

    for (const Waypoint& waypoint : waypoints) {
        stream << waypoint.id;
        stream << waypoint.position;
        stream << waypoint.name;
        stream << waypoint.description;
    }

    return true;
}

bool OTBMFile::loadTowns(QDataStream& stream) {
    towns.clear();
    quint32 count;
    stream >> count;

    for (quint32 i = 0; i < count; ++i) {
        Town town;
        stream >> town.id;
        stream >> town.name;
        stream >> town.position;
        stream >> town.templePosition;
        towns.append(town);
    }

    return true;
}

bool OTBMFile::saveTowns(QDataStream& stream) {
    stream << quint32(towns.size());

    for (const Town& town : towns) {
        stream << town.id;
        stream << town.name;
        stream << town.position;
        stream << town.templePosition;
    }

    return true;
}

bool OTBMFile::loadZones(QDataStream& stream) {
    zones.clear();
    quint32 count;
    stream >> count;

    for (quint32 i = 0; i < count; ++i) {
        Zone zone;
        stream >> zone.id;
        stream >> zone.name;
        stream >> zone.area;
        stream >> zone.description;
        zones.append(zone);
    }

    return true;
}

bool OTBMFile::saveZones(QDataStream& stream) {
    stream << quint32(zones.size());

    for (const Zone& zone : zones) {
        stream << zone.id;
        stream << zone.name;
        stream << zone.area;
        stream << zone.description;
    }

    return true;
}

bool OTBMFile::loadRegions(QDataStream& stream) {
    regions.clear();
    quint32 count;
    stream >> count;

    for (quint32 i = 0; i < count; ++i) {
        Region region;
        stream >> region.id;
        stream >> region.name;
        stream >> region.area;
        stream >> region.description;
        regions.append(region);
    }

    return true;
}

bool OTBMFile::saveRegions(QDataStream& stream) {
    stream << quint32(regions.size());

    for (const Region& region : regions) {
        stream << region.id;
        stream << region.name;
        stream << region.area;
        stream << region.description;
    }

    return true;
}

bool OTBMFile::addHouse(const House& house) {
    // Sprawdź czy wersja mapy wspiera domy
    MapVersionInfo versionInfo = MapVersion::getMapVersionInfo(mapVersion);
    if (!versionInfo.supportsHouses) {
        emit error("Ta wersja mapy nie wspiera domów");
        return false;
    }

    // Sprawdź czy dom o takim ID już istnieje
    for (const House& h : houses) {
        if (h.id == house.id) {
            emit error("Dom o takim ID już istnieje");
            return false;
        }
    }

    houses.append(house);
    return true;
}

bool OTBMFile::removeHouse(uint32_t id) {
    for (int i = 0; i < houses.size(); ++i) {
        if (houses[i].id == id) {
            houses.removeAt(i);
            return true;
        }
    }
    return false;
}

House OTBMFile::getHouse(uint32_t id) const {
    for (const House& house : houses) {
        if (house.id == id) {
            return house;
        }
    }
    return House();
}

bool OTBMFile::addSpawn(const Spawn& spawn) {
    MapVersionInfo versionInfo = MapVersion::getMapVersionInfo(mapVersion);
    if (!versionInfo.supportsSpawns) {
        emit error("Ta wersja mapy nie wspiera spawnów");
        return false;
    }

    for (const Spawn& s : spawns) {
        if (s.id == spawn.id) {
            emit error("Spawn o takim ID już istnieje");
            return false;
        }
    }

    spawns.append(spawn);
    return true;
}

bool OTBMFile::removeSpawn(uint32_t id) {
    for (int i = 0; i < spawns.size(); ++i) {
        if (spawns[i].id == id) {
            spawns.removeAt(i);
            return true;
        }
    }
    return false;
}

Spawn OTBMFile::getSpawn(uint32_t id) const {
    for (const Spawn& spawn : spawns) {
        if (spawn.id == id) {
            return spawn;
        }
    }
    return Spawn();
}

bool OTBMFile::addTeleport(const Teleport& teleport) {
    MapVersionInfo versionInfo = MapVersion::getMapVersionInfo(mapVersion);
    if (!versionInfo.supportsTeleports) {
        emit error("Ta wersja mapy nie wspiera teleportów");
        return false;
    }

    for (const Teleport& t : teleports) {
        if (t.id == teleport.id) {
            emit error("Teleport o takim ID już istnieje");
            return false;
        }
    }

    teleports.append(teleport);
    return true;
}

bool OTBMFile::removeTeleport(uint32_t id) {
    for (int i = 0; i < teleports.size(); ++i) {
        if (teleports[i].id == id) {
            teleports.removeAt(i);
            return true;
        }
    }
    return false;
}

Teleport OTBMFile::getTeleport(uint32_t id) const {
    for (const Teleport& teleport : teleports) {
        if (teleport.id == id) {
            return teleport;
        }
    }
    return Teleport();
}

bool OTBMFile::addWaypoint(const Waypoint& waypoint) {
    MapVersionInfo versionInfo = MapVersion::getMapVersionInfo(mapVersion);
    if (!versionInfo.supportsWaypoints) {
        emit error("Ta wersja mapy nie wspiera waypointów");
        return false;
    }

    for (const Waypoint& w : waypoints) {
        if (w.id == waypoint.id) {
            emit error("Waypoint o takim ID już istnieje");
            return false;
        }
    }

    waypoints.append(waypoint);
    return true;
}

bool OTBMFile::removeWaypoint(uint32_t id) {
    for (int i = 0; i < waypoints.size(); ++i) {
        if (waypoints[i].id == id) {
            waypoints.removeAt(i);
            return true;
        }
    }
    return false;
}

Waypoint OTBMFile::getWaypoint(uint32_t id) const {
    for (const Waypoint& waypoint : waypoints) {
        if (waypoint.id == id) {
            return waypoint;
        }
    }
    return Waypoint();
}

bool OTBMFile::addTown(const Town& town) {
    MapVersionInfo versionInfo = MapVersion::getMapVersionInfo(mapVersion);
    if (!versionInfo.supportsTowns) {
        emit error("Ta wersja mapy nie wspiera miast");
        return false;
    }

    for (const Town& t : towns) {
        if (t.id == town.id) {
            emit error("Miasto o takim ID już istnieje");
            return false;
        }
    }

    towns.append(town);
    return true;
}

bool OTBMFile::removeTown(uint32_t id) {
    for (int i = 0; i < towns.size(); ++i) {
        if (towns[i].id == id) {
            towns.removeAt(i);
            return true;
        }
    }
    return false;
}

Town OTBMFile::getTown(uint32_t id) const {
    for (const Town& town : towns) {
        if (town.id == id) {
            return town;
        }
    }
    return Town();
}

bool OTBMFile::addZone(const Zone& zone) {
    MapVersionInfo versionInfo = MapVersion::getMapVersionInfo(mapVersion);
    if (!versionInfo.supportsZones) {
        emit error("Ta wersja mapy nie wspiera stref");
        return false;
    }

    for (const Zone& z : zones) {
        if (z.id == zone.id) {
            emit error("Strefa o takim ID już istnieje");
            return false;
        }
    }

    zones.append(zone);
    return true;
}

bool OTBMFile::removeZone(uint32_t id) {
    for (int i = 0; i < zones.size(); ++i) {
        if (zones[i].id == id) {
            zones.removeAt(i);
            return true;
        }
    }
    return false;
}

Zone OTBMFile::getZone(uint32_t id) const {
    for (const Zone& zone : zones) {
        if (zone.id == id) {
            return zone;
        }
    }
    return Zone();
}

bool OTBMFile::addRegion(const Region& region) {
    MapVersionInfo versionInfo = MapVersion::getMapVersionInfo(mapVersion);
    if (!versionInfo.supportsRegions) {
        emit error("Ta wersja mapy nie wspiera regionów");
        return false;
    }

    for (const Region& r : regions) {
        if (r.id == region.id) {
            emit error("Region o takim ID już istnieje");
            return false;
        }
    }

    regions.append(region);
    return true;
}

bool OTBMFile::removeRegion(uint32_t id) {
    for (int i = 0; i < regions.size(); ++i) {
        if (regions[i].id == id) {
            regions.removeAt(i);
            return true;
        }
    }
    return false;
}

Region OTBMFile::getRegion(uint32_t id) const {
    for (const Region& region : regions) {
        if (region.id == id) {
            return region;
        }
    }
    return Region();
} 

bool OTBMFile::writeTileArea() {
    // Implementacja zapisu obszaru kafelków
    for (const auto& tile : tiles) {
        if (!writeTile(tile)) {
            emit error("Błąd zapisywania kafelka");
            return false;
        }
    }
    return true;
}

bool OTBMFile::writeTile(const Tile& tile) {
    // Implementacja zapisu kafelka
    stream << tile.position;
    for (const auto& item : tile.items) {
        if (!writeItem(item)) {
            emit error("Błąd zapisywania przedmiotu");
            return false;
        }
    }
    return true;
}

bool OTBMFile::readItem() {
    quint16 itemId;
    if (!file.readU16(itemId)) {
        return false;
    }

    quint8 nodeType;
    while (file.readU8(nodeType)) {
        // Implementacja odczytu atrybutów przedmiotu
        if (nodeType == OTBM_ATTRIBUTE) {
            quint8 attributeType;
            file.readU8(attributeType);
            // Read attribute based on type
        }
    }

    return true;
}

bool OTBMFile::writeItem(const Item& item) {
    // Implementacja zapisu przedmiotu
    stream << item.id;
    for (const auto& attribute : item.attributes) {
        stream << attribute.type << attribute.value;
    }
    return true;
}