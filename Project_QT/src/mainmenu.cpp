#include "mainmenu.h"
#include <QApplication> // For qApp->quit()
#include <QKeySequence> // For standard shortcuts
#include <QDebug>       // For debugging output
#include <QDesktopServices> // For QUrl::fromLocalFile for opening URLs
#include <QDir>         // For handling recent file paths

// Constructor
MainMenu::MainMenu(MainWindow* parent) :
    QMenuBar(parent),
    parentWindow(parent)
{
    // Create and add all top-level menus
    fileMenu = addMenu(tr("&File"));
    editMenu = addMenu(tr("&Edit"));
    viewMenu = addMenu(tr("&View"));
    mapMenu = addMenu(tr("&Map"));
    toolsMenu = addMenu(tr("&Tools"));
    addMenu(tr("&Window")); // Window menu for dock widgets and other views (Minimap)
    createNetworkMenu(); // Handles Live Editor
    helpMenu = addMenu(tr("&Help"));

    // Populate each menu with actions and connect signals/slots
    createFileMenu();
    createEditMenu();
    createViewMenu();
    createMapMenu();
    createToolsMenu();
    createHelpMenu();

    // Update actions that depend on initial state (e.g., Undo/Redo disabled)
    updateUndoRedoActions(parentWindow->getUndoStack()->canUndo(), parentWindow->getUndoStack()->canRedo());
    updateSelectionActions(parentWindow->getMap() && !parentWindow->getMap()->getSelection().isEmpty());
    updateViewActions(parentWindow->getMapView()->getShowGrid(), parentWindow->getMapView()->getShowCollisions(),
                      parentWindow->statusBar()->isVisible(), parentWindow->mainToolBar->isVisible(), parentWindow->isFullScreen());
    updateToolActions(parentWindow->getMapView()->getBrush()->getType()); // Sets the active tool based on mainwindow state.

    // Load recent files on startup
    loadRecentFiles();
}

// Destructor (QObjects handle child deletion, but explict clear if necessary)
MainMenu::~MainMenu()
{
    // Recent file actions cleanup (they are also children of `recentFilesMenu`)
    qDeleteAll(recentFileActions); // Ensure QActions are deleted.
}

// Helper to create and add actions to a menu
QAction* MainMenu::createAndAddAction(QMenu* menu, const QString& text, QObject* receiver, const char* slot, QKeySequence shortcut, bool checkable) {
    QAction* action = menu->addAction(text, receiver, slot, shortcut);
    if (checkable) {
        action->setCheckable(true);
    }
    return action;
}

void MainMenu::createFileMenu()
{
    // New, Open, Save, Save As
    createAndAddAction(fileMenu, tr("&New Map"), this, SLOT(onNewMap()), QKeySequence("P")); // Hotkey from XML
    createAndAddAction(fileMenu, tr("&Open Map..."), this, SLOT(onOpenMap()), QKeySequence::Open); // Ctrl+O
    createAndAddAction(fileMenu, tr("&Save Map"), this, SLOT(onSaveMap()), QKeySequence::Save); // Ctrl+S
    createAndAddAction(fileMenu, tr("Save Map &As..."), this, SLOT(onSaveMapAs()), QKeySequence("Ctrl+Alt+S")); // Hotkey from XML
    generateMapAction = createAndAddAction(fileMenu, tr("&Generate Map"), this, SLOT(onGenerateMap()), QKeySequence("Ctrl+Shift+G"));
    closeMapAction = createAndAddAction(fileMenu, tr("&Close"), this, SLOT(onCloseMap()), QKeySequence("Shift+B"));
    
    fileMenu->addSeparator();

    // Import Submenu
    importSubMenu = fileMenu->addMenu(tr("&Import"));
    importMapAction = createAndAddAction(importSubMenu, tr("Import Map..."), this, SLOT(onImportMapFile()));
    importMonstersAction = createAndAddAction(importSubMenu, tr("Import Monsters/NPC..."), this, SLOT(onImportMonsters()));

    // Export Submenu
    exportSubMenu = fileMenu->addMenu(tr("&Export"));
    createAndAddAction(exportSubMenu, tr("Export Minimap..."), this, SLOT(onExportMinimap())); // Assuming onExportMinimap exists
    exportTilesetsAction = createAndAddAction(exportSubMenu, tr("Export Tilesets..."), this, SLOT(onExportTilesets()));

    // Reload Submenu
    reloadSubMenu = fileMenu->addMenu(tr("&Reload"));
    reloadDataAction = createAndAddAction(reloadSubMenu, tr("Reload Data"), this, SLOT(onReloadData()), QKeySequence(Qt::Key_F5));
    
    fileMenu->addSeparator();

    // Recent Files Submenu
    recentFilesMenu = new QMenu(tr("Open Recent"), this);
    fileMenu->addMenu(recentFilesMenu);
    for (int i = 0; i < MaxRecentFiles; ++i) {
        recentFileActions.append(new QAction(this));
        recentFileActions.last()->setVisible(false);
        connect(recentFileActions.last(), &QAction::triggered, this, &MainMenu::onOpenRecent);
        recentFilesMenu->addAction(recentFileActions.last());
    }
    separatorAction = recentFilesMenu->addSeparator(); // Separator within recent files menu if needed, or place in fileMenu
    fileMenu->insertMenu(separatorAction, recentFilesMenu); // Insert recent files menu before the separator leading to Preferences
    updateRecentFilesMenu();

    // Preferences & Exit
    createAndAddAction(fileMenu, tr("&Preferences..."), this, SLOT(onPreferences()), QKeySequence("Ctrl+Shift+V"));
    createAndAddAction(fileMenu, tr("E&xit"), this, SLOT(onExit()), QKeySequence::Quit);

    // Load SPR/DAT (existing custom action, keep if needed, or integrate into Reload)
    createAndAddAction(fileMenu, tr("Load SPR/DAT..."), this, SLOT(onLoadSprDat()));
}

