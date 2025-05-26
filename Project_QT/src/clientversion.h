#ifndef CLIENTVERSION_H
#define CLIENTVERSION_H

#include <QString>
#include <QMap>
#include <QDir>

// Wersje klienta
enum ClientVersionID {
    CLIENT_VERSION_NONE = -1,
    CLIENT_VERSION_ALL = -2,
    CLIENT_VERSION_740 = 0,
    CLIENT_VERSION_750 = 1,
    CLIENT_VERSION_755 = 2,
    CLIENT_VERSION_760 = 3,
    CLIENT_VERSION_770 = 4,
    CLIENT_VERSION_780 = 5,
    CLIENT_VERSION_790 = 6,
    CLIENT_VERSION_792 = 7,
    CLIENT_VERSION_800 = 8,
    CLIENT_VERSION_810 = 9,
    CLIENT_VERSION_811 = 10,
    CLIENT_VERSION_820 = 11,
    CLIENT_VERSION_830 = 12,
    CLIENT_VERSION_840 = 13,
    CLIENT_VERSION_841 = 14,
    CLIENT_VERSION_842 = 15,
    CLIENT_VERSION_850 = 16,
    CLIENT_VERSION_854 = 17,
    CLIENT_VERSION_855 = 18,
    CLIENT_VERSION_860 = 19,
    CLIENT_VERSION_861 = 20,
    CLIENT_VERSION_862 = 21,
    CLIENT_VERSION_870 = 22,
    CLIENT_VERSION_871 = 23,
    CLIENT_VERSION_872 = 24,
    CLIENT_VERSION_873 = 25,
    CLIENT_VERSION_900 = 26,
    CLIENT_VERSION_910 = 27,
    CLIENT_VERSION_920 = 28,
    CLIENT_VERSION_940 = 29,
    CLIENT_VERSION_944 = 30,
    CLIENT_VERSION_946 = 31,
    CLIENT_VERSION_950 = 32,
    CLIENT_VERSION_952 = 33,
    CLIENT_VERSION_953 = 34,
    CLIENT_VERSION_954 = 35,
    CLIENT_VERSION_960 = 36,
    CLIENT_VERSION_961 = 37,
    CLIENT_VERSION_963 = 38,
    CLIENT_VERSION_970 = 39,
    CLIENT_VERSION_980 = 40,
    CLIENT_VERSION_981 = 41,
    CLIENT_VERSION_982 = 42,
    CLIENT_VERSION_983 = 43,
    CLIENT_VERSION_985 = 44,
    CLIENT_VERSION_986 = 45,
    CLIENT_VERSION_1010 = 46,
    CLIENT_VERSION_1020 = 47,
    CLIENT_VERSION_1021 = 48,
    CLIENT_VERSION_1030 = 49,
    CLIENT_VERSION_1031 = 50,
    CLIENT_VERSION_1097 = 51,
    CLIENT_VERSION_1098 = 52,
    CLIENT_VERSION_1100 = 53,
    CLIENT_VERSION_1140 = 54,
    CLIENT_VERSION_1150 = 55,
    CLIENT_VERSION_1160 = 56,
    CLIENT_VERSION_1170 = 57,
    CLIENT_VERSION_1180 = 58,
    CLIENT_VERSION_1190 = 59,
    CLIENT_VERSION_1200 = 60,
    CLIENT_VERSION_1210 = 61,
    CLIENT_VERSION_1220 = 62,
    CLIENT_VERSION_1230 = 63,
    CLIENT_VERSION_1240 = 64,
    CLIENT_VERSION_1250 = 65,
    CLIENT_VERSION_1260 = 66,
    CLIENT_VERSION_1270 = 67,
    CLIENT_VERSION_1280 = 68,
    CLIENT_VERSION_1290 = 69,
    CLIENT_VERSION_1300 = 70,
    CLIENT_VERSION_1310 = 71,
    CLIENT_VERSION_1320 = 72
};

