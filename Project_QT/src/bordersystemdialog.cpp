#include "bordersystemdialog.h"
#include "bordersystem.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QHeaderView>
#include <QMessageBox>

BorderSystemDialog::BorderSystemDialog(BorderSystem* borderSystem, QWidget* parent)
    : QDialog(parent)
    , borderSystem(borderSystem)
{
    setupUi();
    loadMappings();
}

BorderSystemDialog::~BorderSystemDialog()
{
}

void BorderSystemDialog::setupUi()
{
    // Set window properties
    setWindowTitle(tr("Automagic Border System Configuration"));
    setMinimumSize(500, 400);
    
    // Create main layout
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // Create enable checkbox
    enableCheckBox = new QCheckBox(tr("Enable Automagic Border System"), this);
    enableCheckBox->setChecked(borderSystem ? borderSystem->isEnabled() : false);
    connect(enableCheckBox, &QCheckBox::toggled, this, &BorderSystemDialog::onEnableToggled);
    mainLayout->addWidget(enableCheckBox);
    
    // Create mappings table
    QLabel* mappingsLabel = new QLabel(tr("Border Tile Mappings:"), this);
    mainLayout->addWidget(mappingsLabel);
    
    mappingsTable = new QTableWidget(0, 2, this);
    mappingsTable->setHorizontalHeaderLabels({tr("Source Tile ID"), tr("Border Tile IDs")});
    mappingsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    mappingsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    mappingsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    mainLayout->addWidget(mappingsTable);
    
    // Create buttons for adding/removing mappings
    QHBoxLayout* mappingButtonsLayout = new QHBoxLayout();
    
    addButton = new QPushButton(tr("Add Mapping"), this);
    connect(addButton, &QPushButton::clicked, this, &BorderSystemDialog::onAddMapping);
    mappingButtonsLayout->addWidget(addButton);
    
    removeButton = new QPushButton(tr("Remove Mapping"), this);
    connect(removeButton, &QPushButton::clicked, this, &BorderSystemDialog::onRemoveMapping);
    mappingButtonsLayout->addWidget(removeButton);
    
    mainLayout->addLayout(mappingButtonsLayout);
    
    // Create dialog buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    
    applyButton = new QPushButton(tr("Apply"), this);
    connect(applyButton, &QPushButton::clicked, this, &BorderSystemDialog::onApply);
    buttonLayout->addWidget(applyButton);
    
    okButton = new QPushButton(tr("OK"), this);
    okButton->setDefault(true);
    connect(okButton, &QPushButton::clicked, this, &BorderSystemDialog::onOk);
    buttonLayout->addWidget(okButton);
    
    cancelButton = new QPushButton(tr("Cancel"), this);
    connect(cancelButton, &QPushButton::clicked, this, &BorderSystemDialog::onCancel);
    buttonLayout->addWidget(cancelButton);
    
    mainLayout->addLayout(buttonLayout);
    
    // Update UI state
    onEnableToggled(enableCheckBox->isChecked());
}

void BorderSystemDialog::loadMappings()
{
    if (!borderSystem) {
        return;
    }
    
    // Clear existing mappings
    mappingsTable->setRowCount(0);
    
    // TODO: Load mappings from the border system
    // This would require adding methods to the BorderSystem class to get the mappings
    // For now, we'll just show a placeholder message
    
    // Example mapping (placeholder)
    mappingsTable->insertRow(0);
    mappingsTable->setItem(0, 0, new QTableWidgetItem("1"));
    mappingsTable->setItem(0, 1, new QTableWidgetItem("101, 102, 103, 104, 105, 106, 107, 108"));
    
    mappingsTable->insertRow(1);
    mappingsTable->setItem(1, 0, new QTableWidgetItem("2"));
    mappingsTable->setItem(1, 1, new QTableWidgetItem("201, 202, 203, 204, 205, 206, 207, 208"));
    
    mappingsTable->insertRow(2);
    mappingsTable->setItem(2, 0, new QTableWidgetItem("3"));
    mappingsTable->setItem(2, 1, new QTableWidgetItem("301, 302, 303, 304, 305, 306, 307, 308"));
}

void BorderSystemDialog::saveMappings()
{
    if (!borderSystem) {
        return;
    }
    
    // Set border system enabled state
    borderSystem->setEnabled(enableCheckBox->isChecked());
    
    // TODO: Save mappings to the border system
    // This would require adding methods to the BorderSystem class to set the mappings
    // For now, we'll just show a message
    
    // Process each row in the table
    for (int row = 0; row < mappingsTable->rowCount(); ++row) {
        // Get source tile ID
        int sourceTileId = mappingsTable->item(row, 0)->text().toInt();
        
        // Get border tile IDs
        QString borderTilesStr = mappingsTable->item(row, 1)->text();
        QStringList borderTilesList = borderTilesStr.split(",", Qt::SkipEmptyParts);
        
        QVector<int> borderTileIds;
        for (const QString& idStr : borderTilesList) {
            bool ok;
            int id = idStr.trimmed().toInt(&ok);
            if (ok) {
                borderTileIds.append(id);
            }
        }
        
        // Register the mapping
        borderSystem->registerBorderSource(sourceTileId, borderTileIds);
    }
}

void BorderSystemDialog::onAddMapping()
{
    int row = mappingsTable->rowCount();
    mappingsTable->insertRow(row);
    mappingsTable->setItem(row, 0, new QTableWidgetItem("0"));
    mappingsTable->setItem(row, 1, new QTableWidgetItem(""));
    
    // Select the new row
    mappingsTable->selectRow(row);
    
    // Focus on the source tile ID cell
    mappingsTable->setCurrentCell(row, 0);
    mappingsTable->editItem(mappingsTable->item(row, 0));
}

void BorderSystemDialog::onRemoveMapping()
{
    int row = mappingsTable->currentRow();
    if (row >= 0) {
        mappingsTable->removeRow(row);
    }
}

void BorderSystemDialog::onEnableToggled(bool checked)
{
    // Enable/disable UI elements based on checkbox state
    mappingsTable->setEnabled(checked);
    addButton->setEnabled(checked);
    removeButton->setEnabled(checked);
}

void BorderSystemDialog::onApply()
{
    saveMappings();
    QMessageBox::information(this, tr("Settings Applied"), tr("Border system settings have been applied."));
}

void BorderSystemDialog::onOk()
{
    saveMappings();
    accept();
}

void BorderSystemDialog::onCancel()
{
    reject();
}
