#include "spritemanager.h"
#include <QPainter>
#include <QFile>
#include <QDataStream>
#include <QDebug>

// Implementacja Animator
Animator::Animator(int frameCount, int startFrame, int loopCount, bool async) :
    frameCount(frameCount),
    startFrame(startFrame),
    loopCount(loopCount),
    async(async),
    currentFrame(0),
    currentLoop(0),
    currentDuration(0),
    totalDuration(0),
    direction(ANIMATION_FORWARD),
    lastTime(0),
    isComplete(false)
{
    Q_ASSERT(startFrame >= -1 && startFrame < frameCount);

    for (int i = 0; i < frameCount; i++) {
        durations.push_back(new FrameDuration(ITEM_FRAME_DURATION, ITEM_FRAME_DURATION));
    }

    reset();
}

Animator::~Animator() {
    for (int i = 0; i < frameCount; i++) {
        delete durations[i];
    }
    durations.clear();
}

int Animator::getStartFrame() const {
    if (startFrame > -1) {
        return startFrame;
    }
    return qrand() % frameCount;
}

FrameDuration* Animator::getFrameDuration(int frame) {
    Q_ASSERT(frame >= 0 && frame < frameCount);
    return durations[frame];
}

int Animator::getFrame() {
    qint64 time = QDateTime::currentMSecsSinceEpoch();
    if (time != lastTime && !isComplete) {
        qint64 elapsed = time - lastTime;
        if (elapsed >= currentDuration) {
            int frame = 0;
            if (loopCount < 0) {
                frame = getPingPongFrame();
            } else {
                frame = getLoopFrame();
            }

            if (currentFrame != frame) {
                int duration = getDuration(frame) - (elapsed - currentDuration);
                if (duration < 0 && !async) {
                    calculateSynchronous();
                } else {
                    currentFrame = frame;
                    currentDuration = qMax(0, duration);
                }
            } else {
                isComplete = true;
            }
        } else {
            currentDuration -= elapsed;
        }

        lastTime = time;
    }
    return currentFrame;
}

void Animator::setFrame(int frame) {
    Q_ASSERT(frame == -1 || frame == 255 || frame == 254 || (frame >= 0 && frame < frameCount));

    if (currentFrame == frame) {
        return;
    }

    if (async) {
        if (frame == 255) { // Async mode
            currentFrame = 0;
        } else if (frame == 254) { // Random mode
            currentFrame = qrand() % frameCount;
        } else if (frame >= 0 && frame < frameCount) {
            currentFrame = frame;
        } else {
            currentFrame = getStartFrame();
        }

        isComplete = false;
        lastTime = QDateTime::currentMSecsSinceEpoch();
        currentDuration = getDuration(currentFrame);
        currentLoop = 0;
    } else {
        calculateSynchronous();
    }
}

void Animator::reset() {
    totalDuration = 0;
    for (int i = 0; i < frameCount; i++) {
        totalDuration += durations[i]->max;
    }

    isComplete = false;
    direction = ANIMATION_FORWARD;
    currentLoop = 0;
    async = false;
    setFrame(-1);
}

int Animator::getDuration(int frame) const {
    Q_ASSERT(frame >= 0 && frame < frameCount);
    return durations[frame]->getDuration();
}

int Animator::getPingPongFrame() {
    int count = direction == ANIMATION_FORWARD ? 1 : -1;
    int nextFrame = currentFrame + count;
    if (nextFrame < 0 || nextFrame >= frameCount) {
        direction = direction == ANIMATION_FORWARD ? ANIMATION_BACKWARD : ANIMATION_FORWARD;
        count *= -1;
    }
    return currentFrame + count;
}

int Animator::getLoopFrame() {
    int nextPhase = currentFrame + 1;
    if (nextPhase < frameCount) {
        return nextPhase;
    }

    if (loopCount == 0) {
        return 0;
    }

    if (currentLoop < (loopCount - 1)) {
        currentLoop++;
        return 0;
    }
    return currentFrame;
}

