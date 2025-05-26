#include "clientversion.h"
#include <QSettings>
#include <QFile>
#include <QDir>
#include <QDebug>

ClientVersion* ClientVersion::instance = nullptr;

ClientVersion& ClientVersion::getInstance() {
    if (!instance) {
        instance = new ClientVersion();
    }
    return *instance;
}

ClientVersion::ClientVersion() :
    versionID(CLIENT_VERSION_NONE),
    preferredMapVersion(MAP_OTBM_UNKNOWN),
    datFormat(DAT_FORMAT_UNKNOWN),
    otbFormat(OTB_VERSION_1)
{
}

ClientVersion::~ClientVersion() {
    if (instance == this) {
        instance = nullptr;
    }
}

void ClientVersion::loadVersions() {
    QSettings settings("IdlersMapEditor", "ClientVersions");
    
    // Wczytaj ścieżki
    clientPath = settings.value("clientPath").toString();
    dataPath = settings.value("dataPath").toString();
    localDataPath = settings.value("localDataPath").toString();
    
    // Wczytaj wersję
    QString versionStr = settings.value("version").toString();
    versionID = stringToVersionID(versionStr);
    versionName = versionIDToString(versionID);
    
    // Ustaw formaty
    datFormat = getDatFormatForVersion(versionID);
    otbFormat = getOtbFormatForVersion(versionID);
    
    // Ustaw preferowaną wersję mapy
    if (versionID >= CLIENT_VERSION_820) {
        preferredMapVersion = MAP_OTBM_2;
    } else {
        preferredMapVersion = MAP_OTBM_1;
    }
}

void ClientVersion::unloadVersions() {
    clientPath.clear();
    dataPath.clear();
    localDataPath.clear();
    versionID = CLIENT_VERSION_NONE;
    versionName.clear();
    preferredMapVersion = MAP_OTBM_UNKNOWN;
    datFormat = DAT_FORMAT_UNKNOWN;
    otbFormat = OTB_VERSION_1;
}

void ClientVersion::saveVersions() {
    QSettings settings("IdlersMapEditor", "ClientVersions");
    
    settings.setValue("clientPath", clientPath);
    settings.setValue("dataPath", dataPath);
    settings.setValue("localDataPath", localDataPath);
    settings.setValue("version", versionIDToString(versionID));
}

void ClientVersion::setClientPath(const QString& path) {
    clientPath = path;
}

void ClientVersion::setDataPath(const QString& path) {
    dataPath = path;
}

void ClientVersion::setLocalDataPath(const QString& path) {
    localDataPath = path;
}

bool ClientVersion::hasValidPaths() const {
    if (clientPath.isEmpty() || dataPath.isEmpty() || localDataPath.isEmpty()) {
        return false;
    }

    QDir clientDir(clientPath);
    QDir dataDir(dataPath);
    QDir localDataDir(localDataPath);

    return clientDir.exists() && dataDir.exists() && localDataDir.exists();
}

bool ClientVersion::loadValidPaths() {
    if (!hasValidPaths()) {
        return false;
    }

    // Sprawdź czy pliki istnieją
    QFile datFile(dataPath + "/tibia.dat");
    QFile sprFile(dataPath + "/tibia.spr");
    QFile otbFile(localDataPath + "/items.otb");

    return datFile.exists() && sprFile.exists() && otbFile.exists();
}

