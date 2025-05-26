#ifndef OUTFIT_H
#define OUTFIT_H

#include <QtGlobal> // For quint8, quint16 if used, or use C++ standard types

class Outfit {
public:
    Outfit() :
        lookType(0),
        lookItem(0),
        lookMount(0),
        lookAddons(0),
        lookHead(0),
        lookBody(0),
        lookLegs(0),
        lookFeet(0),
        lookMountHead(0), // Specific for Tibia 12.x+ mounts
        lookMountBody(0),
        lookMountLegs(0),
        lookMountFeet(0)
    {}

    // Members based on wxwidgets/creatures.h (CreatureType::outfit)
    // and common Tibia outfit parameters.
    // Using int for simplicity, can be changed to quint16 if specific range is critical.
    int lookType;       // Base creature looktype (sprite ID)
    int lookItem;       // Item held by creature (e.g., for typeex)
    int lookMount;      // Mount looktype (sprite ID of the mount)
    int lookAddons;     // Bitmask for creature addons (0-3, or specific values)

    // Standard outfit components
    int lookHead;
    int lookBody;
    int lookLegs;
    int lookFeet;

    // Mount outfit components (for newer Tibia versions where mounts also have parts)
    int lookMountHead;
    int lookMountBody;
    int lookMountLegs;
    int lookMountFeet;

    // Add any other relevant outfit properties if found in wxwidgets/outfit.h
    // or if needed for rendering (e.g., colors, animation details - though these
    // might be part of SpriteManager's responsibility when given a lookType).
};

#endif // OUTFIT_H
