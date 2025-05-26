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
    void onImportMap();
    void onExportMap();
    void onExit();
    void onPreferences(); // Opens preferences dialog
    
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
    void onGenerateIsland(); // Island Generator dialog
    void onCreateBorder(); // Open Border Editor

    // Tools Menu Slots
    void onBrushTool();
    void onEraserTool();
    void onSelectionTool();
    void onFloodFillTool();
    void onJumpToBrush(); // Jump to a specific brush by name/ID
    void onJumpToItemBrush(); // Jump to a specific item brush (RAW)
    void onSelectionToDoodad(); // Convert selection to a doodad brush
    void onGenerateMap(); // Original Generate Map option

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

    enum { MaxRecentFiles = 10 }; // Max number of recent files to display
    QList<QAction*> recentFileActions; // List of QActions for recent files
    QAction* separatorAction; // Separator between recent files and "Exit"

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