// Wersje OTBM
enum MapVersionID {
    MAP_OTBM_UNKNOWN = -1,
    MAP_OTBM_1 = 0,
    MAP_OTBM_2 = 1,
    MAP_OTBM_3 = 2,
    MAP_OTBM_4 = 3
};

// Format DAT
enum DatFormat {
    DAT_FORMAT_UNKNOWN,
    DAT_FORMAT_74,
    DAT_FORMAT_755,
    DAT_FORMAT_78,
    DAT_FORMAT_86,
    DAT_FORMAT_96,
    DAT_FORMAT_1010,
    DAT_FORMAT_1050,
    DAT_FORMAT_1057
};

// Format OTB
enum OtbFormatVersion {
    OTB_VERSION_1 = 1,
    OTB_VERSION_2 = 2,
    OTB_VERSION_3 = 3
};

// Flagi DAT
enum DatFlags {
    DatFlagGround = 0,
    DatFlagGroundBorder = 1,
    DatFlagOnBottom = 2,
    DatFlagOnTop = 3,
    DatFlagContainer = 4,
    DatFlagStackable = 5,
    DatFlagForceUse = 6,
    DatFlagMultiUse = 7,
    DatFlagWritable = 8,
    DatFlagWritableOnce = 9,
    DatFlagFluidContainer = 10,
    DatFlagSplash = 11,
    DatFlagNotWalkable = 12,
    DatFlagNotMoveable = 13,
    DatFlagBlockProjectile = 14,
    DatFlagNotPathable = 15,
    DatFlagPickupable = 16,
    DatFlagHangable = 17,
    DatFlagHookSouth = 18,
    DatFlagHookEast = 19,
    DatFlagRotateable = 20,
    DatFlagLight = 21,
    DatFlagDontHide = 22,
    DatFlagTranslucent = 23,
    DatFlagDisplacement = 24,
    DatFlagElevation = 25,
    DatFlagLyingCorpse = 26,
    DatFlagAnimateAlways = 27,
    DatFlagMinimapColor = 28,
    DatFlagLensHelp = 29,
    DatFlagFullGround = 30,
    DatFlagLook = 31,
    DatFlagCloth = 32,
    DatFlagMarket = 33,
    DatFlagUsable = 34,
    DatFlagWrappable = 35,
    DatFlagUnwrappable = 36,
    DatFlagTopEffect = 37,
    DatFlagFloorChange = 252,
    DatFlagNoMoveAnimation = 253,
    DatFlagChargeable = 254,
    DatFlagLast = 255
};

class ClientVersion {
public:
    static ClientVersion& getInstance();

    // Wczytywanie wersji
    void loadVersions();
    void unloadVersions();
    void saveVersions();

    // Pobieranie wersji
    ClientVersionID getVersionID() const { return versionID; }
    QString getVersionName() const { return versionName; }
    MapVersionID getPreferredMapVersion() const { return preferredMapVersion; }
    DatFormat getDatFormat() const { return datFormat; }
    OtbFormatVersion getOtbFormat() const { return otbFormat; }

    // Ścieżki
    QString getClientPath() const { return clientPath; }
    QString getDataPath() const { return dataPath; }
    QString getLocalDataPath() const { return localDataPath; }

    // Ustawianie ścieżek
    void setClientPath(const QString& path);
    void setDataPath(const QString& path);
    void setLocalDataPath(const QString& path);

    // Sprawdzanie ścieżek
    bool hasValidPaths() const;
    bool loadValidPaths();

    // Konwersja wersji
    static QString versionIDToString(ClientVersionID id);
    static ClientVersionID stringToVersionID(const QString& str);
    static DatFormat getDatFormatForVersion(ClientVersionID id);
    static OtbFormatVersion getOtbFormatForVersion(ClientVersionID id);

private:
    explicit ClientVersion();
    ~ClientVersion();

    static ClientVersion* instance;

    ClientVersionID versionID;
    QString versionName;
    MapVersionID preferredMapVersion;
    DatFormat datFormat;
    OtbFormatVersion otbFormat;

    QString clientPath;
    QString dataPath;
    QString localDataPath;
};

#endif // CLIENTVERSION_H 