void Animator::calculateSynchronous() {
    qint64 time = QDateTime::currentMSecsSinceEpoch();
    if (time > 0 && totalDuration > 0) {
        qint64 elapsed = time % totalDuration;
        int totalTime = 0;
        for (int i = 0; i < frameCount; i++) {
            int duration = getDuration(i);
            if (elapsed >= totalTime && elapsed < totalTime + duration) {
                currentFrame = i;
                currentDuration = duration - (elapsed - totalTime);
                break;
            }
            totalTime += duration;
        }
        lastTime = time;
    }
}

// Implementacja Sprite
Sprite::Sprite() {
}

Sprite::~Sprite() {
}

// Implementacja EditorSprite
EditorSprite::EditorSprite(QImage* b16x16, QImage* b32x32, QImage* b64x64) {
    bm[SPRITE_SIZE_16x16] = b16x16;
    bm[SPRITE_SIZE_32x32] = b32x32;
    bm[SPRITE_SIZE_64x64] = b64x64;
}

EditorSprite::~EditorSprite() {
    unloadDC();
}

void EditorSprite::drawTo(QPainter* painter, SpriteSize size, int startX, int startY, int width, int height) {
    if (width == -1) {
        width = (size == SPRITE_SIZE_32x32) ? 32 : (size == SPRITE_SIZE_64x64 ? 64 : 16);
    }
    if (height == -1) {
        height = (size == SPRITE_SIZE_32x32) ? 32 : (size == SPRITE_SIZE_64x64 ? 64 : 16);
    }

    QImage* sp = bm[size];
    if (sp) {
        painter->drawImage(startX, startY, *sp);
    } else if (size == SPRITE_SIZE_64x64 && bm[SPRITE_SIZE_32x32]) {
        // Fallback do 32x32 i skalowanie jeśli nie ma 64x64
        QImage img = bm[SPRITE_SIZE_32x32]->scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        painter->drawImage(startX, startY, img);
    }
}

void EditorSprite::unloadDC() {
    delete bm[SPRITE_SIZE_16x16];
    delete bm[SPRITE_SIZE_32x32];
    delete bm[SPRITE_SIZE_64x64];
    bm[SPRITE_SIZE_16x16] = nullptr;
    bm[SPRITE_SIZE_32x32] = nullptr;
    bm[SPRITE_SIZE_64x64] = nullptr;
}

// Implementacja GameSprite
GameSprite::GameSprite() :
    height(0),
    width(0),
    layers(0),
    patternX(0),
    patternY(0),
    patternZ(0),
    frames(0),
    numSprites(0),
    animator(nullptr),
    drawHeight(0),
    drawOffsetX(0),
    drawOffsetY(0),
    minimapColor(0)
{
}

GameSprite::~GameSprite() {
    unloadDC();
    delete animator;
}

void GameSprite::clean(int time) {
    // Implementacja czyszczenia pamięci
}

void GameSprite::unloadDC() {
    for (QImage* img : spriteList) {
        delete img;
    }
    spriteList.clear();
}

int GameSprite::getDrawHeight() const {
    return drawHeight;
}

QPair<int, int> GameSprite::getDrawOffset() const {
    return qMakePair(drawOffsetX, drawOffsetY);
}

uint8_t GameSprite::getMiniMapColor() const {
    return minimapColor;
}

int GameSprite::getIndex(int width, int height, int layer, int patternX, int patternY, int patternZ, int frame) const {
    return ((((((frame % this->frames) * this->patternZ + patternZ) * this->patternY + patternY) * this->patternX + patternX) * this->layers + layer) * this->height + height) * this->width + width;
}

