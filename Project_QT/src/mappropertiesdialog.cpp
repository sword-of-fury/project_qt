#include "mappropertiesdialog.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>

MapPropertiesDialog::MapPropertiesDialog(QWidget* parent, Map* map)
    : QDialog(parent),
      map(map)
{
    setWindowTitle(tr("Map Properties"));
    setMinimumWidth(400);
    setupUi();
    loadMapProperties();
}

MapPropertiesDialog::~MapPropertiesDialog()
{
}

void MapPropertiesDialog::setupUi()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // General properties group
    QGroupBox* generalGroup = new QGroupBox(tr("General Properties"), this);
    QFormLayout* generalLayout = new QFormLayout(generalGroup);
    
    descriptionEdit = new QLineEdit(this);
    generalLayout->addRow(tr("Description:"), descriptionEdit);
    
    widthSpinBox = new QSpinBox(this);
    widthSpinBox->setRange(1, 2048);
    widthSpinBox->setSingleStep(1);
    generalLayout->addRow(tr("Width:"), widthSpinBox);
    
    heightSpinBox = new QSpinBox(this);
    heightSpinBox->setRange(1, 2048);
    heightSpinBox->setSingleStep(1);
    generalLayout->addRow(tr("Height:"), heightSpinBox);
    
    // File properties group
    QGroupBox* fileGroup = new QGroupBox(tr("File Properties"), this);
    QFormLayout* fileLayout = new QFormLayout(fileGroup);
    
    QHBoxLayout* spawnLayout = new QHBoxLayout();
    spawnFileEdit = new QLineEdit(this);
    browseSpawnButton = new QPushButton(tr("Browse..."), this);
    spawnLayout->addWidget(spawnFileEdit);
    spawnLayout->addWidget(browseSpawnButton);
    fileLayout->addRow(tr("Spawn File:"), spawnLayout);
    
    QHBoxLayout* houseLayout = new QHBoxLayout();
    houseFileEdit = new QLineEdit(this);
    browseHouseButton = new QPushButton(tr("Browse..."), this);
    houseLayout->addWidget(houseFileEdit);
    houseLayout->addWidget(browseHouseButton);
    fileLayout->addRow(tr("House File:"), houseLayout);
    
    // Button box
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Reset, this);
    
    // Add widgets to main layout
    mainLayout->addWidget(generalGroup);
    mainLayout->addWidget(fileGroup);
    mainLayout->addWidget(buttonBox);
    
    // Connect signals
    connect(buttonBox, &QDialogButtonBox::accepted, this, &MapPropertiesDialog::onOkClicked);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &MapPropertiesDialog::onCancelClicked);
    connect(buttonBox->button(QDialogButtonBox::Reset), &QPushButton::clicked, this, &MapPropertiesDialog::onResetClicked);
    
    connect(browseSpawnButton, &QPushButton::clicked, [this]() {
        QString filePath = QFileDialog::getOpenFileName(this, tr("Select Spawn File"), 
                                                       QString(), tr("XML Files (*.xml);;All Files (*.*)"));
        if (!filePath.isEmpty()) {
            spawnFileEdit->setText(filePath);
        }
    });
    
    connect(browseHouseButton, &QPushButton::clicked, [this]() {
        QString filePath = QFileDialog::getOpenFileName(this, tr("Select House File"), 
                                                       QString(), tr("XML Files (*.xml);;All Files (*.*)"));
        if (!filePath.isEmpty()) {
            houseFileEdit->setText(filePath);
        }
    });
}

void MapPropertiesDialog::loadMapProperties()
{
    if (!map) return;
    
    descriptionEdit->setText(map->getDescription());
    widthSpinBox->setValue(map->getSize().width());
    heightSpinBox->setValue(map->getSize().height());
    spawnFileEdit->setText(map->getSpawnFile());
    houseFileEdit->setText(map->getHouseFile());
}

void MapPropertiesDialog::saveMapProperties()
{
    if (!map) return;
    
    map->setDescription(descriptionEdit->text());
    map->setSize(QSize(widthSpinBox->value(), heightSpinBox->value()));
    map->setSpawnFile(spawnFileEdit->text());
    map->setHouseFile(houseFileEdit->text());
}

QString MapPropertiesDialog::getDescription() const
{
    return descriptionEdit->text();
}

QString MapPropertiesDialog::getSpawnFile() const
{
    return spawnFileEdit->text();
}

QString MapPropertiesDialog::getHouseFile() const
{
    return houseFileEdit->text();
}

void MapPropertiesDialog::onOkClicked()
{
    saveMapProperties();
    accept();
}

void MapPropertiesDialog::onCancelClicked()
{
    reject();
}

void MapPropertiesDialog::onResetClicked()
{
    loadMapProperties();
}
