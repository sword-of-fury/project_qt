#include "toolspanel.h"
#include "itemmanager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QToolButton>
#include <QButtonGroup>
#include <QScrollArea>
#include <QIcon>
#include <QPushButton>

ToolsPanel::ToolsPanel(QWidget* parent)
    : QWidget(parent)
    , mainLayout(new QVBoxLayout(this))
    , toolsGroupBox(new QGroupBox(tr("Tools"), this))
    , layersGroupBox(new QGroupBox(tr("Layers"), this))
    , itemsGroupBox(new QGroupBox(tr("Items"), this))
    , toolsButtonGroup(new QButtonGroup(this))
    , layersButtonGroup(new QButtonGroup(this))
    , itemsButtonGroup(new QButtonGroup(this))
    , itemsScrollArea(new QScrollArea(this))
{
    setupUi();
}

ToolsPanel::~ToolsPanel()
{
}

void ToolsPanel::setupUi()
{
    // Ustawienia głównego układu
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Tools group
    QVBoxLayout* toolsLayout = new QVBoxLayout(toolsGroupBox);
    toolsLayout->setContentsMargins(5, 5, 5, 5);
    toolsLayout->setSpacing(2);

    // Tool buttons
    toolsLayout->addWidget(createToolButton(tr("Pencil"), QIcon(":/images/brush.png"), static_cast<int>(Brush::Type::Pencil)));
    toolsLayout->addWidget(createToolButton(tr("Eraser"), QIcon(":/images/eraser.png"), static_cast<int>(Brush::Type::Eraser)));
    toolsLayout->addWidget(createToolButton(tr("Fill"), QIcon(":/images/floodfill.png"), static_cast<int>(Brush::Type::FloodFill)));
    toolsLayout->addWidget(createToolButton(tr("Selection"), QIcon(":/images/selection.png"), static_cast<int>(Brush::Type::Selection)));
    toolsLayout->addStretch();

    // Layers group
    QVBoxLayout* layersLayout = new QVBoxLayout(layersGroupBox);
    layersLayout->setContentsMargins(5, 5, 5, 5);
    layersLayout->setSpacing(2);

    // Layer buttons
    for (int i = 0; i < 10; ++i) {
        layersLayout->addWidget(createLayerButton(i));
    }
    layersLayout->addStretch();

    // Items group
    QVBoxLayout* itemsLayout = new QVBoxLayout(itemsGroupBox);
    itemsLayout->setContentsMargins(5, 5, 5, 5);
    itemsLayout->setSpacing(2);

    // Item buttons
    for (Item* item : ItemManager::getInstance().getAllItems()) {
        itemsLayout->addWidget(createItemButton(item));
    }
    itemsLayout->addStretch();

    // Items scroll area
    itemsScrollArea->setWidget(itemsGroupBox);
    itemsScrollArea->setWidgetResizable(true);
    itemsScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // Add all groups to main layout
    mainLayout->addWidget(toolsGroupBox);
    mainLayout->addWidget(layersGroupBox);
    mainLayout->addWidget(itemsScrollArea);

    // Connect signals
    connect(toolsButtonGroup, &QButtonGroup::idClicked, this, &ToolsPanel::onToolButtonClicked);
    connect(layersButtonGroup, &QButtonGroup::idClicked, this, &ToolsPanel::onLayerButtonClicked);
    connect(itemsButtonGroup, &QButtonGroup::idClicked, this, &ToolsPanel::onItemButtonClicked);

    // Set default tool and layer
    toolsButtonGroup->button(static_cast<int>(Brush::Type::Pencil))->setChecked(true);
    layersButtonGroup->button(0)->setChecked(true);
}

QPushButton* ToolsPanel::createToolButton(const QString& text, const QIcon& icon, int id)
{
    QPushButton* button = new QPushButton(text, this);
    button->setIcon(icon);
    button->setIconSize(QSize(24, 24));
    button->setCheckable(true);
    button->setToolTip(text);
    toolsButtonGroup->addButton(button, id);
    return button;
}

QPushButton* ToolsPanel::createLayerButton(int layer)
{
    QPushButton* button = new QPushButton(tr("Layer %1").arg(layer), this);
    button->setCheckable(true);
    button->setToolTip(tr("Layer %1").arg(layer));
    layersButtonGroup->addButton(button, layer);
    return button;
}

QPushButton* ToolsPanel::createItemButton(Item* item)
{
    QPushButton* button = new QPushButton(item->getName(), this);
    button->setIcon(item->getIcon());
    button->setIconSize(QSize(24, 24));
    button->setCheckable(true);
    button->setToolTip(item->getName());
    itemsButtonGroup->addButton(button, itemsButtonGroup->buttons().size());
    return button;
}

void ToolsPanel::onToolButtonClicked(int id)
{
    emit toolChanged(static_cast<Brush::Type>(id));
}

void ToolsPanel::onLayerButtonClicked(int id)
{
    emit layerChanged(id);
}

void ToolsPanel::onItemButtonClicked(int id)
{
    QPushButton* button = itemsButtonGroup->button(id);
    if (button) {
        QString itemName = button->text();
        Item* item = ItemManager::getInstance().getItemByName(itemName);
        if (item) {
            emit itemSelected(item);
        }
    }
} 