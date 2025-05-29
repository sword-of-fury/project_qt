#include "aboutdialog.h"
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDialogButtonBox>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QApplication> // For qApp->applicationDirPath() if needed, or use QStandardPaths
#include <QStandardPaths> // For locating common paths
#include <QDir>

// Define placeholder application name and version details
// These would ideally come from a central project configuration.
const QString APP_NAME = "Remere's Map Editor NG"; // Or your specific project name
const QString APP_VERSION = "0.1.0"; // Placeholder version
const QString COMPILE_DATE = __DATE__; // Standard macro for compile date
const QString COMPILE_TIME = __TIME__; // Standard macro for compile time

AboutDialog::AboutDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(tr("About %1").arg(APP_NAME));

    // Main Layout
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // Title Label
    titleLabel = new QLabel(QString("<b>%1</b><br>Version %2").arg(APP_NAME).arg(APP_VERSION));
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    // Main Text Label
    QString aboutText = QString(
        "<p align="center">A 2D map editor for tile-based game worlds.</p>"
        "<p align="center">Originally based on Remere's Map Editor.</p>"
        "<p align="center">Migrated to Qt6.</p>"
        "<p align="center">Compiled on %1 at %2.</p>"
        "<p align="center">This program is free software and comes with ABSOLUTELY NO WARRANTY.</p>"
    ).arg(COMPILE_DATE).arg(COMPILE_TIME);
    
    mainTextLabel = new QLabel(aboutText);
    mainTextLabel->setAlignment(Qt::AlignCenter);
    mainTextLabel->setWordWrap(true);
    mainLayout->addWidget(mainTextLabel);

    // Button Box
    buttonBox = new QDialogButtonBox();
    QPushButton* licenseButton = buttonBox->addButton(tr("View License..."), QDialogButtonBox::ActionRole);
    buttonBox->addButton(QDialogButtonBox::Ok);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept); // OK button
    connect(licenseButton, &QPushButton::clicked, this, &AboutDialog::showLicense);

    mainLayout->addWidget(buttonBox);

    setLayout(mainLayout);
    adjustSize(); 
}

AboutDialog::~AboutDialog()
{
}

void AboutDialog::showLicense()
{
    QString licenseFilePath;
    QStringList searchPaths;
    searchPaths << QApplication::applicationDirPath(); 
    searchPaths << QDir::currentPath(); 
    
    // Attempt to locate standard data locations if available through QStandardPaths
    QStringList dataLocations = QStandardPaths::standardLocations(QStandardPaths::DataLocation);
    for(const QString& dataPath : dataLocations) {
        searchPaths << dataPath;
    }
    // Add AppLocalDataLocation and AppConfigLocation as well for broader search
    QStringList appDataLocations = QStandardPaths::standardLocations(QStandardPaths::AppLocalDataLocation);
     for(const QString& dataPath : appDataLocations) {
        searchPaths << dataPath;
    }
    QStringList appConfigLocations = QStandardPaths::standardLocations(QStandardPaths::AppConfigLocation);
     for(const QString& dataPath : appConfigLocations) {
        searchPaths << dataPath;
    }


    QStringList licenseFileNames = {"LICENSE.txt", "COPYING.txt", "LICENSE", "COPYING"};

    for (const QString& path : searchPaths) {
        if (path.isEmpty()) continue;
        QDir dir(path);
        for (const QString& fileName : licenseFileNames) {
            if (dir.exists(fileName)) {
                licenseFilePath = dir.filePath(fileName);
                break;
            }
        }
        if (!licenseFilePath.isEmpty()) break;
    }
    
    if (licenseFilePath.isEmpty()) {
        // Fallback for development environment or if not found in standard paths
        QDir baseDir(QApplication::applicationDirPath());
        // Try to go up a few directories common in build setups
        baseDir.cdUp(); // Potentially out of build/debug to project root
        for (const QString& fileName : licenseFileNames) {
            if (baseDir.exists(fileName)) {
                licenseFilePath = baseDir.filePath(fileName);
                break;
            }
        }
        if (licenseFilePath.isEmpty()) {
             baseDir.cdUp(); // Try one more level up
             for (const QString& fileName : licenseFileNames) {
                if (baseDir.exists(fileName)) {
                    licenseFilePath = baseDir.filePath(fileName);
                    break;
                }
            }
        }
    }


    if (!licenseFilePath.isEmpty()) {
        QFile licenseFile(licenseFilePath);
        if (licenseFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&licenseFile);
            QString licenseText = in.readAll();
            licenseFile.close();

            QMessageBox licenseMsgBox(this);
            licenseMsgBox.setWindowTitle(tr("License Information"));
            licenseMsgBox.setTextFormat(Qt::PlainText); 
            licenseMsgBox.setText(licenseText);
            licenseMsgBox.setIcon(QMessageBox::Information);
            licenseMsgBox.addButton(QMessageBox::Ok);
            licenseMsgBox.exec();

        } else {
            QMessageBox::warning(this, tr("License Not Found"),
                                 tr("Could not open the license file: %1").arg(licenseFilePath));
        }
    } else {
        QMessageBox::information(this, tr("License Information"),
                                 tr("The license file (LICENSE.txt or COPYING.txt) was not found."));
    }
}
