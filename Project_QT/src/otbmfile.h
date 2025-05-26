#ifndef OTBMFILE_H
#define OTBMFILE_H

#include "mapversion.h"
#include <QObject>
#include <QFile>
#include <QDataStream>
#include <QMap>
#include <QVector>
#include <QPoint>

// Struktury dla różnych elementów mapy
struct House {
    uint32_t id;
    QString name;
    QPoint position;
    uint32_t rent;
    uint32_t size;
    bool isGuildHall;
};

struct Spawn {
    uint32_t id;
    QPoint position;
    uint32_t radius;
    QVector<uint32_t> creatures;
};

struct Teleport {
    uint32_t id;
    QPoint position;
    QPoint destination;
    QString description;
};

struct Waypoint {
    uint32_t id;
    QPoint position;
    QString name;
    QString description;
};

struct Town {
    uint32_t id;
    QString name;
    QPoint position;
    uint32_t templePosition;
};

struct Zone {
    uint32_t id;
    QString name;
    QRect area;
    QString description;
};

struct Region {
    uint32_t id;
    QString name;
    QRect area;
    QString description;
};

class OTBMFile : public QObject {
    Q_OBJECT

public:
    explicit OTBMFile(QObject *parent = nullptr);
    ~OTBMFile();

    // Wczytywanie i zapisywanie
    bool load(const QString& filename);
    bool save(const QString& filename);

    // Pobieranie informacji o mapie
    MapVersionID getMapVersion() const { return mapVersion; }
    QString getMapDescription() const { return mapDescription; }
    QPoint getMapSize() const { return mapSize; }

    // Zarządzanie domami
    bool addHouse(const House& house);
    bool removeHouse(uint32_t id);
    House getHouse(uint32_t id) const;
    QVector<House> getHouses() const { return houses; }

    // Zarządzanie spawnami
    bool addSpawn(const Spawn& spawn);
    bool removeSpawn(uint32_t id);
    Spawn getSpawn(uint32_t id) const;
    QVector<Spawn> getSpawns() const { return spawns; }

    // Zarządzanie teleportami
    bool addTeleport(const Teleport& teleport);
    bool removeTeleport(uint32_t id);
    Teleport getTeleport(uint32_t id) const;
    QVector<Teleport> getTeleports() const { return teleports; }

    // Zarządzanie waypointami
    bool addWaypoint(const Waypoint& waypoint);
    bool removeWaypoint(uint32_t id);
    Waypoint getWaypoint(uint32_t id) const;
    QVector<Waypoint> getWaypoints() const { return waypoints; }

    // Zarządzanie miastami
    bool addTown(const Town& town);
    bool removeTown(uint32_t id);
    Town getTown(uint32_t id) const;
    QVector<Town> getTowns() const { return towns; }

    // Zarządzanie strefami
    bool addZone(const Zone& zone);
    bool removeZone(uint32_t id);
    Zone getZone(uint32_t id) const;
    QVector<Zone> getZones() const { return zones; }

    // Zarządzanie regionami
    bool addRegion(const Region& region);
    bool removeRegion(uint32_t id);
    Region getRegion(uint32_t id) const;
    QVector<Region> getRegions() const { return regions; }

signals:
    void mapLoaded();
    void mapSaved();
    void error(const QString& message);

private:
    // Wczytywanie nagłówka
    bool loadHeader(QDataStream& stream);
    bool saveHeader(QDataStream& stream);

    // Wczytywanie elementów mapy
    bool loadHouses(QDataStream& stream);
    bool loadSpawns(QDataStream& stream);
    bool loadTeleports(QDataStream& stream);
    bool loadWaypoints(QDataStream& stream);
    bool loadTowns(QDataStream& stream);
    bool loadZones(QDataStream& stream);
    bool loadRegions(QDataStream& stream);

    // Zapisywanie elementów mapy
    bool saveHouses(QDataStream& stream);
    bool saveSpawns(QDataStream& stream);
    bool saveTeleports(QDataStream& stream);
    bool saveWaypoints(QDataStream& stream);
    bool saveTowns(QDataStream& stream);
    bool saveZones(QDataStream& stream);
    bool saveRegions(QDataStream& stream);

    MapVersionID mapVersion;
    QString mapDescription;
    QPoint mapSize;

    QVector<House> houses;
    QVector<Spawn> spawns;
    QVector<Teleport> teleports;
    QVector<Waypoint> waypoints;
    QVector<Town> towns;
    QVector<Zone> zones;
    QVector<Region> regions;
};

#endif // OTBMFILE_H 