void MainMenu::createEditMenu()
{
    // Undo/Redo
    createAndAddAction(editMenu, tr("&Undo"), this, SLOT(onUndo()), QKeySequence::Undo); // Ctrl+Z
    createAndAddAction(editMenu, tr("&Redo"), this, SLOT(onRedo()), QKeySequence("Ctrl+Shift+Z")); // Hotkey from XML
    editMenu->addSeparator();

    // Replace / Refresh
    replaceItemsAction = createAndAddAction(editMenu, tr("&Replace Items..."), this, SLOT(onReplaceItems()), QKeySequence("Ctrl+Shift+F"));
    refreshItemsAction = createAndAddAction(editMenu, tr("Refresh Items"), this, SLOT(onRefreshItems()));
    editMenu->addSeparator();

    // Border Options Submenu
    borderOptionsSubMenu = editMenu->addMenu(tr("Border Options"));
    automagicAction = createAndAddAction(borderOptionsSubMenu, tr("Border Automagic"), this, SLOT(onToggleBorderAutomagic()), QKeySequence(Qt::Key_A));
    automagicAction->setCheckable(true); // This is a toggle
    borderOptionsSubMenu->addSeparator();
    borderizeSelectionAction = createAndAddAction(borderOptionsSubMenu, tr("Borderize Selection"), this, SLOT(onBorderizeSelection()), QKeySequence("Ctrl+B"));
    borderizeMapAction = createAndAddAction(borderOptionsSubMenu, tr("Borderize Map"), this, SLOT(onBorderizeMap()));
    randomizeSelectionAction = createAndAddAction(borderOptionsSubMenu, tr("Randomize Selection"), this, SLOT(onRandomizeSelection()));
    randomizeMapAction = createAndAddAction(borderOptionsSubMenu, tr("Randomize Map"), this, SLOT(onRandomizeMap()));

    // Other Options Submenu
    otherOptionsSubMenu = editMenu->addMenu(tr("Other Options"));
    removeUnreachableTilesAction = createAndAddAction(otherOptionsSubMenu, tr("Remove all Unreachable Tiles..."), this, SLOT(onMapRemoveUnreachable())); // Slot exists
    clearInvalidHousesAction = createAndAddAction(otherOptionsSubMenu, tr("Clear Invalid Houses"), this, SLOT(onClearHouseTiles()));       // Slot exists
    clearModifiedStateAction = createAndAddAction(otherOptionsSubMenu, tr("Clear Modified State"), this, SLOT(onClearModifiedState()));     // Slot exists
    
    editMenu->addSeparator();
    
    // Cut/Copy/Paste (Delete is often separate or handled by context)
    createAndAddAction(editMenu, tr("Cu&t"), this, SLOT(onCut()), QKeySequence::Cut); // Ctrl+X
    createAndAddAction(editMenu, tr("&Copy"), this, SLOT(onCopy()), QKeySequence::Copy); // Ctrl+C
    createAndAddAction(editMenu, tr("&Paste"), this, SLOT(onPaste()), QKeySequence::Paste); // Ctrl+V
    
    // Delete action is usually context-dependent or has its own key (Del).
    // It's in the original Edit menu, so we keep it.
    createAndAddAction(editMenu, tr("&Delete"), this, SLOT(onDelete()), QKeySequence::Delete); 
    editMenu->addSeparator();

    // Selection commands
    createAndAddAction(editMenu, tr("Select &All"), this, SLOT(onSelectAll()), QKeySequence::SelectAll);
    createAndAddAction(editMenu, tr("&Deselect All"), this, SLOT(onDeselectAll()));
}