QString ClientVersion::versionIDToString(ClientVersionID id) {
    switch (id) {
        case CLIENT_VERSION_740: return "7.40";
        case CLIENT_VERSION_750: return "7.50";
        case CLIENT_VERSION_755: return "7.55";
        case CLIENT_VERSION_760: return "7.60";
        case CLIENT_VERSION_770: return "7.70";
        case CLIENT_VERSION_780: return "7.80";
        case CLIENT_VERSION_790: return "7.90";
        case CLIENT_VERSION_792: return "7.92";
        case CLIENT_VERSION_800: return "8.00";
        case CLIENT_VERSION_810: return "8.10";
        case CLIENT_VERSION_811: return "8.11";
        case CLIENT_VERSION_820: return "8.20";
        case CLIENT_VERSION_830: return "8.30";
        case CLIENT_VERSION_840: return "8.40";
        case CLIENT_VERSION_841: return "8.41";
        case CLIENT_VERSION_842: return "8.42";
        case CLIENT_VERSION_850: return "8.50";
        case CLIENT_VERSION_854: return "8.54";
        case CLIENT_VERSION_855: return "8.55";
        case CLIENT_VERSION_860: return "8.60";
        case CLIENT_VERSION_861: return "8.61";
        case CLIENT_VERSION_862: return "8.62";
        case CLIENT_VERSION_870: return "8.70";
        case CLIENT_VERSION_871: return "8.71";
        case CLIENT_VERSION_872: return "8.72";
        case CLIENT_VERSION_873: return "8.73";
        case CLIENT_VERSION_900: return "9.00";
        case CLIENT_VERSION_910: return "9.10";
        case CLIENT_VERSION_920: return "9.20";
        case CLIENT_VERSION_940: return "9.40";
        case CLIENT_VERSION_944: return "9.44";
        case CLIENT_VERSION_946: return "9.46";
        case CLIENT_VERSION_950: return "9.50";
        case CLIENT_VERSION_952: return "9.52";
        case CLIENT_VERSION_953: return "9.53";
        case CLIENT_VERSION_954: return "9.54";
        case CLIENT_VERSION_960: return "9.60";
        case CLIENT_VERSION_961: return "9.61";
        case CLIENT_VERSION_963: return "9.63";
        case CLIENT_VERSION_970: return "9.70";
        case CLIENT_VERSION_980: return "9.80";
        case CLIENT_VERSION_981: return "9.81";
        case CLIENT_VERSION_982: return "9.82";
        case CLIENT_VERSION_983: return "9.83";
        case CLIENT_VERSION_985: return "9.85";
        case CLIENT_VERSION_986: return "9.86";
        case CLIENT_VERSION_1010: return "10.10";
        case CLIENT_VERSION_1020: return "10.20";
        case CLIENT_VERSION_1021: return "10.21";
        case CLIENT_VERSION_1030: return "10.30";
        case CLIENT_VERSION_1031: return "10.31";
        case CLIENT_VERSION_1097: return "10.97";
        case CLIENT_VERSION_1098: return "10.98";
        case CLIENT_VERSION_1100: return "11.00";
        case CLIENT_VERSION_1140: return "11.40";
        case CLIENT_VERSION_1150: return "11.50";
        case CLIENT_VERSION_1160: return "11.60";
        case CLIENT_VERSION_1170: return "11.70";
        case CLIENT_VERSION_1180: return "11.80";
        case CLIENT_VERSION_1190: return "11.90";
        case CLIENT_VERSION_1200: return "12.00";
        case CLIENT_VERSION_1210: return "12.10";
        case CLIENT_VERSION_1220: return "12.20";
        case CLIENT_VERSION_1230: return "12.30";
        case CLIENT_VERSION_1240: return "12.40";
        case CLIENT_VERSION_1250: return "12.50";
        case CLIENT_VERSION_1260: return "12.60";
        case CLIENT_VERSION_1270: return "12.70";
        case CLIENT_VERSION_1280: return "12.80";
        case CLIENT_VERSION_1290: return "12.90";
        case CLIENT_VERSION_1300: return "13.00";
        case CLIENT_VERSION_1310: return "13.10";
        case CLIENT_VERSION_1320: return "13.20";
        default: return "Unknown";
    }
}

