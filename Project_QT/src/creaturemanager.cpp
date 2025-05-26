#include "creaturemanager.h"
#include <QDebug>
#include <QFile>
#include <QXmlStreamReader>

CreatureManager::CreatureManager(QObject* parent)
    : QObject(parent)
{
    // TODO: Initialize creature data structures
}

CreatureManager::~CreatureManager()
{
    // TODO: Clean up creature data
    qDeleteAll(creatures);
    creatures.clear();
}

CreatureManager& CreatureManager::getInstance()
{
    static CreatureManager instance;
    return instance;
}

bool CreatureManager::loadCreatures(const QString& filePath)
{
    qDebug() << "Loading creatures from:" << filePath;

    QFile file(filePath);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        qWarning() << "Cannot read file" << filePath << ":" << file.errorString();
        return false;
    }

    // Clear existing creatures before loading
    qDeleteAll(creatures);
    creatures.clear();

    QXmlStreamReader xml(&file);

    while (!xml.atEnd() && !xml.hasError()) {
        QXmlStreamReader::TokenType token = xml.readNext();
        if (token == QXmlStreamReader::StartDocument) {
            continue;
        }
        if (token == QXmlStreamReader::StartElement) {
            if (xml.name() == "creatures") {
                continue;
            }
            if (xml.name() == "creature") {
                int id = xml.attributes().value("id").toInt();
                QString name = xml.attributes().value("name").toString();
                int spriteId = xml.attributes().value("spriteId").toInt();

                if (id > 0 && !name.isEmpty() && spriteId > 0) {
                    Creature* creature = new Creature(id, name, spriteId);
                    creatures.insert(id, creature);
                    qDebug() << "Loaded creature:" << name << "(ID:" << id << ", SpriteID:" << spriteId << ")";
                } else {
                    qWarning() << "Skipping creature with invalid data: ID=" << id << ", Name=" << name << ", SpriteID=" << spriteId;
                }
            }
        }
    }

    if (xml.hasError()) {
        qWarning() << "Error parsing XML file" << filePath << ":" << xml.errorString();
        return false;
    }

    file.close();
    qDebug() << "Finished loading" << creatures.size() << "creatures.";
    return true;
}

Creature* CreatureManager::getCreatureById(int id) const
{
    if (creatures.contains(id)) {
        return creatures.value(id);
    }
    return nullptr;
}

QList<Creature*> CreatureManager::getAllCreatures() const
{
    return creatures.values();
}