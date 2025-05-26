#include "mainwindow.h"
#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QDebug>
#include "spritemanager.h"
#include "itemmanager.h"
#include "gamesprite.h" // Required for dynamic_cast to GameSprite

// Function to run tests
void runTemporaryTests() {
    qDebug() << "--- Starting Temporary Loader Tests ---";

    // Obtain Manager Instances
    SpriteManager* spriteManager = SpriteManager::getInstance();
    ItemManager* itemManager = &ItemManager::getInstance();

    // Test SpriteManager Loading
    qDebug() << "\n--- Testing SpriteManager Loading ---";
    // Assuming Tibia.spr and Tibia.dat are in "data/" relative to the execution path
    // SpriteManager's loadSprites now takes sprPath and an optional datPath.
    // For this test, we provide both as ItemManager's loadTibiaDat will also be tested.
    bool spritesLoaded = spriteManager->loadSprites("data/Tibia.spr", "data/Tibia.dat");
    if (!spritesLoaded) {
        qDebug() << "TEST ERROR: SpriteManager failed to load Tibia.spr/Tibia.dat. Ensure 'data/Tibia.spr' and 'data/Tibia.dat' exist.";
    } else {
        qDebug() << "TEST INFO: SpriteManager potentially loaded Tibia.spr/Tibia.dat. Sprite count:" << spriteManager->getSpriteCount(); // Assuming getSpriteCount() exists

        // Attempt to get a known sprite ID
        int testSpriteId = 1; // A common sprite ID, like a blank or early one
        Sprite* testSpriteBase = spriteManager->getSprite(testSpriteId);
        if (!testSpriteBase) {
            qDebug() << "TEST ERROR: SpriteManager could not retrieve sprite ID" << testSpriteId << "after loading.";
        } else {
            qDebug() << "TEST INFO: SpriteManager retrieved a sprite for ID" << testSpriteId << ".";
            GameSprite* testGameSprite = dynamic_cast<GameSprite*>(testSpriteBase);
            if (testGameSprite) {
                if (!testGameSprite->spriteList.isEmpty() && testGameSprite->spriteList.first()) {
                    qDebug() << "TEST INFO: GameSprite for ID" << testSpriteId << "has a QImage of size:" << testGameSprite->spriteList.first()->size();
                } else {
                    qDebug() << "TEST ERROR: GameSprite for ID" << testSpriteId << "has an empty spriteList or null QImage.";
                }
            } else {
                 qDebug() << "TEST WARNING: Sprite ID" << testSpriteId << "is not a GameSprite. Type:" << testSpriteBase->metaObject()->className();
            }
        }
         // Test another sprite ID, e.g. one that might correspond to an item
        testSpriteId = 2160; // Often gold coin or similar, assuming its sprite is directly mapped
        testSpriteBase = spriteManager->getSprite(testSpriteId);
        if (!testSpriteBase) {
            qDebug() << "TEST INFO: SpriteManager could not retrieve sprite ID" << testSpriteId << "(potential item sprite). This might be okay if item sprites are mapped differently.";
        } else {
            qDebug() << "TEST INFO: SpriteManager retrieved a sprite for ID" << testSpriteId << ".";
             GameSprite* gameSprite = dynamic_cast<GameSprite*>(testSpriteBase);
            if (gameSprite && !gameSprite->spriteList.isEmpty() && gameSprite->spriteList.first()) {
                qDebug() << "TEST INFO: GameSprite for ID" << gameSprite << "has a QImage of size:" << gameSprite->spriteList.first()->size();
            } else {
                 qDebug() << "TEST ERROR: GameSprite for ID" << gameSprite << "has an empty spriteList or null QImage.";
            }
        }
    }

    // Test ItemManager Loading (.dat)
    qDebug() << "\n--- Testing ItemManager Loading (.dat) ---";
    // ItemManager::loadItems will dispatch to loadTibiaDat based on extension
    bool itemsLoaded = itemManager->loadItems("data/Tibia.dat"); 
    if (!itemsLoaded) {
        qDebug() << "TEST ERROR: ItemManager failed to load Tibia.dat. Ensure 'data/Tibia.dat' exists.";
    } else {
        qDebug() << "TEST INFO: ItemManager potentially loaded Tibia.dat. Item property count:" << itemManager->getLoadedItemPropertiesCount(); // Assuming such a method

        // Attempt to get properties for a known item ID
        int testItemId = 2160; // Gold coin in many Tibia versions
        QString itemName = itemManager->getItemName(testItemId);
        if (itemName.isEmpty()) {
            qDebug() << "TEST ERROR: ItemManager did not load properties for item ID" << testItemId << "(e.g., gold coin). Name is empty.";
        } else {
            bool isStackable = itemManager->isItemStackable(testItemId);
            qDebug() << "TEST INFO: Item ID" << testItemId << "Name:" << itemName << "Stackable:" << isStackable;
        }

        int anotherTestItemId = 3031; // Example: Manafluid might be ID 2006 in some versions, or 3031 for life ring etc.
                                      // Using a common item ID.
        itemName = itemManager->getItemName(anotherTestItemId);
         if (itemName.isEmpty()) {
            qDebug() << "TEST WARNING: ItemManager did not load properties for item ID" << anotherTestItemId << ". Name is empty. This might be an unknown item in this Tibia.dat.";
        } else {
            qDebug() << "TEST INFO: Item ID" << anotherTestItemId << "Name:" << itemName;
        }
    }

    // Test ItemManager Retrieving Sprite (Integration Test)
    qDebug() << "\n--- Testing ItemManager Sprite Retrieval (Integration) ---";
    if (itemsLoaded && spritesLoaded) { // Only run if both managers claim to have loaded data
        int integrationTestItemId = 2160; // Gold coin
        QPixmap itemSpritePixmap = itemManager->getItemIcon(integrationTestItemId); // getItemIcon calls getSprite
        if (itemSpritePixmap.isNull()) {
            qDebug() << "TEST ERROR: ItemManager could not retrieve QPixmap for item ID" << integrationTestItemId << "via SpriteManager.";
            // Further check if the sprite ID itself is valid from ItemProperties
            if(itemManager->m_itemProperties.contains(integrationTestItemId)) { // Accessing m_itemProperties directly for test diagnosis
                 qDebug() << "DIAGNOSTIC: Item ID" << integrationTestItemId << "has spriteId:" << itemManager->m_itemProperties.value(integrationTestItemId).spriteId;
                 Sprite* rawSprite = spriteManager->getSprite(itemManager->m_itemProperties.value(integrationTestItemId).spriteId);
                 if(!rawSprite) {
                    qDebug() << "DIAGNOSTIC: SpriteManager->getSprite() returned null for spriteId:" << itemManager->m_itemProperties.value(integrationTestItemId).spriteId;
                 } else {
                    qDebug() << "DIAGNOSTIC: SpriteManager->getSprite() returned a valid sprite pointer for spriteId:" << itemManager->m_itemProperties.value(integrationTestItemId).spriteId;
                 }
            } else {
                qDebug() << "DIAGNOSTIC: Item ID" << integrationTestItemId << "not found in m_itemProperties.";
            }

        } else {
            qDebug() << "TEST INFO: ItemManager retrieved QPixmap for item ID" << integrationTestItemId << ". Size:" << itemSpritePixmap.size();
        }

        integrationTestItemId = 3031; // Another item, e.g. a ring
        itemSpritePixmap = itemManager->getItemIcon(integrationTestItemId);
        if (itemSpritePixmap.isNull()) {
            qDebug() << "TEST WARNING: ItemManager could not retrieve QPixmap for item ID" << integrationTestItemId << ". This might be an item without a standard sprite or an unknown item.";
            if(itemManager->m_itemProperties.contains(integrationTestItemId)) { // Accessing m_itemProperties directly for test diagnosis
                 qDebug() << "DIAGNOSTIC: Item ID" << integrationTestItemId << "has spriteId:" << itemManager->m_itemProperties.value(integrationTestItemId).spriteId;
            } else {
                qDebug() << "DIAGNOSTIC: Item ID" << integrationTestItemId << "not found in m_itemProperties.";
            }
        } else {
            qDebug() << "TEST INFO: ItemManager retrieved QPixmap for item ID" << integrationTestItemId << ". Size:" << itemSpritePixmap.size();
        }

    } else {
        qDebug() << "TEST INFO: Skipping ItemManager sprite retrieval integration test due to earlier loading failures.";
    }

    qDebug() << "\n--- Finished Temporary Loader Tests ---";
}


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Run temporary test code
    runTemporaryTests();

    // Ustawienia aplikacji
    QApplication::setApplicationName("Idlers Map Editor");
    QApplication::setApplicationVersion("1.0.0");
    QApplication::setOrganizationName("Idlers");
    QApplication::setOrganizationDomain("idlers.com");

    // Wczytanie tłumaczeń
    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "IdlersMapEditor_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            app.installTranslator(&translator);
            break;
        }
    }

    // Utworzenie i wyświetlenie głównego okna
    MainWindow mainWindow;
    mainWindow.show();

    return app.exec();
} 