void MainMenu::createViewMenu()
{
    // Items from "Editor" menu in XML
    newViewAction = createAndAddAction(viewMenu, tr("New View"), this, SLOT(onNewView()), QKeySequence("Ctrl+Shift+N"));
    newDetachedViewAction = createAndAddAction(viewMenu, tr("New Detached View"), this, SLOT(onNewDetachedView()), QKeySequence("Ctrl+Shift+D"));
    takeScreenshotAction = createAndAddAction(viewMenu, tr("Take Screenshot"), this, SLOT(onTakeScreenshot()), QKeySequence(Qt::Key_F10));
    viewMenu->addSeparator();

    // Zoom controls (already exist, ensure they are here)
    createAndAddAction(viewMenu, tr("Zoom &In"), this, SLOT(onZoomIn()), QKeySequence::ZoomIn); // Ctrl++
    createAndAddAction(viewMenu, tr("Zoom &Out"), this, SLOT(onZoomOut()), QKeySequence::ZoomOut); // Ctrl+-
    createAndAddAction(viewMenu, tr("Zoom &Normal"), this, SLOT(onZoomReset()), QKeySequence("Ctrl+0")); // Ctrl+0
    viewMenu->addSeparator();

    // UI visibility toggles (already exist, ensure they are here)
    createAndAddAction(viewMenu, tr("Show Status &Bar"), this, SLOT(onToggleStatusBar()), Qt::NoModifier, true)->setChecked(true);
    createAndAddAction(viewMenu, tr("Show Tool&Bar"), this, SLOT(onToggleToolbar()), Qt::NoModifier, true)->setChecked(true);
    createAndAddAction(viewMenu, tr("Toggle &Fullscreen"), this, SLOT(onToggleFullscreen()), QKeySequence::FullScreen); // F11
    viewMenu->addSeparator();

    // Items from "View" menu in XML
    showAllFloorsAction = createAndAddAction(viewMenu, tr("Show all Floors"), this, SLOT(onToggleShowAllFloors()), QKeySequence("Ctrl+W"), true);
    showAsMinimapAction = createAndAddAction(viewMenu, tr("Show as Minimap"), this, SLOT(onToggleShowAsMinimap()), QKeySequence("Shift+E"), true);
    showOnlyColorsAction = createAndAddAction(viewMenu, tr("Only show Colors"), this, SLOT(onToggleShowOnlyColors()), QKeySequence("Ctrl+E"), true);
    showOnlyModifiedAction = createAndAddAction(viewMenu, tr("Only show Modified"), this, SLOT(onToggleShowOnlyModified()), QKeySequence("Ctrl+M"), true);
    alwaysShowZonesAction = createAndAddAction(viewMenu, tr("Always show zones"), this, SLOT(onToggleAlwaysShowZones()), QKeySequence(), true);
    extendedHouseShaderAction = createAndAddAction(viewMenu, tr("Extended house shader"), this, SLOT(onToggleExtendedHouseShader()), QKeySequence(), true);
    viewMenu->addSeparator();
    showTooltipsAction = createAndAddAction(viewMenu, tr("Show tooltips"), this, SLOT(onToggleShowTooltips()), QKeySequence(Qt::Key_Y), true);
    createAndAddAction(viewMenu, tr("Show &Grid"), this, SLOT(onToggleGrid()), QKeySequence("Shift+G"), true)->setChecked(true); // Existing, ensure hotkey
    showClientBoxAction = createAndAddAction(viewMenu, tr("Show client box"), this, SLOT(onToggleShowClientBox()), QKeySequence("Shift+I"), true);
    viewMenu->addSeparator();
    ghostItemsAction = createAndAddAction(viewMenu, tr("Ghost loose items"), this, SLOT(onToggleGhostItems()), QKeySequence(Qt::Key_G), true);
    ghostHigherFloorsAction = createAndAddAction(viewMenu, tr("Ghost higher floors"), this, SLOT(onToggleGhostHigherFloors()), QKeySequence("Ctrl+L"), true);
    showShadeAction = createAndAddAction(viewMenu, tr("Show shade"), this, SLOT(onToggleShowShade()), QKeySequence(Qt::Key_Q), true);
    viewMenu->addSeparator();

    // Items from "Show" menu in XML (consolidated into View menu)
    showAnimationAction = createAndAddAction(viewMenu, tr("Show Animation"), this, SLOT(onToggleShowAnimation()), QKeySequence(Qt::Key_N), true);
    showLightAction = createAndAddAction(viewMenu, tr("Show Light"), this, SLOT(onToggleShowLight()), QKeySequence(Qt::Key_H), true);
    showLightStrengthAction = createAndAddAction(viewMenu, tr("Show Light Strength"), this, SLOT(onToggleShowLightStrength()), QKeySequence(Qt::AltModifier + Qt::Key_F3), true);
    showTechnicalItemsAction = createAndAddAction(viewMenu, tr("Show Technical Items"), this, SLOT(onToggleShowTechnicalItems()), QKeySequence(Qt::AltModifier + Qt::Key_F4), true);
    viewMenu->addSeparator();
    showZonesAction = createAndAddAction(viewMenu, tr("Show zones"), this, SLOT(onToggleShowZones()), QKeySequence("Shift+N"), true);
    showCreaturesAction = createAndAddAction(viewMenu, tr("Show creatures"), this, SLOT(onToggleShowCreatures()), QKeySequence(Qt::AltModifier + Qt::Key_F5), true);
    showSpawnsAction = createAndAddAction(viewMenu, tr("Show spawns"), this, SLOT(onToggleShowSpawns()), QKeySequence(Qt::AltModifier + Qt::Key_F6), true);
    showSpecialTilesAction = createAndAddAction(viewMenu, tr("Show special"), this, SLOT(onToggleShowSpecialTiles()), QKeySequence(Qt::AltModifier + Qt::Key_F7), true); // Special tiles like PZ
    showHousesAction = createAndAddAction(viewMenu, tr("Show houses"), this, SLOT(onToggleShowHouses()), QKeySequence(Qt::AltModifier + Qt::Key_F7), true); // Note: duplicate hotkey with "Show special" in XML
    showPathingAction = createAndAddAction(viewMenu, tr("Show pathing"), this, SLOT(onToggleShowPathing()), QKeySequence(Qt::AltModifier + Qt::Key_F8), true);
    showTownsAction = createAndAddAction(viewMenu, tr("Show towns"), this, SLOT(onToggleShowTowns()), QKeySequence(Qt::AltModifier + Qt::Key_F9), true);
    showWaypointsAction = createAndAddAction(viewMenu, tr("Show waypoints"), this, SLOT(onToggleShowWaypoints()), QKeySequence(Qt::AltModifier + Qt::Key_F10), true);
    viewMenu->addSeparator();
    highlightItemsAction = createAndAddAction(viewMenu, tr("Highlight Items"), this, SLOT(onToggleHighlightItems()), QKeySequence(Qt::AltModifier + Qt::Key_F11), true);
    highlightLockedDoorsAction = createAndAddAction(viewMenu, tr("Highlight Locked Doors"), this, SLOT(onToggleHighlightLockedDoors()), QKeySequence(Qt::AltModifier + Qt::Key_F12), true);
    showWallHooksAction = createAndAddAction(viewMenu, tr("Show Wall Hooks"), this, SLOT(onToggleShowWallHooks()), QKeySequence(Qt::Key_K), true);
    
    // Floor (Layer) menu dynamically populated
    QMenu* floorMenu = viewMenu->addMenu(tr("F&loor"));
    for (int i = 0; i < Map::LayerCount; ++i) { 
        QAction* floorAction = floorMenu->addAction(tr("Floor %1").arg(i), this, [this, i](){
            parentWindow->changeFloor(i);
            updateLayerActions(i);
        });
        floorAction->setCheckable(true);
        if (i < 12) floorAction->setShortcut(QKeySequence(QString("F%1").arg(i + 1)));
    }
    updateLayerActions(parentWindow->getCurrentLayer()); 

    // View Settings (e.g. LOD, Fog - from Source/src/main_menubar.cpp)
    // Assuming onChangeViewSettings is a placeholder for a dialog that groups these
    // viewMenu->addSeparator();
    // createAndAddAction(viewMenu, tr("View &Settings..."), this, SLOT(onChangeViewSettings())); 
}

