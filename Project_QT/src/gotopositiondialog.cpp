#include "gotopositiondialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QMessageBox>
#include "mainwindow.h"

GotoPositionDialog::GotoPositionDialog(QWidget* parent, Map* map)
    : QDialog(parent),
      map(map)
{
    setWindowTitle(tr("Go to Position"));
    setupUi();
}

GotoPositionDialog::~GotoPositionDialog()
{
}

void GotoPositionDialog::setupUi()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // Position group
    QGroupBox* positionGroup = new QGroupBox(tr("Position"), this);
    QFormLayout* positionLayout = new QFormLayout(positionGroup);
    
    xSpinBox = new QSpinBox(this);
    xSpinBox->setRange(0, map ? map->getSize().width() - 1 : 2048);
    xSpinBox->setSingleStep(1);
    positionLayout->addRow(tr("X:"), xSpinBox);
    
    ySpinBox = new QSpinBox(this);
    ySpinBox->setRange(0, map ? map->getSize().height() - 1 : 2048);
    ySpinBox->setSingleStep(1);
    positionLayout->addRow(tr("Y:"), ySpinBox);
    
    zSpinBox = new QSpinBox(this);
    zSpinBox->setRange(0, 15);
    zSpinBox->setSingleStep(1);
    positionLayout->addRow(tr("Z (Floor):"), zSpinBox);
    
    // Button box
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    
    currentPositionButton = new QPushButton(tr("Current Position"), this);
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    
    buttonLayout->addWidget(currentPositionButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(buttonBox);
    
    // Add widgets to main layout
    mainLayout->addWidget(positionGroup);
    mainLayout->addLayout(buttonLayout);
    
    // Connect signals
    connect(buttonBox, &QDialogButtonBox::accepted, this, &GotoPositionDialog::onOkClicked);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &GotoPositionDialog::onCancelClicked);
    connect(currentPositionButton, &QPushButton::clicked, this, &GotoPositionDialog::onCurrentPositionClicked);
}

QPoint GotoPositionDialog::getPosition() const
{
    return QPoint(xSpinBox->value(), ySpinBox->value());
}

int GotoPositionDialog::getLayer() const
{
    return zSpinBox->value();
}

void GotoPositionDialog::onOkClicked()
{
    if (!map) {
        QMessageBox::warning(this, tr("Error"), tr("No map is currently loaded."));
        return;
    }
    
    QPoint pos = getPosition();
    int layer = getLayer();
    
    // Check if position is valid
    if (pos.x() < 0 || pos.x() >= map->getSize().width() ||
        pos.y() < 0 || pos.y() >= map->getSize().height() ||
        layer < 0 || layer >= 16) {
        QMessageBox::warning(this, tr("Invalid Position"), 
                           tr("The specified position is outside the map boundaries."));
        return;
    }
    
    accept();
}

void GotoPositionDialog::onCancelClicked()
{
    reject();
}

void GotoPositionDialog::onCurrentPositionClicked()
{
    MainWindow* mainWindow = qobject_cast<MainWindow*>(parent());
    if (mainWindow) {
        QPoint currentPos = mainWindow->getCurrentPosition();
        int currentLayer = mainWindow->getCurrentLayer();
        
        xSpinBox->setValue(currentPos.x());
        ySpinBox->setValue(currentPos.y());
        zSpinBox->setValue(currentLayer);
    }
}
