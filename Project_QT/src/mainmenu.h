#ifndef MAINMENU_H
#define MAINMENU_H

#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QStringList> // For recent files
#include <QList> // For recent files (alternatively std::vector)
#include <QMap> // For action map lookups (if ActionIDs are used)

#include "gui_ids.h" // For EditorActionID, ToolBarID if mapped to QAction::objectName/data
#include "mainwindow.h" // For casting parent and connecting slots
#include "map.h" // For LayerType usage for floor menus
#include "brush.h" // For Brush::Type in Tools menu

class MainWindow; // Forward declaration
// enum ActionID from source/main_menubar.h - Mapping this to enum property of QAction or string ID for robustness

/**
 * @brief The MainMenu class manages the application's menu bar (QMenuBar).
 * It creates menus, actions, and connects them to MainWindow's slots.
 * This class directly translates the structure and functionality from Source/src/main_menubar.h/cpp.
 */
class MainMenu : public QMenuBar
{
    Q_OBJECT
public:
    explicit MainMenu(MainWindow* parent = nullptr);
    virtual ~MainMenu();

    // Update methods to control action states (enabled/checked) dynamically
    void updateUndoRedoActions(bool canUndo, bool canRedo);
    void updateSelectionActions(bool hasSelection); // Handles Cut, Copy, Delete, Paste enablement
    void updateLayerActions(int currentLayer); // Manages floor radio buttons (if implemented)
    void updateViewActions(bool showGrid, bool showCollisions, bool showStatusBar, bool showToolBar, bool isFullscreen); // Updates View menu
    void updateToolActions(Brush::Type currentToolType); // Sets which brush tool is checked

    // Recent Files Management (from Source/main_menubar.cpp)
    void addRecentFile(const QString& filePath);
    void loadRecentFiles(); // Loads from QSettings typically
    void saveRecentFiles() const; // Saves to QSettings

signals:
    // Signals to MainWindow for specific actions if direct slot connection isn't preferred
    void newMapRequested();
    void openMapRequested();
    void saveMapRequested();
    void saveMapAsRequested();
    void quitApplicationRequested();

private slots:
    // File Menu Slots (from Source/src/main_menubar.cpp)
    void onNewMap();
    void onOpenMap();
    void onOpenRecent(); // Submenu handler for recent files
    void onSaveMap();
    void onSaveMapAs();
    // void onImportMap(); // Will be specific, e.g., onImportMapFile
    // void onExportMap(); // Will be specific, e.g., onExportMinimap
    void onExit();
    void onPreferences(); // Opens preferences dialog

    // New File Menu Slots
    void onGenerateMap();
    void onCloseMap();
    void onImportMapFile(); // For "Import Map..."
    void onImportMonsters(); // For "Import Monsters/NPC..."
    void onExportTilesets();
    void onReloadData();
    
    // Edit Menu Slots
    void onUndo();
    void onRedo();
    void onCut();
    void onCopy();
    void onPaste();
    void onDelete();
    void onSelectAll();
    void onDeselectAll();

    // View Menu Slots
    void onZoomIn();
    void onZoomOut();
    void onZoomReset();
    void onToggleGrid();
    void onToggleCollisions();
    void onToggleStatusBar();
    void onToggleToolbar();
    void onToggleFullscreen(); // From Source/main_menubar.h
    // New View Menu Slots (consolidating Editor, View, Show from XML)
    void onNewView();
    void onNewDetachedView();
    void onTakeScreenshot();
    void onToggleShowAllFloors();
    void onToggleShowAsMinimap();
    void onToggleShowOnlyColors();
    void onToggleShowOnlyModified();
    void onToggleAlwaysShowZones();
    void onToggleExtendedHouseShader();
    void onToggleShowTooltips();
    void onToggleShowClientBox();
    void onToggleGhostItems();
    void onToggleGhostHigherFloors();
    void onToggleShowShade();
    void onToggleShowAnimation();
    void onToggleShowLight();
    void onToggleShowLightStrength();
    void onToggleShowTechnicalItems();
    void onToggleShowZones();
    void onToggleShowCreatures();
    void onToggleShowSpawns();
    void onToggleShowSpecialTiles();
    void onToggleShowHouses();
    void onToggleShowPathing();
    void onToggleShowTowns();
    void onToggleShowWaypoints();
    void onToggleHighlightItems();
    void onToggleHighlightLockedDoors();
    void onToggleShowWallHooks();


    // Map Menu Slots
    void onMapProperties();
    void onMapStatistics();
    void onGotoPosition();
    void onFindItem(); // Used for multiple find operations
    void onFindCreature();
    void onFindSimilarItems();
    void onMapRemoveItems();
    void onMapRemoveCorpses(); // Monsters not part of spawn system
    void onMapRemoveUnreachable();
    void onClearHouseTiles();
    void onClearModifiedState(); // Mark map as unmodified
    void onMapEditTowns();
    void onMapEditItems(); // Legacy Item properties, usually points to generic props
    void onMapEditMonsters(); // Legacy Creature properties
    void onMapCleanHouseItems(); // Removes items not assigned to any house
    void onMapCleanup(); // Generic map cleanup dialog (from main_menubar)
    void onMapRemoveDuplicates(); // Removes duplicate items on map
    void onMapValidateGround(); // Validates ground layers
    // void onGenerateIsland(); // Island Generator dialog - This seems to be MAP_GENERATE_ISLAND, not GENERATE_MAP
    void onCreateBorder(); // Open Border Editor

