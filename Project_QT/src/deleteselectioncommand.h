#ifndef DELETESELECTIONCOMMAND_H
#define DELETESELECTIONCOMMAND_H

#include "mapcommand.h"
#include "map.h"
#include "tile.h"
#include <QRect>
#include <QList>

class DeleteSelectionCommand : public MapCommand
{
public:
    DeleteSelectionCommand(Map* map, const QRect& selection, QUndoCommand* parent = nullptr);

    void undo() override;
    void redo() override;

private:
    Map* map;
    QRect selection;
    QRect originalSelection; // Store the selection before deletion
    // Store position (x, y), layer (z), and list of items
    QList<QPair<QPoint, QPair<int, QList<Item>>>> removedItems;
};

#endif // DELETESELECTIONCOMMAND_H