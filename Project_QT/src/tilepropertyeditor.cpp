#include "tilepropertyeditor.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QMessageBox>

TilePropertyEditor::TilePropertyEditor(QWidget* parent)
    : PropertyEditor(parent)
    , tile(nullptr)
{
    setupUi();
}

TilePropertyEditor::~TilePropertyEditor()
{
}

void TilePropertyEditor::setupUi()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // Create group box for tile flags
    QGroupBox* flagsGroup = new QGroupBox(tr("Tile Flags"), this);
    QGridLayout* flagsLayout = new QGridLayout(flagsGroup);
    
    // Collision flag
    collisionCheckBox = new QCheckBox(tr("Collision"), flagsGroup);
    collisionCheckBox->setToolTip(tr("Tile cannot be walked on"));
    connect(collisionCheckBox, &QCheckBox::toggled, this, &TilePropertyEditor::onCollisionChanged);
    flagsLayout->addWidget(collisionCheckBox, 0, 0);
    
    // Movement cost
    QLabel* movementCostLabel = new QLabel(tr("Movement Cost:"), flagsGroup);
    movementCostSpinBox = new QSpinBox(flagsGroup);
    movementCostSpinBox->setRange(1, 100);
    movementCostSpinBox->setToolTip(tr("Cost to move across this tile (1-100)"));
    connect(movementCostSpinBox, qOverload<int>(&QSpinBox::valueChanged), 
            this, &TilePropertyEditor::onMovementCostChanged);
    flagsLayout->addWidget(movementCostLabel, 1, 0);
    flagsLayout->addWidget(movementCostSpinBox, 1, 1);
    
    // Protection zone flag
    protectionZoneCheckBox = new QCheckBox(tr("Protection Zone"), flagsGroup);
    protectionZoneCheckBox->setToolTip(tr("Players cannot attack each other in this zone"));
    connect(protectionZoneCheckBox, &QCheckBox::toggled, this, &TilePropertyEditor::onProtectionZoneChanged);
    flagsLayout->addWidget(protectionZoneCheckBox, 2, 0);
    
    // No logout flag
    noLogoutCheckBox = new QCheckBox(tr("No Logout"), flagsGroup);
    noLogoutCheckBox->setToolTip(tr("Players cannot logout on this tile"));
    connect(noLogoutCheckBox, &QCheckBox::toggled, this, &TilePropertyEditor::onNoLogoutChanged);
    flagsLayout->addWidget(noLogoutCheckBox, 3, 0);
    
    // No move items flag
    noMoveItemsCheckBox = new QCheckBox(tr("No Move Items"), flagsGroup);
    noMoveItemsCheckBox->setToolTip(tr("Items cannot be moved on this tile"));
    connect(noMoveItemsCheckBox, &QCheckBox::toggled, this, &TilePropertyEditor::onNoMoveItemsChanged);
    flagsLayout->addWidget(noMoveItemsCheckBox, 4, 0);
    
    // No move creatures flag
    noMoveCreaturesCheckBox = new QCheckBox(tr("No Move Creatures"), flagsGroup);
    noMoveCreaturesCheckBox->setToolTip(tr("Creatures cannot be moved on this tile"));
    connect(noMoveCreaturesCheckBox, &QCheckBox::toggled, this, &TilePropertyEditor::onNoMoveCreaturesChanged);
    flagsLayout->addWidget(noMoveCreaturesCheckBox, 5, 0);
    
    // No spawn flag
    noSpawnCheckBox = new QCheckBox(tr("No Spawn"), flagsGroup);
    noSpawnCheckBox->setToolTip(tr("Creatures cannot spawn on this tile"));
    connect(noSpawnCheckBox, &QCheckBox::toggled, this, &TilePropertyEditor::onNoSpawnChanged);
    flagsLayout->addWidget(noSpawnCheckBox, 6, 0);
    
    mainLayout->addWidget(flagsGroup);
    
    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    
    applyButton = new QPushButton(tr("Apply"), this);
    connect(applyButton, &QPushButton::clicked, this, &TilePropertyEditor::onApplyClicked);
    buttonLayout->addWidget(applyButton);
    
    resetButton = new QPushButton(tr("Reset"), this);
    connect(resetButton, &QPushButton::clicked, this, &TilePropertyEditor::onResetClicked);
    buttonLayout->addWidget(resetButton);
    
    mainLayout->addLayout(buttonLayout);
    
    // Disable all controls initially
    setEnabled(false);
}

void TilePropertyEditor::setTile(Tile* tile)
{
    this->tile = tile;
    
    if (tile) {
        // Load tile properties
        setProperty("collision", tile->hasCollision());
        setProperty("movementCost", tile->getMovementCost());
        setProperty("protectionZone", tile->isProtectionZone());
        setProperty("noLogout", tile->isNoLogout());
        setProperty("noMoveItems", tile->isNoMoveItems());
        setProperty("noMoveCreatures", tile->isNoMoveCreatures());
        setProperty("noSpawn", tile->isNoSpawn());
        
        updateUi();
        setEnabled(true);
    } else {
        clearProperties();
        setEnabled(false);
    }
}

Tile* TilePropertyEditor::getTile() const
{
    return tile;
}

void TilePropertyEditor::updateUi()
{
    if (!tile) return;
    
    // Update UI elements with current property values
    collisionCheckBox->setChecked(getProperty("collision").toBool());
    movementCostSpinBox->setValue(getProperty("movementCost").toInt());
    protectionZoneCheckBox->setChecked(getProperty("protectionZone").toBool());
    noLogoutCheckBox->setChecked(getProperty("noLogout").toBool());
    noMoveItemsCheckBox->setChecked(getProperty("noMoveItems").toBool());
    noMoveCreaturesCheckBox->setChecked(getProperty("noMoveCreatures").toBool());
    noSpawnCheckBox->setChecked(getProperty("noSpawn").toBool());
}

void TilePropertyEditor::applyChanges()
{
    if (!tile) return;
    
    // Apply property changes to the tile
    tile->setCollision(getProperty("collision").toBool());
    tile->setMovementCost(getProperty("movementCost").toInt());
    tile->setProtectionZone(getProperty("protectionZone").toBool());
    tile->setNoLogout(getProperty("noLogout").toBool());
    tile->setNoMoveItems(getProperty("noMoveItems").toBool());
    tile->setNoMoveCreatures(getProperty("noMoveCreatures").toBool());
    tile->setNoSpawn(getProperty("noSpawn").toBool());
}

void TilePropertyEditor::onCollisionChanged(bool checked)
{
    setProperty("collision", checked);
}

void TilePropertyEditor::onMovementCostChanged(int value)
{
    setProperty("movementCost", value);
}

void TilePropertyEditor::onProtectionZoneChanged(bool checked)
{
    setProperty("protectionZone", checked);
}

void TilePropertyEditor::onNoLogoutChanged(bool checked)
{
    setProperty("noLogout", checked);
}

void TilePropertyEditor::onNoMoveItemsChanged(bool checked)
{
    setProperty("noMoveItems", checked);
}

void TilePropertyEditor::onNoMoveCreaturesChanged(bool checked)
{
    setProperty("noMoveCreatures", checked);
}

void TilePropertyEditor::onNoSpawnChanged(bool checked)
{
    setProperty("noSpawn", checked);
}

void TilePropertyEditor::onApplyClicked()
{
    applyChanges();
    QMessageBox::information(this, tr("Properties Applied"), tr("Tile properties have been applied."));
}

void TilePropertyEditor::onResetClicked()
{
    if (tile) {
        setTile(tile); // Reload properties from the tile
    }
}