void MainMenu::createMapMenu()
{
    createAndAddAction(mapMenu, tr("Map &Properties..."), this, SLOT(onMapProperties()), QKeySequence("Ctrl+P"));
    createAndAddAction(mapMenu, tr("Map &Statistics..."), this, SLOT(onMapStatistics()), QKeySequence(Qt::Key_F8));
    createAndAddAction(mapMenu, tr("Go To &Position..."), this, SLOT(onGotoPosition())); // No hotkey in XML, but common
    // createAndAddAction(mapMenu, tr("Clear &Modified State"), this, SLOT(onClearModifiedState())); // This is in Edit->Other Options in XML

    mapMenu->addSeparator();
    createAndAddAction(mapMenu, tr("Edit &Towns..."), this, SLOT(onMapEditTowns()), QKeySequence("Ctrl+T"));
    mapMenu->addSeparator();
    
    // Cleanup Actions from XML "Map" menu
    createAndAddAction(mapMenu, tr("Cleanup..."), this, SLOT(onMapCleanup())); 
    // Other cleanup actions from XML "Idler" menu or "Edit->Other Options"
    // are handled in createEditMenu or specific MainWindow methods.
    // "Remove Duplicates..."
    createAndAddAction(mapMenu, tr("Remove &Duplicates (on map)..."), this, SLOT(onMapRemoveDuplicates()));
    // "Validate Ground..." (Not in XML, but was in previous MainMenu::createMapMenu)
    createAndAddAction(mapMenu, tr("&Validate Ground (on map)..."), this, SLOT(onMapValidateGround()));


    // These were in the old createMapMenu, but XML places them differently or they are more specific.
    // createAndAddAction(mapMenu, tr("&Borderize Selection (Ctrl+B)"), this, SLOT(onBorderizeSelection())); // In Edit->Border Options
    // createAndAddAction(mapMenu, tr("Borderize Map..."), this, SLOT(onBorderizeMap())); // In Edit->Border Options
    // createAndAddAction(mapMenu, tr("Randomize Selection"), this, SLOT(onRandomizeSelection())); // In Edit->Border Options
    // createAndAddAction(mapMenu, tr("Randomize Map..."), this, SLOT(onRandomizeMap())); // In Edit->Border Options
    // createAndAddAction(mapMenu, tr("Remove Items (on map)..."), this, SLOT(onMapRemoveItems())); // In Idler
    // createAndAddAction(mapMenu, tr("Remove Corpses (on map)..."), this, SLOT(onMapRemoveCorpses())); // In Idler
    // createAndAddAction(mapMenu, tr("Remove &Unreachable Areas (on map)..."), this, SLOT(onMapRemoveUnreachable())); // In Edit->Other Options
    // createAndAddAction(mapMenu, tr("Clean &House Items (on map)..."), this, SLOT(onMapCleanHouseItems())); // In Edit->Other Options
    // createAndAddAction(mapMenu, tr("Edit &Items (on map)..."), this, SLOT(onMapEditItems())); // Legacy
    // createAndAddAction(mapMenu, tr("Edit &Monsters (on map)..."), this, SLOT(onMapEditMonsters())); // Legacy
}

void MainMenu::createToolsMenu()
{
    // Brush Tool Selection
    createAndAddAction(toolsMenu, tr("&Normal Brush"), this, SLOT(onBrushTool()), QKeySequence(Qt::Key_N), true);
    createAndAddAction(toolsMenu, tr("&Eraser"), this, SLOT(onEraserTool()), QKeySequence(Qt::Key_E), true);
    createAndAddAction(toolsMenu, tr("Sele&ction"), this, SLOT(onSelectionTool()), QKeySequence(Qt::Key_S), true);
    createAndAddAction(toolsMenu, tr("F&lood Fill"), this, SLOT(onFloodFillTool()), QKeySequence(Qt::Key_F), true);
    toolsMenu->addSeparator();

    // Specialized Creation Tools from XML "Tools" menu (if any) or general editor tools
    createAndAddAction(toolsMenu, tr("Island &Generator..."), this, SLOT(onGenerateIsland())); // Action: GENERATE_ISLAND from XML
    createAndAddAction(toolsMenu, tr("&Border Editor..."), this, SLOT(onCreateBorder()));     // Action: CREATE_BORDER from XML
    tilesetEditorAction = createAndAddAction(toolsMenu, tr("&Tileset Editor..."), this, SLOT(onTilesetEditor())); // No specific XML action, but common tool
    selectionToDoodadAction = createAndAddAction(toolsMenu, tr("Selection to Doodad"), this, SLOT(onSelectionToDoodad())); // No specific XML action
    toolsMenu->addSeparator();

    // Automagic toggle was in Edit->Border Options via XML (action: AUTOMAGIC), but also fits Tools
    // Re-using automagicAction from Edit menu if it's global, or create a new one if context specific.
    // For now, assuming the one in Edit menu is the primary toggle.
    // If a dedicated Tools menu toggle is needed:
    // createAndAddAction(toolsMenu, tr("Toggle &Automagic Borders"), this, SLOT(onToggleBorderAutomagic()), QKeySequence(Qt::Key_A), true);


    // Connect radio-like behavior for tools
    QActionGroup* toolGroup = new QActionGroup(this);
    toolGroup->setExclusive(true);
    // Add previously created tool actions to this group
    // Ensure actions are added in the order they appear above the separator.
    for(int i=0; i < 4 && i < toolsMenu->actions().size(); ++i) { // Only add the first 4 (brush tools)
        toolGroup->addAction(toolsMenu->actions().at(i));
    }
    
    updateToolActions(parentWindow->getMapView()->getBrush()->getType());
}

void MainMenu::createNetworkMenu() {
    QMenu* networkMenu = addMenu(tr("&Network"));
    createAndAddAction(networkMenu, tr("Start &Live Session..."), this, SLOT(onStartLive()));
    createAndAddAction(networkMenu, tr("Join &Live Session..."), this, SLOT(onJoinLive()));
    createAndAddAction(networkMenu, tr("Close &Live Session"), this, SLOT(onCloseLive()));
    networkMenu->addSeparator();
    createAndAddAction(networkMenu, tr("Show &Status..."), this, SLOT(onLiveStatus()));
}


void MainMenu::createHelpMenu()
{
    createAndAddAction(helpMenu, tr("&Help Topics"), this, SLOT(onHelp()));
    createAndAddAction(helpMenu, tr("Show Hotkeys..."), this, SLOT(onShowHotkeys()));
    createAndAddAction(helpMenu, tr("Reload Data Files..."), this, SLOT(onReloadDataFiles()));
    createAndAddAction(helpMenu, tr("List Extensions..."), this, SLOT(onListExtensions()));
    helpMenu->addSeparator();
    createAndAddAction(helpMenu, tr("Go to &Website"), this, SLOT(onGotoWebsite()));
    helpMenu->addSeparator();
    createAndAddAction(helpMenu, tr("&About"), this, SLOT(onAbout()));
    createAndAddAction(helpMenu, tr("About Qt"), qApp, SLOT(aboutQt())); // Standard Qt about
    createAndAddAction(helpMenu, tr("Debug View Dat"), this, SLOT(onDebugViewDat()));
}


// --- Dynamic Update Methods (Control UI states) ---

