#include "mapversion.h"
#include <QDebug>

MapVersion* MapVersion::instance = nullptr;
QMap<MapVersionID, MapVersionInfo> MapVersion::versionInfoMap;

MapVersion& MapVersion::getInstance() {
    if (!instance) {
        instance = new MapVersion();
    }
    return *instance;
}

MapVersion::MapVersion() :
    versionID(MAP_OTBM_UNKNOWN),
    supportsHouses(false),
    supportsSpawns(false),
    supportsTeleports(false),
    supportsWaypoints(false),
    supportsAttributes(false),
    supportsProperties(false),
    supportsTowns(false),
    supportsWays(false),
    supportsZones(false),
    supportsRegions(false)
{
    initializeVersionInfo();
}

MapVersion::~MapVersion() {
    if (instance == this) {
        instance = nullptr;
    }
}

void MapVersion::initializeVersionInfo() {
    // OTBM 1 (7.40 - 8.19)
    MapVersionInfo otbm1;
    otbm1.id = MAP_OTBM_1;
    otbm1.name = "OTBM 1";
    otbm1.description = "Podstawowa wersja formatu mapy (7.40 - 8.19)";
    otbm1.supportsHouses = true;
    otbm1.supportsSpawns = true;
    otbm1.supportsTeleports = true;
    otbm1.supportsWaypoints = false;
    otbm1.supportsAttributes = false;
    otbm1.supportsProperties = false;
    otbm1.supportsTowns = true;
    otbm1.supportsWays = false;
    otbm1.supportsZones = false;
    otbm1.supportsRegions = false;
    versionInfoMap[MAP_OTBM_1] = otbm1;

    // OTBM 2 (8.20 - 10.99)
    MapVersionInfo otbm2;
    otbm2.id = MAP_OTBM_2;
    otbm2.name = "OTBM 2";
    otbm2.description = "Rozszerzona wersja formatu mapy (8.20 - 10.99)";
    otbm2.supportsHouses = true;
    otbm2.supportsSpawns = true;
    otbm2.supportsTeleports = true;
    otbm2.supportsWaypoints = true;
    otbm2.supportsAttributes = true;
    otbm2.supportsProperties = true;
    otbm2.supportsTowns = true;
    otbm2.supportsWays = true;
    otbm2.supportsZones = false;
    otbm2.supportsRegions = false;
    versionInfoMap[MAP_OTBM_2] = otbm2;

    // OTBM 3 (11.00 - 12.99)
    MapVersionInfo otbm3;
    otbm3.id = MAP_OTBM_3;
    otbm3.name = "OTBM 3";
    otbm3.description = "Nowa wersja formatu mapy (11.00 - 12.99)";
    otbm3.supportsHouses = true;
    otbm3.supportsSpawns = true;
    otbm3.supportsTeleports = true;
    otbm3.supportsWaypoints = true;
    otbm3.supportsAttributes = true;
    otbm3.supportsProperties = true;
    otbm3.supportsTowns = true;
    otbm3.supportsWays = true;
    otbm3.supportsZones = true;
    otbm3.supportsRegions = true;
    versionInfoMap[MAP_OTBM_3] = otbm3;

    // OTBM 4 (13.00+)
    MapVersionInfo otbm4;
    otbm4.id = MAP_OTBM_4;
    otbm4.name = "OTBM 4";
    otbm4.description = "Najnowsza wersja formatu mapy (13.00+)";
    otbm4.supportsHouses = true;
    otbm4.supportsSpawns = true;
    otbm4.supportsTeleports = true;
    otbm4.supportsWaypoints = true;
    otbm4.supportsAttributes = true;
    otbm4.supportsProperties = true;
    otbm4.supportsTowns = true;
    otbm4.supportsWays = true;
    otbm4.supportsZones = true;
    otbm4.supportsRegions = true;
    versionInfoMap[MAP_OTBM_4] = otbm4;
}

MapVersionID MapVersion::getMapVersionForClientVersion(ClientVersionID clientVersion) {
    if (clientVersion >= CLIENT_VERSION_1300) return MAP_OTBM_4;
    if (clientVersion >= CLIENT_VERSION_1100) return MAP_OTBM_3;
    if (clientVersion >= CLIENT_VERSION_820) return MAP_OTBM_2;
    if (clientVersion >= CLIENT_VERSION_740) return MAP_OTBM_1;
    return MAP_OTBM_UNKNOWN;
}

QString MapVersion::mapVersionIDToString(MapVersionID id) {
    switch (id) {
        case MAP_OTBM_1: return "OTBM 1";
        case MAP_OTBM_2: return "OTBM 2";
        case MAP_OTBM_3: return "OTBM 3";
        case MAP_OTBM_4: return "OTBM 4";
        default: return "Unknown";
    }
}

MapVersionID MapVersion::stringToMapVersionID(const QString& str) {
    if (str == "OTBM 1") return MAP_OTBM_1;
    if (str == "OTBM 2") return MAP_OTBM_2;
    if (str == "OTBM 3") return MAP_OTBM_3;
    if (str == "OTBM 4") return MAP_OTBM_4;
    return MAP_OTBM_UNKNOWN;
}

MapVersionInfo MapVersion::getMapVersionInfo(MapVersionID id) {
    if (versionInfoMap.contains(id)) {
        return versionInfoMap[id];
    }
    
    // Zwróć pustą informację dla nieznanej wersji
    MapVersionInfo unknown;
    unknown.id = MAP_OTBM_UNKNOWN;
    unknown.name = "Unknown";
    unknown.description = "Nieznana wersja formatu mapy";
    unknown.supportsHouses = false;
    unknown.supportsSpawns = false;
    unknown.supportsTeleports = false;
    unknown.supportsWaypoints = false;
    unknown.supportsAttributes = false;
    unknown.supportsProperties = false;
    unknown.supportsTowns = false;
    unknown.supportsWays = false;
    unknown.supportsZones = false;
    unknown.supportsRegions = false;
    return unknown;
} 