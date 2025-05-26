#include "layerwidget.h"
#include <QLabel>
#include <QSpinBox> // Original had wxSpinCtrl equivalents
#include <QDebug> // For debugging

// Constructor for LayerWidget
LayerWidget::LayerWidget(QWidget* parent) :
    QWidget(parent),
    currentMap(nullptr)
{
    setupUi(); // Initialize all UI components

    // Initial connections for UI elements
    connect(layerSlider, &QSlider::valueChanged, this, &LayerWidget::onLayerSliderChanged);
    connect(layerCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LayerWidget::onLayerComboChanged);

    // Initial layer visibility setup for checkboxes. Connect these after map is set for dynamic access.
    // Initial state is "all visible" but relies on the actual Layer objects from the Map.
    for (int i = 0; i < Layer::Type::Count; ++i) { // Loop all defined Layer::Type enums
        Layer::Type layerType = static_cast<Layer::Type>(i);
        QCheckBox* checkBox = layerVisibilityCheckboxes.value(layerType);
        if (checkBox) {
            // These connections will be live when the Map is set via `setMap`.
            connect(checkBox, &QCheckBox::toggled, this, &LayerWidget::onLayerCheckBoxToggled); // Generic handler
        }
    }
}

// Destructor
LayerWidget::~LayerWidget()
{
    // All QObject children are automatically deleted by Qt's parent-child hierarchy.
    // currentMap is not owned, no deletion here.
}

void LayerWidget::setMap(Map* map)
{
    if (currentMap == map) return;

    if (currentMap) {
        // Disconnect from previous map's layer objects before assigning a new map.
        for (int i = 0; i < Layer::Type::Count; ++i) {
            Layer::Type layerType = static_cast<Layer::Type>(i);
            Layer* layerObj = currentMap->getLayer(layerType);
            if (layerObj) {
                // Disconnect this widget from previous layer object signals
                disconnect(layerObj, &Layer::visibilityChanged, this, &LayerWidget::onMapLayerVisibilityChanged);
            }
        }
    }

    currentMap = map;

    if (currentMap) {
        // Connect to new map's layer objects.
        for (int i = 0; i < Layer::Type::Count; ++i) {
            Layer::Type layerType = static_cast<Layer::Type>(i);
            Layer* layerObj = currentMap->getLayer(layerType);
            if (layerObj) {
                // Connect Layer object's internal visibility changes to update UI.
                connect(layerObj, &Layer::visibilityChanged, this, &LayerWidget::onMapLayerVisibilityChanged);
            }
        }
        loadLayerStates(); // Update UI to reflect new map's layer states.
    }
}

