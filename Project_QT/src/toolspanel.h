#ifndef TOOLSPANEL_H
#define TOOLSPANEL_H

#include <QWidget>
#include <QGroupBox>
#include <QButtonGroup>
#include <QScrollArea>
#include "brush.h"
#include "item.h"

class MapView;

class ToolsPanel : public QWidget
{
    Q_OBJECT

public:
    explicit ToolsPanel(QWidget* parent = nullptr);
    ~ToolsPanel();

    void setMapView(MapView* mapView) { this->mapView = mapView; }

signals:
    void toolChanged(Brush::Type type);
    void layerChanged(int layer);
    void itemSelected(Item* item);

private slots:
    void onToolButtonClicked(int id);
    void onLayerButtonClicked(int id);
    void onItemButtonClicked(int id);

private:
    void setupUi();
    void createTools();
    void createLayers();
    void createItems();

    QPushButton* createToolButton(const QString& text, const QIcon& icon, int id);
    QPushButton* createLayerButton(int layer);
    QPushButton* createItemButton(Item* item);

    QVBoxLayout* mainLayout;
    QGroupBox* toolsGroupBox;
    QGroupBox* layersGroupBox;
    QGroupBox* itemsGroupBox;
    QButtonGroup* toolsButtonGroup;
    QButtonGroup* layersButtonGroup;
    QButtonGroup* itemsButtonGroup;
    QScrollArea* itemsScrollArea;
    MapView* mapView;
};

#endif // TOOLSPANEL_H