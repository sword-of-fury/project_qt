#include "mainwindow.h"
#include "item.h" // Needed for Item class in populating lists
#include "creature.h" // Needed for Creature class
#include "itemmanager.h" // Needed for ItemManager singleton
#include "creaturemanager.h" // Needed for CreatureManager singleton
#include "otbmfile.h" // Needed for OTBMFile for map loading
#include "otb.h" // For OTB types
#include "tile.h" // For Tile properties
#include "layer.h" // For Layer::Type
#include "bordersystem.h" // For automagic (already included from Map)
#include "clientversion.h" // For ClientVersion to load item/sprite data
#include "settingsmanager.h"

#include <QApplication> // For qApp->quit()
#include <QFileDialog>  // For file dialogs
#include <QMessageBox>  // For message boxes
#include <QJsonDocument> // For clipboard operations or other JSON usage
#include <QDebug>       // For debugging output
#include <QStatusBar>   // Ensure QLabel and QStatusBar functions correctly
#include <QSettings> // For saving/loading window state, last paths etc. (later integration)

// Dummy implementations for MapCommand (for compilation, will be fully integrated by QUndoStack later)
// This simplifies immediate compilation but relies on the actual Undo Commands later.
class MapCommand : public QUndoCommand {
protected:
    Map* map;
public:
    MapCommand(Map* mapPtr, QUndoCommand* parent = nullptr) : QUndoCommand(parent), map(mapPtr) {}
    // pure virtual functions left as is.
};

// Assuming AddItemCommand and other specific commands already exist/compile in `Project_QT`.
// They rely on Map being set correctly and directly operating on map properties.

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    mapView(nullptr),
    mainMenu(nullptr),
    mainToolBar(nullptr),
    selectionToolbar(nullptr),
    toolsDock(nullptr),
    layersDock(nullptr),
    itemDock(nullptr),
    creatureDock(nullptr),
    propertyEditorDock(nullptr),
    toolsPanel(nullptr),
    layerWidget(nullptr),
    itemListWidget(nullptr),
    creatureListWidget(nullptr),
    propertyEditor(nullptr),
    undoStack(new QUndoStack(this)), // Undo stack owned by MainWindow
    currentMap(&Map::getInstance()), // Get singleton instance
    currentLayer(static_cast<int>(Layer::Type::Ground)), // Initial layer set
    isPasting(false)
{
    // Initialize common managers once (these are singletons but need to be instantiated)
    ItemManager::getInstance();
    CreatureManager::getInstance();
    SpriteManager::getInstance();
    BrushManager::getInstance(); // Access brushes (like NormalBrush, etc)
    // ClientVersion::getInstance(); // Will be instantiated during initial load or in app init.
    // Setting up the automagic border system is handled in Map constructor for consistency.
    borderSystem = currentMap->getBorderSystem();

    // Setup UI components and main window layout.
    setupUi(); // SpriteManager test will be temporarily inside setupUi

    createDockWindows(); // Set up dockable panels.
    createStatusBar(); // Configure status bar.
    
    // Connect Map's signals for UI updates.
    connect(currentMap, &Map::mapChanged, this, &MainWindow::updateWindowTitle);
    connect(currentMap, &Map::mapChanged, this, &MainWindow::updateStatusBar);
    connect(currentMap, &Map::selectionChanged, this, &MainWindow::onSelectionChanged); // Updates selection info in status bar.
    connect(undoStack, &QUndoStack::canUndoChanged, this, &MainWindow::onUndoStackChanged);
    connect(undoStack, &QUndoStack::canRedoChanged, this, &MainWindow::onUndoStackChanged);

    // Initial map setup (if no map is loaded from command line)
    // if (!currentMap->hasFile()) { // Assumed `Map::hasFile()` method to check if a map is already open
        createNewMap(); // Create a default empty map on startup.
    // }

    // Initial population of item/creature lists for palettes.
    populateItemList();
    populateCreatureList();
    
    // Set map in the LayerWidget (after layers are initialized in Map)
    if (layerWidget && currentMap) {
        layerWidget->setMap(currentMap);
        layerWidget->setCurrentLayer(currentLayer); // Ensure LayerWidget syncs its initial display
        layerWidget->loadLayerStates(); // Force initial sync with Map's internal states.
    }
    
    // Connect signals from specific panels (tools, layers) to main window slots.
    if (toolsPanel) {
        connect(toolsPanel, &ToolsPanel::toolChanged, this, &MainWindow::onToolSelected);
        connect(toolsPanel, &ToolsPanel::itemSelected, this, &MainWindow::onItemSelected); // When an item is selected from palette
        connect(toolsPanel, &ToolsPanel::creatureSelected, this, &MainWindow::onCreatureSelected); // When creature is selected
    }
    if (layerWidget) {
        connect(layerWidget, &LayerWidget::layerChanged, this, &MainWindow::onLayerChanged); // Active layer selection
        // `LayerWidget` also signals layerVisibilityChanged, MapView connected to this via Map events
        // connect(layerWidget, &LayerWidget::layerVisibilityChanged, mapView->getScene(), &MapScene::updateVisibleTiles);
    }
    if (propertyEditor) {
        // Connect property editor's signals for updates (e.g. tile properties changed).
        // For instance, when TilePropertyEditor in propertyEditor signals propertiesChanged.
        // connect(propertyEditor, &PropertyEditorDock::propertyChanged, this, &MainWindow::onPropertyEditorChanged);
    }

    // Connect MapView's custom signals
    connect(mapView, &MapView::mousePositionChanged, this, &MainWindow::onMousePositionChanged);
    connect(mapView, &MapView::tileSelected, this, &MainWindow::onTileSelected); // Opens tile properties
    connect(mapView, &MapView::itemSelected, this, &MainWindow::onMapItemSelected); // Opens item properties
    connect(mapView, &MapView::creatureSelected, this, &MainWindow::onMapCreatureSelected); // Opens creature properties
    connect(mapView, &MapView::objectDeselected, this, &MainWindow::onObjectDeselected); // Deselect in property panel
    connect(mapView, &MapView::copyRequest, this, &MainWindow::onMapViewCopyRequest);
    connect(mapView, &MapView::cutRequest, this, &MainWindow::onMapViewCutRequest);
    connect(mapView, &MapView::pasteRequest, this, &MainWindow::onMapViewPasteRequest);
    connect(mapView, &MapView::deleteRequest, this, &MainWindow::onMapViewDeleteRequest);


    // Final UI updates
    updateWindowTitle();
    updateStatusBar();

    // Set initial tool
    setCurrentTool(Brush::Type::Normal); // Default to Normal brush.

    // Register default border tiles in the automagic system for the current map version
    registerDefaultBorderTiles();
}

