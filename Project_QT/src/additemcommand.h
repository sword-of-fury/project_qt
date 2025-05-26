#ifndef ADDITEMCOMMAND_H
#define ADDITEMCOMMAND_H

#include "mapcommand.h"
#include "item.h"
#include "map.h"

class AddItemCommand : public MapCommand
{
public:
    AddItemCommand(Map* map, const QPoint& pos, int layer, const Item& item, QUndoCommand* parent = nullptr);

    void undo() override;
    void redo() override;

private:
    Map* map;
    QPoint pos;
    int layer;
    Item item;
};

#endif // ADDITEMCOMMAND_H