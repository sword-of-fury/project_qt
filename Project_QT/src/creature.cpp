#include "creature.h"
#include "spritemanager.h" // Assuming SpriteManager exists for getting sprites

Creature::Creature(int id, const QString& name, int spriteId)
    : id(id)
    , name(name)
    , spriteId(spriteId)
{
}

Creature::~Creature()
{
}

QPixmap Creature::getSprite() const
{
    // Assuming SpriteManager has a method to get a pixmap by sprite ID
    return SpriteManager::getInstance().getSprite(spriteId);
}