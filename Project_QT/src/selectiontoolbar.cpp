#include "selectiontoolbar.h"
#include "mapview.h"
#include "selectionbrush.h"
#include <QAction>
#include <QSpinBox>
#include <QComboBox>
#include <QLabel>
#include <QHBoxLayout>
#include <QWidget>
#include <QIcon>
#include <QDebug>

SelectionToolbar::SelectionToolbar(const QString& title, QWidget* parent)
    : QToolBar(title, parent)
    , mapView(nullptr)
    , selectionBrush(nullptr)
{
    setupActions();
    updateActionStates(false);
}

SelectionToolbar::~SelectionToolbar()
{
}

void SelectionToolbar::setupActions()
{
    // Create selection mode combo box
    selectionModeCombo = new QComboBox(this);
    selectionModeCombo->addItem("Replace", SelectionBrush::Replace);
    selectionModeCombo->addItem("Add", SelectionBrush::Add);
    selectionModeCombo->addItem("Subtract", SelectionBrush::Subtract);
    selectionModeCombo->addItem("Toggle", SelectionBrush::Toggle);
    selectionModeCombo->setToolTip(tr("Selection Mode"));
    connect(selectionModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &SelectionToolbar::onSelectionModeChanged);
    addWidget(selectionModeCombo);
    
    addSeparator();
    
    // Create move action
    moveAction = new QAction(QIcon(":/icons/move.png"), tr("Move Selection"), this);
    moveAction->setToolTip(tr("Move the current selection"));
    connect(moveAction, &QAction::triggered, this, &SelectionToolbar::onMoveSelection);
    addAction(moveAction);
    
    // Create copy action
    copyAction = new QAction(QIcon(":/icons/copy.png"), tr("Copy Selection"), this);
    copyAction->setToolTip(tr("Copy the current selection"));
    connect(copyAction, &QAction::triggered, this, &SelectionToolbar::onCopySelection);
    addAction(copyAction);
    
    // Create offset spinboxes
    QWidget* offsetWidget = new QWidget(this);
    QHBoxLayout* offsetLayout = new QHBoxLayout(offsetWidget);
    offsetLayout->setContentsMargins(2, 2, 2, 2);
    offsetLayout->setSpacing(2);
    
    QLabel* offsetLabel = new QLabel(tr("Offset:"), offsetWidget);
    offsetLayout->addWidget(offsetLabel);
    
    offsetXSpin = new QSpinBox(offsetWidget);
    offsetXSpin->setRange(-1000, 1000);
    offsetXSpin->setValue(1);
    offsetXSpin->setToolTip(tr("X Offset"));
    offsetLayout->addWidget(offsetXSpin);
    
    offsetYSpin = new QSpinBox(offsetWidget);
    offsetYSpin->setRange(-1000, 1000);
    offsetYSpin->setValue(1);
    offsetYSpin->setToolTip(tr("Y Offset"));
    offsetLayout->addWidget(offsetYSpin);
    
    addWidget(offsetWidget);
    
    addSeparator();
    
    // Create rotate action
    rotateAction = new QAction(QIcon(":/icons/rotate.png"), tr("Rotate Selection"), this);
    rotateAction->setToolTip(tr("Rotate the current selection"));
    connect(rotateAction, &QAction::triggered, this, &SelectionToolbar::onRotateSelection);
    addAction(rotateAction);
    
    // Create rotation combo box
    rotationCombo = new QComboBox(this);
    rotationCombo->addItem("90°", 90);
    rotationCombo->addItem("180°", 180);
    rotationCombo->addItem("270°", 270);
    rotationCombo->setToolTip(tr("Rotation Angle"));
    addWidget(rotationCombo);
    
    addSeparator();
    
    // Create flip actions
    flipHAction = new QAction(QIcon(":/icons/flip_h.png"), tr("Flip Horizontally"), this);
    flipHAction->setToolTip(tr("Flip the selection horizontally"));
    connect(flipHAction, &QAction::triggered, this, &SelectionToolbar::onFlipHorizontally);
    addAction(flipHAction);
    
    flipVAction = new QAction(QIcon(":/icons/flip_v.png"), tr("Flip Vertically"), this);
    flipVAction->setToolTip(tr("Flip the selection vertically"));
    connect(flipVAction, &QAction::triggered, this, &SelectionToolbar::onFlipVertically);
    addAction(flipVAction);
    
    addSeparator();
    
    // Create delete action
    deleteAction = new QAction(QIcon(":/icons/delete.png"), tr("Delete Selection"), this);
    deleteAction->setToolTip(tr("Delete the current selection"));
    connect(deleteAction, &QAction::triggered, this, &SelectionToolbar::onDeleteSelection);
    addAction(deleteAction);
    
    addSeparator();
    
    // Create selection info label
    selectionInfoLabel = new QLabel(tr("No Selection"), this);
    addWidget(selectionInfoLabel);
}

