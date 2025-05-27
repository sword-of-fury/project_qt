#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMenu>
#include <QToolBar>
#include <QStatusBar>
#include <QDockWidget>
#include <QListWidget> // For item/creature managers in dock widgets
#include <QUndoStack> // For Undo/Redo

// Project specific headers
#include "map.h"
#include "mapview.h" // Now a QGraphicsView
#include "mainmenu.h" // The main menu class (Qt-based)
#include "maintoolbar.h" // The main toolbar class (Qt-based)
#include "selectiontoolbar.h" // The selection toolbar (Qt-based)
#include "toolspanel.h" // Tools palette (dockable)
#include "layerwidget.h" // Layer controls (dockable)
#include "propertyeditordock.h" // Property editor (dockable)
#include "brush.h" // For Brush::Type enum

// Forward declarations of related classes
class Item;
class Creature;
class Tile;
class Map; // Already included but good practice for pointers
class MapView; // Already included
class MainMenu; // Already included
class MainToolBar; // Already included
class LayerWidget; // Already included
class ToolsPanel; // Already included
class PropertyEditorDock; // Already included
class Brush; // Already included
class BorderSystem; // For access to border automagic (part of Map class now)


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    virtual ~MainWindow();

    // Map properties dialog (placeholder methods for now, will open proper dialogs later)
    void showMapPropertiesDialog();
    void showMapStatisticsDialog();
    void showGotoPositionDialog();
    void showFindItemDialog();      // Currently doubles as 'Browse Tile' & 'Find Similar'
    void showFindCreatureDialog();
    void showFindSimilarItemsDialog(); // Could be merged into find item
    void showBorderSystemDialog();  // Dialog for border automagic settings
    
    // Global editor operations triggered from menu/toolbar/hotkeys
    void createNewMap();
    void openMap();
    bool loadMap(const QString& filePath); // Actual loading logic
    bool saveMap();
    bool saveMapAs();
    
    void undo();
    void redo();
    void cutSelection();
    void copySelection();
    void pasteSelection();
    void deleteSelection();
    void selectAll();
    void deselectAll();

    // View related actions
    void zoomIn();
    void zoomOut();
    void zoomReset();
    void toggleGrid(bool show);
    void toggleCollisions(bool show);
    void toggleStatusBar(bool show);
    void toggleToolbar(bool show);
    void toggleBorderSystem(bool enabled); // Automagic toggle
    void toggleFullscreen(); // From Source/main_menubar.h

    // Tool/Brush related actions
    void setCurrentTool(Brush::Type toolType);
    void increaseBrushSize();
    void decreaseBrushSize();
    void switchToolMode(); // Toggle between selection and drawing mode
    void selectPreviousBrush(); // Selects previous brush after undo/mode switch

    // Floor/Layer actions
    void changeFloor(int layer); // Changes the active editing layer
    int getCurrentLayer() const { return currentLayer; }

    // State Getters (for connecting other widgets/updating UI)
    QUndoStack* getUndoStack() const { return undoStack; } // For connecting Undo/Redo actions
    Map* getMap() const { return &Map::getInstance(); }
    MapView* getMapView() const { return mapView; }
    BorderSystem* getBorderSystem() const { return borderSystem; } // Access the Map's BorderSystem

    bool IsPasting() const { return isPasting; }
    void StartPasting() { isPasting = true; } // For paste preview mode
    void EndPasting() { isPasting = false; } // Exits paste preview mode

    void loadSprDatFiles(); // New method for loading SPR/DAT files

    // New public methods for actions from MainMenu
    void generateMap();
    void closeMap();
    void importMapFile();
    void importMonsters();
    void exportTilesets();
    void reloadData();
    void replaceItems();
    void refreshItems();
    // toggleBorderSystem(bool) already exists
    void borderizeSelection();
    void borderizeMap();
    void randomizeSelection();
    void randomizeMap();
    void mapRemoveUnreachable(); // Corresponds to onMapRemoveUnreachable
    void clearInvalidHouses();   // Corresponds to onClearHouseTiles
    void clearMapModifiedState(); // Corresponds to onClearModifiedState

    // New public methods for View menu actions
    void newView(); // Placeholder for MainMenu::onNewView
    void newDetachedView(); // Placeholder for MainMenu::onNewDetachedView
    void takeScreenshot();
    void toggleShowAllFloors(bool checked);
    void toggleShowAsMinimap(bool checked);
    void toggleShowOnlyColors(bool checked);
    void toggleShowOnlyModified(bool checked);
    void toggleAlwaysShowZones(bool checked);
    void toggleExtendedHouseShader(bool checked);
    void toggleShowTooltips(bool checked);
    void toggleShowClientBox(bool checked);
    void toggleGhostItems(bool checked);
    void toggleGhostHigherFloors(bool checked);
    void toggleShowShade(bool checked);
    void toggleShowAnimation(bool checked);
    void toggleShowLight(bool checked);
    void toggleShowLightStrength(bool checked);
    void toggleShowTechnicalItems(bool checked);
    void toggleShowZones(bool checked);
    void toggleShowCreatures(bool checked);
    void toggleShowSpawns(bool checked);
    void toggleShowSpecialTiles(bool checked);
    void toggleShowHouses(bool checked);
    void toggleShowPathing(bool checked);
    void toggleShowTowns(bool checked);
    void toggleShowWaypoints(bool checked);
    void toggleHighlightItems(bool checked);
    void toggleHighlightLockedDoors(bool checked);
    void toggleShowWallHooks(bool checked);

    // New public methods for Map menu actions
    void mapValidateGround();

    // New public methods for Tools menu actions
    void openTilesetEditor();
    void selectionToDoodad();
    // void generateIsland(); // Already exists via onGenerateIsland in MainMenu, connected to MainWindow::generateIsland
    // void createBorder(); // Already exists via onCreateBorder in MainMenu, connected to MainWindow::createBorder (placeholder)

    // New public methods for Search menu actions
    void findZones();
    void findUniqueItems();
    void findActionIdItems();
    void findContainers();
    void findWriteableItems();
    void findEverythingSpecial();

    // New public methods for Selection menu actions
    void replaceOnSelection();
    void findItemOnSelection();
    void removeItemOnSelection();
    void findEverythingOnSelection();
    void findZonesOnSelection();
    void findUniqueOnSelection();
    void findActionIdOnSelection();
    void findContainerOnSelection();
    void findWriteableOnSelection();
    void setSelectionMode(int mode);

    // New public methods for Navigate menu actions
    void gotoPreviousPosition();

    // New public methods for Window menu actions
    void showMinimap();
    void newPalette();
    void selectPalette(const QString& paletteName);
    void toggleToolbarVisibility(const QString& toolbarName);

    // New public methods for Experimental menu actions
    void toggleExperimentalFog(bool checked);


