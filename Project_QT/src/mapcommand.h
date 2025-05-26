#ifndef MAPCOMMAND_H
#define MAPCOMMAND_H

#include <QUndoCommand>

class MapCommand : public QUndoCommand
{
public:
    explicit MapCommand(const QString& text = "", QUndoCommand* parent = nullptr);
    virtual ~MapCommand();

    // QUndoCommand interface
    void undo() override = 0;
    void redo() override = 0;
};

#endif // MAPCOMMAND_H