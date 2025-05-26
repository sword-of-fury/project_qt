#include "creature.h"
#include "spritemanager.h" // Assuming SpriteManager exists for getting sprites

Creature::Creature(int id, const QString& name, int spriteId)
    : id(id),
      name(name),
      spriteId(spriteId),
      m_health(100),         // Default value
      m_maxHealth(100),    // Default value
      m_direction(Direction::South), // Default value
      m_speed(100),          // Default value
      m_isNpc(false),        // Default value
      m_spawnTime(60)        // Default value (e.g., seconds)
{
}

Creature::~Creature()
{
}

QPixmap Creature::getSprite() const
{
    // Assuming SpriteManager has a method to get a pixmap by sprite ID
    // This might need adjustment based on actual SpriteManager implementation
    if (SpriteManager::getInstance().hasSprite(spriteId)) {
         return SpriteManager::getInstance().getSprite(spriteId);
    }
    return QPixmap(); // Return an empty pixmap if sprite not found
}

// Getters
int Creature::getHealth() const {
    return m_health;
}

int Creature::getMaxHealth() const {
    return m_maxHealth;
}

Direction Creature::getDirection() const {
    return m_direction;
}

int Creature::getSpeed() const {
    return m_speed;
}

bool Creature::isNpc() const {
    return m_isNpc;
}

int Creature::getSpawnTime() const {
    return m_spawnTime;
}

// Setters
void Creature::setHealth(int health) {
    m_health = health;
    // Potentially add signal emission here if health changes need to be observed
}

void Creature::setMaxHealth(int maxHealth) {
    m_maxHealth = maxHealth;
    // Potentially add signal emission here
}

void Creature::setDirection(Direction direction) {
    m_direction = direction;
    // Potentially add signal emission here
}

void Creature::setSpeed(int speed) {
    m_speed = speed;
    // Potentially add signal emission here
}

void Creature::setIsNpc(bool isNpc) {
    m_isNpc = isNpc;
    // Potentially add signal emission here
}

void Creature::setSpawnTime(int spawnTime) {
    m_spawnTime = spawnTime;
    // Potentially add signal emission here
}

// Optional: String conversion helpers
/*
QString DirectionToString(Direction dir) {
    switch (dir) {
        case Direction::North: return "North";
        case Direction::East:  return "East";
        case Direction::South: return "South";
        case Direction::West:  return "West";
        default: return "Unknown"; // Should not happen if enum is used correctly
    }
}

Direction StringToDirection(const QString& str) {
    QString lowerStr = str.toLower();
    if (lowerStr == "north") return Direction::North;
    if (lowerStr == "east") return Direction::East;
    if (lowerStr == "south") return Direction::South;
    if (lowerStr == "west") return Direction::West;
    return Direction::South; // Default direction or throw an error
}
*/