void MainMenu::updateRecentFilesMenu()
{
    // Clear existing recent file actions (except for separators).
    while (recentFilesMenu->actions().count() > 0 && recentFilesMenu->actions().at(0) != separatorAction) {
        QAction* oldAction = recentFilesMenu->actions().at(0);
        recentFilesMenu->removeAction(oldAction);
        delete oldAction;
    }
    
    // Retrieve recent files from settings (assume QSettings used by MainWindow).
    QSettings settings;
    QStringList recentFiles = settings.value("recentFiles").toStringList();
    
    // Populate with actual files
    int numRecentFiles = qMin(recentFiles.size(), MaxRecentFiles);
    for (int i = 0; i < numRecentFiles; ++i) {
        QString fileName = recentFiles[i];
        QString text = QString("&%1 %2").arg(i + 1).arg(QFileInfo(fileName).fileName());
        
        QAction* action = recentFileActions.at(i);
        action->setText(text);
        action->setData(fileName); // Store the full path in the action's data
        action->setVisible(true);
        recentFilesMenu->insertAction(separatorAction, action); // Insert before separator
    }
    
    // Hide unused actions and separator if no recent files.
    for (int i = numRecentFiles; i < MaxRecentFiles; ++i) {
        recentFileActions.at(i)->setVisible(false);
    }
    separatorAction->setVisible(numRecentFiles > 0);
}

void MainMenu::addRecentFile(const QString& filePath) {
    if (filePath.isEmpty()) return;

    QSettings settings;
    QStringList recentFiles = settings.value("recentFiles").toStringList();

    // Remove if already exists to move to front.
    recentFiles.removeAll(filePath);

    // Add to front.
    recentFiles.prepend(filePath);

    // Keep only MaxRecentFiles.
    while (recentFiles.size() > MaxRecentFiles) {
        recentFiles.removeLast();
    }
    settings.setValue("recentFiles", recentFiles);
    updateRecentFilesMenu(); // Refresh the UI.
}

void MainMenu::loadRecentFiles() {
    updateRecentFilesMenu(); // Just trigger update for loading from settings.
}

void MainMenu::saveRecentFiles() const {
    // Already handled implicitly by addRecentFile modifying QSettings.
}

void MainMenu::updateUndoRedoActions(bool canUndo, bool canRedo)
{
    // Find "Undo" and "Redo" actions by text/objectName.
    // Assuming unique text for direct access. For larger menus, store pointers.
    QAction* undoAction = findChild<QAction*>(QString("Undo"));
    QAction* redoAction = findChild<QAction*>(QString("Redo"));

    // Update state of menu items.
    if (undoAction) undoAction->setEnabled(canUndo);
    if (redoAction) redoAction->setEnabled(canRedo);
}

void MainMenu::updateSelectionActions(bool hasSelection)
{
    // Actions are usually by objectName for specific enabling
    QAction* cutAction = findChild<QAction*>(QString("Cut"));
    QAction* copyAction = findChild<QAction*>(QString("Copy"));
    QAction* deleteAction = findChild<QAction*>(QString("Delete"));
    // Paste action enabled by clipboard data itself.
    QAction* pasteAction = findChild<QAction*>(QString("Paste"));
    if (pasteAction) pasteAction->setEnabled(ClipboardData::getInstance().hasClipboardData());

    if (cutAction) cutAction->setEnabled(hasSelection);
    if (copyAction) copyAction->setEnabled(hasSelection);
    if (deleteAction) deleteAction->setEnabled(hasSelection);

    // Select/Deselect all (always enabled if map exists, or conditional by Map content)
    QAction* selectAllAction = findChild<QAction*>(QString("Select All"));
    QAction* deselectAllAction = findChild<QAction*>(QString("Deselect All"));
    if (parentWindow->getMap()) {
        if (selectAllAction) selectAllAction->setEnabled(true);
        if (deselectAllAction) deselectAllAction->setEnabled(hasSelection);
    } else {
        if (selectAllAction) selectAllAction->setEnabled(false);
        if (deselectAllAction) deselectAllAction->setEnabled(false);
    }

    // Other selection-dependent items from main_menubar.h
    QAction* searchItemOnSelectionAction = findChild<QAction*>(QString("Search for Item on Selection"));
    QAction* replaceItemsOnSelectionAction = findChild<QAction*>(QString("Replace Items on Selection..."));
    QAction* removeItemOnSelectionAction = findChild<QAction*>(QString("Remove Item on Selection..."));
    // ... etc. (Need to find by their objectNames, not all created explicitly above)

    if (searchItemOnSelectionAction) searchItemOnSelectionAction->setEnabled(hasSelection);
    if (replaceItemsOnSelectionAction) replaceItemsOnSelectionAction->setEnabled(hasSelection);
    if (removeItemOnSelectionAction) removeItemOnSelectionAction->setEnabled(hasSelection);

    // Toolbar (via MainWindow connection in toolbar setup)
}

void MainMenu::updateLayerActions(int currentLayer)
{
    // Update radio buttons in Floor menu (from Source/src/main_menubar.cpp).
    // Find all floor actions and check the one matching currentLayer.
    QList<QAction*> floorActions = viewMenu->findChildren<QAction*>(QString(), Qt::FindDirectChildrenOnly);
    for (QAction* action : floorActions) {
        // Assume text is "Floor X" and extract X
        if (action->text().startsWith(tr("Floor "))) {
            bool ok;
            int floorId = action->text().mid(tr("Floor ").length()).toInt(&ok);
            if (ok && floorId == currentLayer) {
                action->setChecked(true); // Check the action for the current layer
            } else {
                action->setChecked(false);
            }
        }
    }
}

void MainMenu::updateViewActions(bool showGrid, bool showCollisions, bool showStatusBar, bool showToolBar, bool isFullscreen)
{
    // Update checked state for view toggles.
    if (QAction* toggleGridAction = findChild<QAction*>(QString("Show Grid"))) {
        toggleGridAction->setChecked(showGrid);
    }
    if (QAction* toggleCollisionsAction = findChild<QAction*>(QString("Show Collisions"))) {
        toggleCollisionsAction->setChecked(showCollisions);
    }
    if (QAction* toggleStatusBarAction = findChild<QAction*>(QString("Show Status Bar"))) {
        toggleStatusBarAction->setChecked(showStatusBar);
    }
    if (QAction* toggleToolBarAction = findChild<QAction*>(QString("Show ToolBar"))) {
        toggleToolBarAction->setChecked(showToolBar);
    }
    if (QAction* toggleFullscreenAction = findChild<QAction*>(QString("Toggle Fullscreen"))) {
        toggleFullscreenAction->setChecked(isFullscreen);
    }
    if (QAction* toggleAutomagicAction = findChild<QAction*>(QString("Toggle Automagic Borders (A)"))) {
        toggleAutomagicAction->setChecked(parentWindow->getBorderSystem()->isEnabled()); // Directly get state from BorderSystem.
    }
}

