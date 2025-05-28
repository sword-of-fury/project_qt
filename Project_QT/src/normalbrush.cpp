#include "normalbrush.h"
#include "mapview.h" // For view->mapToTile
#include <QDebug>
#include <QPainter>
#include <QIcon>
#include <QMouseEvent> // Required for event->pos() and event->type()

NormalBrush::NormalBrush(QObject* parent)
    : Brush(parent), currentItemToDraw(nullptr)
{
    setType(Brush::Normal);
    setName("Normal Brush");
    // Optionally set a default cursor or icon here if desired
    // setCursor(Qt::CrossCursor); 
}

void NormalBrush::mousePressEvent(QMouseEvent* event, MapView* view)
{
    QPoint viewPos = event->pos();
    QPoint tilePos = view->mapToTile(viewPos); // Assuming mapToTile exists on MapView
    qDebug() << "NormalBrush: Mouse Press at tile" << tilePos << "view" << viewPos;
    // Placeholder for actual drawing logic (deferred to later tasks)
}

void NormalBrush::mouseMoveEvent(QMouseEvent* event, MapView* view)
{
    if (event->buttons() & Qt::LeftButton) { // Only draw if left button is held
        QPoint viewPos = event->pos();
        QPoint tilePos = view->mapToTile(viewPos);
        qDebug() << "NormalBrush: Mouse Move (Left Button Down) at tile" << tilePos << "view" << viewPos;
        // Placeholder for actual drawing logic (deferred to later tasks)
    }
}

void NormalBrush::mouseReleaseEvent(QMouseEvent* event, MapView* view)
{
    QPoint viewPos = event->pos();
    QPoint tilePos = view->mapToTile(viewPos);
    qDebug() << "NormalBrush: Mouse Release at tile" << tilePos << "view" << viewPos;
    // Placeholder for actual drawing logic (deferred to later tasks)
}

void NormalBrush::drawPreview(QPainter& painter, const QPoint& pos, double zoom)
{
    Q_UNUSED(zoom); // Zoom might be used later for scaling preview elements
    painter.setPen(Qt::white);
    painter.setBrush(Qt::NoBrush);
    // Draw a simple 4x4 square centered at pos for preview
    painter.drawRect(pos.x() - 2, pos.y() - 2, 4, 4); 
}

QIcon NormalBrush::getIcon()
{
    // Placeholder: Return a default QIcon or load from resources if available
    // For example, could create a simple pixmap icon:
    // QPixmap pixmap(32, 32);
    // pixmap.fill(Qt::darkCyan);
    // QPainter painter(&pixmap);
    // painter.setPen(Qt::white);
    // painter.drawText(pixmap.rect(), Qt::AlignCenter, "N");
    // return QIcon(pixmap);
    return QIcon(); 
}

void NormalBrush::setCurrentItem(Item* item)
{
    currentItemToDraw = item;
    // Future: Update icon or other properties based on the item
    // For now, just stores it.
}
