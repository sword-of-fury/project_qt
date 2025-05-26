#include "additemcommand.h"

AddItemCommand::AddItemCommand(Map* map, const QPoint& pos, int layer, const Item& item, QUndoCommand* parent)
    : MapCommand(parent)
    , map(map)
    , pos(pos)
    , layer(layer)
    , item(item)
{
    setText(QString("Add Item %1 at (%2, %3, %4)").arg(item.getId()).arg(pos.x()).arg(pos.y()).arg(layer));
}

void AddItemCommand::undo()
{
    if (map) {
        map->removeItem(pos.x(), pos.y(), layer, item);
    }
}

void AddItemCommand::redo()
{
    if (map) {
        map->addItem(pos.x(), pos.y(), layer, item);
    }
}