#ifndef ITEM_H
#define ITEM_H

#include <QString>
#include <QMap>
#include <QVariant>
#include <QPixmap>
#include <QObject>
#include <QPointF>
#include <QIcon>

class QPainter;

class Item : public QObject
{
    Q_OBJECT

public:
    enum Type {
        Ground = 0,
        Container,
        Teleport,
        Wall,
        Border,
        MagicWall,
        Creature,
        Count
    };

    Item();
    Item(int id, const QString& name = QString());
    Item(int id, Type type);
    Item(int id, const ItemManager::ItemProperties* properties); // New constructor
    virtual ~Item();

    // Basic properties
    int getId() const { return id; }
    void setId(int newId) { id = newId; }

    QString getName() const;
    void setName(const QString& newName);

    QString getType() const;
    void setType(const QString& newType);

    QIcon getIcon() const;
    void setIcon(const QPixmap& newIcon);

    QPointF getOffset() const;
    void setOffset(const QPoint& newOffset);

    int getLayer() const { return layer; }
    void setLayer(int newLayer);

    // Collision properties
    bool isBlocking() const;
    void setBlocking(bool newBlocking);

    bool isWalkable() const;
    void setWalkable(bool newWalkable);

    bool isCollidable() const;
    void setCollision(bool value) { collision = value; }

    // For ItemPropertyEditor
    int getCount() const;
    void setCount(int count);
    int getActionId() const;
    void setActionId(int actionId);
    int getUniqueId() const;
    void setUniqueId(int uniqueId);
    const QPoint& getDestPosition() const; 
    void setDestPosition(const QPoint& pos);
    QString getText() const;
    void setText(const QString& text);
    QString getDescription() const;
    void setDescription(const QString& description);

    // Properties from DatItem/ItemProperties
    bool hasProperty(ItemProperty prop) const; // Keep for now, might be needed
    bool isGroundTile() const; // Read from properties
    bool isDoor() const; // Read from properties
    bool isContainer() const; // Read from properties
    bool isFluidContainer() const; // Read from properties
    bool isSplash() const; // Read from properties
    bool isTranslucent() const; // Read from properties
    bool isBlockingMissiles() const; // Read from properties
    bool isBlockingPath() const; // Read from properties
    bool isPickupable() const; // Read from properties
    bool isHangable() const; // Read from properties
    bool isRotatable() const; // Read from properties
    bool hasLight() const; // Read from properties
    const SpriteLight& getLight() const; // Read from properties
    bool isTop() const; // Read from properties
    bool isReadable() const; // Read from properties
    bool isWriteable() const; // Read from properties
    bool isChargeable() const; // Read from properties
    bool isLookThrough() const; // Read from properties
    bool isStackable() const; // Read from properties
    bool isUseable() const; // Read from properties
    bool isTeleporter() const; // Read from properties
    bool isMagicField() const; // Read from properties
    bool isDisguise() const; // Read from properties
    bool isAlwaysOnTop() const; // Read from properties

    int getDrawHeight() const; // Read from properties
    QPair<int, int> getDrawOffset() const; // Read from properties
    uint8_t getFrames() const; // Read from properties

    // Attributes
    void setAttribute(const QString& key, const QVariant& value);
    QVariant getAttribute(const QString& key) const;
    bool hasAttribute(const QString& key) const;
    const QMap<QString, QVariant>& getAttributes() const { return attributes; } // Added getter

    // Rendering
    void draw(QPainter& painter, const QPoint& pos, double zoom = 1.0) const;

    // Type conversion
    static Type stringToType(const QString& type);
    static QString typeToString(Type type);

signals:
    void nameChanged(const QString& name);
    void typeChanged(const QString& type);
    void iconChanged(const QIcon& icon);
    void offsetChanged(const QPointF& offset);
    void layerChanged(int layer);
    void blockingChanged(bool blocking);
    void walkableChanged(bool walkable);
    void collidableChanged(bool collidable); // Added signal

protected:
    int id;
    QString name;
    QString type;
    QIcon icon;
    QPointF offset;
    int layer;
    bool collision;
    bool walkable;
    bool blocking;
    QMap<QString, QVariant> attributes;
    const ItemManager::ItemProperties* m_properties; // Pointer to item properties
};

// Specialized item classes
class GroundItem : public Item
{
public:
    GroundItem(int id);
};

class ContainerItem : public Item
{
public:
    ContainerItem(int id);
};

class TeleportItem : public Item
{
public:
    TeleportItem(int id);
};

class WallItem : public Item
{
public:
    WallItem(int id);
};

class BorderItem : public Item
{
public:
    BorderItem(int id);
};

class MagicWallItem : public Item
{
public:
    MagicWallItem(int id);
};

class CreatureItem : public Item
{
public:
    CreatureItem(int id);
};

#endif // ITEM_H