#ifndef SELECTIONBRUSH_H
#define SELECTIONBRUSH_H

#include "brush.h" // Base class
#include "tile.h"  // For Tile and item interaction
#include <QPoint>  // For QPoint (x,y coordinates)
#include <QRect>   // For QRect (selection area)
#include <QObject>
#include <QJsonDocument> // For clipboard data serialization (as in Source/selection.cpp)
#include <QJsonObject>   // For JSON objects
#include <QSet>          // For tracking fragmented selections (from Source/selection.cpp)

// Forward declarations
class MapView;
class Map;
class Editor; // The original selection used editor directly (Source/selection.cpp)

// Helper struct for Clipboard data, directly mirroring Source/selection.cpp
// This is not part of this brush, but ensures methods signature from source match.
struct ClipboardData {
    QJsonObject data; // Use QJsonObject for data
    QRect boundingBox; // Bounding box of copied selection (for relative pasting)
    bool hasData;

    ClipboardData() : hasData(false) {}
    // copy, paste, etc operations would go here.
    static ClipboardData& getInstance() { // Simplified singleton access.
        static ClipboardData instance;
        return instance;
    }
    bool hasClipboardData() const { return hasData && !boundingBox.isEmpty(); }
    void copyData(const QJsonObject& obj, const QRect& bounds) {
        data = obj; boundingBox = bounds; hasData = true;
    }
    const QJsonObject& getCopyData() const { return data; }
    const QRect& getBoundingBox() const { return boundingBox; }
    void clear() { data = QJsonObject(); boundingBox = QRect(); hasData = false; }
};

/**
 * @brief The SelectionBrush class provides tools for selecting areas on the map.
 * It supports different selection modes (replace, add, subtract, toggle) and basic transformations.
 * This directly translates Source/src/selection.h/cpp, which often works in tandem with `Editor`.
 */
class SelectionBrush : public Brush
{
    Q_OBJECT
public:
    // Selection modes as per Source/src/selection.h
    enum SelectionMode {
        Replace = 0,    // Replaces current selection
        Add,            // Adds to current selection
        Subtract,       // Subtracts from current selection
        Toggle          // Toggles selection state of clicked tiles
    };

    explicit SelectionBrush(QObject* parent = nullptr);
    virtual ~SelectionBrush() = default;

    // Override virtual methods from Brush base class for mouse and keyboard events.
    void mousePressEvent(QMouseEvent* event, MapView* view) override;
    void mouseMoveEvent(QMouseEvent* event, MapView* view) override;
    void mouseReleaseEvent(QMouseEvent* event, MapView* view) override;
    void keyPressEvent(QKeyEvent* event, MapView* view) override; // For transformations/selection
    void keyReleaseEvent(QKeyEvent* event, MapView* view) override; // To clear modifier states
    
    // Draw preview of selection box or visual feedback (handled by MapScene directly now via MapSelectionItem).
    void drawPreview(QPainter& painter, const QPoint& pos, double zoom) override;

    // Brush type and name identification.
    Type getType() const override { return Type::Selection; }
    QString getName() const override { return tr("Selection"); }
    QIcon getIcon() override; // Provides icon for toolbar/palette.

    // Selection operations that this brush triggers on the Map.
    // These typically delegate to `Map`'s selection methods (`setSelection`, `addToSelection` etc.)
    // and also to `Editor` (or `MainWindow` that controls `Editor`) for actual operations like `moveSelection`.
    QRect getSelection() const { return currentSelection; } // Current bounding box of actual selected tiles.
    void clearSelection(MapView* view);
    void moveSelection(MapView* view, const QPoint& offset, bool copy = false);
    void rotateSelection(MapView* view, int degrees); // Rotate 90 degree increments.
    void flipSelectionHorizontally(MapView* view);
    void flipSelectionVertically(MapView* view);
    void copySelection(MapView* view);   // Copies content to internal clipboard (ClipboardData)
    void pasteSelection(MapView* view);  // Pastes content from internal clipboard
    void deleteSelection(MapView* view); // Deletes current selection (from Map)

    // Selection mode (managed by external UI, set via this setter)
    void setSelectionMode(SelectionMode mode);
    SelectionMode getSelectionMode() const { return selectionMode; }

signals:
    void selectionCleared(); // Emitted when selection is cleared (for UI update)

private:
    QRect currentSelection; // The bounding rectangle of the tiles currently *dragged* for selection (for display).
    // The actual set of selected positions (QSet<QPoint>) is managed by the Map class (`Map::selectedPositions`).
    QPoint startDragPos; // The mouse position (tile coordinates) when selection drag started.
    bool isDragging;        // Whether a mouse drag for selection is currently active.
    SelectionMode selectionMode; // Current selection mode.
    
    // Modifier key states, to alter selection behavior (e.g. Add/Subtract modes).
    bool shiftPressed;
    bool ctrlPressed;
    bool altPressed;

    // ClipboardData (`QJsonObject`) from Source/selection.cpp (`clipboard_data`).
    // No direct member needed as it's a singleton helper `ClipboardData::getInstance()`.
    // QJsonObject clipboardData; // Moved to static ClipboardData helper

    // Helper functions for actual selection modification or transformation on map data.
    // These might directly interact with `Map` singleton or call `Editor` actions.
    void applySelection(MapView* view, const QRect& newRect, bool isFinalRelease = false); // Core selection application.
    void toggleSingleTile(MapView* view, const QPoint& tilePos); // For Toggle mode on click.

    // Key handlers that apply transformations to `Map` (via `Map`'s transformation methods)
    void handleTransformKey(MapView* view, Qt::Key key, Qt::KeyboardModifiers modifiers);

    // Private helper for drawing complex selections in preview
    void drawSelectionRects(QPainter& painter, MapView* view, QPoint previewOffset);

    // Temp debug: For clipboard data format validation (similar to Source/selection.cpp).
    // void parseClipboardData(); // Not relevant here directly.
    // QJsonObject generateClipboardData(); // Not relevant here directly.
};

#endif // SELECTIONBRUSH_H