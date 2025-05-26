#ifndef SPRITEMANAGER_H
#define SPRITEMANAGER_H

#include <QObject>
#include <QImage>
#include <QMap>
#include <QVector> // Already present, good.
#include <QTimer>
#include <QString> // For QString parameters
#include <QDataStream> // For QDataStream parameters in header declarations

// Rozmiary sprite'ów
enum SpriteSize {
    SPRITE_SIZE_16x16,
    SPRITE_SIZE_32x32,
    SPRITE_SIZE_64x64,
    SPRITE_SIZE_COUNT
};

// Kierunek animacji
enum AnimationDirection {
    ANIMATION_FORWARD = 0,
    ANIMATION_BACKWARD = 1
};

// Czas trwania klatki animacji
const int ITEM_FRAME_DURATION = 500;

// Struktura przechowująca informacje o świetle sprite'a
struct SpriteLight {
    uint8_t intensity = 0;
    uint8_t color = 0;
};

// Struktura przechowująca informacje o czasie trwania klatki
struct FrameDuration {
    int min;
    int max;

    FrameDuration(int min, int max) : min(min), max(max) {
        Q_ASSERT(min <= max);
    }

    int getDuration() const {
        if (min == max) {
            return min;
        }
        return qrand() % (max - min + 1) + min;
    }

    void setValues(int min, int max) {
        Q_ASSERT(min <= max);
        this->min = min;
        this->max = max;
    }
};

// Klasa zarządzająca animacjami
class Animator {
public:
    Animator(int frameCount, int startFrame, int loopCount, bool async);
    ~Animator();

    int getStartFrame() const;
    FrameDuration* getFrameDuration(int frame);
    int getFrame();
    void setFrame(int frame);
    void reset();

private:
    int getDuration(int frame) const;
    int getPingPongFrame();
    int getLoopFrame();
    void calculateSynchronous();

    int frameCount;
    int startFrame;
    int loopCount;
    bool async;
    QVector<FrameDuration*> durations;
    int currentFrame;
    int currentLoop;
    int currentDuration;
    int totalDuration;
    AnimationDirection direction;
    qint64 lastTime;
    bool isComplete;
};

// Klasa bazowa dla sprite'ów
class Sprite {
public:
    Sprite();
    virtual ~Sprite();

    virtual void drawTo(QPainter* painter, SpriteSize size, int startX, int startY, int width = -1, int height = -1) = 0;
    virtual void unloadDC() = 0;

protected:
    Sprite(const Sprite&);
    Sprite& operator=(const Sprite&);
};

// Klasa dla sprite'ów edytora
class EditorSprite : public Sprite {
public:
    EditorSprite(QImage* b16x16, QImage* b32x32, QImage* b64x64 = nullptr);
    virtual ~EditorSprite();

    virtual void drawTo(QPainter* painter, SpriteSize size, int startX, int startY, int width = -1, int height = -1) override;
    virtual void unloadDC() override;

protected:
    QImage* bm[SPRITE_SIZE_COUNT];
};

// Klasa dla sprite'ów gry
class GameSprite : public Sprite {
public:
    GameSprite();
    ~GameSprite();

    int getIndex(int width, int height, int layer, int patternX, int patternY, int patternZ, int frame) const;
    void drawTo(QPainter* painter, SpriteSize size, int startX, int startY, int width = -1, int height = -1) override;
    void unloadDC() override;

    void clean(int time);

    int getDrawHeight() const;
    QPair<int, int> getDrawOffset() const;
    uint8_t getMiniMapColor() const;

    bool hasLight() const noexcept { return hasLight; }
    const SpriteLight& getLight() const noexcept { return light; }

    // Informacje o sprite'cie
    uint8_t height;
    uint8_t width;
    uint8_t layers;
    uint8_t patternX;
    uint8_t patternY;
    uint8_t patternZ;
    uint8_t frames;
    uint32_t numSprites;

    Animator* animator;

    uint16_t drawHeight;
    uint16_t drawOffsetX;
    uint16_t drawOffsetY;

    uint16_t minimapColor;

    bool hasLight = false;
    SpriteLight light;

    QVector<QImage*> spriteList;
};

// Główna klasa zarządzająca sprite'ami
class SpriteManager : public QObject {
    Q_OBJECT

public:
    static SpriteManager* getInstance();
    ~SpriteManager();

    // Wczytywanie sprite'ów
    bool loadSpritesFromOtbm(const QString& filename); // Renamed from loadSprites
    bool loadSpriteMetadata(const QString& filename);
    bool loadSpriteData(const QString& filename);

    // Pobieranie sprite'ów
    Sprite* getSprite(int id);
    GameSprite* getCreatureSprite(int id);

    // Informacje o sprite'ach
    uint16_t getItemSpriteMaxID() const { return itemCount; }
    uint16_t getCreatureSpriteMaxID() const { return creatureCount; }

    // Czyszczenie pamięci
    void clear();
    void cleanSoftwareSprites();
    void garbageCollection();

    // Właściwości sprite'ów
    bool hasTransparency() const { return hasTransparency; }
    bool isUnloaded() const { return unloaded; }

signals:
    void spritesLoaded();
    void error(const QString& message);

private:
    explicit SpriteManager(QObject* parent = nullptr);
    static SpriteManager* instance;

    bool unloaded;
    QString spriteFile;
    bool loadSpriteDump(QByteArray& target, uint16_t& size, int spriteId);

    // Structs moved from ItemManager for Tibia .dat/.spr loading
    struct DatHeader {
        quint32 version; // Version of .dat file (e.g., 0x00000100 for 1.0)
        quint32 items;   // Number of items
        quint32 outfits;  // Number of outfits
        quint32 effects; // Number of effects
        quint32 missiles;// Number of missiles
        // Qt 6 might require explicit padding or alignment if issues arise
    };

    struct DatItem {
        quint16 id;
        QString name;
        quint16 spriteId;
        quint8 flags;
        quint8 weight;
        quint8 speed;
        quint8 lightLevel;
        quint8 lightColor;
        quint16 wareId;
        quint8 alwaysOnTop;
        quint8 alwaysOnTopOrder;
        quint8 blocking;
        quint8 walkable;
        quint8 collidable;
        // Add other fields if needed from full DAT structure
    };
    
    struct SprHeader {
        quint32 signature; // Should be 0x00000004 for SPRv4
        quint32 spriteCount;
        // Sprite offsets are typically read directly into a QVector<quint32>
    };

    DatHeader datHeader; // To store .dat header info if needed for sprite interpretation
    SprHeader sprHeader; // To store .spr header info

    QMap<int, Sprite*> spriteSpace;
    QMap<int, QImage*> imageSpace;
    QVector<GameSprite*> cleanupList;

    uint16_t itemCount;
    uint16_t creatureCount;
    bool hasTransparency;
    bool hasFrameDurations;
    bool hasFrameGroups;

    // Tibia sprite loading
    bool loadSprites(const QString& sprPath, const QString& datPath = QString());
    bool readTibiaSprHeader(QDataStream& in, quint32& spriteCount, QVector<quint32>& spriteOffsets);
    bool readTibiaDatHeader(QDataStream& in); // Potentially loads into member datHeader
    bool readDatItem(QDataStream& in, DatItem& item); // Helper for reading item data
    QImage convertSpriteDataToImage(const QByteArray& data, int width = 32, int height = 32);
    // Static helper might be better outside class or as private static
    // static QString readDatString(QDataStream& in); // Moved to .cpp as static

    int loadedTextures;
    int lastClean;
    QTimer* animationTimer;
};

#endif // SPRITEMANAGER_H 