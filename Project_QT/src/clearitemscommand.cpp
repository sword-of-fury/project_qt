#include "clearitemscommand.h"

ClearItemsCommand::ClearItemsCommand(Map* map, const QPoint& pos, int layer, QUndoCommand* parent)
    : MapCommand(parent)
    , map(map)
    , pos(pos)
    , layer(layer)
{
    // Store the items before clearing them
    if (map) {
        Tile* tile = map->getTile(pos.x(), pos.y(), layer);
        if (tile) {
            removedItems = tile->getItems();
        }
    }
    setText(QString("Clear Items at (%1, %2, %3)").arg(pos.x()).arg(pos.y()).arg(layer));
}

void ClearItemsCommand::undo()
{
    if (map) {
        // Add the removed items back to the tile
        for (const Item& item : removedItems) {
            map->addItem(pos.x(), pos.y(), layer, item);
        }
    }
}

void ClearItemsCommand::redo()
{
    if (map) {
        // Clear the items from the tile again
        map->clearItems(pos.x(), pos.y(), layer);
    }
}