void GameSprite::drawTo(QPainter* painter, SpriteSize size, int startX, int startY, int width, int height) {
    if (width == -1) {
        width = (size == SPRITE_SIZE_32x32) ? 32 : (size == SPRITE_SIZE_64x64 ? 64 : 16);
    }
    if (height == -1) {
        height = (size == SPRITE_SIZE_32x32) ? 32 : (size == SPRITE_SIZE_64x64 ? 64 : 16);
    }

    if (spriteList.isEmpty()) {
        painter->fillRect(startX, startY, width, height, Qt::red);
        return;
    }

    int frame = animator ? animator->getFrame() : 0;
    int index = getIndex(0, 0, 0, 0, 0, 0, frame);
    if (index >= 0 && index < spriteList.size()) {
        QImage* sprite = spriteList[index];
        if (sprite) {
            if (size == SPRITE_SIZE_64x64) {
                QImage scaled = sprite->scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                painter->drawImage(startX, startY, scaled);
            } else {
                painter->drawImage(startX, startY, *sprite);
            }
        }
    }
}

// Implementacja SpriteManager
SpriteManager* SpriteManager::instance = nullptr;

SpriteManager* SpriteManager::getInstance() {
    if (!instance) {
        instance = new SpriteManager();
    }
    return instance;
}

SpriteManager::SpriteManager(QObject* parent) : QObject(parent),
    unloaded(true),
    itemCount(0),
    creatureCount(0),
    hasTransparency(false),
    hasFrameDurations(false),
    hasFrameGroups(false),
    loadedTextures(0),
    lastClean(0)
{
    animationTimer = new QTimer(this);
    connect(animationTimer, &QTimer::timeout, this, [this]() {
        garbageCollection();
    });
    animationTimer->start(1000); // Sprawdzaj co sekundę
}

SpriteManager::~SpriteManager() {
    clear();
    delete animationTimer;
}

bool SpriteManager::loadSprites(const QString& filename) {
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        emit error("Nie można otworzyć pliku sprite'ów");
        return false;
    }

    QDataStream stream(&file);
    stream.setVersion(QDataStream::Qt_5_0);

    // Wczytaj nagłówek
    quint32 signature;
    stream >> signature;
    if (signature != 0x4D42544F) { // "OTBM"
        emit error("Nieprawidłowy format pliku sprite'ów");
        return false;
    }

    // Wczytaj liczbę sprite'ów
    quint32 totalSprites;
    stream >> totalSprites;

    // Wczytaj sprite'y
    for (quint32 i = 0; i < totalSprites; ++i) {
        GameSprite* sprite = new GameSprite();
        stream >> sprite->width;
        stream >> sprite->height;
        stream >> sprite->layers;
        stream >> sprite->patternX;
        stream >> sprite->patternY;
        stream >> sprite->patternZ;
        stream >> sprite->frames;

        if (sprite->frames > 1) {
            quint8 async;
            qint32 loopCount;
            qint8 startFrame;
            stream >> async;
            stream >> loopCount;
            stream >> startFrame;
            sprite->animator = new Animator(sprite->frames, startFrame, loopCount, async == 1);
        }

        sprite->numSprites = sprite->width * sprite->height * sprite->layers * sprite->patternX * sprite->patternY * sprite->patternZ * sprite->frames;

        // Wczytaj dane sprite'a
        for (quint32 j = 0; j < sprite->numSprites; ++j) {
            QImage* img = new QImage(32, 32, QImage::Format_ARGB32);
            img->fill(Qt::transparent);
            sprite->spriteList.append(img);
        }

        spriteSpace[i] = sprite;
    }

    unloaded = false;
    emit spritesLoaded();
    return true;
}

bool SpriteManager::loadSpriteMetadata(const QString& filename) {
    // Implementacja wczytywania metadanych sprite'ów
    return true;
}

bool SpriteManager::loadSpriteData(const QString& filename) {
    // Implementacja wczytywania danych sprite'ów
    return true;
}

