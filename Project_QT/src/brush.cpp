#include "brush.h" // Includes the base Brush class header
// No other specific brush headers are needed here, as this is the base class implementation.
// Specific brush types will be implemented in their own .cpp files.

// Constructor for the abstract base Brush class.
Brush::Brush(QObject* parent) :
    QObject(parent),
    type(Type::Normal), // Default type, should be overridden by derived classes.
    name(tr("Generic Brush")),
    size(1), // Default size is 1x1.
    layer(0) // Default layer is 0.
{
    // Default cursor (arrow cursor).
    brushCursor = Qt::ArrowCursor; 
}

// Implement pure virtual functions as empty methods if not abstract base or specific type (for example of casting logic).
// These are not actually used by the abstract `Brush` class, but often concrete implementations would override them.
// If your build environment complains about these, ensure you use this only if you define this as an abstract base (`=0`)

// Mouse event handlers - these are pure virtual functions (`=0`) in `brush.h` and must be
// implemented by concrete brush types. So, no implementation here.
/*
void Brush::mousePressEvent(QMouseEvent* event, MapView* view) { Q_UNUSED(event); Q_UNUSED(view); }
void Brush::mouseMoveEvent(QMouseEvent* event, MapView* view) { Q_UNUSED(event); Q_UNUSED(view); }
void Brush::mouseReleaseEvent(QMouseEvent* event, MapView* view) { Q_UNUSED(event); Q_UNUSED(view); }
*/

// drawPreview is also a pure virtual function and must be implemented by concrete brushes.
/*
void Brush::drawPreview(QPainter& painter, const QPoint& pos, double zoom) { Q_UNUSED(painter); Q_UNUSED(pos); Q_UNUSED(zoom); }
*/

// getIcon is a pure virtual function and must be implemented by concrete brushes.
/*
QIcon Brush::getIcon() { return QIcon(); }
*/


// --- Specific Brush Type Casting ---
// These are convenience methods to allow safe downcasting for brush operations.
// These simply implement the virtual `asX()` methods defined in `brush.h` (similar to Source/src/brush.h).

NormalBrush* Brush::asNormal() { return (type == Type::Normal) ? static_cast<NormalBrush*>(this) : nullptr; }
EraserBrush* Brush::asEraser() { return (type == Type::Eraser) ? static_cast<EraserBrush*>(this) : nullptr; }
FloodFillBrush* Brush::asFloodFill() { return (type == Type::FloodFill) ? static_cast<FloodFillBrush*>(this) : nullptr; }
SelectionBrush* Brush::asSelection() { return (type == Type::Selection) ? static_cast<SelectionBrush*>(this) : nullptr; }
PencilBrush* Brush::asPencil() { return (type == Type::Pencil) ? static_cast<PencilBrush*>(this) : nullptr; }

// Add more `asX()` implementations here as you migrate other brush types, for consistency.
/*
CarpetBrush* Brush::asCarpet() { return (type == Type::Carpet) ? static_cast<CarpetBrush*>(this) : nullptr; }
// ... etc. for other brush types.
*/