#include "pencilbrush.h"
#include "mapview.h"
#include "map.h"
#include "item.h"
#include "additemcommand.h" // For undo/redo support
#include "mainwindow.h" // For access to undo stack
#include "bordersystem.h" // For automagic border updates

#include <QDebug>
#include <QMouseEvent>
#include <QtMath> // For qAbs for Bresenham's algorithm

// Constructor
PencilBrush::PencilBrush(QObject* parent)
    : Brush(parent),
      currentItem(nullptr),
      lastPaintPos(QPoint(-1, -1)) // Initialize with an invalid position
{
    setType(Type::Pencil);
    setName(tr("Pencil"));
    setIcon(QIcon(":/images/pencil.png")); // Assuming a pencil.png exists in resources
}

// Mouse Press Event: Starts a new line stroke.
void PencilBrush::mousePressEvent(QMouseEvent* event, MapView* view)
{
    if (event->button() == Qt::LeftButton) {
        QPoint tilePos = view->mapToTile(event->pos());
        if (!currentItem) {
            qDebug() << "PencilBrush: No item selected. Cannot draw.";
            return;
        }
        drawTile(view, tilePos); // Draw initial tile.
        lastPaintPos = tilePos;  // Store starting point for line drawing.
        event->accept();
    }
}

// Mouse Move Event: Continues the line stroke.
void PencilBrush::mouseMoveEvent(QMouseEvent* event, MapView* view)
{
    if (event->buttons() & Qt::LeftButton) { // If left mouse button is held down
        QPoint tilePos = view->mapToTile(event->pos());
        if (lastPaintPos != tilePos) { // If mouse moved to a new tile
            drawLine(view, lastPaintPos, tilePos); // Draw a line between last and current tile.
            lastPaintPos = tilePos; // Update last painted position for next segment.
        }
        event->accept();
    }
}

// Mouse Release Event: Finalizes the line stroke.
void PencilBrush::mouseReleaseEvent(QMouseEvent* event, MapView* view)
{
    lastPaintPos = QPoint(-1, -1); // Reset to allow immediate new line on next press.
    event->accept();
}

// Private helper: Draws a single tile. (Same as NormalBrush's drawBrush for single tile)
void PencilBrush::drawTile(MapView* view, const QPoint& tilePos)
{
    if (!currentItem || !view->getMap()) return;

    if (tilePos.x() < 0 || tilePos.x() >= view->getMap()->getSize().width() ||
        tilePos.y() < 0 || tilePos.y() >= view->getMap()->getSize().height()) {
        qDebug() << "PencilBrush: Attempted to draw outside map bounds at" << tilePos;
        return;
    }

    Tile* targetTile = view->getMap()->getTile(tilePos.x(), tilePos.y(), layer);
    if (!targetTile) {
        qWarning() << "PencilBrush: Target tile is null at " << tilePos;
        return;
    }
    
    // Add Item to the Map, via Undo Stack
    MainWindow* mainWin = qobject_cast<MainWindow*>(view->parentWidget());
    if (mainWin && mainWin->getUndoStack()) {
        AddItemCommand* command = new AddItemCommand(view->getMap(), tilePos, layer, *currentItem);
        mainWin->getUndoStack()->push(command);
        qDebug() << "PencilBrush: Item" << currentItem->getName() << "drawn at" << tilePos << "via Undo Stack.";
    } else {
        targetTile->addItem(*currentItem);
        view->getMap()->setModified(true);
        view->getMap()->emit tileChanged(tilePos);
        qDebug() << "PencilBrush: Item" << currentItem->getName() << "drawn directly at" << tilePos << ". No Undo support.";
    }

    // Trigger automagic border system if enabled.
    if (view->getMap()->getBorderSystem()->isEnabled()) {
        view->getMap()->getBorderSystem()->applyBorders(tilePos, layer);
    }
}

// Private helper: Draws a line between two tile positions (inclusive).
// Uses a basic Bresenham's-like algorithm.
void PencilBrush::drawLine(MapView* view, const QPoint& p1, const QPoint& p2)
{
    // Implementation based on drawing pixels for each step along the line.
    int x0 = p1.x();
    int y0 = p1.y();
    int x1 = p2.x();
    int y1 = p2.y();

    bool steep = qAbs(y1 - y0) > qAbs(x1 - x0);

    if (steep) {
        qSwap(x0, y0);
        qSwap(x1, y1);
    }
    if (x0 > x1) {
        qSwap(x0, x1);
        qSwap(y0, y1);
    }

    int dx = x1 - x0;
    int dy = qAbs(y1 - y0);
    int error = dx / 2;
    int ystep = (y0 < y1) ? 1 : -1;
    int y = y0;

    for (int x = x0; x <= x1; ++x) {
        if (steep) {
            drawTile(view, QPoint(y, x));
        } else {
            drawTile(view, QPoint(x, y));
        }
        error -= dy;
        if (error < 0) {
            y += ystep;
            error += dx;
        }
    }
}


// Draw preview (single tile of the item)
void PencilBrush::drawPreview(QPainter& painter, const QPoint& pos, double zoom)
{
    Q_UNUSED(pos);
    
    painter.setOpacity(0.6); // Semi-transparent for preview.
    if (currentItem) {
        // Call the Item's draw method.
        currentItem->draw(painter, QPointF(0, 0), 1.0); // Draw item into 0,0 of temp pixmap.
    } else {
        // Draw a generic square with an indication that it's a pencil.
        painter.setPen(QPen(Qt::cyan, 2));
        painter.setBrush(QColor(0, 255, 255, 60)); // Transparent cyan fill.
        painter.drawRect(0, 0, MapTileItem::TilePixelSize - 1, MapTileItem::TilePixelSize - 1);
        painter.drawLine(0, 0, MapTileItem::TilePixelSize - 1, MapTileItem::TilePixelSize - 1); // Diagonal line
    }
    painter.setOpacity(1.0); // Reset opacity.
}

// Icon for the pencil brush.
QIcon PencilBrush::getIcon()
{
    if (icon.isNull()) {
        // Need a pencil.png in your resources.qrc
        icon = QIcon(":/images/pencil.png"); // Example, make sure it's linked
    }
    return icon;
}