// Helper function to read a string from datastream (moved from itemmanager.cpp)
// Made static as it doesn't depend on ItemManager instance state.
// Consider placing in a utility class or namespace if used more broadly.
static QString readDatString(QDataStream& in) {
    quint16 length;
    in >> length;
    if (length == 0 || length > 2000) { // Increased sanity check limit
        if (length > 0) { // Only warn if length is non-zero but suspicious
             qWarning() << "Suspiciously long string in DAT, length:" << length << ". Attempting to skip/read cautiously.";
        }
        // For sequential devices, must read to skip. Limit read to avoid OOM.
        // For non-sequential, skipRawData is fine.
        if (in.device() && in.device()->isSequential() && length > 0) {
             QByteArray dummy(qMin(length, (quint16)1024*10), Qt::Uninitialized); // Read at most 10KB
             in.readRawData(dummy.data(), dummy.size());
        } else if (length > 0) {
            in.skipRawData(length); // More efficient for seekable devices
        }
        return QString(); // Return empty or error string
    }
    
    QByteArray data(length, Qt::Uninitialized); // Use Uninitialized for performance
    if (in.readRawData(data.data(), length) != length) {
        qWarning() << "Failed to read string data from DAT, expected length:" << length;
        return QString();
    }
    return QString::fromLatin1(data); // Assuming Latin1 encoding for Tibia DAT strings
}

// --- Start of functions moved and adapted from ItemManager ---

bool SpriteManager::loadSprites(const QString& sprPath, const QString& datPath)
{
    // datPath is currently unused in SpriteManager's implementation but kept for API compatibility for now.
    // SpriteManager focuses on .spr for pixel data. ItemManager would use .dat for item properties.
    Q_UNUSED(datPath);

    QFile sprFile(sprPath);
    if (!sprFile.open(QIODevice::ReadOnly)) {
        qWarning() << "Cannot open sprite file:" << sprPath;
        emit error(tr("Cannot open sprite file: %1").arg(sprPath));
        return false;
    }

    QDataStream in(&sprFile);
    in.setByteOrder(QDataStream::LittleEndian);
    in.setVersion(QDataStream::Qt_5_0); // Consistent with ItemManager's usage

    quint32 spriteCountFromSpr = 0; 
    QVector<quint32> spriteOffsets;
    
    if (!readTibiaSprHeader(in, spriteCountFromSpr, spriteOffsets)) {
        qWarning() << "Invalid .spr file format or version:" << sprPath;
        emit error(tr("Invalid .spr file format or version: %1").arg(sprPath));
        sprFile.close();
        return false;
    }

    qInfo() << "Loading" << spriteCountFromSpr << "sprites from" << sprPath;
    
    // Clear existing sprites before loading new ones
    for (Sprite* s : qAsConst(spriteSpace)) {
        delete s;
    }
    spriteSpace.clear();
    // GameSprite owns its QImage objects, so clearing spriteSpace handles their deletion.

    int successfullyLoadedCount = 0;
    for (quint32 i = 1; i <= spriteCountFromSpr; ++i) { // Sprites are 1-indexed
        if (spriteOffsets.size() <= static_cast<int>(i) || spriteOffsets[i] == 0 || spriteOffsets[i] >= static_cast<quint32>(sprFile.size())) {
            // Skip empty, invalid offset, or out-of-bounds offset
            continue; 
        }
        
        if (!sprFile.seek(spriteOffsets[i])) {
            qWarning() << "Failed to seek to sprite" << i << "at offset" << spriteOffsets[i];
            continue;
        }
        
        // Each sprite in Tibia .spr starts with 2 bytes for transparent color key (usually #FF00FF),
        // which is skipped/ignored. Then 2 bytes for the size of the pixel data that follows.
        in.skipRawData(2); // Skip the 2-byte color key (FF 00 FF -> Pink)

        quint16 pixelDataSize;
        in >> pixelDataSize; 
        
        if (pixelDataSize == 0 || pixelDataSize > 32*32*4 + 1024 ) { // Sanity check for size, allow some RLE overhead
            // qDebug() << "Sprite ID" << i << "has zero or suspicious pixel data size:" << pixelDataSize;
            continue; 
        }
        
        QByteArray pixelData(pixelDataSize, Qt::Uninitialized);
        int bytesRead = in.readRawData(pixelData.data(), pixelDataSize);

        if (bytesRead != pixelDataSize) {
            qWarning() << "Failed to read sprite data for sprite" << i << ". Expected" << pixelDataSize << "got" << bytesRead;
            continue;
        }
        
        QImage spriteImage = convertSpriteDataToImage(pixelData); // Default 32x32
        if (!spriteImage.isNull()) {
            GameSprite* gameSprite = new GameSprite();
            gameSprite->width = 1; 
            gameSprite->height = 1;
            gameSprite->layers = 1;
            gameSprite->patternX = 1;
            gameSprite->patternY = 1;
            gameSprite->patternZ = 1;
            gameSprite->frames = 1; 
            gameSprite->numSprites = 1;
            gameSprite->spriteList.append(new QImage(spriteImage));

            spriteSpace[i] = gameSprite; 
            successfullyLoadedCount++;
        }
        
        if (i > 0 && i % 1000 == 0) {
            qDebug() << "Loaded" << successfullyLoadedCount << "of" << spriteCountFromSpr << "sprites...";
        }
    }

    sprFile.close();
    unloaded = false; 
    emit spritesLoaded();
    qInfo() << "Successfully loaded" << successfullyLoadedCount << "sprites into SpriteManager from" << sprPath;
    return true;
}

