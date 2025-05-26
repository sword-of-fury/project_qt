#ifndef TIBIAFILEHANDLER_H
#define TIBIAFILEHANDLER_H

#include <QObject>
#include <QFile>
#include <QDataStream>
#include <QVector>
#include <QImage>
#include <QMap>
#include <QDebug>

// Forward declarations
class SpriteManager;

class TibiaFileHandler : public QObject
{
    Q_OBJECT

public:
    explicit TibiaFileHandler(QObject *parent = nullptr);
    virtual ~TibiaFileHandler();

    // File loading methods
    bool loadDatFile(const QString &filename);
    bool loadSprFile(const QString &filename);
    bool loadOtbFile(const QString &filename);

    // Sprite access
    QImage getSprite(quint32 spriteId) const;
    bool hasSprite(quint32 spriteId) const;

    // Item properties access
    struct ItemProperties {
        quint16 id;
        QString name;
        quint32 spriteId;
        bool stackable;
        bool container;
        bool fluidContainer;
        bool blocking;
        bool walkable;
        bool collidable;
        quint32 weight;
        quint32 speed;
        // Add more properties as needed
    };

    const ItemProperties* getItemProperties(quint16 itemId) const;
    QList<quint16> getItemIds() const;

    // Client version information
    struct ClientVersion {
        quint32 version;
        QString versionString;
    };

    ClientVersion getClientVersion() const;

signals:
    void progressChanged(int value, int maximum, const QString &message);
    void errorOccurred(const QString &error);
    void loaded(bool success);

private:
    // Internal data structures
    struct DatHeader {
        quint32 version;
        quint32 itemCount;
        quint32 outfitCount;
        quint32 effectCount;
        quint32 projectileCount;
    };

    struct SpriteData {
        QByteArray data;
        quint32 width;
        quint32 height;
        QImage image;
        bool loaded;
    };

    // File format specific readers
    bool readDatHeader(QDataStream &in);
    bool readSprHeader(QDataStream &in);
    bool readOtbHeader(QDataStream &in);

    // Data storage
    QMap<quint16, ItemProperties> m_items;
    QMap<quint32, SpriteData> m_sprites;
    QMap<quint16, quint16> m_clientToServerMap;
    QMap<quint16, quint16> m_serverToClientMap;
    ClientVersion m_clientVersion;
    
    // Sprite manager for handling sprite operations
    SpriteManager *m_spriteManager;
};

#endif // TIBIAFILEHANDLER_H