ClientVersionID ClientVersion::stringToVersionID(const QString& str) {
    if (str == "7.40") return CLIENT_VERSION_740;
    if (str == "7.50") return CLIENT_VERSION_750;
    if (str == "7.55") return CLIENT_VERSION_755;
    if (str == "7.60") return CLIENT_VERSION_760;
    if (str == "7.70") return CLIENT_VERSION_770;
    if (str == "7.80") return CLIENT_VERSION_780;
    if (str == "7.90") return CLIENT_VERSION_790;
    if (str == "7.92") return CLIENT_VERSION_792;
    if (str == "8.00") return CLIENT_VERSION_800;
    if (str == "8.10") return CLIENT_VERSION_810;
    if (str == "8.11") return CLIENT_VERSION_811;
    if (str == "8.20") return CLIENT_VERSION_820;
    if (str == "8.30") return CLIENT_VERSION_830;
    if (str == "8.40") return CLIENT_VERSION_840;
    if (str == "8.41") return CLIENT_VERSION_841;
    if (str == "8.42") return CLIENT_VERSION_842;
    if (str == "8.50") return CLIENT_VERSION_850;
    if (str == "8.54") return CLIENT_VERSION_854;
    if (str == "8.55") return CLIENT_VERSION_855;
    if (str == "8.60") return CLIENT_VERSION_860;
    if (str == "8.61") return CLIENT_VERSION_861;
    if (str == "8.62") return CLIENT_VERSION_862;
    if (str == "8.70") return CLIENT_VERSION_870;
    if (str == "8.71") return CLIENT_VERSION_871;
    if (str == "8.72") return CLIENT_VERSION_872;
    if (str == "8.73") return CLIENT_VERSION_873;
    if (str == "9.00") return CLIENT_VERSION_900;
    if (str == "9.10") return CLIENT_VERSION_910;
    if (str == "9.20") return CLIENT_VERSION_920;
    if (str == "9.40") return CLIENT_VERSION_940;
    if (str == "9.44") return CLIENT_VERSION_944;
    if (str == "9.46") return CLIENT_VERSION_946;
    if (str == "9.50") return CLIENT_VERSION_950;
    if (str == "9.52") return CLIENT_VERSION_952;
    if (str == "9.53") return CLIENT_VERSION_953;
    if (str == "9.54") return CLIENT_VERSION_954;
    if (str == "9.60") return CLIENT_VERSION_960;
    if (str == "9.61") return CLIENT_VERSION_961;
    if (str == "9.63") return CLIENT_VERSION_963;
    if (str == "9.70") return CLIENT_VERSION_970;
    if (str == "9.80") return CLIENT_VERSION_980;
    if (str == "9.81") return CLIENT_VERSION_981;
    if (str == "9.82") return CLIENT_VERSION_982;
    if (str == "9.83") return CLIENT_VERSION_983;
    if (str == "9.85") return CLIENT_VERSION_985;
    if (str == "9.86") return CLIENT_VERSION_986;
    if (str == "10.10") return CLIENT_VERSION_1010;
    if (str == "10.20") return CLIENT_VERSION_1020;
    if (str == "10.21") return CLIENT_VERSION_1021;
    if (str == "10.30") return CLIENT_VERSION_1030;
    if (str == "10.31") return CLIENT_VERSION_1031;
    if (str == "10.97") return CLIENT_VERSION_1097;
    if (str == "10.98") return CLIENT_VERSION_1098;
    if (str == "11.00") return CLIENT_VERSION_1100;
    if (str == "11.40") return CLIENT_VERSION_1140;
    if (str == "11.50") return CLIENT_VERSION_1150;
    if (str == "11.60") return CLIENT_VERSION_1160;
    if (str == "11.70") return CLIENT_VERSION_1170;
    if (str == "11.80") return CLIENT_VERSION_1180;
    if (str == "11.90") return CLIENT_VERSION_1190;
    if (str == "12.00") return CLIENT_VERSION_1200;
    if (str == "12.10") return CLIENT_VERSION_1210;
    if (str == "12.20") return CLIENT_VERSION_1220;
    if (str == "12.30") return CLIENT_VERSION_1230;
    if (str == "12.40") return CLIENT_VERSION_1240;
    if (str == "12.50") return CLIENT_VERSION_1250;
    if (str == "12.60") return CLIENT_VERSION_1260;
    if (str == "12.70") return CLIENT_VERSION_1270;
    if (str == "12.80") return CLIENT_VERSION_1280;
    if (str == "12.90") return CLIENT_VERSION_1290;
    if (str == "13.00") return CLIENT_VERSION_1300;
    if (str == "13.10") return CLIENT_VERSION_1310;
    if (str == "13.20") return CLIENT_VERSION_1320;
    return CLIENT_VERSION_NONE;
}

DatFormat ClientVersion::getDatFormatForVersion(ClientVersionID id) {
    if (id >= CLIENT_VERSION_1050) return DAT_FORMAT_1050;
    if (id >= CLIENT_VERSION_1010) return DAT_FORMAT_1010;
    if (id >= CLIENT_VERSION_960) return DAT_FORMAT_96;
    if (id >= CLIENT_VERSION_860) return DAT_FORMAT_86;
    if (id >= CLIENT_VERSION_780) return DAT_FORMAT_78;
    if (id >= CLIENT_VERSION_755) return DAT_FORMAT_755;
    if (id >= CLIENT_VERSION_740) return DAT_FORMAT_74;
    return DAT_FORMAT_UNKNOWN;
}

OtbFormatVersion ClientVersion::getOtbFormatForVersion(ClientVersionID id) {
    if (id >= CLIENT_VERSION_1050) return OTB_VERSION_3;
    if (id >= CLIENT_VERSION_1010) return OTB_VERSION_2;
    return OTB_VERSION_1;
} 