bool SpriteManager::readTibiaSprHeader(QDataStream& in, quint32& spriteCountParam, QVector<quint32>& spriteOffsets) {
    // Tibia .spr files typically start with a 4-byte signature (e.g., "SPR\0" or version number)
    // followed by a 4-byte sprite count.
    // The ItemManager version checked for 0x00000004.
    
    in >> sprHeader.signature; // Reads into member variable sprHeader.signature
    
    // A common signature for some Tibia .spr versions is actually the sprite count itself,
    // or a different magic number. The 0x00000004 might be for a specific client version.
    // For flexibility, we might not want to hard fail on signature mismatch unless we are certain.
    if (sprHeader.signature != 0x00000004 && sprHeader.signature != 0x52505300 /* "SPR\0" little endian */) { 
        qWarning() << "Unusual .spr signature:" << Qt::hex << sprHeader.signature << ". Proceeding cautiously.";
        // If the signature was actually the count, the next read will fail or be wrong.
        // For robust loading, one might try to read count, then offsets, and if that fails,
        // assume the first read was the count. This is complex. Sticking to one format for now.
    }
    
    in >> sprHeader.spriteCount; 
    spriteCountParam = sprHeader.spriteCount; // Pass out via parameter
    
    if (spriteCountParam == 0 || spriteCountParam > 200000) { 
        qWarning() << "Invalid or suspicious sprite count in .spr file:" << spriteCountParam;
        // If signature was 0x00000004, and count is also 0x00000004, it's likely misinterpreting.
        // However, some empty .spr files might legitimately have 0 sprites.
        if (spriteCountParam == 0) {
             qInfo() << ".spr file contains 0 sprites.";
             return true; // Valid empty file
        }
        return false; 
    }
    
    spriteOffsets.resize(spriteCountParam + 1); // Sprite IDs are 1-based
    for (quint32 i = 1; i <= spriteCountParam; ++i) {
        in >> spriteOffsets[i];
        if (in.status() != QDataStream::Ok) {
            qWarning() << "Failed to read sprite offset for sprite ID:" << i;
            spriteOffsets.clear(); // Clear potentially partial data
            return false;
        }
    }
    
    return true;
}