void LayerWidget::setupUi()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(5, 5, 5, 5); // Add some padding

    // --- Active Layer Selection ---
    QGroupBox* layerSelectionGroup = new QGroupBox(tr("Active Layer"), this);
    QVBoxLayout* selectionLayout = new QVBoxLayout(layerSelectionGroup);

    // Layer Slider (vertical in RME original)
    layerSlider = new QSlider(Qt::Vertical, layerSelectionGroup);
    layerSlider->setRange(0, Map::LayerCount - 1); // From 0 to 15 (inclusive, total 16 layers)
    layerSlider->setValue(static_cast<int>(Layer::Type::Ground)); // Default to Ground layer (e.g., Layer 7 in Tibia internal Z)
    layerSlider->setTickPosition(QSlider::TicksBothSides);
    layerSlider->setTickInterval(1);
    layerSlider->setMinimumHeight(150); // Make slider tall enough
    // Add QLabel for slider's current value
    layerLabel = new QLabel(Layer::typeToString(static_cast<Layer::Type>(layerSlider->value())), layerSelectionGroup);
    layerLabel->setAlignment(Qt::AlignCenter);

    // Layer Combo Box (Dropdown)
    layerCombo = new QComboBox(layerSelectionGroup);
    for (int i = 0; i < Map::LayerCount; ++i) {
        // Add names from lowest layer to highest (e.g., Z=0 to Z=15, if UI represents reversed layers)
        // Or reverse the display order for layers based on Tibia's perspective (high Z = lowest visual, low Z = highest visual in editor)
        // Original RME maps 0 to 15, and `LayerWidget` displays floors: 7=Ground(0), 0=roofs(7), 15=deep(15) etc.
        // Assuming your Map has Layer 0 (Ground) and progresses numerically.
        layerCombo->addItem(Layer::typeToString(static_cast<Layer::Type>(i)), i); // Store layer type as UserData
    }
    // Set default selection based on current layer in code (Ground)
    layerCombo->setCurrentIndex(static_cast<int>(Layer::Type::Ground));
    
    selectionLayout->addWidget(layerLabel, 0, Qt::AlignCenter);
    selectionLayout->addWidget(layerCombo);
    selectionLayout->addWidget(layerSlider); // Slider is visual, order depends on preference.
    layerSelectionGroup->setLayout(selectionLayout);
    mainLayout->addWidget(layerSelectionGroup);


    // --- Layer Visibility Checkboxes ---
    QGroupBox* visibilityGroup = new QGroupBox(tr("Layer Visibility"), this);
    QVBoxLayout* visibilityLayout = new QVBoxLayout(visibilityGroup);

    // Create checkboxes for each layer type. Map Layer::Type to a display name.
    // Tibia usually uses these logical layers + 8 additional layers 0-7, which map to 0-15 overall.
    // Original RME LayerWidget used 5 checkboxes: Ground, Items, Creatures, Effects, Roofs.
    // For this migration, map the provided `Layer::Type` to `Map::LayerCount`.
    
    // We'll map Layer::Type to checkboxes directly here.
    // Need to account for sparse `Layer::Type` values and iterate through the defined enum values.
    // Store in `layerVisibilityCheckboxes` map for lookup.
    
    // A temporary map to define order for display.
    QMap<Layer::Type, QString> displayOrder;
    displayOrder[Layer::Type::Ground] = tr("Ground");
    displayOrder[Layer::Type::GroundDetail] = tr("Ground Detail");
    displayOrder[Layer::Type::Objects] = tr("Objects");
    displayOrder[Layer::Type::Items] = tr("Items");
    displayOrder[Layer::Type::Creatures] = tr("Creatures");
    displayOrder[Layer::Type::Effects] = tr("Effects");
    displayOrder[Layer::Type::Roofs] = tr("Roofs");
    displayOrder[Layer::Type::Walls] = tr("Walls");
    displayOrder[Layer::Type::Water] = tr("Water");
    displayOrder[Layer::Type::Top] = tr("Topmost");

    QList<Layer::Type> sortedLayerTypes = displayOrder.keys();
    // Sort by enum value for consistent order.
    std::sort(sortedLayerTypes.begin(), sortedLayerTypes.end(), [](Layer::Type a, Layer::Type b){
        return static_cast<int>(a) < static_cast<int>(b);
    });

    for (Layer::Type type : sortedLayerTypes) {
        if (type == Layer::Type::Count) continue; // Skip Count enum value
        
        QCheckBox* checkBox = new QCheckBox(displayOrder.value(type), visibilityGroup);
        checkBox->setChecked(true); // Default all layers visible.
        checkBox->setProperty("LayerType", QVariant::fromValue(static_cast<int>(type))); // Store LayerType as QVariant.
        visibilityLayout->addWidget(checkBox);
        layerVisibilityCheckboxes.insert(type, checkBox);

        // Connect specific checkbox to handler when toggled.
        // The `onLayerCheckBoxToggled` slot will retrieve LayerType from property.
        connect(checkBox, &QCheckBox::toggled, this, &LayerWidget::onLayerCheckBoxToggled);
    }
    
    visibilityGroup->setLayout(visibilityLayout);
    mainLayout->addWidget(visibilityGroup);

    setLayout(mainLayout);
}

void LayerWidget::loadLayerStates()
{
    // Updates the UI elements (slider, combo, checkboxes) to reflect the current map's layer states.
    if (!currentMap) return;

    // Active Layer (Slider & ComboBox)
    // Temporarily block signals to prevent recursive updates during UI refresh.
    layerSlider->blockSignals(true);
    layerCombo->blockSignals(true);

    int mapCurrentLayer = currentMap->getCurrentLayer(); // Assuming Map has this property or it's from MainWindow
    layerSlider->setValue(mapCurrentLayer);
    layerLabel->setText(Layer::typeToString(static_cast<Layer::Type>(mapCurrentLayer)));
    
    // Find index in combo box.
    int comboIndex = layerCombo->findData(mapCurrentLayer); // findData uses QVariant data stored with addItem.
    if (comboIndex != -1) {
        layerCombo->setCurrentIndex(comboIndex);
    } else {
        qWarning() << "LayerWidget: Could not find layer" << mapCurrentLayer << "in combo box.";
        layerCombo->setCurrentIndex(0); // Default to first item.
    }

    layerSlider->blockSignals(false);
    layerCombo->blockSignals(false);

    // Layer Visibility Checkboxes
    for (int i = 0; i < Layer::Type::Count; ++i) {
        Layer::Type layerType = static_cast<Layer::Type>(i);
        QCheckBox* checkBox = layerVisibilityCheckboxes.value(layerType);
        Layer* layerObj = currentMap->getLayer(layerType);

        if (checkBox && layerObj) {
            checkBox->blockSignals(true); // Prevent UI update from re-triggering signal
            checkBox->setChecked(layerObj->isVisible());
            checkBox->blockSignals(false);
        }
    }
}

