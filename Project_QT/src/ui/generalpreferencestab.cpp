#include "generalpreferencestab.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QGroupBox>
#include <QRadioButton>

namespace GlobalSettingsPlaceholder {
    bool getBoolean(const std::string& key, bool defaultValue = false) { Q_UNUSED(key); return defaultValue; }
    int getInteger(const std::string& key, int defaultValue = 0) { Q_UNUSED(key); return defaultValue; }
}
namespace ConfigPlaceholder {
    const std::string WELCOME_DIALOG = "WelcomeDialog";
    const std::string ALWAYS_MAKE_BACKUP = "AlwaysMakeBackup";
    const std::string USE_UPDATER = "UseUpdater";
    const std::string ONLY_ONE_INSTANCE = "OnlyOneInstance";
    const std::string SHOW_TILESET_EDITOR = "ShowTilesetEditor";
    const std::string AUTO_SELECT_RAW_ON_RIGHTCLICK = "AutoSelectRawOnRightClick";
    const std::string AUTO_SAVE_ENABLED = "AutoSaveEnabled";
    const std::string AUTO_SAVE_INTERVAL = "AutoSaveInterval";
    const std::string UNDO_SIZE = "UndoSize";
    const std::string UNDO_MEM_SIZE = "UndoMemSize";
    const std::string WORKER_THREADS = "WorkerThreads";
    const std::string REPLACE_SIZE = "ReplaceSize";
    const std::string COPY_POSITION_FORMAT = "CopyPositionFormat";
}

