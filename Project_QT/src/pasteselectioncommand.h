#ifndef PASTESELECTIONCOMMAND_H
#define PASTESELECTIONCOMMAND_H

#include "mapcommand.h"
#include <QPoint>
#include <QJsonObject>
#include <QList>
#include "item.h"

class Map;
class Tile;

class PasteSelectionCommand : public MapCommand
{
public:
    PasteSelectionCommand(Map* map, const QPoint& pastePos, const QJsonObject& clipboardData, QUndoCommand* parent = nullptr);

    void undo() override;
    void redo() override;

private:
    void processTiles(bool pasting); // Helper function to handle tile iteration and processing

    Map* map;
    QPoint pastePos;
    QJsonObject clipboardData;
    QList<QPair<QPoint, QPair<int, QList<Item>>>> replacedItems;
};

#endif // PASTESELECTIONCOMMAND_H