MainWindow::~MainWindow()
{
    // The main window handles deleting its owned child widgets via Qt's parent-child system.
    // `undoStack` is owned by `this`.
    // `currentMap` is a singleton, not deleted here.
}

void MainWindow::setupUi()
{
    // Create main UI elements
    mapView = new MapView(this);
    setCentralWidget(mapView); // Set MapView as the central widget
    
    // Create Menu Bar and Toolbars
    mainMenu = new MainMenu(this); // Passes this MainWindow as parent to MainMenu
    setMenuBar(mainMenu); // Set MainMenu as the QMainWindow's menuBar.
    
    mainToolBar = new MainToolBar(this); // Passes this MainWindow as parent
    addToolBar(Qt::TopToolBarArea, mainToolBar); // Add the main toolbar.

    selectionToolbar = new SelectionToolbar(tr("Selection Tools"), this); // Placeholder name
    addToolBar(Qt::BottomToolBarArea, selectionToolbar); // Add selection toolbar.
    selectionToolbar->setVisible(false); // Hidden by default, shown when selection tool active.
    
    // Initialize common Managers and load data
    qDebug() << "--- Initializing Managers and Loading Data ---";

    // Connect error signals for debugging
    connect(&ItemManager::getInstance(), &ItemManager::error, [](const QString& message){
        qWarning() << "ItemManager Error:" << message;
    });
    connect(&SpriteManager::getInstance(), &SpriteManager::error, [](const QString& message){
        qWarning() << "SpriteManager Error:" << message;
    });
    // Assuming CreatureManager might also have an error signal if file parsing fails
    // connect(&CreatureManager::getInstance(), &CreatureManager::error, ...);


    // Load Tibia.dat for item properties and Tibia.spr for sprites
    QString datPath = "data/Tibia.dat";
    QString sprPath = "data/Tibia.spr";
    QString creaturesXmlPath = "data/creatures.xml";
    QString testMapPath = "data/testmap.otbm"; // Placeholder for a simple test map

    qDebug() << "Loading item properties from:" << datPath;
    if (!ItemManager::getInstance().loadTibiaDat(datPath)) {
        qWarning() << "Failed to load Tibia.dat from" << datPath;
    } else {
        qDebug() << "Tibia.dat loaded successfully.";
    }

    qDebug() << "Loading sprites from:" << sprPath << "and using DAT:" << datPath;
    if (!SpriteManager::getInstance()->loadSprites(sprPath, datPath)) {
        qWarning() << "Failed to load Tibia.spr from" << sprPath;
    } else {
        qDebug() << "Tibia.spr loaded successfully. Item Sprites Max ID:" << SpriteManager::getInstance()->getItemSpriteMaxID();
    }
    
    qDebug() << "Loading creatures from:" << creaturesXmlPath;
    if (!CreatureManager::getInstance().loadCreatures(creaturesXmlPath)) {
        qWarning() << "Failed to load creatures.xml from" << creaturesXmlPath;
    } else {
        qDebug() << "creatures.xml loaded successfully.";
    }

    // Load the test map or create a dummy map
    qDebug() << "Attempting to load map from:" << testMapPath;
    bool mapLoaded = currentMap->loadFromFile(testMapPath);
    if (!mapLoaded) {
        qWarning() << "Failed to load" << testMapPath << "! Creating a dummy map.";
        currentMap->clear(); // Ensure map is empty before creating dummy
        currentMap->setSize(QSize(20, 15)); // Example: 20x15 map
        
        // Attempt to get a ground item (e.g., ID 100, common grass or dirt)
        // This requires ItemManager to have loaded items from Tibia.dat successfully.
        const Item* groundItem = ItemManager::getInstance().getItemById(100); 
        if (groundItem) {
            qDebug() << "Using item ID 100 as ground tile for dummy map.";
            for (int x = 0; x < 20; ++x) {
                for (int y = 0; y < 15; ++y) {
                    // addItem in Map takes a const Item&.
                    currentMap->addItem(x, y, Layer::Type::Ground, *groundItem);
                }
            }
        } else {
            qWarning() << "Could not find item ID 100 to use as ground for dummy map. Dummy map will be empty.";
        }
        currentMap->setName("Dummy Map");
    } else {
        qDebug() << testMapPath << "loaded successfully.";
    }
    
    qDebug() << "--- Finished Initializing Managers and Loading Data ---";

    // Link brushes with mapview and map instance after creation.
    mapView->setMap(currentMap); // Ensure MapView has the correct map instance.

    // Link selection toolbar with brush (after `mainToolBar` init in `createToolBar` not `setupUi`)
    if (selectionToolbar && currentMap) {
        Brush* selectionBrush = BrushManager::getInstance().getBrush(Brush::Type::Selection);
        selectionToolbar->setSelectionBrush(static_cast<SelectionBrush*>(selectionBrush));
        // You also need to provide selectionToolbar with `MapView` access for coordinate transformations
        selectionToolbar->setMapView(mapView); // Let selectionToolbar control mapview/map based on selection.
    }
    // Ensure MapView has the map set AFTER map is loaded/created
    mapView->setMap(currentMap);
}