void MainMenu::updateToolActions(Brush::Type currentToolType)
{
    // Update checked state for brush tool menu items (radio group effect).
    QList<QAction*> toolActions = toolsMenu->actions();
    for (QAction* action : toolActions) {
        // Compare text to BrushManager names for mapping.
        QString actionName = action->text().simplified().remove('&'); // Remove '&' for comparison.
        if (actionName.startsWith(tr("Normal Brush")) && currentToolType == Brush::Type::Normal) {
            action->setChecked(true);
        } else if (actionName.startsWith(tr("Eraser")) && currentToolType == Brush::Type::Eraser) {
            action->setChecked(true);
        } else if (actionName.startsWith(tr("Selection")) && currentToolType == Brush::Type::Selection) {
            action->setChecked(true);
        } else if (actionName.startsWith(tr("Flood Fill")) && currentToolType == Brush::Type::FloodFill) {
            action->setChecked(true);
        } else {
            action->setChecked(false); // Deselect others
        }
    }
}

// --- File Menu Slots (Delegate to MainWindow) ---

void MainMenu::onNewMap() { parentWindow->createNewMap(); }
void MainMenu::onOpenMap() { parentWindow->openMap(); }
void MainMenu::onOpenRecent() {
    QAction* action = qobject_cast<QAction*>(sender());
    if (action) {
        parentWindow->loadMap(action->data().toString());
    }
}
void MainMenu::onSaveMap() { parentWindow->saveMap(); }
void MainMenu::onSaveMapAs() { parentWindow->saveMapAs(); }
void MainMenu::onImportMap() {
    QMessageBox::information(parentWindow, tr("Import"), tr("Import Map not yet implemented."));
}
void MainMenu::onExportMap() {
    QMessageBox::information(parentWindow, tr("Export"), tr("Export Map not yet implemented."));
}
void MainMenu::onExit() { QApplication::quit(); } // Exit the application.
void MainMenu::onPreferences() {
    // This needs to open the Preferences Dialog (`PreferencesWindow` from Source).
    QMessageBox::information(parentWindow, tr("Preferences"), tr("Preferences dialog not yet implemented."));
}

void MainMenu::onLoadSprDat() {
    parentWindow->loadSprDatFiles();
}

// --- New File Menu Slot Implementations ---
void MainMenu::onGenerateMap() { 
    parentWindow->generateMap(); 
}
void MainMenu::onCloseMap() { 
    parentWindow->closeMap(); 
}
void MainMenu::onImportMapFile() { 
    parentWindow->importMapFile();
}
void MainMenu::onImportMonsters() { 
    parentWindow->importMonsters();
}
void MainMenu::onExportTilesets() { 
    parentWindow->exportTilesets();
}
void MainMenu::onReloadData() { 
    parentWindow->reloadData(); 
}


// --- Edit Menu Slots ---
void MainMenu::onUndo() { parentWindow->undo(); }
void MainMenu::onRedo() { parentWindow->redo(); }
void MainMenu::onCut() { parentWindow->cutSelection(); }
void MainMenu::onCopy() { parentWindow->copySelection(); }
void MainMenu::onPaste() { parentWindow->pasteSelection(); }
void MainMenu::onDelete() { parentWindow->deleteSelection(); }
void MainMenu::onSelectAll() { parentWindow->selectAll(); }
void MainMenu::onDeselectAll() { parentWindow->deselectAll(); }

// Search/Replace/Cleanup related from main_menubar.cpp, delegating to MainWindow placeholders
void MainMenu::onFindItem() { parentWindow->showFindItemDialog(); }
void MainMenu::onFindCreature() { parentWindow->showFindCreatureDialog(); }
void MainMenu::onFindSimilarItems() { parentWindow->showFindSimilarItemsDialog(); }
// void MainMenu::onReplaceItems() { QMessageBox::information(parentWindow, tr("Replace Items"), tr("Replace Items dialog not yet implemented.")); } // Now a new slot
void MainMenu::onMapCleanup() { QMessageBox::information(parentWindow, tr("Map Cleanup"), tr("Map Cleanup dialog not yet implemented.")); }
void MainMenu::onClearHouseTiles() { 
    // parentWindow->clearInvalidHouses(); // Call MainWindow method
    QMessageBox::information(parentWindow, tr("Clear House Tiles"), tr("Clear Invalid House Tiles not yet implemented.")); 
}
void MainMenu::onClearModifiedState() { 
    parentWindow->getMap()->setModified(false); 
    parentWindow->updateWindowTitle(); 
} 
void MainMenu::onJumpToBrush() { QMessageBox::information(parentWindow, tr("Jump to Brush"), tr("Jump to Brush dialog not yet implemented.")); }
void MainMenu::onJumpToItemBrush() { QMessageBox::information(parentWindow, tr("Jump to Item Brush"), tr("Jump to Item Brush dialog not yet implemented.")); }

// --- New Edit Menu Slot Implementations ---
void MainMenu::onReplaceItems() {
    // parentWindow->replaceItems(); // Assuming MainWindow will have this
    QMessageBox::information(parentWindow, tr("Replace Items"), tr("Replace Items not yet implemented."));
}
void MainMenu::onRefreshItems() {
    // parentWindow->refreshItems();
    QMessageBox::information(parentWindow, tr("Refresh Items"), tr("Refresh Items not yet implemented."));
}
void MainMenu::onToggleBorderAutomagic() {
    bool checked = automagicAction->isChecked();
    parentWindow->toggleBorderSystem(checked); // Pass checked state to MainWindow
    // The updateViewActions should reflect the check state if BorderSystem signals back or is polled.
}
void MainMenu::onBorderizeSelection() {
    parentWindow->borderizeSelection();
}
void MainMenu::onBorderizeMap() {
    parentWindow->borderizeMap();
}
void MainMenu::onRandomizeSelection() {
    parentWindow->randomizeSelection();
}
void MainMenu::onRandomizeMap() {
    parentWindow->randomizeMap();
}

