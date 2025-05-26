#include "creaturepropertyeditor.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QMessageBox>

CreaturePropertyEditor::CreaturePropertyEditor(QWidget* parent)
    : PropertyEditor(parent)
    , creature(nullptr)
{
    setupUi();
}

CreaturePropertyEditor::~CreaturePropertyEditor()
{
}

void CreaturePropertyEditor::setupUi()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // Create group box for basic properties
    QGroupBox* basicGroup = new QGroupBox(tr("Basic Properties"), this);
    QGridLayout* basicLayout = new QGridLayout(basicGroup);
    
    // Creature ID
    QLabel* creatureIdLabel = new QLabel(tr("Creature ID:"), basicGroup);
    creatureIdSpinBox = new QSpinBox(basicGroup);
    creatureIdSpinBox->setRange(1, 999999);
    creatureIdSpinBox->setToolTip(tr("The ID of the creature"));
    connect(creatureIdSpinBox, qOverload<int>(&QSpinBox::valueChanged), 
            this, &CreaturePropertyEditor::onCreatureIdChanged);
    basicLayout->addWidget(creatureIdLabel, 0, 0);
    basicLayout->addWidget(creatureIdSpinBox, 0, 1);
    
    // Name
    QLabel* nameLabel = new QLabel(tr("Name:"), basicGroup);
    nameEdit = new QLineEdit(basicGroup);
    nameEdit->setToolTip(tr("The name of the creature"));
    connect(nameEdit, &QLineEdit::textChanged, this, &CreaturePropertyEditor::onNameChanged);
    basicLayout->addWidget(nameLabel, 1, 0);
    basicLayout->addWidget(nameEdit, 1, 1);
    
    // Is NPC
    isNpcCheckBox = new QCheckBox(tr("Is NPC"), basicGroup);
    isNpcCheckBox->setToolTip(tr("Whether the creature is an NPC"));
    connect(isNpcCheckBox, &QCheckBox::toggled, this, &CreaturePropertyEditor::onIsNpcChanged);
    basicLayout->addWidget(isNpcCheckBox, 2, 0, 1, 2);
    
    mainLayout->addWidget(basicGroup);
    
    // Create group box for stats
    QGroupBox* statsGroup = new QGroupBox(tr("Stats"), this);
    QGridLayout* statsLayout = new QGridLayout(statsGroup);
    
    // Health
    QLabel* healthLabel = new QLabel(tr("Health:"), statsGroup);
    healthSpinBox = new QSpinBox(statsGroup);
    healthSpinBox->setRange(1, 999999);
    healthSpinBox->setToolTip(tr("Current health of the creature"));
    connect(healthSpinBox, qOverload<int>(&QSpinBox::valueChanged), 
            this, &CreaturePropertyEditor::onHealthChanged);
    statsLayout->addWidget(healthLabel, 0, 0);
    statsLayout->addWidget(healthSpinBox, 0, 1);
    
    // Max Health
    QLabel* maxHealthLabel = new QLabel(tr("Max Health:"), statsGroup);
    maxHealthSpinBox = new QSpinBox(statsGroup);
    maxHealthSpinBox->setRange(1, 999999);
    maxHealthSpinBox->setToolTip(tr("Maximum health of the creature"));
    connect(maxHealthSpinBox, qOverload<int>(&QSpinBox::valueChanged), 
            this, &CreaturePropertyEditor::onMaxHealthChanged);
    statsLayout->addWidget(maxHealthLabel, 1, 0);
    statsLayout->addWidget(maxHealthSpinBox, 1, 1);
    
    // Speed
    QLabel* speedLabel = new QLabel(tr("Speed:"), statsGroup);
    speedSpinBox = new QSpinBox(statsGroup);
    speedSpinBox->setRange(1, 1000);
    speedSpinBox->setToolTip(tr("Movement speed of the creature"));
    connect(speedSpinBox, qOverload<int>(&QSpinBox::valueChanged), 
            this, &CreaturePropertyEditor::onSpeedChanged);
    statsLayout->addWidget(speedLabel, 2, 0);
    statsLayout->addWidget(speedSpinBox, 2, 1);
    
    mainLayout->addWidget(statsGroup);
    
    // Create group box for spawn properties
    QGroupBox* spawnGroup = new QGroupBox(tr("Spawn Properties"), this);
    QGridLayout* spawnLayout = new QGridLayout(spawnGroup);
    
    // Direction
    QLabel* directionLabel = new QLabel(tr("Direction:"), spawnGroup);
    directionCombo = new QComboBox(spawnGroup);
    directionCombo->addItem(tr("North"), 0);
    directionCombo->addItem(tr("East"), 1);
    directionCombo->addItem(tr("South"), 2);
    directionCombo->addItem(tr("West"), 3);
    directionCombo->setToolTip(tr("The direction the creature is facing"));
    connect(directionCombo, qOverload<int>(&QComboBox::currentIndexChanged), 
            this, &CreaturePropertyEditor::onDirectionChanged);
    spawnLayout->addWidget(directionLabel, 0, 0);
    spawnLayout->addWidget(directionCombo, 0, 1);
    
    // Spawn Time
    QLabel* spawnTimeLabel = new QLabel(tr("Spawn Time:"), spawnGroup);
    spawnTimeSpinBox = new QSpinBox(spawnGroup);
    spawnTimeSpinBox->setRange(0, 86400); // 0 to 24 hours in seconds
    spawnTimeSpinBox->setToolTip(tr("Time in seconds until the creature respawns"));
    connect(spawnTimeSpinBox, qOverload<int>(&QSpinBox::valueChanged), 
            this, &CreaturePropertyEditor::onSpawnTimeChanged);
    spawnLayout->addWidget(spawnTimeLabel, 1, 0);
    spawnLayout->addWidget(spawnTimeSpinBox, 1, 1);
    
    mainLayout->addWidget(spawnGroup);
    
    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    
    applyButton = new QPushButton(tr("Apply"), this);
    connect(applyButton, &QPushButton::clicked, this, &CreaturePropertyEditor::onApplyClicked);
    buttonLayout->addWidget(applyButton);
    
    resetButton = new QPushButton(tr("Reset"), this);
    connect(resetButton, &QPushButton::clicked, this, &CreaturePropertyEditor::onResetClicked);
    buttonLayout->addWidget(resetButton);
    
    mainLayout->addLayout(buttonLayout);
    
    // Disable all controls initially
    setEnabled(false);
}