void MainWindow::createDockWindows()
{
    // Left side docks
    toolsDock = new QDockWidget(tr("Tools"), this);
    toolsPanel = new ToolsPanel(this); // Panel inside dock
    toolsDock->setWidget(toolsPanel);
    addDockWidget(Qt::LeftDockWidgetArea, toolsDock);

    layersDock = new QDockWidget(tr("Layers"), this);
    layerWidget = new LayerWidget(this); // Panel inside dock
    layersDock->setWidget(layerWidget);
    addDockWidget(Qt::LeftDockWidgetArea, layersDock);

    // Bottom side docks (Item, Creature Lists, can be tabbed with Tools, Layers)
    itemDock = new QDockWidget(tr("Items"), this);
    itemListWidget = new QListWidget(this); // Simple list
    itemDock->setWidget(itemListWidget);
    addDockWidget(Qt::BottomDockWidgetArea, itemDock);

    creatureDock = new QDockWidget(tr("Creatures"), this);
    creatureListWidget = new QListWidget(this); // Simple list
    creatureDock->setWidget(creatureListWidget);
    addDockWidget(Qt::BottomDockWidgetArea, creatureDock);

    // Right side dock (Properties)
    propertyEditorDock = new QDockWidget(tr("Properties"), this);
    propertyEditor = new PropertyEditorDock(tr("Properties"), this); // This will handle stacked editors
    propertyEditorDock->setWidget(propertyEditor);
    addDockWidget(Qt::RightDockWidgetArea, propertyEditorDock);

    // Tab adjacent docks together for a better user experience
    tabifyDockWidget(toolsDock, layersDock);
    tabifyDockWidget(itemDock, creatureDock);
    // You can set initial active tabs if desired
    toolsDock->raise(); // Make Tools active initially.
    itemDock->raise(); // Make Items active.

    // Connect MapView item selection signals to PropertyEditorDock
    connect(mapView, &MapView::tileSelected, propertyEditor, &PropertyEditorDock::setTile);
    connect(mapView, &MapView::itemSelected, propertyEditor, &PropertyEditorDock::setItem);
    connect(mapView, &MapView::creatureSelected, propertyEditor, &PropertyEditorDock::setCreature);
    connect(mapView, &MapView::objectDeselected, propertyEditor, &PropertyEditorDock::clearEditors);
    
    // Connect palette item/creature selection to ToolsPanel as well.
    if (toolsPanel) {
        // toolsPanel::onItemSelected takes Item*, populateItemList() also should return items.
        // This will be managed by ToolsPanel itself to trigger current item/creature for brush.
    }
}