// --- View Menu Slots ---
void MainMenu::onZoomIn() { parentWindow->zoomIn(); }
void MainMenu::onZoomOut() { parentWindow->zoomOut(); }
void MainMenu::onZoomReset() { parentWindow->zoomReset(); }
void MainMenu::onToggleGrid() { parentWindow->toggleGrid(qobject_cast<QAction*>(sender())->isChecked()); }
void MainMenu::onToggleCollisions() { parentWindow->toggleCollisions(qobject_cast<QAction*>(sender())->isChecked()); }
void MainMenu::onToggleStatusBar() { parentWindow->toggleStatusBar(qobject_cast<QAction*>(sender())->isChecked()); }
void MainMenu::onToggleToolbar() { parentWindow->toggleToolbar(qobject_cast<QAction*>(sender())->isChecked()); }
void MainMenu::onToggleFullscreen() { parentWindow->toggleFullscreen(); }
// void MainMenu::onChangeViewSettings() { QMessageBox::information(parentWindow, tr("View Settings"), tr("View Settings dialog not yet implemented (LOD, fog etc).")); }

// New View Menu Slot Implementations
void MainMenu::onNewView() { QMessageBox::information(parentWindow, tr("New View"), tr("New View not implemented.")); }
void MainMenu::onNewDetachedView() { QMessageBox::information(parentWindow, tr("New Detached View"), tr("New Detached View not implemented.")); }
void MainMenu::onTakeScreenshot() { parentWindow->takeScreenshot(); }
void MainMenu::onToggleShowAllFloors() { parentWindow->toggleShowAllFloors(qobject_cast<QAction*>(sender())->isChecked()); }
void MainMenu::onToggleShowAsMinimap() { parentWindow->toggleShowAsMinimap(qobject_cast<QAction*>(sender())->isChecked()); }
void MainMenu::onToggleShowOnlyColors() { parentWindow->toggleShowOnlyColors(qobject_cast<QAction*>(sender())->isChecked()); }
void MainMenu::onToggleShowOnlyModified() { parentWindow->toggleShowOnlyModified(qobject_cast<QAction*>(sender())->isChecked()); }
void MainMenu::onToggleAlwaysShowZones() { parentWindow->toggleAlwaysShowZones(qobject_cast<QAction*>(sender())->isChecked()); }
void MainMenu::onToggleExtendedHouseShader() { parentWindow->toggleExtendedHouseShader(qobject_cast<QAction*>(sender())->isChecked()); }
void MainMenu::onToggleShowTooltips() { parentWindow->toggleShowTooltips(qobject_cast<QAction*>(sender())->isChecked()); }
void MainMenu::onToggleShowClientBox() { parentWindow->toggleShowClientBox(qobject_cast<QAction*>(sender())->isChecked()); }
void MainMenu::onToggleGhostItems() { parentWindow->toggleGhostItems(qobject_cast<QAction*>(sender())->isChecked()); }
void MainMenu::onToggleGhostHigherFloors() { parentWindow->toggleGhostHigherFloors(qobject_cast<QAction*>(sender())->isChecked()); }
void MainMenu::onToggleShowShade() { parentWindow->toggleShowShade(qobject_cast<QAction*>(sender())->isChecked()); }
void MainMenu::onToggleShowAnimation() { parentWindow->toggleShowAnimation(qobject_cast<QAction*>(sender())->isChecked()); }
void MainMenu::onToggleShowLight() { parentWindow->toggleShowLight(qobject_cast<QAction*>(sender())->isChecked()); }
void MainMenu::onToggleShowLightStrength() { parentWindow->toggleShowLightStrength(qobject_cast<QAction*>(sender())->isChecked()); }
void MainMenu::onToggleShowTechnicalItems() { parentWindow->toggleShowTechnicalItems(qobject_cast<QAction*>(sender())->isChecked()); }
void MainMenu::onToggleShowZones() { parentWindow->toggleShowZones(qobject_cast<QAction*>(sender())->isChecked()); }
void MainMenu::onToggleShowCreatures() { parentWindow->toggleShowCreatures(qobject_cast<QAction*>(sender())->isChecked()); }
void MainMenu::onToggleShowSpawns() { parentWindow->toggleShowSpawns(qobject_cast<QAction*>(sender())->isChecked()); }
void MainMenu::onToggleShowSpecialTiles() { parentWindow->toggleShowSpecialTiles(qobject_cast<QAction*>(sender())->isChecked()); }
void MainMenu::onToggleShowHouses() { parentWindow->toggleShowHouses(qobject_cast<QAction*>(sender())->isChecked()); }
void MainMenu::onToggleShowPathing() { parentWindow->toggleShowPathing(qobject_cast<QAction*>(sender())->isChecked()); }
void MainMenu::onToggleShowTowns() { parentWindow->toggleShowTowns(qobject_cast<QAction*>(sender())->isChecked()); }
void MainMenu::onToggleShowWaypoints() { parentWindow->toggleShowWaypoints(qobject_cast<QAction*>(sender())->isChecked()); }
void MainMenu::onToggleHighlightItems() { parentWindow->toggleHighlightItems(qobject_cast<QAction*>(sender())->isChecked()); }
void MainMenu::onToggleHighlightLockedDoors() { parentWindow->toggleHighlightLockedDoors(qobject_cast<QAction*>(sender())->isChecked()); }
void MainMenu::onToggleShowWallHooks() { parentWindow->toggleShowWallHooks(qobject_cast<QAction*>(sender())->isChecked()); }


