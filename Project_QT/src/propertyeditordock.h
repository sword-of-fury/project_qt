#ifndef PROPERTYEDITORDOCK_H
#define PROPERTYEDITORDOCK_H

#include <QDockWidget>
#include <QStackedWidget>
#include <QLabel>

class TilePropertyEditor;
class ItemPropertyEditor;
class CreaturePropertyEditor;
class Tile;
class Item;
class Creature;

/**
 * @brief Dock widget for property editors
 * 
 * This class provides a dock widget that contains property editors
 * for tiles, items, and creatures. It automatically switches between
 * editors based on the selected object.
 */
class PropertyEditorDock : public QDockWidget
{
    Q_OBJECT

public:
    explicit PropertyEditorDock(const QString& title, QWidget* parent = nullptr);
    ~PropertyEditorDock();

public slots:
    /**
     * @brief Set the tile to edit
     * @param tile Tile to edit
     */
    void setTile(Tile* tile);

    /**
     * @brief Set the item to edit
     * @param item Item to edit
     */
    void setItem(Item* item);

    /**
     * @brief Set the creature to edit
     * @param creature Creature to edit
     */
    void setCreature(Creature* creature);

    /**
     * @brief Clear all editors
     */
    void clearEditors();

private:
    void setupUi();

    QStackedWidget* stackedWidget;
    QLabel* noSelectionLabel;
    TilePropertyEditor* tileEditor;
    ItemPropertyEditor* itemEditor;
    CreaturePropertyEditor* creatureEditor;
};

#endif // PROPERTYEDITORDOCK_H
