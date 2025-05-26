#include "floodfillbrush.h"
#include "mapview.h"
#include "map.h"
#include "tile.h"
#include "item.h"
#include <QPainter>
#include <QMouseEvent>
#include <QDebug>
#include <QStack>
#include <QSet>

FloodFillBrush::FloodFillBrush(QObject *parent) : Brush(parent)
    , currentItem(nullptr)
    , currentLayer(Tile::Layer::Ground)
{
    setCursor(Qt::CrossCursor);
}

FloodFillBrush::~FloodFillBrush()
{
}

void FloodFillBrush::mousePressEvent(QMouseEvent* event, MapView* view)
{
    if (!view || !view->getMap() || !view->getCurrentItem()) return;

    if (event->button() == Qt::LeftButton) {
        QPoint startPos = view->mapToTile(event->pos());
        floodFill(view, startPos, *view->getCurrentItem(), view->getCurrentLayer());
    }
}

void FloodFillBrush::mouseMoveEvent(QMouseEvent* event, MapView* view)
{
    // Nie potrzebujemy implementacji dla wypełniania
}

void FloodFillBrush::mouseReleaseEvent(QMouseEvent* event, MapView* view)
{
    // Nie potrzebujemy implementacji dla wypełniania
}

void FloodFillBrush::draw(QPainter* painter, MapView* view)
{
    // Podgląd pędzla (np. obramowanie kafelka pod kursorem)
    if (!view) return;
    QPoint mouse = view->mapFromGlobal(QCursor::pos());
    QPoint tile = view->mapToTile(mouse);
    QRect rect(tile.x() * view->tileSize, tile.y() * view->tileSize, view->tileSize, view->tileSize);
    painter->setPen(QPen(Qt::green, 2, Qt::DashLine));
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(rect);
}

void FloodFillBrush::floodFill(MapView* view, const QPoint& startPos, const Item& fillItem, int fillLayer)
{
    Map* map = view->getMap();
    if (!map) return;

    Tile* startTile = map->getTile(startPos.x(), startPos.y(), fillLayer);
    if (!startTile) {
        // Jeśli kafelek startowy nie istnieje, a wypełniamy pustką, tworzymy go
        if (fillItem.getId() == 0) {
             map->addItem(startPos.x(), startPos.y(), fillLayer, fillItem);
             view->update();
        }
        return;
    }

    // Pobierz ID przedmiotu na pozycji startowej (lub 0 jeśli kafelek pusty na tej warstwie)
    int targetItemId = 0;
    if (!startTile->getItems().isEmpty()) {
         // Get the item on the current fill layer
         for (const Item& item : startTile->getItems()) {
             // Check if the item is on the correct layer for flood fill
             if (item.getLayer() == fillLayer) {
                 targetItemId = item.getId();
                 break; // Take the first item on the correct layer
             }
         }
    }

     // Jeśli przedmiot do wypełnienia jest taki sam jak przedmiot docelowy, nic nie robimy
     if (fillItem.getId() == targetItemId) {
         return;
     }

    QStack<QPoint> stack;
    stack.push(startPos);
    QSet<QPoint> visited;
    visited.insert(startPos);

    while (!stack.isEmpty()) {
        QPoint currentPos = stack.pop();

        // Sprawdź, czy kafelek na aktualnej pozycji ma ten sam przedmiot docelowy
        Tile* currentTile = map->getTile(currentPos.x(), currentPos.y(), fillLayer);
        bool shouldFill = false;

        if (currentTile) {
            int currentItemId = 0;
            if (!currentTile->getItems().isEmpty()) {
                 for (const Item& item : currentTile->getItems()) {
                    // Check if the item is on the correct layer for flood fill
                    if (item.getLayer() == fillLayer) {
                        currentItemId = item.getId();
                        break;
                    }
                 }
            }
             shouldFill = (currentItemId == targetItemId);
        } else {
            // Jeśli kafelek nie istnieje, a targetItemId to 0 (pustka), to możemy wypełnić
            shouldFill = (targetItemId == 0);
        }

        if (shouldFill) {
             // Usuń istniejące przedmioty na tej warstwie
            if (currentTile) {
                map->clearItems(currentPos.x(), currentPos.y(), fillLayer);
            } else if (fillItem.getId() != 0) {
                 // Jeśli kafelek nie istniał, a dodajemy przedmiot, stwórz kafelek
                 map->addItem(currentPos.x(), currentPos.y(), fillLayer, fillItem);
            }

             // Dodaj nowy przedmiot, jeśli fillItem nie jest pustką
            if (fillItem.getId() != 0 && currentTile) {
                 map->addItem(currentPos.x(), currentPos.y(), fillLayer, fillItem);
            } else if (fillItem.getId() != 0 && !currentTile) {
                 // Już dodaliśmy wyżej
            }

            // Dodaj sąsiadów do stosu, jeśli są w granicach mapy i nieodwiedzeni
            QPoint neighbors[] = {
                QPoint(currentPos.x() + 1, currentPos.y()),
                QPoint(currentPos.x() - 1, currentPos.y()),
                QPoint(currentPos.x(), currentPos.y() + 1),
                QPoint(currentPos.x(), currentPos.y() - 1)
            };

            for (const QPoint& neighbor : neighbors) {
                if (neighbor.x() >= 0 && neighbor.x() < map->getWidth() &&
                    neighbor.y() >= 0 && neighbor.y() < map->getHeight() &&
                    !visited.contains(neighbor))
                {
                    stack.push(neighbor);
                    visited.insert(neighbor);
                }
            }
        }
    }
    view->update();
}

