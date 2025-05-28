**Task10: Migrate `wxAuiManager` docking for palettes/minimap/properties/etc. (Core Panels & Layout Management)**
- Task: **Transform the dynamic window layout system previously managed by `wxAuiManager` (or equivalent docking library in `wxwidgets`) into Qt's `QDockWidget` framework within `project_qt`'s `MainWindow`.**
    - **Analyze Existing `MainWindow` Structure (`Project_QT/src`):** Examine how `MainWindow` in `Project_QT/src` is currently structured. This task will involve adding `QDockWidget`s for specified tool windows and potentially refactoring `MainWindow` to manage them.
    - **Target Tool Windows:** Focus on migrating the docking behavior for the core tool windows explicitly mentioned:
        -   Palettes (e.g., tile palette, item palette, creature palette – create placeholder `QWidget`s for their content for now if their full classes aren't ready).
        -   Minimap window (placeholder content).
        -   Properties window (placeholder content).
        -   `propertiesDock`, `tilelistDock`, `actionlistDock`, `toolsPanel`, and `minimapWindow` (from original description if these map to specific dockable areas/widgets).
        -   Other panels designated as primary dockable elements in `Task10.md`.
    - **`QDockWidget` Implementation:** For each target tool window:
        -   Create a `QDockWidget`.
        -   Set its allowed docking areas and initial docked location (e.g., `Qt::LeftDockWidgetArea`).
        -   Set its window title.
        -   Set its content widget (a placeholder `QWidget` or the actual panel widget if already migrated/stubbed from other tasks).
        -   Add the `QDockWidget` to `MainWindow` (`addDockWidget()`).
    - **Layout Management (Initial Implementation):**
        -   Implement basic mechanisms in `MainWindow` for float/dock behavior, resizing, and showing/hiding these `QDockWidget`s (e.g., via `QAction`s in a "View" menu toggling `QDockWidget::setVisible()`).
        -   If `wxAuiManager` supported saving/loading/switching between multiple named layouts ("perspectives"): Implement *placeholder* functionality in `MainWindow` for these actions (`saveLayoutState()`, `restoreLayoutState()`). Actual saving to settings is part of Task 97.
    - **Visibility Tracking & Preferences:**
        -   If the visibility state of these panels was saved in preferences in `wxwidgets` (e.g., via `AutomagicSettingsDialog` or a main preferences window), create placeholder logic for `MainWindow` to query these settings (from a stub settings manager) on startup to set initial visibility and to save their state on exit.
    - **Focus:** The primary goal is migrating the *docking structure and fundamental management* of these key panels to `QDockWidget`s. Full content and complex interactions within these panels are covered by other tasks.
    - **`Task10.md` is critical and must detail the names of all dockable panels from `wxAuiManager`, their default locations, any specific docking restrictions, and the features of the perspective/layout management system that need to be replicated (even if initially as stubs).**


---