public slots: // Slots connected to various UI signals or map signals
    // General UI updates
    void updateWindowTitle();
    void updateStatusBar();
    void updateDockWindows(); // For docking state changes (if implemented)
    void loadSettings();      // Loads application settings
    void saveSettings();      // Saves application settings

    // Map/Tool/Selection-related
    void onToolSelected(int toolId);        // From ToolsPanel
    void onLayerChanged(int layer);         // From LayerWidget / LayerStack (Active Layer)
    void onItemSelected(Item* item);        // From ItemList in ToolsPanel (Brush palette selection)
    void onToolChanged(Brush::Type type);   // From Brush (brush is activated programmatically)
    void onCreatureSelected(QListWidgetItem* item); // From CreatureList (Brush palette selection)
    void onZoomChanged(double zoom);        // From MapView
    void onSelectionChanged();              // From Map (when selection rectangle changes)
    void onUndoStackChanged();              // From QUndoStack
    void onMapModified();                   // From Map
    void onMousePositionChanged(const QPoint& position); // From MapView (mouse hover)

    // Property Editor related (from MapView clicks or specific object selectors)
    void onTileSelected(Tile* tile);
    void onMapItemSelected(Item* item);
    void onMapCreatureSelected(Creature* creature);
    void onObjectDeselected(); // For property editor to show "no selection"

    // Implementation of copy/cut/paste/delete logic requested by MapView context menu
    void onMapViewCopyRequest(const QRect& selectionRect);
    void onMapViewCutRequest(const QRect& selectionRect);
    void onMapViewPasteRequest(const QPoint& targetPos);
    void onMapViewDeleteRequest(const QRect& selectionRect);


protected:
    void setupUi();            // Initializes all main UI components
    void createDockWindows();  // Creates all dockable panels
    void createStatusBar();    // Sets up the status bar layout and labels

private:
    MapView* mapView; // The main map display widget
    MainMenu* mainMenu; // Main menu bar logic
    MainToolBar* mainToolBar; // Main toolbar logic
    SelectionToolbar* selectionToolbar; // Toolbar specific to selection tool

    // Dock Widgets
    QDockWidget* toolsDock;
    QDockWidget* layersDock;
    QDockWidget* itemDock;
    QDockWidget* creatureDock;
    QDockWidget* propertyEditorDock;
    
    // Widgets within docks (managed by QDockWidget)
    ToolsPanel* toolsPanel; // Tool buttons, item/creature selection for brush
    LayerWidget* layerWidget; // Layer controls (active layer, visibility)
    QListWidget* itemListWidget; // For item selection (direct use in palette dock)
    QListWidget* creatureListWidget; // For creature selection
    PropertyEditorDock* propertyEditor; // Dock with stacked property editors

    QUndoStack* undoStack; // Central Undo/Redo stack
    Map* currentMap; // Reference to the singleton Map object

    // Status Bar Labels (for display in createStatusBar)
    QLabel* positionLabel;
    QLabel* layerLabel;
    QLabel* selectionLabel;
    QLabel* zoomLabel;
    QLabel* currentBrushLabel; // This was already added in a previous attempt, ensure it's correct.
    
    // Map data for display in lists
    QList<Item*> loadedItems; // Pointers to all loaded items for the item list
    QList<Creature*> loadedCreatures; // Pointers to all loaded creatures for creature list

    // Internal state variables
    QString currentMapFile; // Current map file path
    int currentLayer; // Tracks current active layer, matches LayerWidget value
    bool isPasting; // Flag for paste preview mode

    BorderSystem* borderSystem; // The automagic border system from Map

    // Helper methods for internal logic or setup
    void updateWindowTitle();    // Updates window title based on map file and modified state
    void populateItemList();     // Fills the item list widget
    void populateCreatureList(); // Fills the creature list widget
    void registerDefaultBorderTiles(); // For BorderSystem init, from original mainwindow
    void createMenus();          // Old stub, now implemented via MainMenu class
    void createToolBar();        // Old stub, now implemented via MainToolBar class
    void createDockPanels();     // Placeholder
};

#endif // MAINWINDOW_H