bool SpriteManager::readTibiaDatHeader(QDataStream& in) {
    // This function is mostly for ItemManager's needs.
    // SpriteManager currently doesn't use datHeader for sprite rendering.
    quint32 signature;
    in >> signature;
    
    if (signature != 0x00000100) { // Example: DAT format version 1.0
        qWarning() << "Unsupported .dat format/version in SpriteManager (read attempt):" << Qt::hex << signature;
        // Not failing, as SpriteManager doesn't critically depend on .dat contents for sprites yet.
    }
    datHeader.version = signature;

    in >> datHeader.items;
    in >> datHeader.outfits;
    in >> datHeader.effects;
    in >> datHeader.missiles;
    
    if (datHeader.items > 150000) { // Increased limit slightly
        qWarning() << "Suspicious item count in .dat file (read by SpriteManager):" << datHeader.items;
    }
    
    return in.status() == QDataStream::Ok;
}

bool SpriteManager::readDatItem(QDataStream& in, DatItem& item) {
    // Primarily for ItemManager. SpriteManager calls this if it processes a .dat file,
    // but doesn't use most of the DatItem fields itself.
    in >> item.id; 
    
    item.name = readDatString(in);
    if (item.name.isNull() && in.status() != QDataStream::Ok) {
        qWarning() << "Failed to read item name for DAT item ID:" << item.id;
        return false; // Critical error in reading item structure
    }
    
    in >> item.spriteId; // This is the link ItemManager uses.
    
    in >> item.flags;
    in >> item.weight;
    in >> item.speed;
    in >> item.lightLevel;
    in >> item.lightColor;
    in >> item.wareId;
    in >> item.alwaysOnTop;
    in >> item.alwaysOnTopOrder;
    in >> item.blocking;
    in >> item.walkable;
    in >> item.collidable;
    
    return in.status() == QDataStream::Ok;
}

QImage SpriteManager::convertSpriteDataToImage(const QByteArray& pixelData, int width, int height) {
    // This function decodes Tibia sprite pixel data.
    // Tibia .spr RLE format:
    // Repeats:
    //   WORD: Number of transparent pixels to skip.
    //   WORD: Number of colored pixels to draw.
    //   For each colored pixel: 3 bytes (Red, Green, Blue).
    // The magenta color (R:255, G:0, B:255) is the transparent key for pixels not covered by RLE skips.

    QImage image(width, height, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent); // Start with a fully transparent image

    const unsigned char* dataPtr = reinterpret_cast<const unsigned char*>(pixelData.constData());
    int dataSize = pixelData.size();
    int currentByteIndex = 0;
    int currentPixel = 0;

    while (currentByteIndex < dataSize && currentPixel < width * height) {
        if (currentByteIndex + 1 >= dataSize) break; // Need at least 2 bytes for transparent count

        quint16 transparentPixels = static_cast<quint16>(dataPtr[currentByteIndex]) | (static_cast<quint16>(dataPtr[currentByteIndex + 1]) << 8);
        currentByteIndex += 2;

        currentPixel += transparentPixels;

        if (currentByteIndex + 1 >= dataSize || currentPixel >= width * height) break; // Need 2 bytes for colored count or pixels done

        quint16 coloredPixels = static_cast<quint16>(dataPtr[currentByteIndex]) | (static_cast<quint16>(dataPtr[currentByteIndex + 1]) << 8);
        currentByteIndex += 2;

        if (currentByteIndex + (coloredPixels * 3) > dataSize) {
            qWarning() << "Sprite RLE data ended prematurely or is malformed. Expected" << (coloredPixels*3) << "bytes for colored run, got" << (dataSize - currentByteIndex);
            // Attempt to draw what can be drawn
            coloredPixels = (dataSize - currentByteIndex) / 3;
            if (coloredPixels == 0 && (dataSize - currentByteIndex) > 0) {
                 //qDebug() << "Not enough data for even one more colored pixel.";
            }
        }
        
        for (int i = 0; i < coloredPixels && currentPixel < width * height; ++i) {
            if (currentByteIndex + 2 >= dataSize) { // Should be caught by outer check, but good for safety
                 qWarning() << "RLE: Unexpected end of data when reading RGB for colored pixel.";
                 goto end_loops; // Break out of all loops
            }
            // Tibia colors are BGR, not RGB in the file.
            uchar blue  = dataPtr[currentByteIndex++];
            uchar green = dataPtr[currentByteIndex++];
            uchar red   = dataPtr[currentByteIndex++];
            
            int x = currentPixel % width;
            int y = currentPixel / width;

            // The RLE scheme implies colored pixels are opaque unless they are the specific key color.
            // However, standard Tibia RLE does not typically use key color for RLE colored runs.
            // The transparency is handled by the 'transparentPixels' count.
            // If magenta (255,0,255) must still be transparent even in a colored run, add check here.
            // For now, assume colored run means opaque pixels.
            image.setPixel(x, y, qRgb(red, green, blue));
            currentPixel++;
        }
    }

end_loops:; // Label to break out from nested loops if critical error

    if (currentPixel < width * height && currentByteIndex < dataSize) {
        // This might indicate that the sprite data didn't fill the whole 32x32 area,
        // or the data is padded / has extra info, or RLE encoding is different.
        // qDebug() << "Sprite data processed, but not all pixels filled or data remaining. Pixels:" << currentPixel << "Total:" << width*height << " Index:" << currentByteIndex << "Size:" << dataSize;
    }
     else if (currentPixel == width*height && currentByteIndex < dataSize && (dataSize - currentByteIndex) > 3 ) { // Allow a few padding bytes
        // If all pixels are drawn but there's still significant data left
        // qDebug() << "All pixels drawn, but " << (dataSize - currentByteIndex) << " bytes remaining in sprite data block.";
    }


    return image;
}

