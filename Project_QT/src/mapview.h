#ifndef MAPVIEW_H
#define MAPVIEW_H

#include <QGraphicsView>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QPoint>
#include <QAction>
#include <QMenu>
#include <QScrollBar> // Ensure QScrollBar is included if accessed directly for connections
#include <QDebug> // For debugging

#include "brush.h"
#include "map.h"
#include "mapscene.h"
#include "tile.h"
#include "item.h"
#include "creature.h"
#include "clipboarddata.h" // Assumed this will be added for copy/paste from copybuffer
#include "layer.h" // Include Layer.h to get Layer::Type enum


class Map;
class MapScene;

class MapView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit MapView(QWidget* parent = nullptr);
    virtual ~MapView();

    void setMap(Map* map);
    Map* getMap() const;

    void setBrush(Brush* brush);
    Brush* getBrush() const { return currentBrush; }

    void setCurrentTool(Brush::Type type);
    void setCurrentLayer(int layer); // Sets the active editing layer
    void setCurrentItem(Item* item);
    void setCurrentCreature(Creature* creature);

    void setShowGrid(bool show);
    bool getShowGrid() const { return mapScene->getShowGrid(); } // Delegates to MapScene

    void setShowCollisions(bool show);
    bool getShowCollisions() const { return mapScene->getShowCollisions(); } // Delegates to MapScene

    void setZoom(double zoom);
    double getZoom() const { return zoom; }

    MapScene* getScene() const { return mapScene; }

    int getCurrentLayer() const; // Returns the active editing layer
    Item* getCurrentItem() const { return currentItem; }
    Creature* getCurrentCreature() const { return currentCreature; }

    QPoint mapToTile(const QPoint& pos) const;
    QPoint tileToMap(const QPoint& pos) const;

    // View state getters/setters
    bool getShowGridState() const { return mapScene ? mapScene->getShowGrid() : false; } // From MapScene
    // void setShowGrid(bool show); // Already exists, delegates to MapScene

    bool getShowCollisionsState() const { return mapScene ? mapScene->getShowCollisions() : false; } // From MapScene
    // void setShowCollisions(bool show); // Already exists, delegates to MapScene

    bool getShowSpawnsState() const { return m_showSpawns; }
    void setShowSpawns(bool show);
    bool getShowAnimationState() const { return m_showAnimation; }
    void setShowAnimation(bool show);
    bool getShowLightState() const { return m_showLight; }
    void setShowLight(bool show);
    bool getShowAllFloorsState() const { return m_showAllFloors; }
    void setShowAllFloors(bool show);
    bool getShowAsMinimapState() const { return m_showAsMinimap; }
    void setShowAsMinimap(bool show);
    bool getShowTooltipsState() const { return m_showTooltips; }
    void setShowTooltips(bool show);
    bool getShowZonesState() const { return m_showZones; }
    void setShowZones(bool show);
    bool getShowPathingState() const { return m_showPathing; }
    void setShowPathing(bool show);
    bool getExperimentalFogState() const { return m_experimentalFog; }
    void setExperimentalFog(bool show);
    bool getShowOnlyColorsState() const { return m_showOnlyColors; }
    void setShowOnlyColors(bool show);
    bool getShowOnlyModifiedState() const { return m_showOnlyModified; }
    void setShowOnlyModified(bool show);
    bool getAlwaysShowZonesState() const { return m_alwaysShowZones; }
    void setAlwaysShowZones(bool show);
    bool getExtendedHouseShaderState() const { return m_extendedHouseShader; }
    void setExtendedHouseShader(bool show);
    bool getShowClientBoxState() const { return m_showClientBox; }
    void setShowClientBox(bool show);
    bool getGhostItemsState() const { return m_ghostItems; }
    void setGhostItems(bool show);
    bool getGhostHigherFloorsState() const { return m_ghostHigherFloors; }
    void setGhostHigherFloors(bool show);
    bool getShowShadeState() const { return m_showShade; }
    void setShowShade(bool show);
    bool getShowLightStrengthState() const { return m_showLightStrength; }
    void setShowLightStrength(bool show);
    bool getShowTechnicalItemsState() const { return m_showTechnicalItems; }
    void setShowTechnicalItems(bool show);
    bool getShowSpecialTilesState() const { return m_showSpecialTiles; }
    void setShowSpecialTiles(bool show);
    bool getShowHousesState() const { return m_showHouses; }
    void setShowHouses(bool show);
    bool getShowTownsState() const { return m_showTowns; }
    void setShowTowns(bool show);
    bool getShowWaypointsState() const { return m_showWaypoints; }
    void setShowWaypoints(bool show);
    bool getHighlightItemsState() const { return m_highlightItems; }
    void setHighlightItems(bool show);
    bool getHighlightLockedDoorsState() const { return m_highlightLockedDoors; }
    void setHighlightLockedDoors(bool show);
    bool getShowWallHooksState() const { return m_showWallHooks; }
    void setShowWallHooks(bool show);


    // Selection methods (forward to Map model)
    void clearSelection();
    void selectTile(const QPoint& position);
    void selectTiles(const QRect& rect);
    QList<QPoint> getSelectedTiles() const;

    // Exposed methods for external invocation from menu/toolbar/context menu
    // These now delegate more directly to Map/Editor logic where appropriate.
    void onUndo();
    void onRedo();
    void onCutSelection();
    void onCopySelection();
    void onPasteSelection();
    void onDeleteSelection();

