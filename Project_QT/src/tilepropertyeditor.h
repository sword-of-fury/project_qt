#ifndef TILEPROPERTYEDITOR_H
#define TILEPROPERTYEDITOR_H

#include "propertyeditor.h"
#include "tile.h"
#include <QSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QGridLayout>

/**
 * @brief Property editor for tiles
 * 
 * This class provides a UI for editing tile properties such as
 * collision flags, movement cost, etc.
 */
class TilePropertyEditor : public PropertyEditor
{
    Q_OBJECT

public:
    explicit TilePropertyEditor(QWidget* parent = nullptr);
    ~TilePropertyEditor();

    /**
     * @brief Set the tile to edit
     * @param tile Tile to edit
     */
    void setTile(Tile* tile);

    /**
     * @brief Get the edited tile
     * @return Edited tile
     */
    Tile* getTile() const;

    /**
     * @brief Apply changes to the tile
     */
    void applyChanges() override;

private slots:
    void onCollisionChanged(bool checked);
    void onMovementCostChanged(int value);
    void onProtectionZoneChanged(bool checked);
    void onNoLogoutChanged(bool checked);
    void onNoMoveItemsChanged(bool checked);
    void onNoMoveCreaturesChanged(bool checked);
    void onNoSpawnChanged(bool checked);
    void onApplyClicked();
    void onResetClicked();

private:
    void setupUi();
    void updateUi();

    Tile* tile;

    // UI elements
    QCheckBox* collisionCheckBox;
    QSpinBox* movementCostSpinBox;
    QCheckBox* protectionZoneCheckBox;
    QCheckBox* noLogoutCheckBox;
    QCheckBox* noMoveItemsCheckBox;
    QCheckBox* noMoveCreaturesCheckBox;
    QCheckBox* noSpawnCheckBox;
    QPushButton* applyButton;
    QPushButton* resetButton;
};

#endif // TILEPROPERTYEDITOR_H
