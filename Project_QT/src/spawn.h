#ifndef SPAWN_H
#define SPAWN_H

#include <QString>
#include <QPoint> // Using QPoint for position as it's common in Qt

// Forward declaration
class Creature; // If Spawn will hold detailed creature info

class Spawn
{
public:
    Spawn(QPoint position, int radius = 3, QString creatureName = "DefaultCreature", int intervalSeconds = 60);
    ~Spawn();

    // Getters
    QPoint getPosition() const { return m_position; }
    int getRadius() const { return m_radius; } // Radius of the spawn area
    QString getCreatureName() const { return m_creatureName; } // For simplicity, one creature type per spawn
    int getInterval() const { return m_intervalSeconds; } // Spawn interval in seconds

    // Setters
    void setPosition(const QPoint& position) { m_position = position; }
    void setRadius(int radius) { m_radius = radius; }
    void setCreatureName(const QString& name) { m_creatureName = name; }
    void setInterval(int seconds) { m_intervalSeconds = seconds; }

    // TODO: Later, this might hold a list of creatures, quantities, etc.
    // QList<Creature*> m_creatures; // Example

private:
    QPoint m_position;      // Center position of the spawn on the map (x, y). Z is implicitly the floor of the tile it was created on.
    int m_radius;           // Radius in tiles
    QString m_creatureName; // Name of the creature to spawn
    int m_intervalSeconds;  // How often the creature(s) spawn
};

#endif // SPAWN_H
