#include "item.h"
#include <QPainter>
#include "spritemanager.h"
#include "itemmanager.h"

Item::Item() : id(0), collision(false), walkable(true), blocking(false) {
}

Item::Item(int id, const QString& name) 
    : id(id), name(name), collision(false), walkable(true), blocking(false) {
}

Item::Item(int id, const ItemManager::ItemProperties* properties)
    : id(id), m_properties(properties), collision(false), walkable(true), blocking(false)
{
    if (m_properties) {
        name = m_properties->name;
        // Initialize other members from properties if needed
        blocking = m_properties->blocking;
        walkable = m_properties->walkable;
        collision = m_properties->collidable;
        // type will be set based on properties later or handled differently
    }
}

Item::~Item()
{
}

void Item::draw(QPainter& painter, const QPoint& pos, double zoom) const {
    // Pobierz GameSprite z ItemManager (który korzysta ze SpriteManager)
    GameSprite* sprite = ItemManager::getInstance()->getItemGameSprite(id);

    if (sprite) {
        // Oblicz docelowy prostokąt do narysowania sprite'a
        // Rozmiar sprite'a zależy od jego właściwości (width, height) i zoomu
        // Use draw offset from properties
        QPoint drawOffset = getDrawOffset();
        QRect targetRect(
            pos.x() - drawOffset.x() * zoom,
            pos.y() - drawOffset.y() * zoom,
            sprite->width * 32 * zoom, // Zakładamy domyślny rozmiar 32x32 kafelka
            sprite->height * 32 * zoom
        );

        // Narysuj sprite
        sprite->drawTo(&painter, SpriteSize::SPRITE_SIZE_32x32, targetRect.x(), targetRect.y(), targetRect.width(), targetRect.height());

    } else if (!icon.isNull()) {
        // Jeśli nie ma GameSprite, ale jest QPixmap (np. dla sprite'ów edytora)
        QRect targetRect(pos.x(), pos.y(), icon.width() * zoom, icon.height() * zoom);
        painter.drawPixmap(targetRect, icon);
    } else {
        // Rysuj placeholder jeśli nie ma ani GameSprite ani QPixmap
        QRect rect(pos.x(), pos.y(), 32 * zoom, 32 * zoom);
        painter.setPen(QPen(Qt::black, 1));
        painter.setBrush(QBrush(Qt::lightGray));
        painter.drawRect(rect);
        
        // Rysuj ID przedmiotu
        painter.setPen(Qt::black);
        painter.drawText(rect, Qt::AlignCenter, QString::number(id));
    }
}

Item::Type Item::stringToType(const QString& type)
{
    if (type == "ground") return Type::Ground;
    if (type == "container") return Type::Container;
    if (type == "teleport") return Type::Teleport;
    if (type == "wall") return Type::Wall;
    if (type == "border") return Type::Border;
    if (type == "magicwall") return Type::MagicWall;
    return Type::Count;
}

QString Item::typeToString(Type type)
{
    switch (type) {
        case Type::Ground: return "ground";
        case Type::Container: return "container";
        case Type::Teleport: return "teleport";
        case Type::Wall: return "wall";
        case Type::Border: return "border";
        case Type::MagicWall: return "magicwall";
        default: return "";
    }
}

// Implementacje konstruktorów dla specjalizacji
GroundItem::GroundItem(int id)
    : Item(id, Type::Ground)
{
}

ContainerItem::ContainerItem(int id)
    : Item(id, Type::Container)
{
}

TeleportItem::TeleportItem(int id)
    : Item(id, Type::Teleport)
{
}

WallItem::WallItem(int id)
    : Item(id, Type::Wall)
{
}

BorderItem::BorderItem(int id)
    : Item(id, Type::Border)
{
}

MagicWallItem::MagicWallItem(int id)
    : Item(id, Type::MagicWall)
{
}

CreatureItem::CreatureItem(int id)
    : Item(id, Type::Creature)
{
}

bool Item::isBlocking() const
{
    return blocking;
}

bool Item::isWalkable() const
{
    return walkable;
}

bool Item::isCollidable() const
{
    return collision;
}

QString Item::getName() const
{
    return name;
}

void Item::setName(const QString& newName)
{
    if (name != newName) {
        name = newName;
        emit nameChanged(name);
    }
}

QString Item::getType() const
{
    return type;
}

void Item::setType(const QString& newType)
{
    if (type != newType) {
        type = newType;
        emit typeChanged(type);
    }
}

QIcon Item::getIcon() const
{
    return icon;
}

void Item::setIcon(const QPixmap& newIcon)
{
    if (icon.cacheKey() != newIcon.cacheKey()) {
        icon = newIcon;
        emit iconChanged(icon);
    }
}

QPointF Item::getOffset() const
{
    return offset;
}

void Item::setOffset(const QPoint& newOffset)
{
    if (offset != newOffset) {
        offset = newOffset;
        emit offsetChanged(offset);
    }
}

