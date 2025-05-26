#include "xmlfile.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>

XMLFile::XMLFile(QObject* parent) : QObject(parent) {
}

XMLFile::~XMLFile() {
}

bool XMLFile::loadSpawns(const QString& filename) {
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Nie można otworzyć pliku spawnów:" << filename;
        return false;
    }

    QDomDocument doc;
    QString errorMsg;
    int errorLine, errorColumn;
    if (!doc.setContent(&file, &errorMsg, &errorLine, &errorColumn)) {
        qDebug() << "Błąd parsowania XML:" << errorMsg << "w linii" << errorLine;
        file.close();
        return false;
    }
    file.close();

    QDomElement root = doc.documentElement();
    if (root.tagName() != "spawns") {
        qDebug() << "Nieprawidłowy format pliku spawnów";
        return false;
    }

    spawns.clear();
    QDomElement spawnElement = root.firstChildElement("spawn");
    while (!spawnElement.isNull()) {
        Spawn spawn;
        if (parseSpawnElement(spawnElement, spawn)) {
            spawns.append(spawn);
        }
        spawnElement = spawnElement.nextSiblingElement("spawn");
    }

    return true;
}

bool XMLFile::saveSpawns(const QString& filename) {
    QDomDocument doc;
    QDomElement root = doc.createElement("spawns");
    doc.appendChild(root);

    for (const Spawn& spawn : spawns) {
        root.appendChild(createSpawnElement(doc, spawn));
    }

    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "Nie można zapisać pliku spawnów:" << filename;
        return false;
    }

    QTextStream out(&file);
    out << doc.toString(4);
    file.close();

    return true;
}

bool XMLFile::loadHouses(const QString& filename) {
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Nie można otworzyć pliku domów:" << filename;
        return false;
    }

    QDomDocument doc;
    QString errorMsg;
    int errorLine, errorColumn;
    if (!doc.setContent(&file, &errorMsg, &errorLine, &errorColumn)) {
        qDebug() << "Błąd parsowania XML:" << errorMsg << "w linii" << errorLine;
        file.close();
        return false;
    }
    file.close();

    QDomElement root = doc.documentElement();
    if (root.tagName() != "houses") {
        qDebug() << "Nieprawidłowy format pliku domów";
        return false;
    }

    houses.clear();
    QDomElement houseElement = root.firstChildElement("house");
    while (!houseElement.isNull()) {
        House house;
        if (parseHouseElement(houseElement, house)) {
            houses.append(house);
        }
        houseElement = houseElement.nextSiblingElement("house");
    }

    return true;
}

bool XMLFile::saveHouses(const QString& filename) {
    QDomDocument doc;
    QDomElement root = doc.createElement("houses");
    doc.appendChild(root);

    for (const House& house : houses) {
        root.appendChild(createHouseElement(doc, house));
    }

    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "Nie można zapisać pliku domów:" << filename;
        return false;
    }

    QTextStream out(&file);
    out << doc.toString(4);
    file.close();

    return true;
}

void XMLFile::addSpawn(const Spawn& spawn) {
    spawns.append(spawn);
}

void XMLFile::removeSpawn(const QString& name) {
    for (int i = 0; i < spawns.size(); ++i) {
        if (spawns[i].name == name) {
            spawns.removeAt(i);
            break;
        }
    }
}

void XMLFile::addHouse(const House& house) {
    houses.append(house);
}

void XMLFile::removeHouse(const QString& name) {
    for (int i = 0; i < houses.size(); ++i) {
        if (houses[i].name == name) {
            houses.removeAt(i);
            break;
        }
    }
}

bool XMLFile::parseSpawnElement(const QDomElement& element, Spawn& spawn) {
    if (!element.hasAttribute("name") || !element.hasAttribute("x") || 
        !element.hasAttribute("y") || !element.hasAttribute("radius")) {
        return false;
    }

    spawn.name = element.attribute("name");
    spawn.position = QPoint(element.attribute("x").toInt(), element.attribute("y").toInt());
    spawn.radius = element.attribute("radius").toInt();

    QDomElement creatureElement = element.firstChildElement("creature");
    while (!creatureElement.isNull()) {
        if (creatureElement.hasAttribute("id") && creatureElement.hasAttribute("count")) {
            int id = creatureElement.attribute("id").toInt();
            int count = creatureElement.attribute("count").toInt();
            spawn.creatures.append(qMakePair(id, count));
        }
        creatureElement = creatureElement.nextSiblingElement("creature");
    }

    return true;
}

bool XMLFile::parseHouseElement(const QDomElement& element, House& house) {
    if (!element.hasAttribute("name") || !element.hasAttribute("x") || 
        !element.hasAttribute("y") || !element.hasAttribute("size")) {
        return false;
    }

    house.name = element.attribute("name");
    house.position = QPoint(element.attribute("x").toInt(), element.attribute("y").toInt());
    house.size = element.attribute("size").toInt();
    house.rent = element.attribute("rent", "0").toInt();
    house.owner = element.attribute("owner", "");

    QDomElement doorElement = element.firstChildElement("door");
    while (!doorElement.isNull()) {
        if (doorElement.hasAttribute("x") && doorElement.hasAttribute("y")) {
            house.doors.append(QPoint(doorElement.attribute("x").toInt(), 
                                    doorElement.attribute("y").toInt()));
        }
        doorElement = doorElement.nextSiblingElement("door");
    }

    QDomElement bedElement = element.firstChildElement("bed");
    while (!bedElement.isNull()) {
        if (bedElement.hasAttribute("x") && bedElement.hasAttribute("y")) {
            house.beds.append(QPoint(bedElement.attribute("x").toInt(), 
                                   bedElement.attribute("y").toInt()));
        }
        bedElement = bedElement.nextSiblingElement("bed");
    }

    return true;
}

QDomElement XMLFile::createSpawnElement(QDomDocument& doc, const Spawn& spawn) {
    QDomElement element = doc.createElement("spawn");
    element.setAttribute("name", spawn.name);
    element.setAttribute("x", spawn.position.x());
    element.setAttribute("y", spawn.position.y());
    element.setAttribute("radius", spawn.radius);

    for (const auto& creature : spawn.creatures) {
        QDomElement creatureElement = doc.createElement("creature");
        creatureElement.setAttribute("id", creature.first);
        creatureElement.setAttribute("count", creature.second);
        element.appendChild(creatureElement);
    }

    return element;
}

QDomElement XMLFile::createHouseElement(QDomDocument& doc, const House& house) {
    QDomElement element = doc.createElement("house");
    element.setAttribute("name", house.name);
    element.setAttribute("x", house.position.x());
    element.setAttribute("y", house.position.y());
    element.setAttribute("size", house.size);
    element.setAttribute("rent", house.rent);
    if (!house.owner.isEmpty()) {
        element.setAttribute("owner", house.owner);
    }

    for (const QPoint& door : house.doors) {
        QDomElement doorElement = doc.createElement("door");
        doorElement.setAttribute("x", door.x());
        doorElement.setAttribute("y", door.y());
        element.appendChild(doorElement);
    }

    for (const QPoint& bed : house.beds) {
        QDomElement bedElement = doc.createElement("bed");
        bedElement.setAttribute("x", bed.x());
        bedElement.setAttribute("y", bed.y());
        element.appendChild(bedElement);
    }

    return element;
} 