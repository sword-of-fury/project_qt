#include "mapstatisticsdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include "itemmanager.h"
#include "creaturemanager.h"

MapStatisticsDialog::MapStatisticsDialog(QWidget* parent, Map* map)
    : QDialog(parent),
      map(map),
      totalTiles(0),
      emptyTiles(0),
      itemCount(0),
      creatureCount(0)
{
    setWindowTitle(tr("Map Statistics"));
    setMinimumSize(600, 500);
    setupUi();
    calculateStatistics();
    populateStatistics();
}

MapStatisticsDialog::~MapStatisticsDialog()
{
}

void MapStatisticsDialog::setupUi()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // General statistics group
    QGroupBox* generalGroup = new QGroupBox(tr("General Statistics"), this);
    QGridLayout* generalLayout = new QGridLayout(generalGroup);
    
    mapSizeLabel = new QLabel(this);
    totalTilesLabel = new QLabel(this);
    emptyTilesLabel = new QLabel(this);
    itemCountLabel = new QLabel(this);
    creatureCountLabel = new QLabel(this);
    
    generalLayout->addWidget(new QLabel(tr("Map Size:"), this), 0, 0);
    generalLayout->addWidget(mapSizeLabel, 0, 1);
    generalLayout->addWidget(new QLabel(tr("Total Tiles:"), this), 1, 0);
    generalLayout->addWidget(totalTilesLabel, 1, 1);
    generalLayout->addWidget(new QLabel(tr("Empty Tiles:"), this), 2, 0);
    generalLayout->addWidget(emptyTilesLabel, 2, 1);
    generalLayout->addWidget(new QLabel(tr("Item Count:"), this), 3, 0);
    generalLayout->addWidget(itemCountLabel, 3, 1);
    generalLayout->addWidget(new QLabel(tr("Creature Count:"), this), 4, 0);
    generalLayout->addWidget(creatureCountLabel, 4, 1);
    
    // Item statistics group
    QGroupBox* itemGroup = new QGroupBox(tr("Item Statistics"), this);
    QVBoxLayout* itemLayout = new QVBoxLayout(itemGroup);
    
    itemStatisticsTree = new QTreeWidget(this);
    itemStatisticsTree->setHeaderLabels({tr("Item Type"), tr("Count"), tr("Percentage")});
    itemStatisticsTree->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    itemLayout->addWidget(itemStatisticsTree);
    
    // Creature statistics group
    QGroupBox* creatureGroup = new QGroupBox(tr("Creature Statistics"), this);
    QVBoxLayout* creatureLayout = new QVBoxLayout(creatureGroup);
    
    creatureStatisticsTree = new QTreeWidget(this);
    creatureStatisticsTree->setHeaderLabels({tr("Creature Type"), tr("Count"), tr("Percentage")});
    creatureStatisticsTree->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    creatureLayout->addWidget(creatureStatisticsTree);
    
    // Layer statistics group
    QGroupBox* layerGroup = new QGroupBox(tr("Layer Statistics"), this);
    QVBoxLayout* layerLayout = new QVBoxLayout(layerGroup);
    
    layerStatisticsTree = new QTreeWidget(this);
    layerStatisticsTree->setHeaderLabels({tr("Layer"), tr("Item Count"), tr("Percentage")});
    layerStatisticsTree->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    layerLayout->addWidget(layerStatisticsTree);
    
    // Button box
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    
    refreshButton = new QPushButton(tr("Refresh"), this);
    exportButton = new QPushButton(tr("Export..."), this);
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Close, this);
    
    buttonLayout->addWidget(refreshButton);
    buttonLayout->addWidget(exportButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(buttonBox);
    
    // Add widgets to main layout
    mainLayout->addWidget(generalGroup);
    mainLayout->addWidget(itemGroup);
    mainLayout->addWidget(creatureGroup);
    mainLayout->addWidget(layerGroup);
    mainLayout->addLayout(buttonLayout);
    
    // Connect signals
    connect(refreshButton, &QPushButton::clicked, this, &MapStatisticsDialog::onRefreshClicked);
    connect(exportButton, &QPushButton::clicked, this, &MapStatisticsDialog::onExportClicked);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &MapStatisticsDialog::onCloseClicked);
}