void FloodFillBrush::drawPreview(QPainter& painter, const QPoint& pos, double zoom)
{
    if (!currentItem) return;

    // Rysowanie podglądu elementu
    currentItem->draw(painter, pos, zoom);

    // Rysowanie obramowania
    QRect rect(pos.x(), pos.y(), 32 * zoom, 32 * zoom);
    painter.setPen(QPen(Qt::green, 2));
    painter.setBrush(QColor(0, 255, 0, 50));
    painter.drawRect(rect);
}

void FloodFillBrush::start(const QPoint& pos)
{
    if (!map) {
        qDebug() << "Brak mapy dla pędzla";
        return;
    }

    startPos = pos;
    isActive = true;
    fill(pos);
}

void FloodFillBrush::move(const QPoint& pos)
{
    // Wypełnianie nie reaguje na ruch myszy
}

void FloodFillBrush::end(const QPoint& pos)
{
    if (!isActive) {
        return;
    }

    isActive = false;
}

void FloodFillBrush::fill(const QPoint& pos)
{
    if (!map) {
        return;
    }

    // Pobierz płytkę na pozycji startowej
    Tile* startTile = map->getTile(pos.x(), pos.y(), currentLayer);
    if (!startTile) {
        return;
    }

    // Pobierz przedmiot do wypełnienia
    Item targetItem;
    if (!startTile->getItems(currentLayer).isEmpty()) {
        targetItem = startTile->getItems(currentLayer).first();
    }

    // Wypełnij obszar
    QSet<QPoint> visited;
    fillRecursive(pos.x(), pos.y(), currentLayer, targetItem, visited);
}

void FloodFillBrush::fillRecursive(int x, int y, int z, const Item& targetItem, QSet<QPoint>& visited)
{
    // Sprawdź granice mapy
    if (x < 0 || x >= map->getWidth() || y < 0 || y >= map->getHeight()) {
        return;
    }

    // Sprawdź czy pozycja była już odwiedzona
    QPoint pos(x, y);
    if (visited.contains(pos)) {
        return;
    }
    visited.insert(pos);

    // Pobierz płytkę na aktualnej pozycji
    Tile* tile = map->getTile(x, y, z);
    if (!tile) {
        return;
    }

    // Sprawdź czy płytka zawiera ten sam przedmiot co płytka startowa
    bool shouldFill = false;
    if (tile->getItems(z).isEmpty()) {
        shouldFill = targetItem.getId() == 0;
    } else {
        shouldFill = tile->getItems(z).first().getId() == targetItem.getId();
    }

    if (shouldFill) {
        // Usuń istniejące przedmioty
        for (const Item& item : tile->getItems(z)) {
            map->removeItem(x, y, z, item);
        }

        // Dodaj nowy przedmiot
        if (targetItem.getId() != 0) {
            map->addItem(x, y, z, targetItem);
        }

        // Rekurencyjnie wypełnij sąsiednie płytki
        fillRecursive(x + 1, y, z, targetItem, visited);
        fillRecursive(x - 1, y, z, targetItem, visited);
        fillRecursive(x, y + 1, z, targetItem, visited);
        fillRecursive(x, y - 1, z, targetItem, visited);
    }
}

bool FloodFillBrush::shouldFill(const QPoint& pos) const
{
    Map* map = Map::getInstance();
    if (!map) return false;

    Tile* tile = map->getTile(pos);
    if (!tile) return false;

    // Sprawdź, czy kafelek jest pusty w danej warstwie
    return tile->getItems(currentLayer).isEmpty();
} 