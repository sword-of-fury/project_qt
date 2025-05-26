#include "propertyeditordock.h"
#include "tilepropertyeditor.h"
#include "itempropertyeditor.h"
#include "creaturepropertyeditor.h"
#include "tile.h"
#include "item.h"
#include "creature.h"
#include <QVBoxLayout>

PropertyEditorDock::PropertyEditorDock(const QString& title, QWidget* parent)
    : QDockWidget(title, parent)
{
    setupUi();
}

PropertyEditorDock::~PropertyEditorDock()
{
}

void PropertyEditorDock::setupUi()
{
    // Create a widget to hold the stacked widget
    QWidget* widget = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(widget);
    
    // Create stacked widget
    stackedWidget = new QStackedWidget(widget);
    
    // Create "no selection" label
    noSelectionLabel = new QLabel(tr("No object selected"), widget);
    noSelectionLabel->setAlignment(Qt::AlignCenter);
    stackedWidget->addWidget(noSelectionLabel);
    
    // Create property editors
    tileEditor = new TilePropertyEditor(widget);
    stackedWidget->addWidget(tileEditor);
    
    itemEditor = new ItemPropertyEditor(widget);
    stackedWidget->addWidget(itemEditor);
    
    creatureEditor = new CreaturePropertyEditor(widget);
    stackedWidget->addWidget(creatureEditor);
    
    // Add stacked widget to layout
    layout->addWidget(stackedWidget);
    
    // Set widget as dock widget content
    setWidget(widget);
    
    // Show "no selection" label initially
    stackedWidget->setCurrentWidget(noSelectionLabel);
}

void PropertyEditorDock::setTile(Tile* tile)
{
    if (tile) {
        tileEditor->setTile(tile);
        stackedWidget->setCurrentWidget(tileEditor);
    } else {
        clearEditors();
    }
}

void PropertyEditorDock::setItem(Item* item)
{
    if (item) {
        itemEditor->setItem(item);
        stackedWidget->setCurrentWidget(itemEditor);
    } else {
        clearEditors();
    }
}

void PropertyEditorDock::setCreature(Creature* creature)
{
    if (creature) {
        creatureEditor->setCreature(creature);
        stackedWidget->setCurrentWidget(creatureEditor);
    } else {
        clearEditors();
    }
}

void PropertyEditorDock::clearEditors()
{
    tileEditor->setTile(nullptr);
    itemEditor->setItem(nullptr);
    creatureEditor->setCreature(nullptr);
    stackedWidget->setCurrentWidget(noSelectionLabel);
}