// --- UI Event Handlers (Propagating changes to Map/MapScene) ---

void LayerWidget::onLayerSliderChanged(int value)
{
    // When slider changes, update combo box and emit `layerChanged` signal.
    layerCombo->blockSignals(true);
    // Find index in combo box corresponding to the slider value.
    int comboIndex = layerCombo->findData(value);
    if (comboIndex != -1) {
        layerCombo->setCurrentIndex(comboIndex);
    }
    layerCombo->blockSignals(false);

    layerLabel->setText(Layer::typeToString(static_cast<Layer::Type>(value))); // Update label.
    emit layerChanged(value); // Signal MapView about new active layer.
}

void LayerWidget::onLayerComboChanged(int index)
{
    // When combo box changes, update slider and emit `layerChanged` signal.
    int layerValue = layerCombo->itemData(index).toInt(); // Get integer value stored.
    layerSlider->blockSignals(true);
    layerSlider->setValue(layerValue);
    layerSlider->blockSignals(false);

    layerLabel->setText(Layer::typeToString(static_cast<Layer::Type>(layerValue))); // Update label.
    emit layerChanged(layerValue); // Signal MapView about new active layer.
}

void LayerWidget::onLayerCheckBoxToggled(bool checked)
{
    // When a visibility checkbox is toggled, update the corresponding `Layer` object in the Map.
    QCheckBox* checkBox = qobject_cast<QCheckBox*>(sender()); // Identify which checkbox emitted.
    if (checkBox) {
        Layer::Type layerType = static_cast<Layer::Type>(checkBox->property("LayerType").toInt());
        if (currentMap) {
            Layer* layerObj = currentMap->getLayer(layerType);
            if (layerObj) {
                layerObj->setVisible(checked); // Update Layer object (it will emit signal itself).
            }
        }
        // Emit own signal `layerVisibilityChanged` too, if something external relies directly on this widget's signals.
        emit layerVisibilityChanged(layerType, checked);
    }
}

void LayerWidget::setCurrentLayer(int layer)
{
    // Programmatic setter for current layer. Updates UI and propagates to MapView.
    if (layerCombo->blockSignals(true)) {
        layerCombo->setCurrentIndex(layerCombo->findData(layer));
        layerCombo->blockSignals(false);
    }
    if (layerSlider->blockSignals(true)) {
        layerSlider->setValue(layer);
        layerSlider->blockSignals(false);
    }
    layerLabel->setText(Layer::typeToString(static_cast<Layer::Type>(layer)));
    // Emitting layerChanged here would be redundant if triggered from onLayerComboChanged/onLayerSliderChanged.
    // Only emit if this setter is called externally bypassing those.
    // For consistency, let's emit:
    emit layerChanged(layer);
}

bool LayerWidget::isLayerVisible(Layer::Type type) const
{
    if (currentMap) {
        Layer* layerObj = currentMap->getLayer(type);
        if (layerObj) {
            return layerObj->isVisible();
        }
    }
    return false; // Default if no map or layer.
}

void LayerWidget::setLayerVisible(Layer::Type type, bool visible)
{
    if (currentMap) {
        Layer* layerObj = currentMap->getLayer(type);
        if (layerObj) {
            layerObj->setVisible(visible); // Updates Layer object and it signals for UI updates.
        }
    }
}

void LayerWidget::onMapLayerVisibilityChanged(Layer::Type type, bool visible)
{
    // Slot that responds to `Layer` object's own `visibilityChanged` signal.
    // Ensures LayerWidget's checkboxes are in sync if `Layer`'s visibility changed internally.
    QCheckBox* checkBox = layerVisibilityCheckboxes.value(type);
    if (checkBox && checkBox->isChecked() != visible) {
        checkBox->blockSignals(true); // Avoid loop.
        checkBox->setChecked(visible);
        checkBox->blockSignals(false);
    }
    // Also, could re-emit LayerWidget's signal here if needed for external listeners.
    emit layerVisibilityChanged(type, visible);
}