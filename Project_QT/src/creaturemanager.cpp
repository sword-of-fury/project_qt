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
    // m_creatureProperties stores objects directly, so no need for qDeleteAll
    m_creatureProperties.clear();
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
    // qDeleteAll(creatures); // Old map
    // creatures.clear(); // Old map
    m_creatureProperties.clear(); // Clear the new map

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
            if (xml.name() == "creature" || xml.name() == "monster" || xml.name() == "npc") {
                CreatureProperties props;
                props.id = xml.attributes().value("id").toInt(); // Assuming 'id' attribute exists for all types
                props.name = xml.attributes().value("name").toString();
                props.isNpc = (xml.name() == "npc") || xml.attributes().value("is_npc").toString().compare("true", Qt::CaseInsensitive) == 0;

                // Outfit details
                props.outfit.lookType = xml.attributes().value("looktype").toInt();
                props.outfit.lookHead = xml.attributes().value("lookhead").toInt();
                props.outfit.lookBody = xml.attributes().value("lookbody").toInt();
                props.outfit.lookLegs = xml.attributes().value("looklegs").toInt();
                props.outfit.lookFeet = xml.attributes().value("lookfeet").toInt();
                props.outfit.lookAddons = xml.attributes().value("lookaddons").toInt();
                props.outfit.lookItem = xml.attributes().value("lookitem").toInt();    
                props.outfit.lookMount = xml.attributes().value("lookmount").toInt();  

                // Mount outfit components (if present in XML)
                props.outfit.lookMountHead = xml.attributes().value("lookmounthead").toInt();
                props.outfit.lookMountBody = xml.attributes().value("lookmountbody").toInt();
                props.outfit.lookMountLegs = xml.attributes().value("lookmountlegs").toInt();
                props.outfit.lookMountFeet = xml.attributes().value("lookmountfeet").toInt();

                // Health and Speed
                props.maxHealth = xml.attributes().value("health_max").toInt(100); // Default to 100 if not present
                props.speed = xml.attributes().value("speed").toInt(100);         // Default to 100 if not present
                
                // If ID is not in XML, generate or use name as key (requires map type change)
                // For this subtask, we assume 'id' attribute is present and valid.
                if (props.id == 0 && !props.name.isEmpty()) { // Fallback if ID is missing but name exists
                    // This would require m_creatureProperties to be QMap<QString, CreatureProperties>
                    // or a different handling strategy. For now, we stick to int IDs.
                    // Consider logging a warning if ID is missing.
                    qWarning() << "Creature type" << props.name << "is missing a numeric ID in XML. Skipping.";
                    continue;
                }

                if (props.id > 0 && !props.name.isEmpty()) {
                    m_creatureProperties.insert(props.id, props);
                    qDebug() << "Loaded creature type:" << props.name << "(ID:" << props.id << ")";
                } else {
                    qWarning() << "Skipping creature type with invalid data: ID=" << props.id << ", Name=" << props.name;
                }
            }
        }
    }

    if (xml.hasError()) {
        qWarning() << "Error parsing XML file" << filePath << ":" << xml.errorString();
        return false;
    }

    file.close();
    qDebug() << "Finished loading" << m_creatureProperties.size() << "creature types.";
    emit creaturesLoaded(); // Emit the signal after loading
    return true;
}

const CreatureManager::CreatureProperties* CreatureManager::getCreatureProperties(int id) const {
    if (m_creatureProperties.contains(id)) {
        return &m_creatureProperties.value(id); // Return pointer to value in QMap
    }
    return nullptr;
}

QList<CreatureManager::CreatureProperties> CreatureManager::getAllCreatureProperties() const {
    return m_creatureProperties.values();
}

const CreatureManager::CreatureProperties* CreatureManager::getCreaturePropertiesByName(const QString& name) const {
    for (const auto& props : m_creatureProperties) { // Iterate over QMap values
        if (props.name.compare(name, Qt::CaseInsensitive) == 0) {
            return &props; // Return pointer to the value (CreatureProperties)
        }
    }
    return nullptr;
}

// Creature* CreatureManager::getCreatureById(int id) const
// {
//     Q_UNUSED(id); 
//     return nullptr; 
// }

// QList<Creature*> CreatureManager::getAllCreatures() const
// {
//     return QList<Creature*>(); 
// }