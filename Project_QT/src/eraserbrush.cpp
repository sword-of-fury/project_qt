#include "eraserbrush.h"
#include "mapview.h" // For view->mapToTile
#include <QDebug>
#include <QPainter>
#include <QIcon>
#include <QMouseEvent> // Required for event->pos() and event->type()

EraserBrush::EraserBrush(QObject* parent)
    : Brush(parent)
{
    setType(Brush::Eraser);
    setName("Eraser Brush");
    // Optionally set a specific cursor or icon for the eraser
    // setCursor(Qt::PointingHandCursor); 
}

void EraserBrush::mousePressEvent(QMouseEvent* event, MapView* view)
{
    QPoint viewPos = event->pos();
    QPoint tilePos = view->mapToTile(viewPos);
    qDebug() << "EraserBrush: Mouse Press at tile" << tilePos << "view" << viewPos;
    // Placeholder for actual erasing logic (deferred)
}

void EraserBrush::mouseMoveEvent(QMouseEvent* event, MapView* view)
{
    if (event->buttons() & Qt::LeftButton) { // Only erase if left button is held
        QPoint viewPos = event->pos();
        QPoint tilePos = view->mapToTile(viewPos);
        qDebug() << "EraserBrush: Mouse Move (Left Button Down) at tile" << tilePos << "view" << viewPos;
        // Placeholder for actual erasing logic (deferred)
    }
}

void EraserBrush::mouseReleaseEvent(QMouseEvent* event, MapView* view)
{
    QPoint viewPos = event->pos();
    QPoint tilePos = view->mapToTile(viewPos);
    qDebug() << "EraserBrush: Mouse Release at tile" << tilePos << "view" << viewPos;
    // Placeholder for actual erasing logic (deferred)
}

void EraserBrush::drawPreview(QPainter& painter, const QPoint& pos, double zoom)
{
    Q_UNUSED(zoom);
    painter.setPen(Qt::red); // Different color for eraser preview
    painter.setBrush(Qt::NoBrush);
    // Draw a simple X or different shape for eraser
    painter.drawLine(pos.x() - 2, pos.y() - 2, pos.x() + 2, pos.y() + 2);
    painter.drawLine(pos.x() + 2, pos.y() - 2, pos.x() - 2, pos.y() + 2);
}

QIcon EraserBrush::getIcon()
{
    // Placeholder: Return a default QIcon or load from resources
    // QPixmap pixmap(32, 32);
    // pixmap.fill(Qt::lightGray);
    // QPainter painter(&pixmap);
    // painter.setPen(Qt::red);
    // painter.drawLine(8, 8, 24, 24);
    // painter.drawLine(24, 8, 8, 24);
    // return QIcon(pixmap);
    return QIcon();
}
