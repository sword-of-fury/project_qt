#include "maintoolbar.h"
#include "mainwindow.h"
#include "brush.h"
#include "brushmanager.h" // To get brush icons
#include "settingsmanager.h" // For default zoom settings (if implemented)
#include "resources.qrc" // Implicitly includes resources for images

#include <QActionGroup> // For radio-like button behavior on tools
#include <QLabel> // For floor/zoom labels
#include <QSpinBox> // If using spinboxes for zoom/layer

MainToolBar::MainToolBar(MainWindow* parent) :
    QToolBar(tr("Main Toolbar"), parent), // Set default objectName and parent
    parentWindow(parent),
    // Initialize QAction pointers to nullptr, will be assigned in createXActions methods
    newMapAction(nullptr), openMapAction(nullptr), saveMapAction(nullptr),
    undoAction(nullptr), redoAction(nullptr), cutAction(nullptr), copyAction(nullptr),
    pasteAction(nullptr), deleteAction(nullptr),
    zoomInToolAction(nullptr), zoomOutToolAction(nullptr), zoomResetToolAction(nullptr),
    toggleGridAction(nullptr), toggleCollisionsAction(nullptr),
    brushToolAction(nullptr), eraserToolAction(nullptr), selectionToolAction(nullptr),
    floodFillToolAction(nullptr),
    layerLabel(nullptr), layerCombobox(nullptr), zoomLabel(nullptr), zoomCombobox(nullptr)
{
    // Configure toolbar properties
    setIconSize(QSize(32, 32)); // Default icon size, can be set from settings
    setToolButtonStyle(Qt::ToolButtonTextUnderIcon); // Icons with text below them

    // Create and add action groups to the toolbar
    createFileActions();
    addSeparator(); // Separator between groups
    createEditActions();
    addSeparator();
    createViewActions();
    addSeparator();
    createToolActions(); // Includes actual tool buttons
    addSeparator();
    createLayerControls(); // Add floor combobox
    createZoomControls(); // Add zoom combobox
    addSeparator();

    // Connect toolbar button actions to MainMenu slots (or MainWindow's public slots)
    // The main menu is often the central dispatcher for actions.
    // If not, directly connect to MainWindow: `connect(action, &QAction::triggered, parentWindow, &MainWindow::onActionSlot);`
    // Using mainmenu slots, so that MainMenu is the single dispatcher.
    // (Connecting direct: MapView actions -> MainWindow, MainMenu -> MainWindow, MainToolbar -> MainWindow)

    // The Action Group for tool buttons ensures only one is checked at a time
    QActionGroup* toolButtonGroup = new QActionGroup(this);
    toolButtonGroup->setExclusive(true); // Only one action in group can be checked at a time
    toolButtonGroup->addAction(brushToolAction);
    toolButtonGroup->addAction(eraserToolAction);
    toolButtonGroup->addAction(selectionToolAction);
    toolButtonGroup->addAction(floodFillToolAction);
    
    // Ensure initial tool is correctly selected (set after actions are in group)
    updateToolActions(parentWindow->getMapView()->getBrush()->getType());

    // Update states of actions dynamically after initial setup
    updateUndoRedoActions(parentWindow->getUndoStack()->canUndo(), parentWindow->getUndoStack()->canRedo());
    updateSelectionActions(parentWindow->getMap() && !parentWindow->getMap()->getSelection().isEmpty());
}

MainToolBar::~MainToolBar()
{
    // QActions are children of this toolbar (QToolBar is a QWidget which is a QObject).
    // QObject's destructor automatically deletes child objects. So no manual deletion of actions.
}

// --- Action Creation Methods ---

void MainToolBar::createFileActions()
{
    newMapAction = addAction(QIcon(":/images/new_map.png"), tr("New"), parentWindow, SLOT(createNewMap())); // Placeholder for icon
    openMapAction = addAction(QIcon(":/images/open_map.png"), tr("Open"), parentWindow, SLOT(openMap()));
    saveMapAction = addAction(QIcon(":/images/save_map.png"), tr("Save"), parentWindow, SLOT(saveMap()));
    
    // These actions are directly handled by MainWindow methods for simplicity now.
    // connect(newMapAction, &QAction::triggered, parentWindow->mainMenu, &MainMenu::onNewMap); // Example if mainmenu is dispatcher.
    // Or connect directly to MainWindow slots
}

void MainToolBar::createEditActions()
{
    undoAction = addAction(QIcon(":/images/undo.png"), tr("Undo"), parentWindow, SLOT(undo()));
    redoAction = addAction(QIcon(":/images/redo.png"), tr("Redo"), parentWindow, SLOT(redo()));
    addSeparator();
    cutAction = addAction(QIcon(":/images/cut.png"), tr("Cut"), parentWindow, SLOT(cutSelection()));
    copyAction = addAction(QIcon(":/images/copy.png"), tr("Copy"), parentWindow, SLOT(copySelection()));
    pasteAction = addAction(QIcon(":/images/paste.png"), tr("Paste"), parentWindow, SLOT(pasteSelection()));
    deleteAction = addAction(QIcon(":/images/delete.png"), tr("Delete"), parentWindow, SLOT(deleteSelection()));
}