void MainWindow::createStatusBar()
{
    QStatusBar* statusBar = new QStatusBar(this);
    setStatusBar(statusBar);

    // Add permanent widgets to the status bar (aligned to right, not covered by temp messages).
    positionLabel = new QLabel(tr("Pos: (---, ---, -)"), this);
    layerLabel = new QLabel(tr("Layer: -"), this);
    selectionLabel = new QLabel(tr("Sel: (---,---)-(---,---)"), this);
    zoomLabel = new QLabel(tr("Zoom: 100%"), this);

    statusBar->addPermanentWidget(positionLabel);
    statusBar->addPermanentWidget(layerLabel);
    statusBar->addPermanentWidget(selectionLabel);
    statusBar->addPermanentWidget(zoomLabel);

    updateStatusBar(); // Initial update
}

void MainWindow::updateWindowTitle()
{
    QString title = QApplication::applicationDisplayName();
    if (currentMap) {
        if (!currentMap->getName().isEmpty()) {
            title += " - " + currentMap->getName();
        }
        if (currentMap->isModified()) {
            title += "*";
        }
    }
    setWindowTitle(title);
}

void MainWindow::updateStatusBar()
{
    // Update labels with current map state (zoom, layer, position, selection).
    // Position (cursor) is updated by MapView::mousePositionChanged.
    layerLabel->setText(tr("Layer: %1").arg(currentLayer));
    zoomLabel->setText(tr("Zoom: %1%").arg(static_cast<int>(mapView->getZoom() * 100)));
    
    QRect selection = currentMap->getSelection();
    if (selection.isEmpty()) {
        selectionLabel->setText(tr("Sel: None"));
    } else {
        selectionLabel->setText(tr("Sel: (%1,%2)-(%3,%4) (%5x%6)")
                                .arg(selection.left()).arg(selection.top())
                                .arg(selection.right()).arg(selection.bottom())
                                .arg(selection.width()).arg(selection.height()));
    }
}

// Dialog placeholder implementations for context menu calls
void MainWindow::showMapPropertiesDialog() { QMessageBox::information(this, tr("Map Properties"), tr("Map Properties Dialog is not yet fully implemented.")); }
void MainWindow::showMapStatisticsDialog() { QMessageBox::information(this, tr("Map Statistics"), tr("Map Statistics Dialog is not yet fully implemented.")); }
void MainWindow::showGotoPositionDialog() { QMessageBox::information(this, tr("Go To Position"), tr("Go To Position Dialog is not yet fully implemented.")); }
void MainWindow::showFindItemDialog() { QMessageBox::information(this, tr("Find Item/Browse Tile"), tr("Find Item Dialog (including Browse Tile) is not yet fully implemented.")); }
void MainWindow::showFindCreatureDialog() { QMessageBox::information(this, tr("Find Creature"), tr("Find Creature Dialog is not yet fully implemented.")); }
void MainWindow::showFindSimilarItemsDialog() { QMessageBox::information(this, tr("Find Similar Items"), tr("Find Similar Items Dialog is not yet fully implemented.")); }
void MainWindow::showBorderSystemDialog() { QMessageBox::information(this, tr("Border System Settings"), tr("Border System Dialog is not yet fully implemented.")); }


// --- File Operations ---
void MainWindow::createNewMap()
{
    if (currentMap->isModified()) {
        if (!maybeSave()) return; // Ask to save, if cancelled, return.
    }
    currentMap->clear();
    currentMap->setSize(QSize(100,100)); // Default map size.
    mapView->setMap(currentMap); // Ensure map is refreshed in view.
    setCurrentFile(QString()); // No current file path.
    updateWindowTitle();
    updateStatusBar();
    // Also clear clipboard (not implemented here), clear selection, reset zoom, etc.
}

bool MainWindow::loadMap(const QString& filePath)
{
    if (currentMap->isModified()) {
        if (!maybeSave()) return false;
    }

    if (!currentMap->loadFromFile(filePath)) {
        QMessageBox::critical(this, tr("Error Loading Map"), tr("Could not load map from '%1'.").arg(filePath));
        return false;
    }
    setCurrentFile(filePath);
    mapView->setMap(currentMap); // Ensure map is refreshed in view.
    updateWindowTitle();
    updateStatusBar();
    return true;
}

bool MainWindow::saveMap()
{
    if (currentMap->hasFile()) {
        return currentMap->saveToFile(currentMapFile);
    } else {
        return saveMapAs();
    }
}

