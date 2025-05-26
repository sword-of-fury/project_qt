#ifndef MAPSTATISTICSDIALOG_H
#define MAPSTATISTICSDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QTreeWidget>
#include <QPushButton>
#include <QDialogButtonBox>
#include "map.h"

class MapStatisticsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MapStatisticsDialog(QWidget* parent = nullptr, Map* map = nullptr);
    ~MapStatisticsDialog();

private slots:
    void onRefreshClicked();
    void onExportClicked();
    void onCloseClicked();

private:
    void setupUi();
    void calculateStatistics();
    void populateStatistics();
    void exportStatistics();

    Map* map;

    // Statistics data
    int totalTiles;
    int emptyTiles;
    int itemCount;
    int creatureCount;
    QMap<int, int> itemTypeCounts;
    QMap<int, int> creatureTypeCounts;
    QMap<int, int> layerItemCounts;

    // UI elements
    QLabel* mapSizeLabel;
    QLabel* totalTilesLabel;
    QLabel* emptyTilesLabel;
    QLabel* itemCountLabel;
    QLabel* creatureCountLabel;
    QTreeWidget* itemStatisticsTree;
    QTreeWidget* creatureStatisticsTree;
    QTreeWidget* layerStatisticsTree;
    QPushButton* refreshButton;
    QPushButton* exportButton;
    QDialogButtonBox* buttonBox;
};

#endif // MAPSTATISTICSDIALOG_H