void MainToolBar::createViewActions()
{
    zoomInToolAction = addAction(QIcon(":/images/zoomin.png"), tr("Zoom In"), parentWindow, SLOT(zoomIn()));
    zoomOutToolAction = addAction(QIcon(":/images/zoomout.png"), tr("Zoom Out"), parentWindow, SLOT(zoomOut()));
    zoomResetToolAction = addAction(QIcon(":/images/zoomreset.png"), tr("Zoom Normal"), parentWindow, SLOT(zoomReset()));
    addSeparator();
    toggleGridAction = addAction(QIcon(":/images/grid.png"), tr("Toggle Grid"), parentWindow, SLOT(toggleGrid(bool)));
    toggleGridAction->setCheckable(true); toggleGridAction->setChecked(true); // Default
    toggleCollisionsAction = addAction(QIcon(":/images/collision.png"), tr("Toggle Collisions"), parentWindow, SLOT(toggleCollisions(bool)));
    toggleCollisionsAction->setCheckable(true); toggleCollisionsAction->setChecked(true); // Default
}

void MainToolBar::createToolActions()
{
    // Use Icons from `resources.qrc`. Assuming icons exist for these types.
    brushToolAction = addAction(QIcon(":/images/brush.png"), tr("Normal Brush"), parentWindow, SLOT(setCurrentTool(Brush::Type)));
    brushToolAction->setData(static_cast<int>(Brush::Type::Normal)); brushToolAction->setCheckable(true);
    
    eraserToolAction = addAction(QIcon(":/images/eraser.png"), tr("Eraser"), parentWindow, SLOT(setCurrentTool(Brush::Type)));
    eraserToolAction->setData(static_cast<int>(Brush::Type::Eraser)); eraserToolAction->setCheckable(true);

    selectionToolAction = addAction(QIcon(":/images/selection.png"), tr("Selection"), parentWindow, SLOT(setCurrentTool(Brush::Type)));
    selectionToolAction->setData(static_cast<int>(Brush::Type::Selection)); selectionToolAction->setCheckable(true);
    
    floodFillToolAction = addAction(QIcon(":/images/floodfill.png"), tr("Flood Fill"), parentWindow, SLOT(setCurrentTool(Brush::Type)));
    floodFillToolAction->setData(static_cast<int>(Brush::Type::FloodFill)); floodFillToolAction->setCheckable(true);
    
    // Additional brushes will be implemented later: RAW, Ground, Doodad, Door, Wall, Carpet, Table, Creature, Spawn, House, Waypoint
    // These will be in ToolsPanel (palette) generally.
}

void MainToolBar::createLayerControls()
{
    layerLabel = new QLabel(tr("Floor:"), this);
    addWidget(layerLabel); // Add the label to the toolbar

    layerCombobox = new QComboBox(this);
    for (int i = 0; i < Map::LayerCount; ++i) {
        layerCombobox->addItem(Layer::typeToString(static_cast<Layer::Type>(i)), i); // Store integer LayerType as data
    }
    layerCombobox->setCurrentIndex(static_cast<int>(Layer::Type::Ground)); // Default to Ground layer (0 or other offset)
    addWidget(layerCombobox);
    
    // Connect combobox signal to parentWindow's slot (e.g. to updateMapView layer)
    connect(layerCombobox, qOverload<int>(&QComboBox::currentIndexChanged), this, &MainToolBar::onLayerComboboxChanged);
}

void MainToolBar::createZoomControls()
{
    zoomLabel = new QLabel(tr("Zoom:"), this);
    addWidget(zoomLabel); // Add the label to the toolbar

    zoomCombobox = new QComboBox(this);
    QStringList zoomLevels = { "10%", "25%", "50%", "100%", "200%", "400%", "800%" };
    QList<double> zoomFactors = { 0.1, 0.25, 0.5, 1.0, 2.0, 4.0, 8.0 };
    for (int i = 0; i < zoomLevels.size(); ++i) {
        zoomCombobox->addItem(zoomLevels.at(i), zoomFactors.at(i)); // Store zoom factor as data
    }
    zoomCombobox->setCurrentIndex(zoomFactors.indexOf(1.0)); // Default to 100% zoom
    addWidget(zoomCombobox);
    
    // Connect combobox signal to parentWindow's slot (e.g. to setMapView zoom)
    connect(zoomCombobox, qOverload<int>(&QComboBox::currentIndexChanged), this, &MainToolBar::onZoomComboboxChanged);
}


// --- Dynamic Update Methods (Called from MainWindow) ---

void MainToolBar::updateUndoRedoActions(bool canUndo, bool canRedo)
{
    if (undoAction) undoAction->setEnabled(canUndo);
    if (redoAction) redoAction->setEnabled(canRedo);
}

