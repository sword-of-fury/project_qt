#include "aboutdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QCoreApplication> // For Q_APPLICATION_NAME
#include <QtGlobal> // For QT_VERSION_STR

// Placeholder for version information similar to __W_RME_VERSION__
// This should ideally come from a project-wide configuration (e.g., CMake)
#ifndef PROJECT_QT_VERSION
#define PROJECT_QT_VERSION "0.1.0-dev" 
#endif

// Placeholders for build-time information (set via CMake)
#ifndef COMPILED_ON_DATE
#define COMPILED_ON_DATE __DATE__ // Standard macro
#endif
#ifndef COMPILED_ON_TIME
#define COMPILED_ON_TIME __TIME__ // Standard macro
#endif
#ifndef CXX_COMPILER_ID
#define CXX_COMPILER_ID "Unknown Compiler" // Placeholder
#endif
#ifndef CXX_COMPILER_VERSION
#define CXX_COMPILER_VERSION "" // Placeholder
#endif


AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent)
{
    QString appName = QCoreApplication::applicationName();
    if (appName.isEmpty()) {
        appName = "Map Editor"; // Fallback if not set
    }
    setWindowTitle("About " + appName);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    infoTextLabel = new QLabel(this);
    QString aboutText;
    aboutText += appName + "\n";
    aboutText += "(based on OTA Remere's Map Editor)\n\n";
    aboutText += "This program is a map editor for game servers\n";
    aboutText += "that derivied from OpenTibia project.\n\n";
    aboutText += "Brought to you by Idler enhanced using Cursor.com \n\n";
    
    aboutText += "Version " + QString(PROJECT_QT_VERSION) + " for ";
    
    QString osName;
    #if defined(Q_OS_WIN)
        osName = "Windows";
    #elif defined(Q_OS_LINUX)
        osName = "Linux";
    #elif defined(Q_OS_MACOS)
        osName = "macOS";
    #else
        osName = "other OS";
    #endif
    aboutText += osName + "\n\n";

    aboutText += "Using Qt " + QString(QT_VERSION_STR) + " interface\n\n";

    aboutText += "This program comes with ABSOLUTELY NO WARRANTY;\n";
    aboutText += "for details see the LICENSE file.\n";
    aboutText += "This is free software, and you are welcome to redistribute it\n";
    aboutText += "under certain conditions.\n";
    aboutText += " Just make sure that you include the invite link to discord.\n\n";

    aboutText += "Compiled on: " + QString(COMPILED_ON_DATE);
    aboutText += " : " + QString(COMPILED_ON_TIME) + "\n";
    aboutText += "Compiled with: " + QString(CXX_COMPILER_ID);
    if (QString(CXX_COMPILER_VERSION).length() > 0) {
        aboutText += " " + QString(CXX_COMPILER_VERSION);
    }
    aboutText += "\n";

    infoTextLabel->setText(aboutText);
    infoTextLabel->setTextFormat(Qt::PlainText);
    infoTextLabel->setWordWrap(true);
    // Align text to top if it's small, and allow expansion
    infoTextLabel->setAlignment(Qt::AlignTop); 
    mainLayout->addWidget(infoTextLabel, 1); // Add with stretch factor

    // Standard OK button
    okButton = new QPushButton("OK", this);
    connect(okButton, &QPushButton::clicked, this, &QDialog::accept);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(okButton);
    buttonLayout->addStretch();
    mainLayout->addLayout(buttonLayout);
    
    setLayout(mainLayout);
    
    // Set a reasonable default size, allow user resizing
    resize(400, 350); 
}

AboutDialog::~AboutDialog()
{
    // No explicit deletion needed for child widgets if QObject parenting is correct (which it is by passing `this`)
}