bool MainWindow::saveMapAs()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Map As),
                                                     QDir::homePath(), tr("OTBM Maps (*.otbm);;All Files (*)"));
    if (fileName.isEmpty()) return false;
    
    // Ensure .otbm extension is present (basic handling, a more robust check needed for all map types).
    if (!fileName.toLower().endsWith(".otbm")) {
        fileName += ".otbm";
    }

    if (currentMap->saveToFile(fileName)) {
        setCurrentFile(fileName);
        updateWindowTitle();
        setUnifiedTitleAndToolBarOnMac(true); // Mac specific UI hint
        return true;
    }
    return false;
}

// --- Edit Operations ---
void MainWindow::undo() { undoStack->undo(); updateStatusBar(); mapView->getScene()->update(); }
void MainWindow::redo() { undoStack->redo(); updateStatusBar(); mapView->getScene()->update(); }

// MapView `copyRequest` / `cutRequest` / `pasteRequest` / `deleteRequest` are connected to these slots.
void MainWindow::onMapViewCopyRequest(const QRect& selectionRect) { copySelection(); }
void MainWindow::onMapViewCutRequest(const QRect& selectionRect) { cutSelection(); }
void MainWindow::onMapViewPasteRequest(const QPoint& targetPos) { pasteSelection(); }
void MainWindow::onMapViewDeleteRequest(const QRect& selectionRect) { deleteSelection(); }

void MainWindow::cutSelection() { copySelection(); deleteSelection(); } // Simple implementation: copy then delete
void MainWindow::copySelection() {
    if (currentMap->getSelection().isEmpty()) return;
    // Call CopyBuffer to populate clipboard.
    // `ClipboardData::getInstance().copy(currentMap->getSelection(), currentLayer);` // Needs integration of ClipboardData/CopyBuffer
    // For now, simple textual copy.
    QString copiedText = tr("Map selection copied! (No real data copied yet)");
    if (currentMap->getSelection().isValid()) {
        copiedText = QString("Copied selection: (%1,%2)-(%3,%4)")
                     .arg(currentMap->getSelection().x())
                     .arg(currentMap->getSelection().y())
                     .arg(currentMap->getSelection().x() + currentMap->getSelection().width() -1)
                     .arg(currentMap->getSelection().y() + currentMap->getSelection().height() - 1);
    }
    QApplication::clipboard()->setText(copiedText);
    qDebug() << copiedText;
    // Set ClipboardData for full object copy/paste
    // ClipboardData::getInstance().copy(currentMap, currentMap->getSelection(), currentLayer); // Needs to use map's own methods
}

void MainWindow::pasteSelection() {
    // `currentMap->pasteSelection` which usually uses a `PasteSelectionCommand`.
    // Get mouse position or active tile position for paste.
    // For now, directly triggers action if ClipboardData has content.
    if (ClipboardData::getInstance().hasClipboardData()) {
        QPoint pastePos = mapView->mapToTile(mapView->mapFromGlobal(QCursor::pos())); // Get tile under mouse.
        // `currentMap->moveSelection(pastePos, true);` // Move copy.
        // `undoStack->push(new PasteSelectionCommand(currentMap, pastePos, ClipboardData::getInstance().get()));` // Need ClipboardData for command.
        qDebug() << "Paste action triggered at" << pastePos << ". Actual paste logic not implemented yet.";
        currentMap->setModified(true);
        currentMap->clearSelection();
        mapView->clearSelection(); // Clear visual selection.
        mapView->getScene()->update();
    } else {
        QMessageBox::information(this, tr("Paste"), tr("Clipboard is empty or does not contain valid map data."));
    }
}

void MainWindow::deleteSelection() {
    // This pushes `DeleteSelectionCommand` to `undoStack`.
    if (!currentMap->getSelection().isEmpty()) {
        // undoStack->push(new DeleteSelectionCommand(currentMap, currentMap->getSelection()));
        // For now, directly clear selection, then update visual:
        currentMap->setModified(true);
        currentMap->clearSelection(); // This clears tiles within the selection from Map model
        mapView->clearSelection(); // Updates visual scene
        mapView->getScene()->update();
        qDebug() << "Delete selection action executed (model cleared).";
    }
}

void MainWindow::selectAll() {
    currentMap->setSelection(QRect(0,0, currentMap->getSize().width(), currentMap->getSize().height()));
    // This will emit selectionChanged from Map and update MapView/status bar.
    qDebug() << "Selected all tiles.";
}

void MainWindow::deselectAll() {
    currentMap->clearSelection();
    // This will emit selectionChanged from Map and update MapView/status bar.
    qDebug() << "Deselected all tiles.";
}