void CreaturePropertyEditor::setCreature(Creature* creature)
{
    this->creature = creature;
    
    if (creature) {
        // Load creature properties
        setProperty("creatureId", creature->getId());
        setProperty("name", creature->getName());
        setProperty("health", creature->getHealth());
        setProperty("maxHealth", creature->getMaxHealth());
        setProperty("direction", creature->getDirection());
        setProperty("speed", creature->getSpeed());
        setProperty("isNpc", creature->isNpc());
        setProperty("spawnTime", creature->getSpawnTime());
        
        updateUi();
        setEnabled(true);
    } else {
        clearProperties();
        setEnabled(false);
    }
}

Creature* CreaturePropertyEditor::getCreature() const
{
    return creature;
}

void CreaturePropertyEditor::updateUi()
{
    if (!creature) return;
    
    // Update UI elements with current property values
    creatureIdSpinBox->setValue(getProperty("creatureId").toInt());
    nameEdit->setText(getProperty("name").toString());
    healthSpinBox->setValue(getProperty("health").toInt());
    maxHealthSpinBox->setValue(getProperty("maxHealth").toInt());
    
    int directionIndex = directionCombo->findData(getProperty("direction").toInt());
    if (directionIndex >= 0) {
        directionCombo->setCurrentIndex(directionIndex);
    }
    
    speedSpinBox->setValue(getProperty("speed").toInt());
    isNpcCheckBox->setChecked(getProperty("isNpc").toBool());
    spawnTimeSpinBox->setValue(getProperty("spawnTime").toInt());
}

void CreaturePropertyEditor::applyChanges()
{
    if (!creature) return;
    
    // Apply property changes to the creature
    creature->setId(getProperty("creatureId").toInt());
    creature->setName(getProperty("name").toString());
    creature->setHealth(getProperty("health").toInt());
    creature->setMaxHealth(getProperty("maxHealth").toInt());
    creature->setDirection(getProperty("direction").toInt());
    creature->setSpeed(getProperty("speed").toInt());
    creature->setIsNpc(getProperty("isNpc").toBool());
    creature->setSpawnTime(getProperty("spawnTime").toInt());
}

void CreaturePropertyEditor::onCreatureIdChanged(int value)
{
    setProperty("creatureId", value);
}

void CreaturePropertyEditor::onNameChanged(const QString& text)
{
    setProperty("name", text);
}

void CreaturePropertyEditor::onHealthChanged(int value)
{
    setProperty("health", value);
}

void CreaturePropertyEditor::onMaxHealthChanged(int value)
{
    setProperty("maxHealth", value);
}

void CreaturePropertyEditor::onDirectionChanged(int index)
{
    setProperty("direction", directionCombo->itemData(index).toInt());
}

void CreaturePropertyEditor::onSpeedChanged(int value)
{
    setProperty("speed", value);
}

void CreaturePropertyEditor::onIsNpcChanged(bool checked)
{
    setProperty("isNpc", checked);
}

void CreaturePropertyEditor::onSpawnTimeChanged(int value)
{
    setProperty("spawnTime", value);
}

void CreaturePropertyEditor::onApplyClicked()
{
    applyChanges();
    QMessageBox::information(this, tr("Properties Applied"), tr("Creature properties have been applied."));
}

void CreaturePropertyEditor::onResetClicked()
{
    if (creature) {
        setCreature(creature); // Reload properties from the creature
    }
}