void SelectionToolbar::setMapView(MapView* view)
{
    mapView = view;
}

void SelectionToolbar::setSelectionBrush(SelectionBrush* brush)
{
    if (selectionBrush) {
        disconnect(selectionBrush, &SelectionBrush::selectionChanged, 
                   this, &SelectionToolbar::onSelectionChanged);
        disconnect(selectionBrush, &SelectionBrush::selectionCleared, 
                   this, &SelectionToolbar::onSelectionCleared);
    }
    
    selectionBrush = brush;
    
    if (selectionBrush) {
        connect(selectionBrush, &SelectionBrush::selectionChanged, 
                this, &SelectionToolbar::onSelectionChanged);
        connect(selectionBrush, &SelectionBrush::selectionCleared, 
                this, &SelectionToolbar::onSelectionCleared);
        
        // Set initial selection mode
        int index = selectionModeCombo->findData(selectionBrush->getSelectionMode());
        if (index >= 0) {
            selectionModeCombo->setCurrentIndex(index);
        }
    }
}

void SelectionToolbar::updateActionStates(bool hasSelection)
{
    moveAction->setEnabled(hasSelection);
    copyAction->setEnabled(hasSelection);
    rotateAction->setEnabled(hasSelection);
    flipHAction->setEnabled(hasSelection);
    flipVAction->setEnabled(hasSelection);
    deleteAction->setEnabled(hasSelection);
    offsetXSpin->setEnabled(hasSelection);
    offsetYSpin->setEnabled(hasSelection);
    rotationCombo->setEnabled(hasSelection);
}

void SelectionToolbar::onMoveSelection()
{
    if (!mapView || !selectionBrush) return;
    
    QPoint offset(offsetXSpin->value(), offsetYSpin->value());
    selectionBrush->moveSelection(mapView, offset);
}

void SelectionToolbar::onCopySelection()
{
    if (!mapView || !selectionBrush) return;
    
    QPoint offset(offsetXSpin->value(), offsetYSpin->value());
    selectionBrush->moveSelection(mapView, offset, true);
}

void SelectionToolbar::onRotateSelection()
{
    if (!mapView || !selectionBrush) return;
    
    int degrees = rotationCombo->currentData().toInt();
    selectionBrush->rotateSelection(mapView, degrees);
}

void SelectionToolbar::onFlipHorizontally()
{
    if (!mapView || !selectionBrush) return;
    
    selectionBrush->flipSelectionHorizontally(mapView);
}

void SelectionToolbar::onFlipVertically()
{
    if (!mapView || !selectionBrush) return;
    
    selectionBrush->flipSelectionVertically(mapView);
}

void SelectionToolbar::onDeleteSelection()
{
    if (!mapView || !selectionBrush) return;
    
    selectionBrush->deleteSelection(mapView);
}

void SelectionToolbar::onSelectionModeChanged(int index)
{
    if (!selectionBrush) return;
    
    SelectionBrush::SelectionMode mode = static_cast<SelectionBrush::SelectionMode>(
        selectionModeCombo->itemData(index).toInt());
    selectionBrush->setSelectionMode(mode);
}

void SelectionToolbar::onSelectionChanged(const QRect& selection)
{
    updateActionStates(!selection.isEmpty());
    
    if (!selection.isEmpty()) {
        selectionInfoLabel->setText(tr("Selection: %1x%2")
            .arg(selection.width())
            .arg(selection.height()));
    } else {
        selectionInfoLabel->setText(tr("No Selection"));
    }
}

void SelectionToolbar::onSelectionCleared()
{
    updateActionStates(false);
    selectionInfoLabel->setText(tr("No Selection"));
}
