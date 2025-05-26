#include "brushmanager.h"
#include "pencilbrush.h"
#include "eraserbrush.h"
#include "floodfillbrush.h"
#include "selectionbrush.h"
#include <QDebug>

BrushManager* BrushManager::instance = nullptr;

BrushManager& BrushManager::getInstance()
{
    if (!instance) {
        instance = new BrushManager();
    }
    return *instance;
}

BrushManager::BrushManager(QObject* parent)
    : QObject(parent)
{
    initializeTypeNames();
    initializeTypeIcons();
}

BrushManager::~BrushManager()
{
    qDeleteAll(m_brushes);
    m_brushes.clear();
    
    if (instance == this) {
        instance = nullptr;
    }
}

void BrushManager::initializeTypeNames()
{
    m_typeNames[Brush::Type::Pencil] = tr("Pencil");
    m_typeNames[Brush::Type::Eraser] = tr("Eraser");
    m_typeNames[Brush::Type::FloodFill] = tr("Flood Fill");
    m_typeNames[Brush::Type::Selection] = tr("Selection");
}

void BrushManager::initializeTypeIcons()
{
    m_typeIcons[Brush::Type::Pencil] = QIcon(":/icons/pencil.png");
    m_typeIcons[Brush::Type::Eraser] = QIcon(":/icons/eraser.png");
    m_typeIcons[Brush::Type::FloodFill] = QIcon(":/icons/fill.png");
    m_typeIcons[Brush::Type::Selection] = QIcon(":/icons/selection.png");
}

Brush* BrushManager::createBrush(Brush::Type type, QObject* parent)
{
    // Check if brush already exists
    if (m_brushes.contains(type)) {
        return m_brushes[type];
    }
    
    // Create new brush based on type
    Brush* brush = nullptr;
    
    switch (type) {
        case Brush::Type::Pencil:
            brush = new PencilBrush(parent);
            break;
        case Brush::Type::Eraser:
            brush = new EraserBrush(parent);
            break;
        case Brush::Type::FloodFill:
            brush = new FloodFillBrush(parent);
            break;
        case Brush::Type::Selection:
            brush = new SelectionBrush(parent);
            break;
        default:
            qWarning() << "Unknown brush type:" << static_cast<int>(type);
            return nullptr;
    }
    
    // Set brush properties
    brush->setType(type);
    brush->setName(getTypeName(type));
    brush->setIcon(getTypeIcon(type));
    
    // Add to map
    m_brushes[type] = brush;
    
    // Connect signals
    connect(brush, &Brush::changed, this, [this, brush]() {
        emit brushChanged(brush);
    });
    
    // Emit signal
    emit brushCreated(brush);
    
    return brush;
}

Brush* BrushManager::getBrush(Brush::Type type) const
{
    return m_brushes.value(type, nullptr);
}

QList<Brush*> BrushManager::getAllBrushes() const
{
    return m_brushes.values();
}

QString BrushManager::getTypeName(Brush::Type type) const
{
    return m_typeNames.value(type, tr("Unknown"));
}

QIcon BrushManager::getTypeIcon(Brush::Type type) const
{
    return m_typeIcons.value(type, QIcon());
}
