#ifndef MAINTOOLBAR_H
#define MAINTOOLBAR_H

#include <QToolBar>
#include <QAction>
#include <QIcon>
#include <QSize> // For icon size.
#include <QComboBox> // For layer and zoom selectors on toolbar
#include <QSlider> // If zoom slider is used
#include <QSpinBox> // If numeric zoom is used
#include <QLabel> // For layer label beside combobox

#include "mainwindow.h" // For parent MainWindow reference
#include "gui_ids.h" // For ToolBarID enum if still used
#include "brush_enums.h" // For BrushShape
#include "layer.h" // For Layer::Type
#include "brush.h" // For Brush::Type

class MainWindow; // Forward declaration

/**
 * @brief The MainToolBar class manages the application's main toolbar (QToolBar).
 * It creates standard file, edit, view, and tool actions.
 * It largely mirrors the structure from Source/src/main_toolbar.h/cpp.
 */
class MainToolBar : public QToolBar
{
    Q_OBJECT
public:
    explicit MainToolBar(MainWindow* parent = nullptr);
    virtual ~MainToolBar();

    // Update methods to control action states (enabled/checked) dynamically
    void updateUndoRedoActions(bool canUndo, bool canRedo);
    void updateSelectionActions(bool hasSelection); // For Cut, Copy, Delete enablement
    void updateToolActions(Brush::Type currentToolType); // Checks selected tool button

    // Control visibility of this toolbar or specific sub-toolbars/groups
    // From Source/src/main_toolbar.cpp::Show() and HideAll()
    void showToolbar(ToolBarID id, bool show); // Show specific conceptual group. (Not fully supported with single QToolBar)
    void hideAllToolbars(bool update = true); // Hide all conceptually separate sections (MainToolbar just manages this single toolbar for now)
    
    // Recent file actions if needed directly here.
    // void LoadPerspective(); // Not relevant for this simple toolbar.
    // void SavePerspective(); // Not relevant for this simple toolbar.
    
    // Manual hotkey registration (from Source/src/main_toolbar.cpp::RegisterHotkeys)
    // Now handled more globally in MainWindow or specific hotkey manager
    // void RegisterHotkeys(); 

    // Access to current layer selection for UI.
    void setCurrentLayer(int layer);
    int getCurrentLayer() const;

signals:
    // Signals from toolbar UI elements that MainWindow or MapView handles.
    void layerChanged(int layer); // Emitted when layer combobox value changes
    void zoomChanged(double zoomFactor); // Emitted when zoom combobox/slider value changes

private slots:
    // UI Event Handlers for Toolbar buttons.
    // Standard file/edit/view actions simply delegate to MainMenu/MainWindow slots.
    void onNewMapAction();
    void onOpenMapAction();
    void onSaveMapAction();

    void onUndoAction();
    void onRedoAction();
    void onCutAction();
    void onCopyAction();
    void onPasteAction();
    void onDeleteAction();

    void onZoomInToolAction();
    void onZoomOutToolAction();
    void onZoomResetToolAction();
    void onToggleGridAction(bool checked);
    void onToggleCollisionsAction(bool checked);
    
    // Tool buttons (Normal, Eraser, Selection, Flood Fill)
    void onBrushToolAction();
    void onEraserToolAction();
    void onSelectionToolAction();
    void onFloodFillToolAction();

    // Layer controls (e.g., layer selection on toolbar itself)
    void onLayerComboboxChanged(int index);
    // Zoom controls (e.g., zoom factor combobox)
    void onZoomComboboxChanged(int index);

private:
    MainWindow* parentWindow; // Direct reference to parent MainWindow

    // Action Pointers (to easily update their enabled/checked state)
    QAction* newMapAction;
    QAction* openMapAction;
    QAction* saveMapAction;

    QAction* undoAction;
    QAction* redoAction;
    QAction* cutAction;
    QAction* copyAction;
    QAction* pasteAction;
    QAction* deleteAction;

    QAction* zoomInToolAction;
    QAction* zoomOutToolAction;
    QAction* zoomResetToolAction;
    QAction* toggleGridAction;
    QAction* toggleCollisionsAction;

    QAction* brushToolAction;
    QAction* eraserToolAction;
    QAction* selectionToolAction;
    QAction* floodFillToolAction;

    // UI Widgets directly on the toolbar (for layer/zoom selectors)
    QLabel* layerLabel; // "Floor:"
    QComboBox* layerCombobox; // For active layer selection.
    QLabel* zoomLabel; // "Zoom:"
    QComboBox* zoomCombobox; // For zoom factor selection.

    // Private helper methods for creating actions and toolbar layout
    void createFileActions();
    void createEditActions();
    void createViewActions();
    void createToolActions();
    void createLayerControls(); // For combobox and label
    void createZoomControls(); // For zoom combobox
};

#endif // MAINTOOLBAR_H