#include "otb.h"
#include <QDebug>

OTBFile::OTBFile() : majorVersion(1), minorVersion(0) {
}

OTBFile::~OTBFile() {
    file.close();
}

bool OTBFile::load(const QString& filename) {
    if (!file.open(filename, QIODevice::ReadOnly)) {
        qDebug() << "Nie można otworzyć pliku OTB:" << filename;
        return false;
    }

    if (!readHeader()) {
        qDebug() << "Błąd odczytu nagłówka OTB";
        return false;
    }

    while (!file.atEnd()) {
        OTBItem item;
        if (!readItem(item)) {
            qDebug() << "Błąd odczytu przedmiotu OTB";
            return false;
        }
        items[item.serverId] = item;
    }

    return true;
}

bool OTBFile::save(const QString& filename) {
    if (!file.open(filename, QIODevice::WriteOnly)) {
        qDebug() << "Nie można utworzyć pliku OTB:" << filename;
        return false;
    }

    if (!writeHeader()) {
        qDebug() << "Błąd zapisu nagłówka OTB";
        return false;
    }

    for (const auto& item : items) {
        if (!writeItem(item)) {
            qDebug() << "Błąd zapisu przedmiotu OTB";
            return false;
        }
    }

    return true;
}

bool OTBFile::readHeader() {
    // Sprawdź identyfikator OTB
    char identifier[4];
    if (!file.readRaw(identifier, 4) || memcmp(identifier, "OTBI", 4) != 0) {
        qDebug() << "Nieprawidłowy format pliku OTB";
        return false;
    }

    // Odczytaj wersję
    if (!file.readU32(majorVersion) || !file.readU32(minorVersion)) {
        qDebug() << "Błąd odczytu wersji OTB";
        return false;
    }

    return true;
}

bool OTBFile::writeHeader() {
    // Zapisz identyfikator OTB
    if (!file.writeRaw("OTBI", 4)) {
        qDebug() << "Błąd zapisu identyfikatora OTB";
        return false;
    }

    // Zapisz wersję
    if (!file.writeU32(majorVersion) || !file.writeU32(minorVersion)) {
        qDebug() << "Błąd zapisu wersji OTB";
        return false;
    }

    return true;
}

bool OTBFile::readItem(OTBItem& item) {
    // Odczytaj ID serwera
    if (!file.readU16(item.serverId)) {
        return false;
    }

    // Odczytaj atrybuty
    quint8 attribute;
    while (file.readU8(attribute) && attribute != 0) {
        OTBItemAttribute attr = static_cast<OTBItemAttribute>(attribute);
        QVariant value;

        switch (attr) {
            case OTB_ATTR_SERVERID:
            case OTB_ATTR_CLIENTID: {
                quint16 id;
                if (!file.readU16(id)) return false;
                value = id;
                break;
            }
            case OTB_ATTR_SPEED:
            case OTB_ATTR_SLOT:
            case OTB_ATTR_MAXITEMS:
            case OTB_ATTR_WEIGHT:
            case OTB_ATTR_WEAPON:
            case OTB_ATTR_AMU:
            case OTB_ATTR_ARMOR:
            case OTB_ATTR_MAGLEVEL:
            case OTB_ATTR_MAGFIELDTYPE:
            case OTB_ATTR_WRITEABLE:
            case OTB_ATTR_ROTATETO:
            case OTB_ATTR_DECAY:
            case OTB_ATTR_SPRITEHASH:
            case OTB_ATTR_MINIMAPCOLOR:
            case OTB_ATTR_07:
            case OTB_ATTR_08:
            case OTB_ATTR_LIGHT:
            case OTB_ATTR_DECAY2:
            case OTB_ATTR_WEAPON2:
            case OTB_ATTR_AMU2:
            case OTB_ATTR_ARMOR2:
            case OTB_ATTR_WRITEABLE2:
            case OTB_ATTR_LIGHT2:
            case OTB_ATTR_TOPORDER:
            case OTB_ATTR_WRITEABLE3:
            case OTB_ATTR_WAREID: {
                quint16 val;
                if (!file.readU16(val)) return false;
                value = val;
                break;
            }
            default:
                qDebug() << "Nieznany atrybut OTB:" << attribute;
                return false;
        }

        item.attributes[attr] = value;
    }

    // Odczytaj typ i flagi
    if (!file.readU8(item.type) || !file.readU32(item.flags)) {
        return false;
    }

    return true;
}

bool OTBFile::writeItem(const OTBItem& item) {
    // Zapisz ID serwera
    if (!file.writeU16(item.serverId)) {
        return false;
    }

    // Zapisz atrybuty
    for (auto it = item.attributes.begin(); it != item.attributes.end(); ++it) {
        if (!file.writeU8(it.key())) {
            return false;
        }

        switch (it.key()) {
            case OTB_ATTR_SERVERID:
            case OTB_ATTR_CLIENTID:
                if (!file.writeU16(it.value().toUInt())) {
                    return false;
                }
                break;
            case OTB_ATTR_SPEED:
            case OTB_ATTR_SLOT:
            case OTB_ATTR_MAXITEMS:
            case OTB_ATTR_WEIGHT:
            case OTB_ATTR_WEAPON:
            case OTB_ATTR_AMU:
            case OTB_ATTR_ARMOR:
            case OTB_ATTR_MAGLEVEL:
            case OTB_ATTR_MAGFIELDTYPE:
            case OTB_ATTR_WRITEABLE:
            case OTB_ATTR_ROTATETO:
            case OTB_ATTR_DECAY:
            case OTB_ATTR_SPRITEHASH:
            case OTB_ATTR_MINIMAPCOLOR:
            case OTB_ATTR_07:
            case OTB_ATTR_08:
            case OTB_ATTR_LIGHT:
            case OTB_ATTR_DECAY2:
            case OTB_ATTR_WEAPON2:
            case OTB_ATTR_AMU2:
            case OTB_ATTR_ARMOR2:
            case OTB_ATTR_WRITEABLE2:
            case OTB_ATTR_LIGHT2:
            case OTB_ATTR_TOPORDER:
            case OTB_ATTR_WRITEABLE3:
            case OTB_ATTR_WAREID:
                if (!file.writeU16(it.value().toUInt())) {
                    return false;
                }
                break;
            default:
                qDebug() << "Nieznany atrybut OTB:" << it.key();
                return false;
        }
    }

    // Zapisz znacznik końca atrybutów
    if (!file.writeU8(0)) {
        return false;
    }

    // Zapisz typ i flagi
    if (!file.writeU8(item.type) || !file.writeU32(item.flags)) {
        return false;
    }

    return true;
} 