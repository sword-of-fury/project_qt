#ifndef ITEM_H
#define ITEM_H

#include <QString>
#include <QMap>
#include <QVariant>
#include <QPixmap>
#include <QObject>
#include <QPointF>
#include <QIcon>
#include <QFlags> // For Q_DECLARE_FLAGS

class QPainter;

// Define ItemPropertyFlag Enum (Option B)
enum class ItemPropertyFlag : quint32 {
    PropNone                = 0,
    IsBlocking              = 1 << 0,  // FLAG_UNPASSABLE (tile is unpassable)
    BlockMissiles           = 1 << 1,  // FLAG_BLOCK_MISSILES
    BlockPathfinder         = 1 << 2,  // FLAG_BLOCK_PATHFINDER
    HasElevation            = 1 << 3,  // FLAG_HAS_ELEVATION
    IsUseable               = 1 << 4,  // FLAG_USEABLE
    IsPickupable            = 1 << 5,  // FLAG_PICKUPABLE
    IsMoveable              = 1 << 6,  // FLAG_MOVEABLE
    IsStackable             = 1 << 7,  // FLAG_STACKABLE
    IsFloorChangeDown       = 1 << 8,  // FLAG_FLOORCHANGEDOWN (ladder)
    IsFloorChangeNorth      = 1 << 9,  // FLAG_FLOORCHANGENORTH (stairs)
    IsFloorChangeEast       = 1 << 10, // FLAG_FLOORCHANGEEAST (stairs)
    IsFloorChangeSouth      = 1 << 11, // FLAG_FLOORCHANGESOUTH (stairs)
    IsFloorChangeWest       = 1 << 12, // FLAG_FLOORCHANGEWEST (stairs)
    IsAlwaysOnBottom        = 1 << 13, // FLAG_ALWAYSONTOP (wxWidgets: means always on bottom)
    IsReadable              = 1 << 14, // FLAG_READABLE
    IsRotatable             = 1 << 15, // FLAG_ROTATABLE
    IsHangable              = 1 << 16, // FLAG_HANGABLE
    HookEast                = 1 << 17, // FLAG_HOOK_EAST
    HookSouth               = 1 << 18, // FLAG_HOOK_SOUTH
    CannotDecay             = 1 << 19, // FLAG_CANNOTDECAY
    AllowDistRead           = 1 << 20, // FLAG_ALLOWDISTREAD
    // Unused               = 1 << 21, // Skip FLAG_UNUSED
    ClientCharges           = 1 << 22, // FLAG_CLIENTCHARGES (deprecated by some sources)
    IgnoreLook              = 1 << 23, // FLAG_IGNORE_LOOK
    WalkStack               = 1 << 24, // FLAG_WALKSTACK (walk on if stackable)
    FullTile                = 1 << 25, // FLAG_FULLTILE (like water/lava)

    // Additional flags for Qt version (some might be derived differently in wx)
    IsGroundTile            = 1 << 26, // Often derived from ItemGroup in wx
    IsContainer             = 1 << 27,
    IsFluidContainer        = 1 << 28, // (e.g., vials, etc.)
    IsSplash                = 1 << 29,
    IsDoor                  = 1 << 30,
    IsMagicField            = 1U << 31, // Use 1U for the 31st bit
    // Note: Max 32 flags with quint32. If more needed, consider multiple flag variables or larger type.
    
    // Flags that might need different handling or are derived from other properties:
    // IsWall, IsBorder -> often derived from item type/group or specific IDs
    // IsTeleporter -> often has specific action IDs or script handling
    // IsAlwaysOnTop (Actual Top) -> Handled by alwaysOnTopOrder in ItemManager::ItemProperties
    // IsTranslucent, HasLight, IsLookThrough -> These are often direct members in ItemManager::ItemProperties,
    // but can be mirrored as flags if needed for generic Item::hasProperty checks.
    // For this task, we'll focus on the above direct mappings.
    // IsTop, IsBlockingCreature, IsStairs, IsLadder -> these might be specific item properties too.
};
Q_DECLARE_FLAGS(ItemPropertyFlags, ItemPropertyFlag)
Q_DECLARE_OPERATORS_FOR_FLAGS(ItemPropertyFlags)


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
    QPoint getDestPosition() const;  // Changed to return by value
    void setDestPosition(const QPoint& pos);
    QString getText() const;
    void setText(const QString& text);
    QString getDescription() const; // This is for editor-visible "description" attribute
    void setDescription(const QString& description); // Sets "description" attribute

    // Weight
    double getWeight() const;

    // Properties from DatItem/ItemProperties
    // bool hasProperty(ItemProperty prop) const; // Old enum, will be replaced or updated
    bool hasProperty(ItemPropertyFlag flag) const; // Updated to use new flags
    bool isGroundTile() const; // Read from properties
    bool isDoor() const; // Read from properties
    bool isContainer() const; // Read from properties
    bool isFluidContainer() const; // Read from properties
    bool isSplash() const; // Read from properties
    bool isTranslucent() const; // Read from properties
    bool isBlockingMissiles() const; // Read from properties (maps to BlockMissiles flag)
    bool isBlockingPath() const; // Read from properties (maps to BlockPathfinder flag)
    bool isPickupable() const; // Read from properties (maps to IsPickupable flag)
    bool isHangable() const; // Read from properties (maps to IsHangable flag)
    bool isRotatable() const; // Read from properties (maps to IsRotatable flag)
    bool hasLight() const; // Read from properties (direct field in ItemManager::ItemProperties, or a flag)
    const SpriteLight& getLight() const; // Read from properties
    bool isTop() const; // This was likely for display order, maps to alwaysOnTopOrder != 0
    bool isReadable() const; // Read from properties (maps to IsReadable flag)
    bool isWriteable() const; // Read from properties (maps to IsWriteable flag)
    bool isChargeable() const; // Read from properties (maps to ClientCharges flag or specific logic)
    bool isLookThrough() const; // Read from properties (direct field, or a flag like ~FLAG_UNPASSABLE if it means non-blocking)
    bool isStackable() const; // Read from properties (maps to IsStackable flag)
    bool isUseable() const; // Read from properties (maps to IsUseable flag)
    bool isTeleporter() const; // Read from properties (maps to IsTeleporter flag or specific logic)
    bool isMagicField() const; // Read from properties (maps to IsMagicField flag or specific logic)
    bool isDisguise() const; // Read from properties (specific logic or a flag)
    bool isAlwaysOnTop() const; // Actual "on top", from alwaysOnTopOrder, not FLAG_ALWAYSONTOP
    bool isAlwaysOnBottom() const; // From FLAG_ALWAYSONTOP (ItemPropertyFlag::IsAlwaysOnBottom)


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
    void attributesChanged(); // Generic signal for attribute changes

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