#include "settingsmanager.h"
#include <QDebug>
#include <QDir>
#include <QStandardPaths>

// Initialize static members
SettingsManager* SettingsManager::instance = nullptr;
const QString SettingsManager::DEFAULT_DIRECTORY = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
const bool SettingsManager::DEFAULT_AUTO_SAVE = true;
const int SettingsManager::DEFAULT_AUTO_SAVE_INTERVAL = 5; // 5 minutes
const QSize SettingsManager::DEFAULT_WINDOW_SIZE = QSize(1024, 768);
const bool SettingsManager::DEFAULT_WINDOW_MAXIMIZED = false;
const int SettingsManager::DEFAULT_GRID_SIZE = 32;
const bool SettingsManager::DEFAULT_SHOW_GRID = true;
const QColor SettingsManager::DEFAULT_GRID_COLOR = QColor(128, 128, 128, 128);
const bool SettingsManager::DEFAULT_SNAP_TO_GRID = true;
const int SettingsManager::DEFAULT_BRUSH_SIZE = 1;
const int SettingsManager::DEFAULT_LAYER = 0;

SettingsManager& SettingsManager::getInstance()
{
    if (!instance) {
        instance = new SettingsManager();
    }
    return *instance;
}

SettingsManager::SettingsManager(QObject* parent)
    : QObject(parent)
{
    m_settings = new QSettings("IdlersMapEditor", "IdlersMapEditor", this);
    loadSettings();
}

SettingsManager::~SettingsManager()
{
    saveSettings();
    delete m_settings;
    
    if (instance == this) {
        instance = nullptr;
    }
}

void SettingsManager::loadSettings()
{
    qDebug() << "Loading settings from" << m_settings->fileName();
    
    // No need to explicitly load settings here as they are retrieved on demand
    // This method is provided for future expansion
    
    emit settingsChanged();
}

void SettingsManager::saveSettings()
{
    qDebug() << "Saving settings to" << m_settings->fileName();
    
    m_settings->sync();
}

void SettingsManager::resetToDefaults()
{
    m_settings->clear();
    
    // Set default values
    setLastOpenedDirectory(DEFAULT_DIRECTORY);
    setAutoSave(DEFAULT_AUTO_SAVE);
    setAutoSaveInterval(DEFAULT_AUTO_SAVE_INTERVAL);
    setWindowSize(DEFAULT_WINDOW_SIZE);
    setWindowMaximized(DEFAULT_WINDOW_MAXIMIZED);
    setGridSize(DEFAULT_GRID_SIZE);
    setShowGrid(DEFAULT_SHOW_GRID);
    setGridColor(DEFAULT_GRID_COLOR);
    setSnapToGrid(DEFAULT_SNAP_TO_GRID);
    setDefaultBrushSize(DEFAULT_BRUSH_SIZE);
    setDefaultLayer(DEFAULT_LAYER);
    
    saveSettings();
    emit settingsChanged();
}

// General settings
QString SettingsManager::getLastOpenedDirectory() const
{
    return m_settings->value("general/lastOpenedDirectory", DEFAULT_DIRECTORY).toString();
}

void SettingsManager::setLastOpenedDirectory(const QString& directory)
{
    if (QDir(directory).exists()) {
        m_settings->setValue("general/lastOpenedDirectory", directory);
        emit settingsChanged();
    }
}

bool SettingsManager::getAutoSave() const
{
    return m_settings->value("general/autoSave", DEFAULT_AUTO_SAVE).toBool();
}

void SettingsManager::setAutoSave(bool enabled)
{
    m_settings->setValue("general/autoSave", enabled);
    emit settingsChanged();
}

int SettingsManager::getAutoSaveInterval() const
{
    return m_settings->value("general/autoSaveInterval", DEFAULT_AUTO_SAVE_INTERVAL).toInt();
}

void SettingsManager::setAutoSaveInterval(int minutes)
{
    if (minutes > 0) {
        m_settings->setValue("general/autoSaveInterval", minutes);
        emit settingsChanged();
    }
}

// Window settings
QSize SettingsManager::getWindowSize() const
{
    return m_settings->value("window/size", DEFAULT_WINDOW_SIZE).toSize();
}

void SettingsManager::setWindowSize(const QSize& size)
{
    if (size.isValid() && !size.isNull()) {
        m_settings->setValue("window/size", size);
        emit settingsChanged();
    }
}

QPoint SettingsManager::getWindowPosition() const
{
    return m_settings->value("window/position", QPoint(100, 100)).toPoint();
}

void SettingsManager::setWindowPosition(const QPoint& position)
{
    m_settings->setValue("window/position", position);
    emit settingsChanged();
}

bool SettingsManager::getWindowMaximized() const
{
    return m_settings->value("window/maximized", DEFAULT_WINDOW_MAXIMIZED).toBool();
}

void SettingsManager::setWindowMaximized(bool maximized)
{
    m_settings->setValue("window/maximized", maximized);
    emit settingsChanged();
}

// Editor settings
int SettingsManager::getGridSize() const
{
    return m_settings->value("editor/gridSize", DEFAULT_GRID_SIZE).toInt();
}

void SettingsManager::setGridSize(int size)
{
    if (size > 0) {
        m_settings->setValue("editor/gridSize", size);
        emit settingsChanged();
    }
}

bool SettingsManager::getShowGrid() const
{
    return m_settings->value("editor/showGrid", DEFAULT_SHOW_GRID).toBool();
}

void SettingsManager::setShowGrid(bool show)
{
    m_settings->setValue("editor/showGrid", show);
    emit settingsChanged();
}

QColor SettingsManager::getGridColor() const
{
    return m_settings->value("editor/gridColor", DEFAULT_GRID_COLOR).value<QColor>();
}

void SettingsManager::setGridColor(const QColor& color)
{
    if (color.isValid()) {
        m_settings->setValue("editor/gridColor", color);
        emit settingsChanged();
    }
}

bool SettingsManager::getSnapToGrid() const
{
    return m_settings->value("editor/snapToGrid", DEFAULT_SNAP_TO_GRID).toBool();
}

void SettingsManager::setSnapToGrid(bool snap)
{
    m_settings->setValue("editor/snapToGrid", snap);
    emit settingsChanged();
}

// Tool settings
int SettingsManager::getDefaultBrushSize() const
{
    return m_settings->value("tools/defaultBrushSize", DEFAULT_BRUSH_SIZE).toInt();
}

void SettingsManager::setDefaultBrushSize(int size)
{
    if (size > 0) {
        m_settings->setValue("tools/defaultBrushSize", size);
        emit settingsChanged();
    }
}

int SettingsManager::getDefaultLayer() const
{
    return m_settings->value("tools/defaultLayer", DEFAULT_LAYER).toInt();
}

void SettingsManager::setDefaultLayer(int layer)
{
    if (layer >= 0) {
        m_settings->setValue("tools/defaultLayer", layer);
        emit settingsChanged();
    }
}
