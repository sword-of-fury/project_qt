#include "tibiafilehandler.h"
#include "spritemanager.h"

#include <QFileInfo>
#include <QDir>
#include <QBuffer>

TibiaFileHandler::TibiaFileHandler(QObject *parent)
    : QObject(parent)
    , m_spriteManager(new SpriteManager(this))
{
    // Initialize default client version
    m_clientVersion = {0, "Unknown"};
}

TibiaFileHandler::~TibiaFileHandler()
{
    // Cleanup is handled by Qt's parent-child hierarchy
}

bool TibiaFileHandler::loadDatFile(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        emit errorOccurred(tr("Cannot open DAT file: %1").arg(filename));
        return false;
    }

    QDataStream in(&file);
    in.setByteOrder(QDataStream::LittleEndian);

    if (!readDatHeader(in)) {
        emit errorOccurred(tr("Invalid DAT file format"));
        file.close();
        return false;
    }

    // Read items
    for (quint32 i = 0; i < m_datHeader.itemCount; ++i) {
        // Read item properties from DAT file
        // Implementation depends on the specific DAT format version
        
        // Update progress
        if (i % 100 == 0) {
            emit progressChanged(i, m_datHeader.itemCount, 
                              tr("Loading items: %1/%2").arg(i).arg(m_datHeader.itemCount));
        }
    }

    file.close();
    emit progressChanged(m_datHeader.itemCount, m_datHeader.itemCount, tr("Items loaded successfully"));
    return true;
}

bool TibiaFileHandler::loadSprFile(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        emit errorOccurred(tr("Cannot open SPR file: %1").arg(filename));
        return false;
    }

    QDataStream in(&file);
    in.setByteOrder(QDataStream::LittleEndian);

    if (!readSprHeader(in)) {
        emit errorOccurred(tr("Invalid SPR file format"));
        file.close();
        return false;
    }

    // Read sprite data
    for (quint32 i = 0; i < m_sprHeader.spriteCount; ++i) {
        // Read sprite data
        SpriteData sprite;
        
        // Read width and height
        in >> sprite.width >> sprite.height;
        
        // Read pixel data (format depends on SPR version)
        quint32 dataSize;
        in >> dataSize;
        
        sprite.data.resize(dataSize);
        in.readRawData(sprite.data.data(), dataSize);
        
        // Store sprite
        m_sprites[i] = sprite;
        
        // Update progress
        if (i % 100 == 0) {
            emit progressChanged(i, m_sprHeader.spriteCount, 
                              tr("Loading sprites: %1/%2").arg(i).arg(m_sprHeader.spriteCount));
        }
    }

    file.close();
    emit progressChanged(m_sprHeader.spriteCount, m_sprHeader.spriteCount, 
                        tr("Sprites loaded successfully"));
    return true;
}

bool TibiaFileHandler::loadOtbFile(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        emit errorOccurred(tr("Cannot open OTB file: %1").arg(filename));
        return false;
    }

    QDataStream in(&file);
    in.setByteOrder(QDataStream::LittleEndian);

    if (!readOtbHeader(in)) {
        emit errorOccurred(tr("Invalid OTB file format"));
        file.close();
        return false;
    }

    // Read items
    while (!in.atEnd()) {
        // Read OTB item data
        // Implementation depends on the specific OTB format version
        
        // Update progress (if possible to determine total items)
        static quint32 count = 0;
        count++;
        if (count % 100 == 0) {
            emit progressChanged(count, 0, tr("Loading items: %1").arg(count));
        }
    }

    file.close();
    emit progressChanged(100, 100, tr("OTB data loaded successfully"));
    return true;
}

QImage TibiaFileHandler::getSprite(quint32 spriteId) const
{
    if (!m_sprites.contains(spriteId)) {
        qWarning() << "Sprite ID not found:" << spriteId;
        return QImage();
    }
    
    // Lazy-load the image if not already loaded
    SpriteData &sprite = const_cast<SpriteData&>(m_sprites[spriteId]);
    if (!sprite.loaded) {
        // Convert raw sprite data to QImage
        // Implementation depends on the sprite format
        // sprite.image = convertSpriteData(sprite.data, sprite.width, sprite.height);
        sprite.loaded = true;
    }
    
    return sprite.image;
}

bool TibiaFileHandler::hasSprite(quint32 spriteId) const
{
    return m_sprites.contains(spriteId);
}

const TibiaFileHandler::ItemProperties* TibiaFileHandler::getItemProperties(quint16 itemId) const
{
    auto it = m_items.find(itemId);
    return (it != m_items.end()) ? &it.value() : nullptr;
}

QList<quint16> TibiaFileHandler::getItemIds() const
{
    return m_items.keys();
}

TibiaFileHandler::ClientVersion TibiaFileHandler::getClientVersion() const
{
    return m_clientVersion;
}

bool TibiaFileHandler::readDatHeader(QDataStream &in)
{
    // Read DAT header based on the specific format version
    // This is a simplified version - adjust according to actual format
    quint32 signature;
    in >> signature;
    
    if (signature != 0x4D4D4D4D) { // Example signature
        return false;
    }
    
    in >> m_datHeader.version
       >> m_datHeader.itemCount
       >> m_datHeader.outfitCount
       >> m_datHeader.effectCount
       >> m_datHeader.projectileCount;
    
    return true;
}

bool TibiaFileHandler::readSprHeader(QDataStream &in)
{
    // Read SPR header based on the specific format version
    // This is a simplified version - adjust according to actual format
    quint32 signature;
    in >> signature;
    
    if (signature != 0x4D4D4D4D) { // Example signature
        return false;
    }
    
    in >> m_sprHeader.version
       >> m_sprHeader.spriteCount
       >> m_sprHeader.spriteSize;
    
    return true;
}

bool TibiaFileHandler::readOtbHeader(QDataStream &in)
{
    // Read OTB header based on the specific format version
    // This is a simplified version - adjust according to actual format
    char identifier[4];
    in.readRawData(identifier, 4);
    
    if (memcmp(identifier, "OTBI", 4) != 0) {
        return false;
    }
    
    in >> m_otbHeader.version
       >> m_otbHeader.majorVersion
       >> m_otbHeader.minorVersion
       >> m_otbHeader.buildNumber
       >> m_otbHeader.itemCount;
    
    return true;
}
