#ifndef CLEARITEMSCOMMAND_H
#define CLEARITEMSCOMMAND_H

#include "mapcommand.h"
#include "map.h"
#include "tile.h"
#include "item.h"
#include <QPoint>
#include <QVector>

class ClearItemsCommand : public MapCommand
{
public:
    ClearItemsCommand(Map* map, const QPoint& pos, int layer, QUndoCommand* parent = nullptr);

    void undo() override;
    void redo() override;

private:
    Map* map;
    QPoint pos;
    int layer;
    QVector<Item> removedItems;
};

#endif // CLEARITEMSCOMMAND_H