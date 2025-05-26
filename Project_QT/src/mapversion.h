#ifndef MAPVERSION_H
#define MAPVERSION_H

#include "clientversion.h"
#include <QString>
#include <QMap>
#include <QVector>

// Struktura przechowująca informacje o wersji mapy
struct MapVersionInfo {
    MapVersionID id;
    QString name;
    QString description;
    bool supportsHouses;
    bool supportsSpawns;
    bool supportsTeleports;
    bool supportsWaypoints;
    bool supportsAttributes;
    bool supportsProperties;
    bool supportsTowns;
    bool supportsWays;
    bool supportsZones;
    bool supportsRegions;
    bool supportsHouses;
    bool supportsSpawns;
    bool supportsTeleports;
    bool supportsWaypoints;
    bool supportsAttributes;
    bool supportsProperties;
    bool supportsTowns;
    bool supportsWays;
    bool supportsZones;
    bool supportsRegions;
};

class MapVersion {
public:
    static MapVersion& getInstance();

    // Pobieranie informacji o wersji
    MapVersionID getVersionID() const { return versionID; }
    QString getVersionName() const { return versionName; }
    QString getVersionDescription() const { return versionDescription; }
    
    // Sprawdzanie wsparcia dla funkcji
    bool supportsHouses() const { return supportsHouses; }
    bool supportsSpawns() const { return supportsSpawns; }
    bool supportsTeleports() const { return supportsTeleports; }
    bool supportsWaypoints() const { return supportsWaypoints; }
    bool supportsAttributes() const { return supportsAttributes; }
    bool supportsProperties() const { return supportsProperties; }
    bool supportsTowns() const { return supportsTowns; }
    bool supportsWays() const { return supportsWays; }
    bool supportsZones() const { return supportsZones; }
    bool supportsRegions() const { return supportsRegions; }

    // Konwersja między wersjami
    static MapVersionID getMapVersionForClientVersion(ClientVersionID clientVersion);
    static QString mapVersionIDToString(MapVersionID id);
    static MapVersionID stringToMapVersionID(const QString& str);

    // Pobieranie informacji o wersji
    static MapVersionInfo getMapVersionInfo(MapVersionID id);

private:
    explicit MapVersion();
    ~MapVersion();

    static MapVersion* instance;

    MapVersionID versionID;
    QString versionName;
    QString versionDescription;
    
    bool supportsHouses;
    bool supportsSpawns;
    bool supportsTeleports;
    bool supportsWaypoints;
    bool supportsAttributes;
    bool supportsProperties;
    bool supportsTowns;
    bool supportsWays;
    bool supportsZones;
    bool supportsRegions;

    static QMap<MapVersionID, MapVersionInfo> versionInfoMap;
    static void initializeVersionInfo();
};

#endif // MAPVERSION_H 