// --- Map Menu Slots ---
void MainMenu::onMapProperties() { parentWindow->showMapPropertiesDialog(); }
void MainMenu::onMapStatistics() { parentWindow->showMapStatisticsDialog(); }
void MainMenu::onGotoPosition() { parentWindow->showGotoPositionDialog(); }
// Map Transformation/Manipulation
void MainMenu::onBorderizeSelection() {
    if (parentWindow->getMap() && !parentWindow->getMap()->getSelection().isEmpty() && parentWindow->getBorderSystem()) {
        QMessageBox::information(parentWindow, tr("Borderize Selection"), tr("Borderizing selection is not yet fully implemented."));
        // This should trigger borderSystem->applyBordersToRegion on selection, followed by Map::mapChanged() and repaint.
    } else {
        QMessageBox::information(parentWindow, tr("Borderize Selection"), tr("No selection or BorderSystem not enabled."));
    }
}
void MainMenu::onBorderizeMap() {
    QMessageBox::information(parentWindow, tr("Borderize Map"), tr("Borderize Map is not yet implemented."));
}
void MainMenu::onRandomizeSelection() { QMessageBox::information(parentWindow, tr("Randomize Selection"), tr("Randomize Selection not yet implemented.")); }
void MainMenu::onRandomizeMap() { QMessageBox::information(parentWindow, tr("Randomize Map"), tr("Randomize Map not yet implemented.")); }
// More map actions
void MainMenu::onMapRemoveItems() { QMessageBox::information(parentWindow, tr("Remove Items"), tr("Remove Items (on map) not yet implemented.")); }
void MainMenu::onMapRemoveCorpses() { QMessageBox::information(parentWindow, tr("Remove Corpses"), tr("Remove Corpses (on map) not yet implemented.")); }
void MainMenu::onMapRemoveUnreachable() { parentWindow->mapRemoveUnreachable(); }
void MainMenu::onMapRemoveDuplicates() { 
    // parentWindow->mapRemoveDuplicates(); 
    QMessageBox::information(parentWindow, tr("Remove Duplicates"), tr("Remove Duplicates (on map) not yet implemented."));
}
void MainMenu::onMapValidateGround() { 
    // parentWindow->mapValidateGround();
    QMessageBox::information(parentWindow, tr("Validate Ground"), tr("Validate Ground (on map) not yet implemented."));
}
// House/Town Management
void MainMenu::onMapEditTowns() { 
    // parentWindow->mapEditTowns();
    QMessageBox::information(parentWindow, tr("Edit Towns"), tr("Edit Towns dialog not yet implemented.")); 
}
void MainMenu::onMapEditItems() { QMessageBox::information(parentWindow, tr("Edit Items"), tr("Edit Items (on map) not yet implemented.")); }
void MainMenu::onMapEditMonsters() { QMessageBox::information(parentWindow, tr("Edit Monsters"), tr("Edit Monsters (on map) not yet implemented.")); }
void MainMenu::onMapCleanHouseItems() { QMessageBox::information(parentWindow, tr("Clean House Items"), tr("Clean House Items (on map) not yet implemented.")); }


// --- Tools Menu Slots ---
void MainMenu::onBrushTool() { parentWindow->setCurrentTool(Brush::Type::Normal); }
void MainMenu::onEraserTool() { parentWindow->setCurrentTool(Brush::Type::Eraser); }
void MainMenu::onSelectionTool() { parentWindow->setCurrentTool(Brush::Type::Selection); }
void MainMenu::onFloodFillTool() { parentWindow->setCurrentTool(Brush::Type::FloodFill); }
void MainMenu::onGenerateIsland() { 
    // parentWindow->generateIsland();
    QMessageBox::information(parentWindow, tr("Generate Island"), tr("Island Generator dialog not yet implemented.")); 
}
void MainMenu::onCreateBorder() { 
    // parentWindow->createBorder();
    QMessageBox::information(parentWindow, tr("Border Editor"), tr("Border Editor dialog not yet implemented.")); 
}
void MainMenu::onTilesetEditor() { 
    // parentWindow->openTilesetEditor();
    QMessageBox::information(parentWindow, tr("Tileset Editor"), tr("Tileset Editor dialog not yet implemented.")); 
}
void MainMenu::onSelectionToDoodad() { 
    // parentWindow->selectionToDoodad();
    QMessageBox::information(parentWindow, tr("Selection to Doodad"), tr("Selection to Doodad brush conversion not yet implemented.")); 
}
void MainMenu::onToggleAutomagic() { parentWindow->toggleBorderSystem(automagicAction->isChecked()); } 
// void MainMenu::onGenerateMap() { } // This slot is for File menu.


// --- Network/Live Menu Slots ---
void MainMenu::onStartLive() { QMessageBox::information(parentWindow, tr("Live Editor"), tr("Live Editor (Host) not yet implemented.")); }
void MainMenu::onJoinLive() { QMessageBox::information(parentWindow, tr("Live Editor"), tr("Live Editor (Join) not yet implemented.")); }
void MainMenu::onLiveStatus() { QMessageBox::information(parentWindow, tr("Live Editor"), tr("Live Editor Status not yet implemented.")); }
void MainMenu::onCloseLive() { QMessageBox::information(parentWindow, tr("Live Editor"), tr("Live Editor Disconnect not yet implemented.")); }

// --- Help Menu Slots ---
void MainMenu::onHelp() { QMessageBox::information(parentWindow, tr("Help"), tr("Help system not yet implemented.")); }
void MainMenu::onShowHotkeys() { QMessageBox::information(parentWindow, tr("Hotkeys"), tr("Show Hotkeys dialog not yet implemented.")); }
void MainMenu::onReloadDataFiles() {
    // This is from `Source/src/main_menubar.cpp`, loads all items/sprites again.
    // It is important, should reload `ItemManager`, `CreatureManager`, `SpriteManager`.
    if (QMessageBox::question(parentWindow, tr("Reload Data Files"), tr("This will reload all game data files. Are you sure?"), QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        ItemManager::getInstance().clear();
        CreatureManager::getInstance().clear();
        SpriteManager::getInstance().clear(); // This reloads original
        // Then re-load initial data from scratch if it's setup.
        // Assuming `MainWindow` has methods to trigger this, e.g. from `populateItemList()`.
        parentWindow->populateItemList(); // Re-populate UI.
        parentWindow->populateCreatureList(); // Re-populate UI.
        QMessageBox::information(parentWindow, tr("Reload Data"), tr("Data files reloaded."));
        parentWindow->getMapView()->getScene()->update(); // Force map to redraw if item data changed
    }
}
void MainMenu::onListExtensions() { QMessageBox::information(parentWindow, tr("Extensions"), tr("Extensions dialog not yet implemented.")); }
void MainMenu::onGotoWebsite() {
    // Open URL to project website (https://github.com/Wirless/IdlersMapEditor) or original Remere's site.
    QDesktopServices::openUrl(QUrl("https://github.com/Wirless/IdlersMapEditor"));
}
void MainMenu::onAbout() {
    // This needs to open the About dialog (`AboutWindow` from Source).
    QMessageBox::information(parentWindow, tr("About"), tr("About Idlers Map Editor dialog not yet implemented."));
}