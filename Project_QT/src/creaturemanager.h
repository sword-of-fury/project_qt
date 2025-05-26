#ifndef CREATUREMANAGER_H
#define CREATUREMANAGER_H

#include <QObject>
#include <QMap>
#include <QString>
#include <QList>
#include "creature.h" // For Creature class itself if needed, though manager deals with properties primarily
#include "outfit.h"   // For the Outfit struct

// Definition for CreatureProperties
struct CreatureProperties {
    int id;            // Creature Type ID (e.g., from XML attribute)
    QString name;
    Outfit outfit;
    bool isNpc;
    // Base stats - specific instances might have current values differing from these
    int maxHealth;
    int speed;
    // Add any other static properties that define a creature type
    // e.g., mana, skills, resistances, if CreatureManager is to handle these.
    // For now, keep it aligned with what Creature class itself stores as potential type-level info.
};

class CreatureManager : public QObject
{
    Q_OBJECT

public:
    static CreatureManager& getInstance();

    // Prevent copying and assignment
    CreatureManager(const CreatureManager&) = delete;
    CreatureManager& operator=(const CreatureManager&) = delete;

    // TODO: Implement loading creatures from a data file (e.g., XML, custom format)
    bool loadCreatures(const QString& filePath);

    const CreatureProperties* getCreatureProperties(int id) const;
    QList<CreatureProperties> getAllCreatureProperties() const;
    const CreatureProperties* getCreaturePropertiesByName(const QString& name) const;

signals:
    void creaturesLoaded();

private:
    explicit CreatureManager(QObject* parent = nullptr);
    ~CreatureManager();

    // Store creature properties
    QMap<int, CreatureProperties> m_creatureProperties;
};

#endif // CREATUREMANAGER_H