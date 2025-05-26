#include "spawn.h"

Spawn::Spawn(QPoint position, int radius, QString creatureName, int intervalSeconds) :
    m_position(position),
    m_radius(radius),
    m_creatureName(creatureName),
    m_intervalSeconds(intervalSeconds)
{
    // Constructor
}

Spawn::~Spawn()
{
    // Destructor
    // If Spawn owned any dynamic objects (like Creature pointers), delete them here.
    // Currently, Spawn does not own such objects.
}

// Other methods if needed
// No other methods are specified for now.
