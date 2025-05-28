#include "editorpreferencestab.h"
#include <QVBoxLayout>

namespace GlobalSettingsPlaceholder {
    bool getBoolean(const std::string& key, bool defaultValue = false) { Q_UNUSED(key); return defaultValue; }
}
namespace ConfigPlaceholder {
    const std::string GROUP_ACTIONS = "GroupActions";
    const std::string WARN_FOR_DUPLICATE_ID = "WarnForDuplicateID";
    const std::string HOUSE_BRUSH_REMOVE_ITEMS = "HouseBrushRemoveItems";
    const std::string AUTO_ASSIGN_DOORID = "AutoAssignDoorID";
    const std::string DOODAD_BRUSH_ERASE_LIKE = "DoodadBrushEraseLike";
    const std::string ERASER_LEAVE_UNIQUE = "EraserLeaveUnique";
    const std::string AUTO_CREATE_SPAWN = "AutoCreateSpawn";
    const std::string RAW_LIKE_SIMONE = "RawLikeSimone";
    const std::string MERGE_MOVE = "MergeMove";
    const std::string MERGE_PASTE = "MergePaste";
}

EditorPreferencesTab::EditorPreferencesTab(QWidget *parent) : QWidget(parent)
{
    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(5, 5, 5, 5);
    mainLayout->setSpacing(5);

    groupActionsChkBox = new QCheckBox(tr("Group same-type actions"));
    groupActionsChkBox->setToolTip(tr("This will group actions of the same type (drawing, selection..) when several take place in consecutive order."));
    mainLayout->addWidget(groupActionsChkBox);

    duplicateIdWarnChkBox = new QCheckBox(tr("Warn for duplicate IDs"));
    duplicateIdWarnChkBox->setToolTip(tr("Warns for most kinds of duplicate IDs."));
    mainLayout->addWidget(duplicateIdWarnChkBox);

    houseRemoveChkBox = new QCheckBox(tr("House brush removes items"));
    houseRemoveChkBox->setToolTip(tr("When this option is checked, the house brush will automaticly remove items that will respawn every time the map is loaded."));
    mainLayout->addWidget(houseRemoveChkBox);

    autoAssignDoorsChkBox = new QCheckBox(tr("Auto-assign door ids"));
    autoAssignDoorsChkBox->setToolTip(tr("This will auto-assign unique door ids to all doors placed with the door brush (or doors painted over with the house brush).\nDoes NOT affect doors placed using the RAW palette."));
    mainLayout->addWidget(autoAssignDoorsChkBox);

    doodadEraseSameChkBox = new QCheckBox(tr("Doodad brush only erases same"));
    doodadEraseSameChkBox->setToolTip(tr("The doodad brush will only erase items that belongs to the current brush."));
    mainLayout->addWidget(doodadEraseSameChkBox);

    eraserLeaveUniqueChkBox = new QCheckBox(tr("Eraser leaves unique items"));
    eraserLeaveUniqueChkBox->setToolTip(tr("The eraser will leave containers with items in them, items with unique or action id and items."));
    mainLayout->addWidget(eraserLeaveUniqueChkBox);

    autoCreateSpawnChkBox = new QCheckBox(tr("Auto create spawn when placing creature"));
    autoCreateSpawnChkBox->setToolTip(tr("When this option is checked, you can place creatures without placing a spawn manually, the spawn will be place automatically."));
    mainLayout->addWidget(autoCreateSpawnChkBox);

    allowMultipleOrderItemsChkBox = new QCheckBox(tr("Prevent toporder conflict"));
    allowMultipleOrderItemsChkBox->setToolTip(tr("When this option is checked, you can not place several items with the same toporder on one tile using a RAW Brush."));
    mainLayout->addWidget(allowMultipleOrderItemsChkBox);

    mainLayout->addSpacing(10);

    mergeMoveChkBox = new QCheckBox(tr("Use merge move"));
    mergeMoveChkBox->setToolTip(tr("Moved tiles won't replace already placed tiles."));
    mainLayout->addWidget(mergeMoveChkBox);

    mergePasteChkBox = new QCheckBox(tr("Use merge paste"));
    mergePasteChkBox->setToolTip(tr("Pasted tiles won't replace already placed tiles."));
    mainLayout->addWidget(mergePasteChkBox);

    mainLayout->addStretch(1);
    setLayout(mainLayout);
}

EditorPreferencesTab::~EditorPreferencesTab() {}
void EditorPreferencesTab::loadSettings() {}
void EditorPreferencesTab::applySettings() {}
