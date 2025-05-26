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
    createAndAddAction(fileMenu, tr("&New Map"), this, SLOT(onNewMap()), QKeySequence::New);
    createAndAddAction(fileMenu, tr("&Open Map..."), this, SLOT(onOpenMap()), QKeySequence::Open);

    // Recent Files Submenu
    recentFilesMenu = new QMenu(tr("Open Recent"), this);
    fileMenu->addMenu(recentFilesMenu);
    
    // Actions for recent files
    for (int i = 0; i < MaxRecentFiles; ++i) {
        recentFileActions.append(new QAction(this)); // Actions created without text yet
        recentFileActions.last()->setVisible(false); // Hide initially
        connect(recentFileActions.last(), &QAction::triggered, this, &MainMenu::onOpenRecent);
        recentFilesMenu->addAction(recentFileActions.last());
    }
    separatorAction = fileMenu->addSeparator(); // Separator for recent files list.
    updateRecentFilesMenu(); // Initial update of recent files.

    createAndAddAction(fileMenu, tr("&Save Map"), this, SLOT(onSaveMap()), QKeySequence::Save);
    createAndAddAction(fileMenu, tr("Save Map &As..."), this, SLOT(onSaveMapAs()), QKeySequence::SaveAs);
    
    fileMenu->addSeparator();
    
    // Import/Export Submenus (from Source/src/main_menubar.cpp)
    QMenu* importMenu = fileMenu->addMenu(tr("&Import"));
    createAndAddAction(importMenu, tr("Import Map..."), this, SLOT(onImportMap()));
    createAndAddAction(importMenu, tr("Import Monster Data (XML)..."), this, SLOT(onImportMonsterData()));
    createAndAddAction(importMenu, tr("Import Minimap..."), this, SLOT(onImportMinimap()));

    QMenu* exportMenu = fileMenu->addMenu(tr("&Export"));
    createAndaddaction(exportMenu, tr("Export Minimap..."), this, SLOT(onExportMinimap()));
    createAndaddaction(exportMenu, tr("Export Tilesets..."), this, SLOT(onExportTilesets()));
    
    fileMenu->addSeparator();

    // Preferences & Exit
    createAndAddAction(fileMenu, tr("&Preferences..."), this, SLOT(onPreferences()), QKeySequence::Preferences);
    createAndAddAction(fileMenu, tr("E&xit"), this, SLOT(onExit()), QKeySequence::Quit);

    // Load SPR/DAT
    createAndAddAction(fileMenu, tr("Load SPR/DAT..."), this, SLOT(onLoadSprDat()));
}

void MainMenu::createEditMenu()
{
    // Undo/Redo (enabled/disabled dynamically)
    createAndAddAction(editMenu, tr("&Undo"), this, SLOT(onUndo()), QKeySequence::Undo);
    createAndAddAction(editMenu, tr("&Redo"), this, SLOT(onRedo()), QKeySequence::Redo);
    editMenu->addSeparator();

    // Cut/Copy/Paste/Delete (enabled/disabled dynamically by selection state)
    createAndAddAction(editMenu, tr("Cu&t"), this, SLOT(onCut()), QKeySequence::Cut);
    createAndAddAction(editMenu, tr("&Copy"), this, SLOT(onCopy()), QKeySequence::Copy);
    createAndAddAction(editMenu, tr("&Paste"), this, SLOT(onPaste()), QKeySequence::Paste);
    createAndAddAction(editMenu, tr("&Delete"), this, SLOT(onDelete()), QKeySequence::Delete);
    editMenu->addSeparator();

    // Selection commands
    createAndAddAction(editMenu, tr("Select &All"), this, SLOT(onSelectAll()), QKeySequence::SelectAll);
    createAndAddAction(editMenu, tr("&Deselect All"), this, SLOT(onDeselectAll())); // No default shortcut in Qt, use specific ID later if needed.
    
    editMenu->addSeparator();

    // Search/Replace/Cleanup (from Source/src/main_menubar.cpp)
    createAndAddAction(editMenu, tr("Search for &Item..."), this, SLOT(onFindItem()));
    createAndAddAction(editMenu, tr("Search for &Creature..."), this, SLOT(onFindCreature()));
    createAndAddAction(editMenu, tr("Find &Similar Items..."), this, SLOT(onFindSimilarItems()));
    createAndAddAction(editMenu, tr("&Replace Items..."), this, SLOT(onReplaceItems()));
    createAndAddAction(editMenu, tr("Map &Cleanup..."), this, SLOT(onMapCleanup()));
    editMenu->addSeparator();

    // Various edit actions (from Source/src/main_menubar.cpp)
    createAndAddAction(editMenu, tr("Clear Invalid House &Tiles..."), this, SLOT(onClearHouseTiles()));
    createAndAddAction(editMenu, tr("Clear &Modified State"), this, SLOT(onClearModifiedState()));
    createAndAddAction(editMenu, tr("Jump to &Brush..."), this, SLOT(onJumpToBrush()));
    createAndAddAction(editMenu, tr("Jump to &Item Brush..."), this, SLOT(onJumpToItemBrush()));
}