    // New Edit Menu Slots
    void onReplaceItems();
    void onRefreshItems();
    void onToggleBorderAutomagic(); // For "Border Automagic"
    void onBorderizeSelection();
    void onBorderizeMap();
    void onRandomizeSelection();
    void onRandomizeMap();
    // onMapRemoveUnreachable, onClearHouseTiles, onClearModifiedState are already present


    // Tools Menu Slots
    void onBrushTool();
    void onEraserTool();
    void onSelectionTool();
    void onFloodFillTool();
    void onJumpToBrush(); // Jump to a specific brush by name/ID
    void onJumpToItemBrush(); // Jump to a specific item brush (RAW)
    void onSelectionToDoodad(); // Convert selection to a doodad brush
    // void onGenerateMap(); // This slot is for File->Generate Map. Tools->Island Generator is onGenerateIsland.
    void onTilesetEditor(); // Slot for Tools->Tileset Editor

    // Network/Live Menu Slots
    void onStartLive(); // Host server
    void onJoinLive();  // Join client
    void onLiveStatus(); // Check live connection status (perhaps in separate menu)
    void onCloseLive(); // Disconnect live connection
    void onShowHotkeys(); // Opens Hotkey management dialog
    void onDebugViewDat(); // Opens Dat Debug View
    void onListExtensions(); // Opens Extensions dialog
    void onReloadDataFiles(); // Reload game data (items, sprites, creatures)
    void onGotoWebsite(); // Open RME website (or project website)
    void onAbout(); // About window
    void onLoadSprDat(); // New slot for loading SPR/DAT files

private:
    MainWindow* parentWindow; // Direct reference to the main window
    QMenu* fileMenu;
    QMenu* editMenu;
    QMenu* viewMenu;
    QMenu* mapMenu;
    QMenu* toolsMenu;
    QMenu* windowMenu; // Original also had "Window" menu for docks/minimap
    QMenu* helpMenu;
    QMenu* recentFilesMenu; // Submenu for recent files
    QMenu* importSubMenu;   // For File->Import
    QMenu* exportSubMenu;   // For File->Export
    QMenu* reloadSubMenu;   // For File->Reload
    QMenu* borderOptionsSubMenu; // For Edit->Border Options
    QMenu* otherOptionsSubMenu;  // For Edit->Other Options


    enum { MaxRecentFiles = 10 }; // Max number of recent files to display
    QList<QAction*> recentFileActions; // List of QActions for recent files
    QAction* separatorAction; // Separator between recent files and "Exit"

    // QAction members for new items
    // File Menu
    QAction* generateMapAction;
    QAction* closeMapAction;
    QAction* importMapAction;      // "Import Map..."
    QAction* importMonstersAction; // "Import Monsters/NPC..."
    QAction* exportTilesetsAction;
    QAction* reloadDataAction;

    // Edit Menu
    QAction* replaceItemsAction;
    QAction* refreshItemsAction;
    QAction* automagicAction;          // "Border Automagic" (checkable)
    QAction* borderizeSelectionAction;
    QAction* borderizeMapAction;
    QAction* randomizeSelectionAction;
    QAction* randomizeMapAction;
    // Actions for "Other Options" submenu (already have slots, ensure QAction members if needed for state updates)
    QAction* removeUnreachableTilesAction; // For MAP_REMOVE_UNREACHABLE_TILES
    QAction* clearInvalidHousesAction;     // Slot onClearHouseTiles exists
    QAction* clearModifiedStateAction;   // Slot onClearModifiedState exists

    // View Menu Actions (consolidated)
    QAction* newViewAction;
    QAction* newDetachedViewAction;
    QAction* takeScreenshotAction;
    QAction* showAllFloorsAction;
    QAction* showAsMinimapAction;
    QAction* showOnlyColorsAction;
    QAction* showOnlyModifiedAction;
    QAction* alwaysShowZonesAction;
    QAction* extendedHouseShaderAction;
    QAction* showTooltipsAction;
    QAction* showClientBoxAction;
    QAction* ghostItemsAction;
    QAction* ghostHigherFloorsAction;
    QAction* showShadeAction;
    QAction* showAnimationAction;
    QAction* showLightAction;
    QAction* showLightStrengthAction;
    QAction* showTechnicalItemsAction;
    QAction* showZonesAction;
    QAction* showCreaturesAction;
    QAction* showSpawnsAction;
    QAction* showSpecialTilesAction;
    QAction* showHousesAction;
    QAction* showPathingAction;
    QAction* showTownsAction;
    QAction* showWaypointsAction;
    QAction* highlightItemsAction;
    QAction* highlightLockedDoorsAction;
    QAction* showWallHooksAction;

    // Map Menu Actions (if needing explicit members for dynamic updates beyond slots)
    // QAction* validateGroundAction; // Example if needed

    // Tools Menu Actions
    QAction* tilesetEditorAction;
    // QAction* selectionToDoodadAction; // Slot exists, QAction member might not be needed unless state changes


    // Map to quickly look up actions by ID if the original system uses specific integer IDs
    // QMap<int, QAction*> actionMap; // Not strictly needed if all actions connected manually

    // Private helper methods for menu creation
    void createFileMenu();
    void createEditMenu();
    void createViewMenu();
    void createMapMenu();
    void createToolsMenu();
    void createNetworkMenu(); // New, consolidates live editor stuff
    void createHelpMenu();
    // Helper to add recent file entries dynamically
    void updateRecentFilesMenu();
    
    // Internal helper for action creation for consistency
    QAction* createAndAddAction(QMenu* menu, const QString& text, QObject* receiver, const char* slot, QKeySequence shortcut = QKeySequence::Invalid, bool checkable = false);

    // Private member to ensure flags like CHECK_AUTOMAGIC are accessible if linked here.
};

#endif // MAINMENU_H