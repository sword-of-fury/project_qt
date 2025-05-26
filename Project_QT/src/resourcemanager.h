#ifndef RESOURCEMANAGER_H
#define RESOURCEMANAGER_H

#include <QObject>
#include <QPixmap>
#include <QIcon>
#include <QMap>
#include <QString>

class ResourceManager : public QObject
{
    Q_OBJECT

public:
    static ResourceManager& getInstance();
    ~ResourceManager();

    // Sprite management
    QPixmap getSprite(int id) const;
    bool loadSprites(const QString& directory);
    bool saveSprites(const QString& directory);
    int getSpriteCount() const;
    QList<int> getAllSpriteIds() const;

    // Icon management
    QIcon getIcon(const QString& name) const;
    bool loadIcons(const QString& directory);
    int getIconCount() const;
    QList<QString> getAllIconNames() const;

    // Texture management
    QPixmap getTexture(const QString& name) const;
    bool loadTextures(const QString& directory);
    int getTextureCount() const;
    QList<QString> getAllTextureNames() const;

signals:
    void spritesLoaded(int count);
    void iconsLoaded(int count);
    void texturesLoaded(int count);

private:
    ResourceManager(QObject* parent = nullptr);
    static ResourceManager* instance;

    QMap<int, QPixmap> m_sprites;
    QMap<QString, QIcon> m_icons;
    QMap<QString, QPixmap> m_textures;

    bool loadResourcesFromDirectory(const QString& directory, const QString& filter, QMap<QString, QPixmap>& resources);
    bool loadIconsFromDirectory(const QString& directory, const QString& filter);
};

#endif // RESOURCEMANAGER_H