// --- View Operations ---
void MainWindow::zoomIn() { mapView->setZoom(mapView->getZoom() * 1.15); }
void MainWindow::zoomOut() { mapView->setZoom(mapView->getZoom() / 1.15); }
void MainWindow::zoomReset() { mapView->setZoom(1.0); }
void MainWindow::toggleGrid(bool show) { mapView->setShowGrid(show); }
void MainWindow::toggleCollisions(bool show) { mapView->setShowCollisions(show); }
void MainWindow::toggleStatusBar(bool show) { statusBar()->setVisible(show); }
void MainWindow::toggleToolbar(bool show) {
    // This applies to the mainToolbar. Original has specific `ToolBarID`.
    mainToolBar->setVisible(show);
}
void MainWindow::toggleFullscreen() {
    if (isFullScreen()) { showNormal(); }
    else { showFullScreen(); }
}

void MainWindow::toggleBorderSystem(bool enabled) {
    if (currentMap && currentMap->getBorderSystem()) {
        currentMap->getBorderSystem()->setEnabled(enabled);
        qDebug() << "Automagic Borders: " << (enabled ? "Enabled" : "Disabled");
        // May also trigger `Map` to call `applyBordersToRegion` on map to refresh borders
    }
}

// --- Tool/Brush Operations ---
void MainWindow::setCurrentTool(Brush::Type toolType) {
    mapView->setCurrentTool(toolType);
    if (toolType == Brush::Type::Selection) {
        selectionToolbar->setVisible(true); // Show selection-specific toolbar.
    } else {
        selectionToolbar->setVisible(false); // Hide.
    }
    // Update toolbar buttons based on new tool (e.g. check the selected tool).
    mainToolBar->onToolChanged(static_cast<int>(toolType)); // Assumed a similar slot exists in mainToolBar.
}
void MainWindow::increaseBrushSize() {
    // Based on `Source/gui.cpp::IncreaseBrushSize` logic.
    int currentSize = BrushManager::getInstance().getBrush(mapView->getBrush()->getType())->getSize();
    mapView->getBrush()->setSize(currentSize + 1); // Directly increment Brush's size.
    // The brush preview (cursorItem in MapView) should automatically update due to MapView::updateCursor().
}
void MainWindow::decreaseBrushSize() {
    // Based on `Source/gui.cpp::DecreaseBrushSize` logic.
    int currentSize = BrushManager::getInstance().getBrush(mapView->getBrush()->getType())->getSize();
    if (currentSize > 1) { // Min brush size is 1.
        mapView->getBrush()->setSize(currentSize - 1);
    }
}
void MainWindow::switchToolMode() {
    // Toggle between Normal (Drawing) and Selection.
    // Based on `Source/gui.cpp::SwitchMode`.
    if (mapView->getBrush() && mapView->getBrush()->getType() == Brush::Type::Selection) {
        setCurrentTool(Brush::Type::Normal); // Switch to Normal brush.
    } else {
        setCurrentTool(Brush::Type::Selection); // Switch to Selection brush.
    }
}
void MainWindow::selectPreviousBrush() {
    // This typically uses a brush history in `BrushManager` or `GUI`.
    // Placeholder.
    qDebug() << "Select Previous Brush (Not implemented).";
}


// --- Layer/Floor Operations ---
void MainWindow::changeFloor(int layer) {
    // Ensure layer is within bounds [0, 15] based on `Map::LayerCount`.
    layer = qBound(0, layer, Map::LayerCount - 1); // Clamp to valid layer range.
    if (currentLayer == layer) return;

    currentLayer = layer;
    mapView->setCurrentLayer(currentLayer); // Propagate to MapView (which updates MapScene).
    mapView->getScene()->update(); // Force redraw after layer change.
    // Also notify status bar through `onLayerChanged`.
    onLayerChanged(currentLayer);
    qDebug() << "Changed active layer to: " << currentLayer;
}


// --- Slots (Event Handlers) ---
void MainWindow::onToolSelected(int toolId) {
    setCurrentTool(static_cast<Brush::Type>(toolId)); // Updates active tool.
}

void MainWindow::onLayerChanged(int layer) {
    currentLayer = layer; // Update internal current layer for main window.
    mapView->setCurrentLayer(currentLayer); // Propagate if not already set.
    updateStatusBar();
}

void MainWindow::onItemSelected(Item* item) {
    // From ToolsPanel or brush selection (palette).
    setCurrentItem(item); // For MapView to set in NormalBrush or FloodFillBrush.
    propertyEditor->setItem(item); // Show item properties.
    qDebug() << "Item selected from palette: " << item->getName();
}