void MapStatisticsDialog::calculateStatistics()
{
    if (!map) return;
    
    // Reset statistics
    totalTiles = 0;
    emptyTiles = 0;
    itemCount = 0;
    creatureCount = 0;
    itemTypeCounts.clear();
    creatureTypeCounts.clear();
    layerItemCounts.clear();
    
    // Get map size
    QSize mapSize = map->getSize();
    totalTiles = mapSize.width() * mapSize.height();
    
    // Calculate statistics for each tile
    for (int x = 0; x < mapSize.width(); ++x) {
        for (int y = 0; y < mapSize.height(); ++y) {
            bool isEmpty = true;
            
            // Check each layer
            for (int layer = 0; layer < Map::LayerCount; ++layer) {
                QList<Item> items = map->getItems(x, y, static_cast<Layer::Type>(layer));
                
                if (!items.isEmpty()) {
                    isEmpty = false;
                    itemCount += items.size();
                    layerItemCounts[layer] += items.size();
                    
                    // Count item types
                    for (const Item& item : items) {
                        itemTypeCounts[item.getId()]++;
                    }
                }
            }
            
            if (isEmpty) {
                emptyTiles++;
            }
        }
    }
}

void MapStatisticsDialog::populateStatistics()
{
    if (!map) return;
    
    // Update general statistics labels
    QSize mapSize = map->getSize();
    mapSizeLabel->setText(QString("%1 x %2").arg(mapSize.width()).arg(mapSize.height()));
    totalTilesLabel->setText(QString::number(totalTiles));
    emptyTilesLabel->setText(QString::number(emptyTiles));
    itemCountLabel->setText(QString::number(itemCount));
    creatureCountLabel->setText(QString::number(creatureCount));
    
    // Update item statistics tree
    itemStatisticsTree->clear();
    ItemManager& itemManager = ItemManager::getInstance();
    
    for (auto it = itemTypeCounts.constBegin(); it != itemTypeCounts.constEnd(); ++it) {
        int itemId = it.key();
        int count = it.value();
        double percentage = (itemCount > 0) ? (static_cast<double>(count) / itemCount * 100.0) : 0.0;
        
        QTreeWidgetItem* item = new QTreeWidgetItem(itemStatisticsTree);
        Item* itemObj = itemManager.getItemById(itemId);
        QString itemName = itemObj ? itemObj->getName() : QString("Item #%1").arg(itemId);
        
        item->setText(0, itemName);
        item->setText(1, QString::number(count));
        item->setText(2, QString("%1%").arg(percentage, 0, 'f', 2));
    }
    
    // Update creature statistics tree
    creatureStatisticsTree->clear();
    CreatureManager& creatureManager = CreatureManager::getInstance();
    
    for (auto it = creatureTypeCounts.constBegin(); it != creatureTypeCounts.constEnd(); ++it) {
        int creatureId = it.key();
        int count = it.value();
        double percentage = (creatureCount > 0) ? (static_cast<double>(count) / creatureCount * 100.0) : 0.0;
        
        QTreeWidgetItem* item = new QTreeWidgetItem(creatureStatisticsTree);
        Creature* creatureObj = creatureManager.getCreatureById(creatureId);
        QString creatureName = creatureObj ? creatureObj->getName() : QString("Creature #%1").arg(creatureId);
        
        item->setText(0, creatureName);
        item->setText(1, QString::number(count));
        item->setText(2, QString("%1%").arg(percentage, 0, 'f', 2));
    }
    
    // Update layer statistics tree
    layerStatisticsTree->clear();
    
    for (auto it = layerItemCounts.constBegin(); it != layerItemCounts.constEnd(); ++it) {
        int layer = it.key();
        int count = it.value();
        double percentage = (itemCount > 0) ? (static_cast<double>(count) / itemCount * 100.0) : 0.0;
        
        QTreeWidgetItem* item = new QTreeWidgetItem(layerStatisticsTree);
        QString layerName;
        
        switch (layer) {
            case 0: layerName = tr("Ground"); break;
            case 1: layerName = tr("Items"); break;
            case 2: layerName = tr("Creatures"); break;
            default: layerName = tr("Layer %1").arg(layer); break;
        }
        
        item->setText(0, layerName);
        item->setText(1, QString::number(count));
        item->setText(2, QString("%1%").arg(percentage, 0, 'f', 2));
    }
}

