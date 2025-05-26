#ifndef CREATURE_H
#define CREATURE_H

#include <QString>
#include <QPixmap>

class Creature
{
public:
    Creature(int id, const QString& name, int spriteId);
    ~Creature();

    int getId() const { return id; }
    void setId(int newId) { id = newId; } // Assuming ID can be changed

    QString getName() const { return name; }
    void setName(const QString& newName) { name = newName; }

    int getSpriteId() const { return spriteId; }
    void setSpriteId(int newSpriteId) { spriteId = newSpriteId; }

    QPixmap getSprite() const; // Method to retrieve the sprite pixmap

    // Added from property editor usage
    int getHealth() const;
    void setHealth(int health);
    int getMaxHealth() const;
    void setMaxHealth(int maxHealth);
    int getDirection() const; 
    void setDirection(int direction);
    int getSpeed() const;
    void setSpeed(int speed);
    bool isNpc() const;
    void setIsNpc(bool isNpc);
    int getSpawnTime() const;
    void setSpawnTime(int spawnTime);

private:
    int id;
    QString name;
    int spriteId;
    // Add member variables for the new properties
    int m_health;
    int m_maxHealth;
    int m_direction; // Consider an enum for Direction
    int m_speed;
    bool m_isNpc;
    int m_spawnTime;
};

#endif // CREATURE_H