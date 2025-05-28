**Task90: Add Minimap functionality (Full `MinimapWindow`, Tile Rendering, Click Navigation, View Sync)**
- Task: **Recreate a fully functional `Minimap` widget/window in `project_qt`. This includes rendering a scaled-down view of the map using tile color information, handling click events on the minimap to navigate the main `MapView`, and keeping the minimap synchronized with `MapView`'s viewport and content changes.**
    - **Analyze Existing Minimap Stubs:** Review `Project_QT/src` for any basic `Minimap` UI or rendering.
    - **`MinimapWindow` Widget:** Create `MinimapWindow` as a `QWidget` (likely to be placed in a `QDockWidget`).
    - **Rendering Logic:**
        -   `MinimapWindow::paintEvent(QPaintEvent* event)`:
            -   Get the current `Map` from `MainWindow` or `MapManager`.
            -   Iterate through all (or a relevant subset for performance on huge maps) `Tile`s of the current floor (or a composite view if configurable).
            -   For each `Tile`, determine its representative color (e.g., color of the ground item, a specific "minimap color" `Tile` property, or a color derived from `ItemType` flags on the top-most significant item).
            -   Draw a small, scaled rectangle on the `MinimapWindow` at the `Tile`'s corresponding position using this color.
        -   Optimize rendering (e.g., draw to an off-screen `QPixmap` cache that only updates when the map changes significantly).
    - **Click Navigation (`OnClick` / Mouse Events):**
        -   Implement `MinimapWindow::mousePressEvent(QMouseEvent* event)`.
        -   Convert the click coordinates on the minimap widget to logical map tile coordinates.
        -   Signal to `MainWindow` or directly call a method on `MapView` to center the main `MapView` on these map coordinates.
    - **`MapView` Viewport Synchronization (`UpdateView`):**
        -   The minimap should draw a rectangle indicating the current viewport of the main `MapView`.
        -   When `MapView` pans or zooms, it must signal `MinimapWindow` to update this viewport rectangle.
    - **Map Content Synchronization:**
        -   When the `Map` data changes (tiles edited, items added/removed), `Map` should emit signals that `MinimapWindow` listens to. `MinimapWindow` then updates its rendering/cache for the affected areas. (This replaces original `g_gui->RefreshMinimap` or `MinimapWindow` directly calling `MapPanel` refresh).
    - **Zoom Buttons (Optional):** Implement "+" / "-" buttons directly on `MinimapWindow` to zoom its own view of the map if this was a feature.
    - **Interaction with Selection Boxes:** If original minimap displayed global selection boxes (from `MapPanel`), ensure the new minimap can render these based on `MapSelection` data, using `Tile` bounds rather than pixel coordinates for a general boundingBox visualization. This involves the minimap possibly tracking selection state, similar to how `tile->select()` affected `wxwidgets` `map_display`.
    - **Hover Effects/Tooltips:** Optionally, display tile coordinates or basic info as tooltips when hovering over the minimap.
    - **`Task90.md` must provide details on: how original minimap determined tile colors, the appearance and update logic for the `MapView` viewport indicator, specific interactions for clicking, any zoom/filter options it had, and how it synchronized with `map_display` / `Tiles` / `MapView` state changes and item highlights/selection.**

    Perfect! Here are the final 10 tasks, 91 through 100, revised with the same level of detail and enhanced for Jules.
