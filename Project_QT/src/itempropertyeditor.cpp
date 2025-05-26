#include "itempropertyeditor.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QMessageBox>

ItemPropertyEditor::ItemPropertyEditor(QWidget* parent)
    : PropertyEditor(parent)
    , item(nullptr)
{
    setupUi();
}

ItemPropertyEditor::~ItemPropertyEditor()
{
}

void ItemPropertyEditor::setupUi()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // Create group box for basic properties
    QGroupBox* basicGroup = new QGroupBox(tr("Basic Properties"), this);
    QGridLayout* basicLayout = new QGridLayout(basicGroup);
    
    // Item ID
    QLabel* itemIdLabel = new QLabel(tr("Item ID:"), basicGroup);
    itemIdSpinBox = new QSpinBox(basicGroup);
    itemIdSpinBox->setRange(1, 999999);
    itemIdSpinBox->setToolTip(tr("The ID of the item"));
    connect(itemIdSpinBox, qOverload<int>(&QSpinBox::valueChanged), 
            this, &ItemPropertyEditor::onItemIdChanged);
    basicLayout->addWidget(itemIdLabel, 0, 0);
    basicLayout->addWidget(itemIdSpinBox, 0, 1);
    
    // Count
    QLabel* countLabel = new QLabel(tr("Count:"), basicGroup);
    countSpinBox = new QSpinBox(basicGroup);
    countSpinBox->setRange(1, 100);
    countSpinBox->setToolTip(tr("The count/amount of the item (for stackable items)"));
    connect(countSpinBox, qOverload<int>(&QSpinBox::valueChanged), 
            this, &ItemPropertyEditor::onCountChanged);
    basicLayout->addWidget(countLabel, 1, 0);
    basicLayout->addWidget(countSpinBox, 1, 1);
    
    // Action ID
    QLabel* actionIdLabel = new QLabel(tr("Action ID:"), basicGroup);
    actionIdSpinBox = new QSpinBox(basicGroup);
    actionIdSpinBox->setRange(0, 65535);
    actionIdSpinBox->setToolTip(tr("The action ID of the item (for scripting)"));
    connect(actionIdSpinBox, qOverload<int>(&QSpinBox::valueChanged), 
            this, &ItemPropertyEditor::onActionIdChanged);
    basicLayout->addWidget(actionIdLabel, 2, 0);
    basicLayout->addWidget(actionIdSpinBox, 2, 1);
    
    // Unique ID
    QLabel* uniqueIdLabel = new QLabel(tr("Unique ID:"), basicGroup);
    uniqueIdSpinBox = new QSpinBox(basicGroup);
    uniqueIdSpinBox->setRange(0, 65535);
    uniqueIdSpinBox->setToolTip(tr("The unique ID of the item (for scripting)"));
    connect(uniqueIdSpinBox, qOverload<int>(&QSpinBox::valueChanged), 
            this, &ItemPropertyEditor::onUniqueIdChanged);
    basicLayout->addWidget(uniqueIdLabel, 3, 0);
    basicLayout->addWidget(uniqueIdSpinBox, 3, 1);
    
    mainLayout->addWidget(basicGroup);
    
    // Create group box for teleport properties
    QGroupBox* teleportGroup = new QGroupBox(tr("Teleport Properties"), this);
    QGridLayout* teleportLayout = new QGridLayout(teleportGroup);
    
    // Destination X
    QLabel* destPosXLabel = new QLabel(tr("Dest X:"), teleportGroup);
    destPosXSpinBox = new QSpinBox(teleportGroup);
    destPosXSpinBox->setRange(0, 65535);
    destPosXSpinBox->setToolTip(tr("Destination X coordinate (for teleport items)"));
    connect(destPosXSpinBox, qOverload<int>(&QSpinBox::valueChanged), 
            this, &ItemPropertyEditor::onDestPosXChanged);
    teleportLayout->addWidget(destPosXLabel, 0, 0);
    teleportLayout->addWidget(destPosXSpinBox, 0, 1);
    
    // Destination Y
    QLabel* destPosYLabel = new QLabel(tr("Dest Y:"), teleportGroup);
    destPosYSpinBox = new QSpinBox(teleportGroup);
    destPosYSpinBox->setRange(0, 65535);
    destPosYSpinBox->setToolTip(tr("Destination Y coordinate (for teleport items)"));
    connect(destPosYSpinBox, qOverload<int>(&QSpinBox::valueChanged), 
            this, &ItemPropertyEditor::onDestPosYChanged);
    teleportLayout->addWidget(destPosYLabel, 1, 0);
    teleportLayout->addWidget(destPosYSpinBox, 1, 1);
    
    // Destination Z
    QLabel* destPosZLabel = new QLabel(tr("Dest Z:"), teleportGroup);
    destPosZSpinBox = new QSpinBox(teleportGroup);
    destPosZSpinBox->setRange(0, 15);
    destPosZSpinBox->setToolTip(tr("Destination Z coordinate (for teleport items)"));
    connect(destPosZSpinBox, qOverload<int>(&QSpinBox::valueChanged), 
            this, &ItemPropertyEditor::onDestPosZChanged);
    teleportLayout->addWidget(destPosZLabel, 2, 0);
    teleportLayout->addWidget(destPosZSpinBox, 2, 1);
    
    mainLayout->addWidget(teleportGroup);
    
    // Create group box for text properties
    QGroupBox* textGroup = new QGroupBox(tr("Text Properties"), this);
    QGridLayout* textLayout = new QGridLayout(textGroup);
    
    // Text
    QLabel* textLabel = new QLabel(tr("Text:"), textGroup);
    textEdit = new QLineEdit(textGroup);
    textEdit->setToolTip(tr("Text content (for writable items)"));
    connect(textEdit, &QLineEdit::textChanged, this, &ItemPropertyEditor::onTextChanged);
    textLayout->addWidget(textLabel, 0, 0);
    textLayout->addWidget(textEdit, 0, 1);
    
    // Description
    QLabel* descriptionLabel = new QLabel(tr("Description:"), textGroup);
    descriptionEdit = new QLineEdit(textGroup);
    descriptionEdit->setToolTip(tr("Item description"));
    connect(descriptionEdit, &QLineEdit::textChanged, this, &ItemPropertyEditor::onDescriptionChanged);
    textLayout->addWidget(descriptionLabel, 1, 0);
    textLayout->addWidget(descriptionEdit, 1, 1);
    
    mainLayout->addWidget(textGroup);
    
    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    
    applyButton = new QPushButton(tr("Apply"), this);
    connect(applyButton, &QPushButton::clicked, this, &ItemPropertyEditor::onApplyClicked);
    buttonLayout->addWidget(applyButton);
    
    resetButton = new QPushButton(tr("Reset"), this);
    connect(resetButton, &QPushButton::clicked, this, &ItemPropertyEditor::onResetClicked);
    buttonLayout->addWidget(resetButton);
    
    mainLayout->addLayout(buttonLayout);
    
    // Disable all controls initially
    setEnabled(false);
}