void Item::setLayer(int newLayer)
{
    if (layer != newLayer) {
        layer = newLayer;
        emit layerChanged(layer);
    }
}

void Item::setBlocking(bool newBlocking)
{
    if (blocking != newBlocking) {
        blocking = newBlocking;
        emit blockingChanged(blocking);
    }
}

void Item::setWalkable(bool newWalkable)
{
    if (walkable != newWalkable) {
        walkable = newWalkable;
        emit walkableChanged(walkable);
    }
}

void Item::setCollidable(bool newCollidable)
{
    if (collision != newCollidable) {
        collision = newCollidable;
        emit collidableChanged(collision);
    }
}

void Item::setAttribute(const QString& key, const QVariant& value) {
    attributes[key] = value;
}

QVariant Item::getAttribute(const QString& key) const {
    return attributes.value(key);
}

bool Item::hasAttribute(const QString& key) const {
    return attributes.contains(key);
}

bool Item::hasProperty(ItemProperty prop) const {
    // Implement based on m_properties flags
    if (!m_properties) return false;
    // This requires mapping ItemProperty enum to ItemProperties fields/flags
    // For now, return false as a placeholder
    return false; 
}

bool Item::isGroundTile() const { return m_properties ? (m_properties->flags & ItemProperty::IsGroundTile) : false; }
bool Item::isDoor() const { return m_properties ? (m_properties->flags & ItemProperty::IsDoor) : false; }
bool Item::isContainer() const { return m_properties ? (m_properties->flags & ItemProperty::IsContainer) : false; }
bool Item::isFluidContainer() const { return m_properties ? (m_properties->flags & ItemProperty::IsFluidContainer) : false; }
bool Item::isSplash() const { return m_properties ? (m_properties->flags & ItemProperty::IsSplash) : false; }
bool Item::isTranslucent() const { return m_properties ? (m_properties->flags & ItemProperty::IsTranslucent) : false; }
bool Item::isBlockingMissiles() const { return m_properties ? (m_properties->flags & ItemProperty::IsBlockingMissiles) : false; }
bool Item::isBlockingPath() const { return m_properties ? (m_properties->flags & ItemProperty::IsBlockingPath) : false; }
bool Item::isPickupable() const { return m_properties ? (m_properties->flags & ItemProperty::IsPickupable) : false; }
bool Item::isHangable() const { return m_properties ? (m_properties->flags & ItemProperty::IsHangable) : false; }
bool Item::isRotatable() const { return m_properties ? (m_properties->flags & ItemProperty::IsRotatable) : false; }
bool Item::hasLight() const { return m_properties ? (m_properties->flags & ItemProperty::HasLight) : false; }
const SpriteLight& Item::getLight() const { 
    // This assumes SpriteLight is part of ItemProperties or accessible via sprite
    // For now, return a default light if properties or light info is missing
    static SpriteLight defaultLight = {0, 0};
    if (m_properties) {
        // Assuming light info is stored in properties
        defaultLight.intensity = m_properties->lightLevel;
        defaultLight.color = m_properties->lightColor;
        return defaultLight;
    }
    return defaultLight;
}
bool Item::isTop() const { return m_properties ? (m_properties->flags & ItemProperty::IsTop) : false; }
bool Item::isReadable() const { return m_properties ? (m_properties->flags & ItemProperty::IsReadable) : false; }
bool Item::isWriteable() const { return m_properties ? (m_properties->flags & ItemProperty::IsWriteable) : false; }
bool Item::isChargeable() const { return m_properties ? (m_properties->flags & ItemProperty::IsChargeable) : false; }
bool Item::isLookThrough() const { return m_properties ? (m_properties->flags & ItemProperty::IsLookThrough) : false; }
bool Item::isStackable() const { return m_properties ? (m_properties->flags & ItemProperty::IsStackable) : false; }
bool Item::isUseable() const { return m_properties ? (m_properties->flags & ItemProperty::IsUseable) : false; }
bool Item::isTeleporter() const { return m_properties ? (m_properties->flags & ItemProperty::IsTeleporter) : false; }
bool Item::isMagicField() const { return m_properties ? (m_properties->flags & ItemProperty::IsMagicField) : false; }
bool Item::isDisguise() const { return m_properties ? (m_properties->flags & ItemProperty::IsDisguise) : false; }
bool Item::isAlwaysOnTop() const { return m_properties ? (m_properties->flags & ItemProperty::IsAlwaysOnTop) : false; }

int Item::getDrawHeight() const { return m_properties ? m_properties->drawHeight : 0; }
QPair<int, int> Item::getDrawOffset() const { return m_properties ? qMakePair(m_properties->drawOffsetX, m_properties->drawOffsetY) : qMakePair(0, 0); }
uint8_t Item::getFrames() const { return m_properties ? m_properties->frames : 1; }