#ifndef CREATUREMANAGER_H
#define CREATUREMANAGER_H

#include <QObject>
#include <QMap>
#include <QString>
#include <QList>
#include "creature.h"

// Forward declaration for Creature class (assuming it will be created)
// class Creature;

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

    // TODO: Implement methods to get creature data by ID or name
    Creature* getCreatureById(int id) const;
    // Creature* getCreatureByName(const QString& name) const;

    // TODO: Implement a method to get a list of all creatures for populating UI
    QList<Creature*> getAllCreatures() const;

signals:
    // TODO: Add signals if needed, e.g., creaturesLoaded()

private:
    explicit CreatureManager(QObject* parent = nullptr);
    ~CreatureManager();

    // TODO: Store creature data
    QMap<int, Creature*> creatures;
};

#endif // CREATUREMANAGER_H