void ItemPropertyEditor::setItem(Item* item)
{
    this->item = item;
    
    if (item) {
        // Load item properties
        setProperty("itemId", item->getId());
        setProperty("count", item->getCount());
        setProperty("actionId", item->getActionId());
        setProperty("uniqueId", item->getUniqueId());
        setProperty("destPosX", item->getDestPosition().x);
        setProperty("destPosY", item->getDestPosition().y);
        setProperty("destPosZ", item->getDestPosition().z);
        setProperty("text", item->getText());
        setProperty("description", item->getDescription());
        
        updateUi();
        setEnabled(true);
    } else {
        clearProperties();
        setEnabled(false);
    }
}

Item* ItemPropertyEditor::getItem() const
{
    return item;
}

void ItemPropertyEditor::updateUi()
{
    if (!item) return;
    
    // Update UI elements with current property values
    itemIdSpinBox->setValue(getProperty("itemId").toInt());
    countSpinBox->setValue(getProperty("count").toInt());
    actionIdSpinBox->setValue(getProperty("actionId").toInt());
    uniqueIdSpinBox->setValue(getProperty("uniqueId").toInt());
    destPosXSpinBox->setValue(getProperty("destPosX").toInt());
    destPosYSpinBox->setValue(getProperty("destPosY").toInt());
    destPosZSpinBox->setValue(getProperty("destPosZ").toInt());
    textEdit->setText(getProperty("text").toString());
    descriptionEdit->setText(getProperty("description").toString());
}

void ItemPropertyEditor::applyChanges()
{
    if (!item) return;
    
    // Apply property changes to the item
    item->setId(getProperty("itemId").toInt());
    item->setCount(getProperty("count").toInt());
    item->setActionId(getProperty("actionId").toInt());
    item->setUniqueId(getProperty("uniqueId").toInt());
    
    Position destPos;
    destPos.x = getProperty("destPosX").toInt();
    destPos.y = getProperty("destPosY").toInt();
    destPos.z = getProperty("destPosZ").toInt();
    item->setDestPosition(destPos);
    
    item->setText(getProperty("text").toString());
    item->setDescription(getProperty("description").toString());
}

void ItemPropertyEditor::onItemIdChanged(int value)
{
    setProperty("itemId", value);
}

void ItemPropertyEditor::onCountChanged(int value)
{
    setProperty("count", value);
}

void ItemPropertyEditor::onActionIdChanged(int value)
{
    setProperty("actionId", value);
}

void ItemPropertyEditor::onUniqueIdChanged(int value)
{
    setProperty("uniqueId", value);
}

void ItemPropertyEditor::onDestPosXChanged(int value)
{
    setProperty("destPosX", value);
}

void ItemPropertyEditor::onDestPosYChanged(int value)
{
    setProperty("destPosY", value);
}

void ItemPropertyEditor::onDestPosZChanged(int value)
{
    setProperty("destPosZ", value);
}

void ItemPropertyEditor::onTextChanged(const QString& text)
{
    setProperty("text", text);
}

void ItemPropertyEditor::onDescriptionChanged(const QString& text)
{
    setProperty("description", text);
}

void ItemPropertyEditor::onApplyClicked()
{
    applyChanges();
    QMessageBox::information(this, tr("Properties Applied"), tr("Item properties have been applied."));
}

void ItemPropertyEditor::onResetClicked()
{
    if (item) {
        setItem(item); // Reload properties from the item
    }
}
