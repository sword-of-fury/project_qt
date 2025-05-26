#ifndef CREATUREPROPERTYEDITOR_H
#define CREATUREPROPERTYEDITOR_H

#include "propertyeditor.h"
#include "creature.h"
#include <QSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QGridLayout>

/**
 * @brief Property editor for creatures
 * 
 * This class provides a UI for editing creature properties such as
 * creature ID, name, health, etc.
 */
class CreaturePropertyEditor : public PropertyEditor
{
    Q_OBJECT

public:
    explicit CreaturePropertyEditor(QWidget* parent = nullptr);
    ~CreaturePropertyEditor();

    /**
     * @brief Set the creature to edit
     * @param creature Creature to edit
     */
    void setCreature(Creature* creature);

    /**
     * @brief Get the edited creature
     * @return Edited creature
     */
    Creature* getCreature() const;

    /**
     * @brief Apply changes to the creature
     */
    void applyChanges() override;

private slots:
    void onCreatureIdChanged(int value);
    void onNameChanged(const QString& text);
    void onHealthChanged(int value);
    void onMaxHealthChanged(int value);
    void onDirectionChanged(int index);
    void onSpeedChanged(int value);
    void onIsNpcChanged(bool checked);
    void onSpawnTimeChanged(int value);
    void onApplyClicked();
    void onResetClicked();

private:
    void setupUi();
    void updateUi();

    Creature* creature;

    // UI elements
    QSpinBox* creatureIdSpinBox;
    QLineEdit* nameEdit;
    QSpinBox* healthSpinBox;
    QSpinBox* maxHealthSpinBox;
    QComboBox* directionCombo;
    QSpinBox* speedSpinBox;
    QCheckBox* isNpcCheckBox;
    QSpinBox* spawnTimeSpinBox;
    QPushButton* applyButton;
    QPushButton* resetButton;
};

#endif // CREATUREPROPERTYEDITOR_H