signals:
    void mousePositionChanged(const QPoint& position); // Current tile X,Y under mouse
    void tileSelected(Tile* tile);        // Emitted when a tile is conceptually selected (e.g., for properties)
    void itemSelected(Item* item);        // Emitted when an item is selected
    void creatureSelected(Creature* creature); // Emitted when a creature is selected
    void objectDeselected();              // Emitted when a specific object (item/creature) selection is cleared

    // Signals for actions that involve external modules (like CopyBuffer / Editor)
    void copyRequest(const QRect& selectionRect); // Request to MainWindow to copy map selection
    void cutRequest(const QRect& selectionRect);  // Request to MainWindow to cut map selection
    void pasteRequest(const QPoint& targetPos); // Request to MainWindow to paste clipboard data
    void deleteRequest(const QRect& selectionRect); // Request to MainWindow to delete map selection

protected:
    // QGraphicsView event overrides
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    // Removed scrollContentsBy as QGraphicsView handles it internally

private:
    MapScene* mapScene; // The QGraphicsScene managed by this view
    Map* currentMap; // Pointer to the main map data model (not owned)
    Brush* currentBrush; // The currently active brush (not owned)
    Item* currentItem; // The item currently selected in palette for brushes (not owned)
    Creature* currentCreature; // The creature currently selected in palette (not owned)

    QGraphicsPixmapItem* cursorItem; // Visual brush preview drawn on top of the map
    QPointF lastPanPos; // Stores last mouse position for panning

    double zoom; // Current zoom level
    int currentLayer; // The currently active editing layer

    // Selection state for drag operations (e.g., with SelectionBrush)
    bool isSelecting; // Flag indicating if a selection drag is in progress
    QPoint startSelectPos; // Start tile position of a selection drag
    
    QTimer* updateTimer; // Timer for periodic visual updates (e.g., cursor animations)

    // Mouse state variables
    bool draggingMap;
    bool drawingActive;
    bool selectionActive;      // True when left mouse is down in selection mode (for drag/bounding box)
    bool boundingBoxSelection; // True when Shift+LeftClick starts a selection box
    QPoint lastMousePos;       // For calculating deltas in mouseMove for panning etc.
    QPoint dragStartTile;      // Map tile where drag started (for selection/drawing)
    QPointF dragStartScenePos; // Scene position where drag started
    Qt::MouseButton activeMouseButton; // To track which button initiated an action
    bool mouseInsideView;      // True if mouse cursor is inside the view

    // Helper functions for MapView's internal logic
    void updateVisibleTiles(); // Triggers mapScene to update visible items
    void updateCursor(); // Redraws the brush preview cursor
    void createContextMenu(const QPoint& globalPos); // Creates and displays the right-click context menu

    // Right-Click Context Menu Action Handlers (slots) - directly migrate logic from original map_display.cpp
    // These typically delegate to map manipulation or open related dialogs in MainWindow.
    void onCopySelectionAction();
    void onCutSelectionAction();
    void onDeleteSelectionAction();
    void onPasteSelectionAction();
    void onCopyPositionAction();
    void onCopyServerIdAction();
    void onCopyClientIdAction();
    void onCopyNameAction();
    void onRotateItemAction();
    void onSwitchDoorAction();
    void onGotoDestinationAction();
    void onBrowseTileAction();
    void onFindSimilarItemsAction();
    void onGenerateIslandAction();
    void onCreateHouseAction();
    void onPropertiesAction(); // Generic placeholder for Item/Creature properties depending on context
    void onMapPropertiesAction();
    void onItemPropertiesAction();
    void onCreaturePropertiesAction();
    void onTilePropertiesAction();
    void onFillAction(); // For the Fill context menu option

    // Brush Selection Actions (these switch the active brush via MainWindow)
    void onSelectRawBrushAction();
    void onSelectGroundBrushAction();
    void onSelectDoodadBrushAction();
    void onSelectDoorBrushAction();
    void onSelectWallBrushAction();
    void onSelectCarpetBrushAction();
    void onSelectTableBrushAction();
    void onSelectCreatureBrushAction();
    void onSelectSpawnBrushAction();
    void onSelectHouseBrushAction();
    void onSelectCollectionBrushAction();
    void onSelectMoveToAction(); // Selection Tool equivalent to 'MoveTo' brush

    // Private helper for QMenu action creation, for cleaner contextMenu method
    QAction* createAction(QMenu* menu, const QString& text, QObject* receiver, const char* member, bool enabled = true, QKeySequence shortcut = QKeySequence::Invalid);

    // Friend declarations to simplify interaction, or rely on signals/slots/qobject_cast.
    // Assuming parent is MainWindow and will access its public slots where delegated.
    friend class MainWindow;
};

#endif // MAPVIEW_H