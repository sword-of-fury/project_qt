#include "resourcemanager.h"
#include <QDir>
#include <QDebug>
#include <QFileInfo>

ResourceManager* ResourceManager::instance = nullptr;

ResourceManager& ResourceManager::getInstance()
{
    if (!instance) {
        instance = new ResourceManager();
    }
    return *instance;
}

ResourceManager::ResourceManager(QObject* parent)
    : QObject(parent)
{
}

ResourceManager::~ResourceManager()
{
    m_sprites.clear();
    m_icons.clear();
    m_textures.clear();
    
    if (instance == this) {
        instance = nullptr;
    }
}

QPixmap ResourceManager::getSprite(int id) const
{
    return m_sprites.value(id, QPixmap());
}

bool ResourceManager::loadSprites(const QString& directory)
{
    QDir dir(directory);
    if (!dir.exists()) {
        qWarning() << "Sprite directory does not exist:" << directory;
        return false;
    }
    
    // Clear existing sprites
    m_sprites.clear();
    
    // Load sprites from directory
    QStringList filters;
    filters << "*.png" << "*.jpg" << "*.bmp";
    dir.setNameFilters(filters);
    
    QFileInfoList files = dir.entryInfoList(QDir::Files);
    for (const QFileInfo& file : files) {
        // Extract ID from filename (e.g., "sprite_123.png" -> 123)
        QString filename = file.baseName();
        if (filename.startsWith("sprite_")) {
            bool ok;
            int id = filename.mid(7).toInt(&ok);
            if (ok) {
                QPixmap sprite(file.absoluteFilePath());
                if (!sprite.isNull()) {
                    m_sprites[id] = sprite;
                }
            }
        }
    }
    
    qDebug() << "Loaded" << m_sprites.size() << "sprites from" << directory;
    emit spritesLoaded(m_sprites.size());
    
    return !m_sprites.isEmpty();
}

bool ResourceManager::saveSprites(const QString& directory)
{
    QDir dir(directory);
    if (!dir.exists()) {
        if (!dir.mkpath(".")) {
            qWarning() << "Could not create sprite directory:" << directory;
            return false;
        }
    }
    
    // Save sprites to directory
    int savedCount = 0;
    for (auto it = m_sprites.constBegin(); it != m_sprites.constEnd(); ++it) {
        QString filename = QString("%1/sprite_%2.png").arg(directory).arg(it.key());
        if (it.value().save(filename)) {
            savedCount++;
        }
    }
    
    qDebug() << "Saved" << savedCount << "sprites to" << directory;
    
    return savedCount > 0;
}

int ResourceManager::getSpriteCount() const
{
    return m_sprites.size();
}

QList<int> ResourceManager::getAllSpriteIds() const
{
    return m_sprites.keys();
}

QIcon ResourceManager::getIcon(const QString& name) const
{
    return m_icons.value(name, QIcon());
}

bool ResourceManager::loadIcons(const QString& directory)
{
    return loadIconsFromDirectory(directory, "*.png");
}

int ResourceManager::getIconCount() const
{
    return m_icons.size();
}

QList<QString> ResourceManager::getAllIconNames() const
{
    return m_icons.keys();
}

QPixmap ResourceManager::getTexture(const QString& name) const
{
    return m_textures.value(name, QPixmap());
}

bool ResourceManager::loadTextures(const QString& directory)
{
    QMap<QString, QPixmap> textures;
    bool result = loadResourcesFromDirectory(directory, "*.png", textures);
    
    if (result) {
        m_textures = textures;
        emit texturesLoaded(m_textures.size());
    }
    
    return result;
}

int ResourceManager::getTextureCount() const
{
    return m_textures.size();
}

QList<QString> ResourceManager::getAllTextureNames() const
{
    return m_textures.keys();
}

bool ResourceManager::loadResourcesFromDirectory(const QString& directory, const QString& filter, QMap<QString, QPixmap>& resources)
{
    QDir dir(directory);
    if (!dir.exists()) {
        qWarning() << "Resource directory does not exist:" << directory;
        return false;
    }
    
    // Clear existing resources
    resources.clear();
    
    // Load resources from directory
    QStringList filters;
    filters << filter;
    dir.setNameFilters(filters);
    
    QFileInfoList files = dir.entryInfoList(QDir::Files);
    for (const QFileInfo& file : files) {
        QString name = file.baseName();
        QPixmap resource(file.absoluteFilePath());
        if (!resource.isNull()) {
            resources[name] = resource;
        }
    }
    
    qDebug() << "Loaded" << resources.size() << "resources from" << directory;
    
    return !resources.isEmpty();
}

bool ResourceManager::loadIconsFromDirectory(const QString& directory, const QString& filter)
{
    QDir dir(directory);
    if (!dir.exists()) {
        qWarning() << "Icon directory does not exist:" << directory;
        return false;
    }
    
    // Clear existing icons
    m_icons.clear();
    
    // Load icons from directory
    QStringList filters;
    filters << filter;
    dir.setNameFilters(filters);
    
    QFileInfoList files = dir.entryInfoList(QDir::Files);
    for (const QFileInfo& file : files) {
        QString name = file.baseName();
        QIcon icon(file.absoluteFilePath());
        if (!icon.isNull()) {
            m_icons[name] = icon;
        }
    }
    
    qDebug() << "Loaded" << m_icons.size() << "icons from" << directory;
    emit iconsLoaded(m_icons.size());
    
    return !m_icons.isEmpty();
}