GeneralPreferencesTab::GeneralPreferencesTab(QWidget *parent) : QWidget(parent)
{
    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(5, 5, 5, 5);
    mainLayout->setSpacing(5);

    showWelcomeDialogChkBox = new QCheckBox(tr("Show welcome dialog on startup"));
    showWelcomeDialogChkBox->setToolTip(tr("Show welcome dialog when starting the editor."));
    mainLayout->addWidget(showWelcomeDialogChkBox);

    alwaysMakeBackupChkBox = new QCheckBox(tr("Always make map backup"));
    mainLayout->addWidget(alwaysMakeBackupChkBox);

    updateCheckOnStartupChkBox = new QCheckBox(tr("Check for updates on startup"));
    mainLayout->addWidget(updateCheckOnStartupChkBox);

    onlyOneInstanceChkBox = new QCheckBox(tr("Open all maps in the same instance"));
    onlyOneInstanceChkBox->setToolTip(tr("When checked, maps opened using the shell will all be opened in the same instance.\nTo run multiple instances regardless of this setting, use the RunMultipleInstances.bat file or -force-multi-instance parameter."));
    mainLayout->addWidget(onlyOneInstanceChkBox);

    enableTilesetEditingChkBox = new QCheckBox(tr("Enable tileset editing"));
    enableTilesetEditingChkBox->setToolTip(tr("Show tileset editing options."));
    mainLayout->addWidget(enableTilesetEditingChkBox);

    autoSelectRawChkBox = new QCheckBox(tr("Auto-select RAW on right-click"));
    autoSelectRawChkBox->setToolTip(tr("Automatically selects RAW brush when right-clicking items while showing the context menu."));
    mainLayout->addWidget(autoSelectRawChkBox);

    mainLayout->addSpacing(10);

    autosaveLayout = new QHBoxLayout();
    autosaveChkBox = new QCheckBox(tr("Enable autosave"));
    autosaveChkBox->setToolTip(tr("Automatically save a backup of your map periodically"));
    autosaveLayout->addWidget(autosaveChkBox);

    autosaveIntervalSpin = new QSpinBox();
    autosaveIntervalSpin->setRange(1, 7200);
    autosaveIntervalSpin->setToolTip(tr("How often (in seconds) should autosave occur"));
    autosaveLayout->addWidget(autosaveIntervalSpin);

    autosaveLayout->addWidget(new QLabel(tr("seconds")));
    autosaveLayout->addStretch(1);
    mainLayout->addLayout(autosaveLayout);

    gridLayout = new QGridLayout();
    gridLayout->setColumnStretch(1, 1);
    gridLayout->setHorizontalSpacing(10);
    gridLayout->setVerticalSpacing(10);

    QLabel* undoSizeLabel = new QLabel(tr("Undo queue size: "));
    undoSizeSpin = new QSpinBox();
    undoSizeSpin->setRange(0, 0x10000000);
    undoSizeLabel->setToolTip(tr("How many action you can undo, be aware that a high value will increase memory usage."));
    undoSizeSpin->setToolTip(tr("How many action you can undo, be aware that a high value will increase memory usage."));
    gridLayout->addWidget(undoSizeLabel, 0, 0);
    gridLayout->addWidget(undoSizeSpin, 0, 1);

    QLabel* undoMemSizeLabel = new QLabel(tr("Undo maximum memory size (MB): "));
    undoMemSizeSpin = new QSpinBox();
    undoMemSizeSpin->setRange(0, 4096);
    undoMemSizeLabel->setToolTip(tr("The approximite limit for the memory usage of the undo queue."));
    undoMemSizeSpin->setToolTip(tr("The approximite limit for the memory usage of the undo queue."));
    gridLayout->addWidget(undoMemSizeLabel, 1, 0);
    gridLayout->addWidget(undoMemSizeSpin, 1, 1);

    QLabel* workerThreadsLabel = new QLabel(tr("Worker Threads: "));
    workerThreadsSpin = new QSpinBox();
    workerThreadsSpin->setRange(1, 64);
    workerThreadsLabel->setToolTip(tr("How many threads the editor will use for intensive operations. This should be equivalent to the amount of logical processors in your system."));
    workerThreadsSpin->setToolTip(tr("How many threads the editor will use for intensive operations. This should be equivalent to the amount of logical processors in your system."));
    gridLayout->addWidget(workerThreadsLabel, 2, 0);
    gridLayout->addWidget(workerThreadsSpin, 2, 1);

    QLabel* replaceSizeLabel = new QLabel(tr("Replace count: "));
    replaceSizeSpin = new QSpinBox();
    replaceSizeSpin->setRange(0, 100000);
    replaceSizeLabel->setToolTip(tr("How many items you can replace on the map using the Replace Item tool."));
    replaceSizeSpin->setToolTip(tr("How many items you can replace on the map using the Replace Item tool."));
    gridLayout->addWidget(replaceSizeLabel, 3, 0);
    gridLayout->addWidget(replaceSizeSpin, 3, 1);

    mainLayout->addLayout(gridLayout);
    mainLayout->addSpacing(10);

    positionFormatGroupBox = new QGroupBox(tr("Copy Position Format"));
    positionFormatLayout = new QVBoxLayout(positionFormatGroupBox);

    positionFormatRadio0 = new QRadioButton(tr("  {x = 0, y = 0, z = 0}"));
    positionFormatRadio1 = new QRadioButton(tr("  {\"x\":0,\"y\":0,\"z\":0}"));
    positionFormatRadio2 = new QRadioButton(tr("  x, y, z"));
    positionFormatRadio3 = new QRadioButton(tr("  (x, y, z)"));
    positionFormatRadio4 = new QRadioButton(tr("  Position(x, y, z)"));

    positionFormatLayout->addWidget(positionFormatRadio0);
    positionFormatLayout->addWidget(positionFormatRadio1);
    positionFormatLayout->addWidget(positionFormatRadio2);
    positionFormatLayout->addWidget(positionFormatRadio3);
    positionFormatLayout->addWidget(positionFormatRadio4);
    positionFormatRadio0->setChecked(true);

    positionFormatGroupBox->setToolTip(tr("The position format when copying from the map."));
    mainLayout->addWidget(positionFormatGroupBox);

    mainLayout->addStretch(1);
    setLayout(mainLayout);
}

GeneralPreferencesTab::~GeneralPreferencesTab() {}
void GeneralPreferencesTab::loadSettings() {}
void GeneralPreferencesTab::applySettings() {}