void MainToolBar::updateSelectionActions(bool hasSelection)
{
    if (cutAction) cutAction->setEnabled(hasSelection);
    if (copyAction) copyAction->setEnabled(hasSelection);
    if (deleteAction) deleteAction->setEnabled(hasSelection);
    // Paste action enabled/disabled based on ClipboardData content directly (or from MainWindow property).
    // It is best handled by `MainWindow::updateStatusBar()` or `onSelectionChanged` and then calls action to check clipboard.
    // For now:
    if (pasteAction) pasteAction->setEnabled(ClipboardData::getInstance().hasClipboardData());
}

void MainToolBar::updateToolActions(Brush::Type currentToolType)
{
    // Uncheck all other tool actions first.
    QList<QAction*> allTools = findChildren<QAction*>();
    for (QAction* action : allTools) {
        if (action->isCheckable()) {
            action->setChecked(false); // Clear all checks.
        }
    }
    // Then check the one matching currentToolType.
    QAction* targetAction = nullptr;
    if (currentToolType == Brush::Type::Normal && brushToolAction) targetAction = brushToolAction;
    else if (currentToolType == Brush::Type::Eraser && eraserToolAction) targetAction = eraserToolAction;
    else if (currentToolType == Brush::Type::Selection && selectionToolAction) targetAction = selectionToolAction;
    else if (currentToolType == Brush::Type::FloodFill && floodFillToolAction) targetAction = floodFillToolAction;

    if (targetAction) {
        targetAction->setChecked(true); // Set checked if a match is found
        // Ensure action is also enabled
        targetAction->setEnabled(true);
    }
}

void MainToolBar::showToolbar(ToolBarID id, bool show) {
    // This is from Source/main_toolbar.cpp::Show()
    // It would imply multiple internal QToolBar components if needed
    // For now, only affects this one toolbar's visibility as it's the main.
    // If you add sub-toolbars for specific features (e.g., DrawingToolsToolbar, SelectionToolsToolbar),
    // they would be shown/hidden here by `id`.
    if (id == TOOLBAR_MAIN) { // Assuming TOOLBAR_MAIN exists in gui_ids.h
        this->setVisible(show);
    }
}

void MainToolBar::hideAllToolbars(bool update) {
    // Hide this main toolbar. Other toolbars like SelectionToolbar handled in MainWindow.
    this->setVisible(false);
    if (update) { // Trigger update for parent QMainWindow layout
        if (parentWindow) parentWindow->updateDockWindows(); // Or specific layout update method.
    }
}

void MainToolBar::setCurrentLayer(int layer)
{
    // Updates layer combobox based on the `currentLayer` from MainWindow/Map.
    layerCombobox->blockSignals(true); // Avoid triggering slot during programmatic change.
    int index = layerCombobox->findData(layer);
    if (index != -1) {
        layerCombobox->setCurrentIndex(index);
    }
    layerCombobox->blockSignals(false);
}

int MainToolBar::getCurrentLayer() const
{
    return layerCombobox->currentData().toInt();
}

// --- Toolbar Slot Implementations (Delegate to MainWindow) ---
void MainToolBar::onNewMapAction() { parentWindow->createNewMap(); }
void MainToolBar::onOpenMapAction() { parentWindow->openMap(); }
void MainToolBar::onSaveMapAction() { parentWindow->saveMap(); }

void MainToolBar::onUndoAction() { parentWindow->undo(); }
void MainToolBar::onRedoAction() { parentWindow->redo(); }
void MainToolBar::onCutAction() { parentWindow->cutSelection(); }
void MainToolBar::onCopyAction() { parentWindow->copySelection(); }
void MainToolBar::onPasteAction() { parentWindow->pasteSelection(); }
void MainToolBar::onDeleteAction() { parentWindow->deleteSelection(); }

void MainToolBar::onZoomInToolAction() { parentWindow->zoomIn(); }
void MainToolBar::onZoomOutToolAction() { parentWindow->zoomOut(); }
void MainToolBar::onZoomResetToolAction() { parentWindow->zoomReset(); }
void MainToolBar::onToggleGridAction(bool checked) { parentWindow->toggleGrid(checked); }
void MainToolBar::onToggleCollisionsAction(bool checked) { parentWindow->toggleCollisions(checked); }

void MainToolBar::onBrushToolAction() { parentWindow->setCurrentTool(Brush::Type::Normal); }
void MainToolBar::onEraserToolAction() { parentWindow->setCurrentTool(Brush::Type::Eraser); }
void MainToolBar::onSelectionToolAction() { parentWindow->setCurrentTool(Brush::Type::Selection); }
void MainToolBar::onFloodFillToolAction() { parentWindow->setCurrentTool(Brush::Type::FloodFill); }

void MainToolBar::onLayerComboboxChanged(int index) {
    int layerValue = layerCombobox->itemData(index).toInt();
    emit layerChanged(layerValue); // Propagate to MainWindow/MapView/MapScene.
}

void MainToolBar::onZoomComboboxChanged(int index) {
    double zoomFactor = layerCombobox->itemData(index).toDouble();
    emit zoomChanged(zoomFactor); // Propagate to MainWindow/MapView.
}