void MapStatisticsDialog::exportStatistics()
{
    QString filePath = QFileDialog::getSaveFileName(this, tr("Export Statistics"), 
                                                  QString(), tr("Text Files (*.txt);;CSV Files (*.csv);;All Files (*.*)"));
    if (filePath.isEmpty()) return;
    
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("Export Error"), tr("Could not open file for writing."));
        return;
    }
    
    QTextStream out(&file);
    
    // Write general statistics
    out << tr("Map Statistics") << "\n";
    out << tr("=============") << "\n\n";
    
    out << tr("Map Size: %1 x %2").arg(map->getSize().width()).arg(map->getSize().height()) << "\n";
    out << tr("Total Tiles: %1").arg(totalTiles) << "\n";
    out << tr("Empty Tiles: %1 (%2%)").arg(emptyTiles).arg(static_cast<double>(emptyTiles) / totalTiles * 100.0, 0, 'f', 2) << "\n";
    out << tr("Item Count: %1").arg(itemCount) << "\n";
    out << tr("Creature Count: %1").arg(creatureCount) << "\n\n";
    
    // Write item statistics
    out << tr("Item Statistics") << "\n";
    out << tr("==============") << "\n\n";
    out << tr("Item Type,Count,Percentage") << "\n";
    
    ItemManager& itemManager = ItemManager::getInstance();
    for (auto it = itemTypeCounts.constBegin(); it != itemTypeCounts.constEnd(); ++it) {
        int itemId = it.key();
        int count = it.value();
        double percentage = (itemCount > 0) ? (static_cast<double>(count) / itemCount * 100.0) : 0.0;
        
        Item* itemObj = itemManager.getItemById(itemId);
        QString itemName = itemObj ? itemObj->getName() : QString("Item #%1").arg(itemId);
        
        out << QString("\"%1\",%2,%3%").arg(itemName).arg(count).arg(percentage, 0, 'f', 2) << "\n";
    }
    
    out << "\n";
    
    // Write creature statistics
    out << tr("Creature Statistics") << "\n";
    out << tr("===================") << "\n\n";
    out << tr("Creature Type,Count,Percentage") << "\n";
    
    CreatureManager& creatureManager = CreatureManager::getInstance();
    for (auto it = creatureTypeCounts.constBegin(); it != creatureTypeCounts.constEnd(); ++it) {
        int creatureId = it.key();
        int count = it.value();
        double percentage = (creatureCount > 0) ? (static_cast<double>(count) / creatureCount * 100.0) : 0.0;
        
        Creature* creatureObj = creatureManager.getCreatureById(creatureId);
        QString creatureName = creatureObj ? creatureObj->getName() : QString("Creature #%1").arg(creatureId);
        
        out << QString("\"%1\",%2,%3%").arg(creatureName).arg(count).arg(percentage, 0, 'f', 2) << "\n";
    }
    
    out << "\n";
    
    // Write layer statistics
    out << tr("Layer Statistics") << "\n";
    out << tr("================") << "\n\n";
    out << tr("Layer,Item Count,Percentage") << "\n";
    
    for (auto it = layerItemCounts.constBegin(); it != layerItemCounts.constEnd(); ++it) {
        int layer = it.key();
        int count = it.value();
        double percentage = (itemCount > 0) ? (static_cast<double>(count) / itemCount * 100.0) : 0.0;
        
        QString layerName;
        switch (layer) {
            case 0: layerName = tr("Ground"); break;
            case 1: layerName = tr("Items"); break;
            case 2: layerName = tr("Creatures"); break;
            default: layerName = tr("Layer %1").arg(layer); break;
        }
        
        out << QString("\"%1\",%2,%3%").arg(layerName).arg(count).arg(percentage, 0, 'f', 2) << "\n";
    }
    
    file.close();
    
    QMessageBox::information(this, tr("Export Successful"), tr("Statistics exported successfully."));
}

void MapStatisticsDialog::onRefreshClicked()
{
    calculateStatistics();
    populateStatistics();
}

void MapStatisticsDialog::onExportClicked()
{
    exportStatistics();
}

void MapStatisticsDialog::onCloseClicked()
{
    accept();
}
