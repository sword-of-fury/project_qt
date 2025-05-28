**Task60: Implement File Handling and Version Conversion for `Map` (OTBM - Full `NodeFile` IO & TileSet/ItemAttribute Serialization)**
- Task: **Fully migrate the `Map`'s OTBM loading and saving functionality from `wxwidgets map.cpp` (and related IO classes like `IOTBM`) to `project_qt`. This involves robust serialization using the `Map`'s internal structure (Tiles, Items, Houses, Spawns, etc.) and correctly handling different OTBM node types and client versions. This is the primary mechanism for persisting the entire map state.**
    - **Analyze Existing OTBM I/O:** Refactor or complete any existing OTBM I/O logic in `Project_QT/src`.
    - **Core OTBM Library/Classes:** Utilize the `NodeFileWriteHandle`/`NodeFileReadHandle` (or equivalent OTBM stream/node manipulation library ported or adopted for Qt) for all OTBM I/O.
    - **Saving (`Map::saveToOTBM` - Task 51 completion):**
        -   Write the main map attributes node (version, dimensions, description, etc.).
        -   Iterate through all `Tile`s in the `Map`. For each `Tile`:
            -   Write a `TAG_TILE` node (or `TAG_TILE_AREA` if using tile chunks).
            -   Inside the tile node, write attributes for tile flags.
            -   Iterate `Tile::items` and for each `Item`, write a `TAG_ITEM` node. Serialize the `Item`'s ID and all its attributes (using `Item::serializeToOTBM(node)` which should write to the provided OTBM item node, leveraging Task 48/55 for attribute map handling).
        -   Serialize other map-level entities (if part of the OTBM root node structure or designated sub-nodes like `TAG_HOUSES`, `TAG_SPAWNS`, `TAG_WAYPOINTS`):
            -   `Houses`: If `IOTBM::writeTileset`/`TileInfoNode` (from original task desc.) was for houses, use a similar approach, or write house data directly into a dedicated house node.
            -   `Spawns`, `Waypoints`: Serialize their data into appropriate OTBM nodes.
    - **Loading (`Map::loadFromOTBM` - Task 51 completion):**
        -   Parse the OTBM root node and its children.
        -   Read map attributes and configure the `Map` object.
        -   When parsing `TAG_TILE` (or `TAG_TILE_AREA`) nodes:
            -   Create `Tile` objects at the correct coordinates.
            -   Read tile flags.
            -   For each `TAG_ITEM` child node: Create an `Item` using `ItemManager::createItem(itemID)`, then call `item->deserializeFromOTBM(node)` to populate its attributes. Add the `Item` to the `Tile`.
        -   Deserialize `Houses`, `Spawns`, `Waypoints` from their respective OTBM nodes, creating the corresponding objects and adding them to the `Map`.
    - **Item/Doodad/Spawn/House File Formats within OTBM Nodes:** If specific map elements (like a collection of doodads for a tileset, or complex house definitions) had their own sub-format within an OTBM node (rather than just item attributes), implement the specific serialization/deserialization for that using the `ItemAttribute` map from the tile or `Item` properties, or dedicated parsing logic as `Items_OTBM` might have handled. The goal is clear data mapping.
    - **Client Version Handling & Conversions:** Crucially, read the OTBM client version. If specific item IDs, attributes, or node structures changed between versions supported by `wxwidgets`, implement the necessary conversion logic during loading or saving to maintain compatibility or upgrade maps to the editor's current target version, as the original map properties and `Item` data might depend on this.
    - **Tile State Updates (`tile_modified`):** After loading or saving tiles, or during conversion, ensure `Tile::modify()` is called and any necessary map-wide signals (like `mapChanged`) are emitted. The original `tile_change` seemed directly handled by item operations; in Qt, modifying `Tile` contents should ensure `MapView` refreshes. This might also involve re-evaluating things like `isHouseTile` or `isPZ` for tiles based on loaded items if those aren't direct tile flags in OTBM.
    - **Handling `tile->add/remove` updates:** Logic using specific `house_tilesets` from `wxwidgets` might now be replaced by `HouseBrush` placing items whose properties define them as part of a house, simplifying OTBM saving if houses are just collections of items on tiles with certain attributes rather than complex map structures, unless `House` objects themselves have internal structure that needs serializing. Waypoints and towns should also use `Map` data directly.
    - **`Task60.md` is absolutely critical. It must fully specify the OTBM node types (`TAG_XYZ`), their attributes, the byte-level layout for `Item` attributes, how different client versions are identified and handled, any specific sub-formats within OTBM nodes (e.g., for `TileSet`s or `HouseTile` data if `IOTBM::writeTileset` implies this), and how `Map` properties vs. `Tile` properties vs. `Item` properties were structured in the `wxwidgets` OTBM I/O.**


---