void MainWindow::onToolChanged(Brush::Type type) {
    // Propagate brush type changes.
    // Already handled by `setCurrentTool`, this can be removed if not directly hooked to outside tool source.
    qDebug() << "Brush type changed to: " << static_cast<int>(type);
}

void MainWindow::onCreatureSelected(QListWidgetItem* item) {
    // From creature list widget in palette.
    if (item) {
        // Need to get the actual Creature* associated with this list item.
        // Assuming ItemManager has a lookup for Creatures.
        Creature* creature = CreatureManager::getInstance().getCreatureById(item->data(Qt::UserRole).toInt()); // Assume UserRole holds ID.
        if (creature) {
            setCurrentCreature(creature); // For creature brushes.
            propertyEditor->setCreature(creature); // Show creature properties.
            qDebug() << "Creature selected from palette: " << creature->getName();
        }
    }
}

void MainWindow::onZoomChanged(double zoom) {
    updateStatusBar(); // Update status bar's zoom display.
}

void MainWindow::onSelectionChanged() {
    updateStatusBar(); // Update status bar's selection display.
    // Also trigger update of selection toolbar here to reflect enabled/disabled states.
}

void MainWindow::onUndoStackChanged() {
    // Update main menu's Undo/Redo action enabled state via MainMenu.
    mainMenu->updateUndoRedoActions(undoStack->canUndo(), undoStack->canRedo());
    updateStatusBar(); // Refresh status bar which might show Undo/Redo counts.
}

void MainWindow::onMapModified() {
    // Update title bar for '*' modification indicator.
    updateWindowTitle();
    // This is primarily where autosave `CheckAutoSave()` from original `gui.cpp` would be called.
    // `SettingsManager::getInstance().checkAutoSave();` (requires linking/implementing `SettingsManager`)
}

void MainWindow::onMousePositionChanged(const QPoint& position) {
    // Update position label in status bar.
    positionLabel->setText(tr("Pos: (%1, %2, %3)").arg(position.x()).arg(position.y()).arg(mapView->getCurrentLayer()));
}


// --- Property Editor Handlers (triggered from MapView or direct object selection) ---

void MainWindow::onTileSelected(Tile* tile) {
    if (tile) { propertyEditor->setTile(tile); }
    else { propertyEditor->clearEditors(); }
}

void MainWindow::onMapItemSelected(Item* item) {
    if (item) { propertyEditor->setItem(item); }
    else { propertyEditor->clearEditors(); }
}

void MainWindow::onMapCreatureSelected(Creature* creature) {
    if (creature) { propertyEditor->setCreature(creature); }
    else { propertyEditor->clearEditors(); }
}

void MainWindow::onObjectDeselected() {
    // When no object is selected, clear property editor.
    propertyEditor->clearEditors();
}

// --- Settings Management (basic functions) ---
void MainWindow::loadSettings() { /* Impl */ }
void MainWindow::saveSettings() { /* Impl */ }

