#ifndef ITEMPROPERTYEDITOR_H
#define ITEMPROPERTYEDITOR_H

#include "propertyeditor.h"
#include "item.h"
#include <QSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QGridLayout>

/**
 * @brief Property editor for items
 * 
 * This class provides a UI for editing item properties such as
 * item ID, count, actionID, etc.
 */
class ItemPropertyEditor : public PropertyEditor
{
    Q_OBJECT

public:
    explicit ItemPropertyEditor(QWidget* parent = nullptr);
    ~ItemPropertyEditor();

    /**
     * @brief Set the item to edit
     * @param item Item to edit
     */
    void setItem(Item* item);

    /**
     * @brief Get the edited item
     * @return Edited item
     */
    Item* getItem() const;

    /**
     * @brief Apply changes to the item
     */
    void applyChanges() override;

private slots:
    void onItemIdChanged(int value);
    void onCountChanged(int value);
    void onActionIdChanged(int value);
    void onUniqueIdChanged(int value);
    void onDestPosXChanged(int value);
    void onDestPosYChanged(int value);
    void onDestPosZChanged(int value);
    void onTextChanged(const QString& text);
    void onDescriptionChanged(const QString& text);
    void onApplyClicked();
    void onResetClicked();

private:
    void setupUi();
    void updateUi();

    Item* item;

    // UI elements
    QSpinBox* itemIdSpinBox;
    QSpinBox* countSpinBox;
    QSpinBox* actionIdSpinBox;
    QSpinBox* uniqueIdSpinBox;
    QSpinBox* destPosXSpinBox;
    QSpinBox* destPosYSpinBox;
    QSpinBox* destPosZSpinBox;
    QLineEdit* textEdit;
    QLineEdit* descriptionEdit;
    QPushButton* applyButton;
    QPushButton* resetButton;
};

#endif // ITEMPROPERTYEDITOR_H
