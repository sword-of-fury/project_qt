#ifndef LAYER_H
#define LAYER_H

#include <QObject>
#include <QMap>
#include <QPoint>
#include <QList> // For QList<Tile*> in Map class, implicitly needed for Layer class
#include <QVector>

// #include "tile.h" // Removed to break circular dependency, forward declaration is below.
// Forward declarations for Tile, Item, Creature
// To break potential circular dependencies, it's best to define enums
// and small structs like 'Position' that are universally needed in their own small header,
// and then use forward declarations for classes in other headers.
// However, the current project structure might force including.

class Tile; // Forward declaration is sufficient here. tile.cpp will include layer.h

/**
 * @brief The Layer class represents a single layer of the map.
 * This can be ground, items, creatures, effects, roofs, etc.
 */
class Layer : public QObject
{
    Q_OBJECT
public:
    // Define the LayerType enum inside the Layer class as a scoped enum
    enum Type {
        Ground = 0,         // Layer 0: Ground (usually the base layer)
        GroundDetail = 1,   // Layer 1: Ground details (grass, rocks, small items)
        Objects = 2,        // Layer 2: Major objects (trees, walls, doors, teleports)
        Items = 3,          // Layer 3: Smaller items (potions, equipment)
        Creatures = 4,      // Layer 4: Creatures and NPCs
        Effects = 5,        // Layer 5: Visual effects (spells, fires)
        Roofs = 6,          // Layer 6: Roofs and upper structures
        Walls = 7,          // Layer 7: Walls (if handled as a separate layer)
        Water = 8,          // Layer 8: Water and liquids (if not part of ground)
        Top = 15,           // Layer 15: Topmost layer (always rendered last)
        // ... more specific Tibia layers may go here (e.g., floors for Tibia up to 16 total layers usually 0-15)
        // Keep the range 0-15 for consistency with OTBM files usually.
        // The Map::LayerCount constant from source implies up to 16 layers.
        Count               // Number of layer types, for array sizing
    };

    explicit Layer(Type type, QObject* parent = nullptr);
    virtual ~Layer();

    // Basic layer properties
    Type getType() const { return type; }
    void setType(Type type) { this->type = type; } // Usually set in constructor

    QString getName() const; // Get human-readable name of the layer

    bool isVisible() const { return visible; }
    void setVisible(bool value);

    bool isLocked() const { return locked; }
    void setLocked(bool value);

    // Tile management (Layer class can also know which tiles are on it, but Map typically manages tiles directly)
    // For map editor purposes, Map usually manages global tiles and Layer simply refers to a concept/filter.
    // However, if the design keeps per-layer Tile lists within the Layer object, then these methods would apply.
    // Based on Project_QT/src/map.cpp's current tiles vector of vectors, Layer class just has visibility/name.
    // Map will contain actual tile data.
    // void addTile(Tile* tile, const QPoint& position); // Already handled in Map class, where Map manages tile locations directly.
    // void removeTile(const QPoint& position);
    // Tile* getTile(const QPoint& position) const; // Map manages this.
    // bool hasTile(const QPoint& position) const;
    // bool isEmpty() const;
    // void clear();

    // Static helper methods for type <-> string conversion
    static QString typeToString(Type type);
    static Type stringToType(const QString& str);

signals:
    void visibilityChanged(Layer::Type type, bool visible);
    void lockedChanged(Layer::Type type, bool locked);
    void changed(Layer::Type type); // Generic change signal

private:
    Type type;
    QString name;
    bool visible;
    bool locked;

    // Optional: map of tiles directly contained by this layer, if not managed by global Map class.
    // QMap<QPoint, Tile*> tiles; // This can be expensive for very large maps, let Map manage the global tiles array.
};

// Global mapping for easier lookup and avoiding enum-to-index conversion in code if not explicitly in `Map` class.
// Using this for easier array indexing if `Map::LayerCount` needs direct indexing.
static const Layer::Type LayerIndexMapping[] = {
    Layer::Ground,
    Layer::GroundDetail,
    Layer::Objects,
    Layer::Items,
    Layer::Creatures,
    Layer::Effects,
    Layer::Roofs,
    Layer::Walls, // If distinct from objects. In Tibia maps, objects/walls might be same layer, just item type is different.
    Layer::Water,
    // Add more up to Map::LayerCount - 1 (usually 15)
    Layer::Top // For 15. The 'Count' enum makes this index mapping easier.
};


#endif // LAYER_H