// Helper for save confirmation.
bool MainWindow::maybeSave() {
    if (currentMap->isModified()) {
        QMessageBox::StandardButton ret;
        ret = QMessageBox::warning(this, tr("Idlers Map Editor"),
                                   tr("The map has been modified.\nDo you want to save your changes?"),
                                   QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        if (ret == QMessageBox::Save)
            return saveMap();
        else if (ret == QMessageBox::Cancel)
            return false;
    }
    return true;
}

// Store current file path for save/saveAs, update recent files list (from main_menubar)
void MainWindow::setCurrentFile(const QString& filePath) {
    currentMapFile = filePath;
    // Also add to recent files list in main menu (if implemented).
    if (mainMenu) {
        mainMenu->addRecentFile(filePath);
    }
    // Update modified status if path changes from unnamed to named
    if (currentMap && filePath.isEmpty()) {
        currentMap->setModified(true); // If file path cleared, consider it a new/unsaved doc.
        // currentMap->unnamed = true; // No longer has a file assigned
    } else if (currentMap) {
        // currentMap->unnamed = false;
        currentMap->setModified(false); // New file, not modified yet.
    }
}

// (Stub from previous prompt, as per full file replacement).
void MainWindow::createMenus() {
    // Actual menu creation is handled by MainMenu class now.
}
// (Stub from previous prompt, as per full file replacement).
void MainWindow::createToolBar() {
    // Actual toolbar creation handled by MainToolBar class now.
}
// (Stub from previous prompt, as per full file replacement).
void MainWindow::createDockPanels() {
    // Actual dock panel creation handled by createDockWindows.
}

// Populates item/creature list widgets (used in itemDock / creatureDock).
void MainWindow::populateItemList() {
    itemListWidget->clear();
    QList<Item*> allItems = ItemManager::getInstance().getAllItems();
    for (Item* item : allItems) {
        QListWidgetItem* listItem = new QListWidgetItem(item->getName(), itemListWidget);
        listItem->setIcon(item->getIcon()); // Assuming Item has getIcon() returning QIcon or QPixmap
        listItem->setData(Qt::UserRole, item->getId()); // Store item ID
        // Additional properties if needed
        loadedItems.append(item); // Store locally if needed.
    }
    // Sort, if necessary, and connect clicked signals etc.
    connect(itemListWidget, &QListWidget::itemClicked, this, [this](QListWidgetItem* item){
        if (item) {
            Item* selectedItem = ItemManager::getInstance().getItemById(item->data(Qt::UserRole).toInt());
            onItemSelected(selectedItem); // Pass selected item to internal handler.
            mapView->setCurrentItem(selectedItem); // Make MapView aware for brush.
        }
    });
}

void MainWindow::populateCreatureList() {
    creatureListWidget->clear();
    QList<Creature*> allCreatures = CreatureManager::getInstance().getAllCreatures();
    for (Creature* creature : allCreatures) {
        QListWidgetItem* listItem = new QListWidgetItem(creature->getName(), creatureListWidget);
        // Creature has getSprite() which gives a QPixmap. Create QIcon from it.
        listItem->setIcon(QIcon(creature->getSprite()));
        listItem->setData(Qt::UserRole, creature->getId()); // Store creature ID
        loadedCreatures.append(creature);
    }
    connect(creatureListWidget, &QListWidget::itemClicked, this, [this](QListWidgetItem* item){
        if (item) {
            Creature* selectedCreature = CreatureManager::getInstance().getCreatureById(item->data(Qt::UserRole).toInt());
            onCreatureSelected(item); // Pass original list item, onCreatureSelected will extract Creature*
            mapView->setCurrentCreature(selectedCreature); // Make MapView aware for brushes.
        }
    });
}

// From Source/main_toolbar.cpp and Source/mainwindow.cpp. For Automagic border system.
void MainWindow::registerDefaultBorderTiles() {
    if (currentMap && currentMap->getBorderSystem()) {
        BorderSystem* bs = currentMap->getBorderSystem();

        // This information usually comes from XML configuration for actual Tibia tilesets.
        // The original `Source/main_toolbar.cpp::registerDefaultBorderTiles()` hardcoded these.
        // It's just a demonstration of the registration mechanism here.

        // Grass borders (assuming Item IDs exist for these)
        QVector<int> grassBorders = { /* IDs from Source/src/brush_tables.cpp: GroundBrush::border_types */ 
                                     1, 2, 3, 4, 5, 6, 7, 8, /* Example. Needs to be actual values from game */
                                     485,486,487,488, // Example grass-dirt transitions from some Tibia versions.
                                     490,491,492,493 }; // And more for corners.
        bs->registerBorderSource(ClientVersion::stringToVersionID("Grass Item ID (e.g. 100)"), grassBorders);

        // Water borders (assuming Item IDs exist for these)
        QVector<int> waterBorders = { /* IDs from game client/resources for water border transitions */ 
                                     10, 11, 12, 13, 14, 15, 16, 17 };
        bs->registerBorderSource(ClientVersion::stringToVersionID("Water Item ID (e.g. 200)"), waterBorders);

        // Sand borders (if relevant as border sources)
        // ... more terrain type border registrations.

        qDebug() << "Default border tiles registered with BorderSystem.";
    }
}

void MainWindow::loadSprDatFiles() {
    QSettings settings;
    QString lastSpr = settings.value("data/lastSprPath", QDir::homePath()).toString();
    QString lastDat = settings.value("data/lastDatPath", QDir::homePath()).toString();

    QString sprPath = QFileDialog::getOpenFileName(this, tr("Select Tibia SPR file"), lastSpr, tr("Tibia SPR (*.spr);;All Files (*)"));
    if (sprPath.isEmpty()) return;
    QString datPath = QFileDialog::getOpenFileName(this, tr("Select Tibia DAT file"), lastDat, tr("Tibia DAT (*.dat);;All Files (*)"));
    if (datPath.isEmpty()) return;

    settings.setValue("data/lastSprPath", QFileInfo(sprPath).absolutePath());
    settings.setValue("data/lastDatPath", QFileInfo(datPath).absolutePath());

    // Load sprites and items
    bool sprOk = SpriteManager::getInstance()->loadSprites(sprPath, datPath);
    bool datOk = ItemManager::getInstance().loadTibiaDat(datPath);

    if (sprOk && datOk) {
        populateItemList();
        populateCreatureList();
        statusBar()->showMessage(tr("Loaded SPR and DAT files successfully."), 5000);
    } else {
        QMessageBox::critical(this, tr("Error"), tr("Failed to load SPR or DAT file. Check file format and try again."));
    }
}