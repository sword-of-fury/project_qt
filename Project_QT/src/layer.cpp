#include "layer.h"
#include <QDebug> // For debugging

Layer::Layer(Type type, QObject* parent)
    : QObject(parent),
      type(type),
      visible(true), // All layers visible by default
      locked(false)  // All layers unlocked by default
{
    // Set human-readable name based on type
    name = typeToString(type);
    // qDebug() << "Layer created: " << name << ", Type:" << static_cast<int>(type);
}

Layer::~Layer()
{
    // If tiles were stored here, they would be cleared/deleted.
}

void Layer::setVisible(bool value)
{
    if (visible != value) {
        visible = value;
        // qDebug() << name << " visibility set to " << visible;
        emit visibilityChanged(type, visible);
        emit changed(type); // General change notification
    }
}

void Layer::setLocked(bool value)
{
    if (locked != value) {
        locked = value;
        // qDebug() << name << " locked state set to " << locked;
        emit lockedChanged(type, locked);
        emit changed(type); // General change notification
    }
}

QString Layer::getName() const
{
    return name;
}

QString Layer::typeToString(Type type)
{
    switch (type) {
        case Ground: return QObject::tr("Ground");
        case GroundDetail: return QObject::tr("Ground Detail");
        case Objects: return QObject::tr("Objects");
        case Items: return QObject::tr("Items");
        case Creatures: return QObject::tr("Creatures");
        case Effects: return QObject::tr("Effects");
        case Roofs: return QObject::tr("Roofs");
        case Walls: return QObject::tr("Walls"); // If a distinct wall layer exists
        case Water: return QObject::tr("Water");
        case Top: return QObject::tr("Topmost");
        case Count: return QObject::tr("Unknown"); // Should not happen
        default: return QObject::tr("Layer %1").arg(static_cast<int>(type)); // Fallback for numeric layers
    }
}

Layer::Type Layer::stringToType(const QString& str)
{
    if (str == QObject::tr("Ground")) return Ground;
    if (str == QObject::tr("Ground Detail")) return GroundDetail;
    if (str == QObject::tr("Objects")) return Objects;
    if (str == QObject::tr("Items")) return Items;
    if (str == QObject::tr("Creatures")) return Creatures;
    if (str == QObject::tr("Effects")) return Effects;
    if (str == QObject::tr("Roofs")) return Roofs;
    if (str == QObject::tr("Walls")) return Walls;
    if (str == QObject::tr("Water")) return Water;
    if (str == QObject::tr("Topmost")) return Top;

    // Handle generic "Layer %n" conversion
    if (str.startsWith(QObject::tr("Layer "))) {
        bool ok;
        int id = str.right(str.length() - QObject::tr("Layer ").length()).toInt(&ok);
        if (ok && id >= 0 && id < Count) {
            return static_cast<Layer::Type>(id);
        }
    }
    return Count; // Indicate unknown type
}