// --- End of functions moved and adapted from ItemManager ---


Sprite* SpriteManager::getSprite(int id) {
    return spriteSpace.value(id);
}

GameSprite* SpriteManager::getCreatureSprite(int id) {
    if (id < 0) {
        return nullptr;
    }
    return dynamic_cast<GameSprite*>(spriteSpace.value(id + itemCount));
}

void SpriteManager::clear() {
    for (Sprite* sprite : spriteSpace) {
        delete sprite;
    }
    spriteSpace.clear();

    for (QImage* img : imageSpace) {
        delete img;
    }
    imageSpace.clear();

    cleanupList.clear();

    itemCount = 0;
    creatureCount = 0;
    loadedTextures = 0;
    lastClean = QDateTime::currentMSecsSinceEpoch();
    spriteFile.clear();

    unloaded = true;
}

void SpriteManager::cleanSoftwareSprites() {
    for (Sprite* sprite : spriteSpace) {
        sprite->unloadDC();
    }
}

void SpriteManager::garbageCollection() {
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    if (currentTime - lastClean > 1000) { // Co sekundę
        for (GameSprite* sprite : cleanupList) {
            sprite->clean(currentTime);
        }
        lastClean = currentTime;
    }
}

bool SpriteManager::loadSpriteDump(QByteArray& target, uint16_t& size, int spriteId) {
    if (spriteId == 0) {
        size = 0;
        target.clear();
        return true;
    }

    QFile file(spriteFile);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    QDataStream stream(&file);
    stream.setVersion(QDataStream::Qt_5_0);

    // Przejdź do odpowiedniej pozycji w pliku
    if (!file.seek(4 + spriteId * sizeof(quint32))) {
        return false;
    }

    quint32 offset;
    if (stream >> offset) {
        if (file.seek(offset + 3)) {
            quint16 spriteSize;
            if (stream >> spriteSize) {
                target = file.read(spriteSize);
                if (target.size() == spriteSize) {
                    size = spriteSize;
                    return true;
                }
            }
        }
    }

    return false;
} 