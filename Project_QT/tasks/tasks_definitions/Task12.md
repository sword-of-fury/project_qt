**Task12: Port `Tile` class to Qt**
- Migrate `Tile`, reworking it to be friendly with `QGraphicsItem`-based tile rendering and data retrieval. A tile contains many items (`QVector<Item*>`). Adapt tile flags and properties to Qt style using enum classes or type-safe methods where applicable.  A Tile usually signals when a change happens on it which map uses, if needed.