void MainMenu::createViewMenu()
{
    // Zoom controls
    createAndAddAction(viewMenu, tr("Zoom &In"), this, SLOT(onZoomIn()), QKeySequence::ZoomIn);
    createAndAddAction(viewMenu, tr("Zoom &Out"), this, SLOT(onZoomOut()), QKeySequence::ZoomOut);
    createAndAddAction(viewMenu, tr("Zoom &Normal"), this, SLOT(onZoomReset()));
    viewMenu->addSeparator();

    // Visibility toggles
    createAndAddAction(viewMenu, tr("Show &Grid"), this, SLOT(onToggleGrid()), Qt::Key_G, true)->setChecked(true); // Default checked
    createAndAddAction(viewMenu, tr("Show &Collisions"), this, SLOT(onToggleCollisions()), Qt::Key_C, true)->setChecked(true); // Default checked
    viewMenu->addSeparator();

    // UI visibility toggles
    createAndAddAction(viewMenu, tr("Show Status &Bar"), this, SLOT(onToggleStatusBar()), Qt::NoModifier, true)->setChecked(true);
    createAndAddAction(viewMenu, tr("Show Tool&Bar"), this, SLOT(onToggleToolbar()), Qt::NoModifier, true)->setChecked(true); // Default true
    viewMenu->addSeparator();

    createAndAddAction(viewMenu, tr("Toggle &Fullscreen"), this, SLOT(onToggleFullscreen()), QKeySequence::FullScreen);

    // Floor (Layer) menu dynamically populated
    QMenu* floorMenu = viewMenu->addMenu(tr("F&loor"));
    // Original had radio buttons for floors. Add a dummy slot for connecting individual floors.
    for (int i = 0; i < Map::LayerCount; ++i) { // 0-15 layers
        QAction* floorAction = floorMenu->addAction(tr("Floor %1").arg(i), this, [this, i](){
            parentWindow->changeFloor(i);
            // After change, set radio checked here
            updateLayerActions(i);
        });
        floorAction->setCheckable(true);
        // Map keyboard shortcuts (F1-F12 mapped to 0-11, etc.)
        // This is simplified. Proper hotkey manager for specific floor key binding should be used.
        if (i < 12) floorAction->setShortcut(QKeySequence(QString("F%1").arg(i + 1))); // F1-F12 for floors 0-11.
    }
    updateLayerActions(parentWindow->getCurrentLayer()); // Initial check for current layer.

    // View Settings (e.g. LOD, Fog - from Source/src/main_menubar.cpp)
    viewMenu->addSeparator();
    createAndAddAction(viewMenu, tr("View &Settings..."), this, SLOT(onChangeViewSettings())); // Placeholder for preferences dialog
}

