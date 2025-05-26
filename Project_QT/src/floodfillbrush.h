#ifndef FLOODFILLBRUSH_H
#define FLOODFILLBRUSH_H

#include "brush.h"
#include "item.h"
#include <QPoint>
#include <QStack>
#include <QSet>

class FloodFillBrush : public Brush
{
    Q_OBJECT

public:
    explicit FloodFillBrush(QObject *parent = nullptr);
    ~FloodFillBrush();

    // Implementacja metod z klasy Brush
    void onMousePress(QMouseEvent* event, MapView* view) override;
    void onMouseMove(QMouseEvent* event, MapView* view) override;
    void onMouseRelease(QMouseEvent* event, MapView* view) override;
    void drawPreview(QPainter& painter, const QPoint& pos, double zoom) override;

    // Ustawianie aktualnego elementu
    void setCurrentItem(Item* item) { currentItem = item; }
    Item* getCurrentItem() const { return currentItem; }

    // Ustawianie aktualnej warstwy
    void setCurrentLayer(Tile::Layer layer) { currentLayer = layer; }
    Tile::Layer getCurrentLayer() const { return currentLayer; }

    Type getType() const override { return Type::FloodFill; }
    QString getName() const override { return tr("Wypełnianie"); }
    QIcon getIcon() const override;

    void start(const QPoint& pos) override;
    void move(const QPoint& pos) override;
    void end(const QPoint& pos) override;

private:
    Item* currentItem;
    Tile::Layer currentLayer;
    void floodFill(const QPoint& startPos);
    bool shouldFill(const QPoint& pos) const;
    void fill(const QPoint& pos);
    void fillRecursive(int x, int y, int z, const Item& targetItem, QSet<QPoint>& visited);
};

#endif // FLOODFILLBRUSH_H 