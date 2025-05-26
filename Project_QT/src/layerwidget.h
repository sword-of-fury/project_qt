#ifndef LAYERWIDGET_H
#define LAYERWIDGET_H

#include <QWidget>
#include <QComboBox>
#include <QSlider>
#include <QLabel>
#include <QCheckBox>
#include <QMap>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include "map.h" // Needed for Map::LayerCount and access to Layer objects
#include "layer.h" // For Layer::Type enum and Layer class itself

class Map; // Forward declaration for the Map class

/**
 * @brief The LayerWidget class provides UI controls for map layer selection and visibility.
 * It contains a slider and combo box for active layer selection, and checkboxes for layer visibility.
 */
class LayerWidget : public QWidget
{
    Q_OBJECT
public:
    explicit LayerWidget(QWidget* parent = nullptr);
    virtual ~LayerWidget();

    void setMap(Map* map); // Assigns the Map instance to this widget
    Map* getMap() const { return currentMap; }

    int getCurrentLayer() const; // Gets the currently selected active editing layer
    void setCurrentLayer(int layer); // Sets the current active layer programmatically

    bool isLayerVisible(Layer::Type type) const; // Checks if a specific layer type is visible
    void setLayerVisible(Layer::Type type, bool visible); // Sets visibility for a specific layer

signals:
    // Emitted when the active editing layer changes (from slider or combo)
    void layerChanged(int layer);
    // Emitted when a specific layer's visibility is toggled (from checkbox)
    void layerVisibilityChanged(Layer::Type type, bool visible);
    // Emitted when layer lock state changes (for future implementation)
    void layerLockedChanged(Layer::Type type, bool locked);

private slots:
    // UI event handlers for the controls
    void onLayerSliderChanged(int value);
    void onLayerComboChanged(int index);
    void onLayerCheckBoxToggled(bool checked); // Handled by individual lambda connections, but kept for clarity

    // Map layer visibility change handler (internal)
    void onMapLayerVisibilityChanged(Layer::Type type, bool visible);

private:
    Map* currentMap; // Pointer to the main map instance (not owned)

    // UI elements
    QComboBox* layerCombo; // For selecting active layer by name
    QSlider* layerSlider; // For selecting active layer by index (0-15)
    QLabel* layerLabel; // Displays current layer name

    // Checkboxes for individual layer visibility
    QMap<Layer::Type, QCheckBox*> layerVisibilityCheckboxes;

    // Helper to setup the UI components
    void setupUi();
    // Helper to load current layer states from the map
    void loadLayerStates();
    // Helper to save current layer states to the map (typically done by apply changes button in settings or directly)
    // void saveLayerStates();
};

#endif // LAYERWIDGET_H