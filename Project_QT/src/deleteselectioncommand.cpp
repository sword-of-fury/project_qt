#include "deleteselectioncommand.h"
#include "map.h"
#include "tile.h"

DeleteSelectionCommand::DeleteSelectionCommand(Map* map, const QRect& selection, QUndoCommand* parent)
    : MapCommand(parent)
    , map(map)
    , selection(selection)
    , originalSelection(map ? map->getSelection() : QRect()) // Store the current selection
{
    setText(QObject::tr("Delete Selection"));

    // Store removed items for undo
    for (int x = selection.left(); x <= selection.right(); ++x) {
        for (int y = selection.top(); y <= selection.bottom(); ++y) {
            for (int z = 0; z < Map::LayerCount; ++z) {
                if (x >= 0 && x < map->getWidth() &&
                    y >= 0 && y < map->getHeight() &&
                    z >= 0 && z < Map::LayerCount) // Ensure z is within bounds
                {
                    Tile* tile = map->getTile(x, y, z);
                    if (tile && !tile->getItems().isEmpty()) {
                        removedItems.append({QPoint(x, y), {z, tile->getItems()}});
                    }
                }
            }
        }
    }
}

void DeleteSelectionCommand::undo() {
    if (!map) {
        return;
    }

    // Restore removed items
    for (const auto& pair : removedItems) {
        QPoint pos = pair.first;
        int layer = pair.second.first;
        const QList<Item>& items = pair.second.second;
        for (const Item& item : items) {
            map->addItem(pos.x(), pos.y(), layer, item);
        }
    }
    if (map) {
        map->setSelection(originalSelection); // Restore the original selection
        map->updateViews();
    }
}

void DeleteSelectionCommand::redo() {
    if (!map) {
        return;
    }

    // Clear items within the selection
    for (int x = selection.left(); x <= selection.right(); ++x) {
        for (int y = selection.top(); y <= selection.bottom(); ++y) {
            for (int z = 0; z < Map::LayerCount; ++z) {
                if (x >= 0 && x < map->getWidth() &&
                    y >= 0 && y < map->getHeight() &&
                    z >= 0 && z < Map::LayerCount) // Ensure z is within bounds
                {
                    map->clearItems(x, y, z);
                }
            }
        }
    }
    // Note: Selection itself is cleared in SelectionBrush::deleteSelection after pushing the command
    map->updateViews();
}