void MainMenu::createMapMenu()
{
    createAndAddAction(mapMenu, tr("Map &Properties..."), this, SLOT(onMapProperties()));
    createAndAddAction(mapMenu, tr("Map &Statistics..."), this, SLOT(onMapStatistics()));
    createAndAddAction(mapMenu, tr("Go To &Position..."), this, SLOT(onGotoPosition()));
    createAndAddAction(mapMenu, tr("Clear &Modified State"), this, SLOT(onClearModifiedState())); // Replaces the one in Edit menu.
    mapMenu->addSeparator();

    // Map Transformations
    createAndAddAction(mapMenu, tr("&Borderize Selection (Ctrl+B)"), this, SLOT(onBorderizeSelection()));
    createAndAddAction(mapMenu, tr("Borderize Map..."), this, SLOT(onBorderizeMap()));
    createAndAddAction(mapMenu, tr("Randomize Selection"), this, SLOT(onRandomizeSelection()));
    createAndAddAction(mapMenu, tr("Randomize Map..."), this, SLOT(onRandomizeMap()));
    mapMenu->addSeparator();

    // Other map operations
    createAndAddAction(mapMenu, tr("Remove Items (on map)..."), this, SLOT(onMapRemoveItems()));
    createAndAddAction(mapMenu, tr("Remove Corpses (on map)..."), this, SLOT(onMapRemoveCorpses()));
    createAndAddAction(mapMenu, tr("Remove &Unreachable Areas (on map)..."), this, SLOT(onMapRemoveUnreachable()));
    createAndAddAction(mapMenu, tr("Remove &Duplicates (on map)..."), this, SLOT(onMapRemoveDuplicates()));
    createAndAddAction(mapMenu, tr("&Validate Ground (on map)..."), this, SLOT(onMapValidateGround()));
    mapMenu->addSeparator();

    // House/Town Management
    createAndAddAction(mapMenu, tr("Edit &Towns..."), this, SLOT(onMapEditTowns()));
    createAndAddAction(mapMenu, tr("Clean &House Items (on map)..."), this, SLOT(onMapCleanHouseItems()));
    createAndAddAction(mapMenu, tr("Edit &Items (on map)..."), this, SLOT(onMapEditItems())); // Legacy or detailed item editor
    createAndAddAction(mapMenu, tr("Edit &Monsters (on map)..."), this, SLOT(onMapEditMonsters())); // Legacy or detailed creature editor
}

void MainMenu::createToolsMenu()
{
    // Brush Tool Selection (Radio behavior set on toolbar group, not menu)
    createAndAddAction(toolsMenu, tr("&Normal Brush"), this, SLOT(onBrushTool()), QKeySequence(Qt::Key_N), true); // N for Normal
    createAndAddAction(toolsMenu, tr("&Eraser"), this, SLOT(onEraserTool()), QKeySequence(Qt::Key_E), true);     // E for Eraser
    createAndAddAction(toolsMenu, tr("Sele&ction"), this, SLOT(onSelectionTool()), QKeySequence(Qt::Key_S), true); // S for Selection
    createAndAddAction(toolsMenu, tr("F&lood Fill"), this, SLOT(onFloodFillTool()), QKeySequence(Qt::Key_F), true); // F for FloodFill
    toolsMenu->addSeparator();

    // Specialized Creation Tools
    createAndAddAction(toolsMenu, tr("Island &Generator..."), this, SLOT(onGenerateIsland()));
    createAndAddAction(toolsMenu, tr("&Border Editor..."), this, SLOT(onCreateBorder()));
    createAndAddAction(toolsMenu, tr("&Tileset Editor..."), this, SLOT(onTilesetEditor()));
    createAndAddAction(toolsMenu, tr("Selection to Doodad"), this, SLOT(onSelectionToDoodad())); // New from Source
    toolsMenu->addSeparator();

    createAndAddAction(toolsMenu, tr("Toggle &Automagic Borders (A)"), this, SLOT(onToggleAutomagic()), Qt::Key_A, true); // Automagic toggle.

    // Connect radio-like behavior for tools
    QActionGroup* toolGroup = new QActionGroup(this);
    toolGroup->setExclusive(true);
    // Add previously created tool actions to this group
    toolGroup->addAction(toolsMenu->actions().at(0)); // Normal Brush
    toolGroup->addAction(toolsMenu->actions().at(1)); // Eraser
    toolGroup->addAction(toolsMenu->actions().at(2)); // Selection
    toolGroup->addAction(toolsMenu->actions().at(3)); // Flood Fill
    
    // Set initial tool check (based on initial MainWindow/MapView state)
    // Parent window passes initial brush, check its type in the QActionGroup for display
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
void MainMenu::onReplaceItems() { QMessageBox::information(parentWindow, tr("Replace Items"), tr("Replace Items dialog not yet implemented.")); }
void MainMenu::onMapCleanup() { QMessageBox::information(parentWindow, tr("Map Cleanup"), tr("Map Cleanup dialog not yet implemented.")); }
void MainMenu::onClearHouseTiles() { QMessageBox::information(parentWindow, tr("Clear House Tiles"), tr("Clear Invalid House Tiles not yet implemented.")); }
void MainMenu::onClearModifiedState() { parentWindow->getMap()->setModified(false); parentWindow->updateWindowTitle(); } // Clear modified flag in Map.
void MainMenu::onJumpToBrush() { QMessageBox::information(parentWindow, tr("Jump to Brush"), tr("Jump to Brush dialog not yet implemented.")); }
void MainMenu::onJumpToItemBrush() { QMessageBox::information(parentWindow, tr("Jump to Item Brush"), tr("Jump to Item Brush dialog not yet implemented.")); }


// --- View Menu Slots ---
void MainMenu::onZoomIn() { parentWindow->zoomIn(); }
void MainMenu::onZoomOut() { parentWindow->zoomOut(); }
void MainMenu::onZoomReset() { parentWindow->zoomReset(); }
void MainMenu::onToggleGrid() { parentWindow->toggleGrid(qobject_cast<QAction*>(sender())->isChecked()); }
void MainMenu::onToggleCollisions() { parentWindow->toggleCollisions(qobject_cast<QAction*>(sender())->isChecked()); }
void MainMenu::onToggleStatusBar() { parentWindow->toggleStatusBar(qobject_cast<QAction*>(sender())->isChecked()); }
void MainMenu::onToggleToolbar() { parentWindow->toggleToolbar(qobject_cast<QAction*>(sender())->isChecked()); }
void MainMenu::onToggleFullscreen() { parentWindow->toggleFullscreen(); }
void MainMenu::onChangeViewSettings() { QMessageBox::information(parentWindow, tr("View Settings"), tr("View Settings dialog not yet implemented (LOD, fog etc).")); }


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
void MainMenu::onMapRemoveUnreachable() { QMessageBox::information(parentWindow, tr("Remove Unreachable Areas"), tr("Remove Unreachable Areas (on map) not yet implemented.")); }
void MainMenu::onMapRemoveDuplicates() { QMessageBox::information(parentWindow, tr("Remove Duplicates"), tr("Remove Duplicates (on map) not yet implemented.")); }
void MainMenu::onMapValidateGround() { QMessageBox::information(parentWindow, tr("Validate Ground"), tr("Validate Ground (on map) not yet implemented.")); }
// House/Town Management
void MainMenu::onMapEditTowns() { QMessageBox::information(parentWindow, tr("Edit Towns"), tr("Edit Towns dialog not yet implemented.")); }
void MainMenu::onMapEditItems() { QMessageBox::information(parentWindow, tr("Edit Items"), tr("Edit Items (on map) not yet implemented.")); }
void MainMenu::onMapEditMonsters() { QMessageBox::information(parentWindow, tr("Edit Monsters"), tr("Edit Monsters (on map) not yet implemented.")); }
void MainMenu::onMapCleanHouseItems() { QMessageBox::information(parentWindow, tr("Clean House Items"), tr("Clean House Items (on map) not yet implemented.")); }


// --- Tools Menu Slots ---
void MainMenu::onBrushTool() { parentWindow->setCurrentTool(Brush::Type::Normal); }
void MainMenu::onEraserTool() { parentWindow->setCurrentTool(Brush::Type::Eraser); }
void MainMenu::onSelectionTool() { parentWindow->setCurrentTool(Brush::Type::Selection); }
void MainMenu::onFloodFillTool() { parentWindow->setCurrentTool(Brush::Type::FloodFill); }
void MainMenu::onGenerateIsland() { QMessageBox::information(parentWindow, tr("Generate Island"), tr("Island Generator dialog not yet implemented.")); }
void MainMenu::onCreateBorder() { QMessageBox::information(parentWindow, tr("Border Editor"), tr("Border Editor dialog not yet implemented.")); }
void MainMenu::onTilesetEditor() { QMessageBox::information(parentWindow, tr("Tileset Editor"), tr("Tileset Editor dialog not yet implemented.")); }
void MainMenu::onSelectionToDoodad() { QMessageBox::information(parentWindow, tr("Selection to Doodad"), tr("Selection to Doodad brush conversion not yet implemented.")); }
void MainMenu::onToggleAutomagic() { parentWindow->toggleBorderSystem(qobject_cast<QAction*>(sender())->isChecked()); }
void MainMenu::onGenerateMap() {
    QMessageBox::information(parentWindow, tr("Generate Map"), tr("New Map generation from template not yet implemented."));
}


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