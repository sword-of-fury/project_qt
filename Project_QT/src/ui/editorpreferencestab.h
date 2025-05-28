#ifndef EDITORPREFERENCESTAB_H
#define EDITORPREFERENCESTAB_H

#include <QWidget>
#include <QCheckBox>

class QVBoxLayout;

class EditorPreferencesTab : public QWidget
{
    Q_OBJECT

public:
    explicit EditorPreferencesTab(QWidget *parent = nullptr);
    ~EditorPreferencesTab();

    void loadSettings();
    void applySettings();

private:
    QCheckBox *groupActionsChkBox;
    QCheckBox *duplicateIdWarnChkBox;
    QCheckBox *houseRemoveChkBox;
    QCheckBox *autoAssignDoorsChkBox;
    QCheckBox *doodadEraseSameChkBox;
    QCheckBox *eraserLeaveUniqueChkBox;
    QCheckBox *autoCreateSpawnChkBox;
    QCheckBox *allowMultipleOrderItemsChkBox;
    QCheckBox *mergeMoveChkBox;
    QCheckBox *mergePasteChkBox;

    QVBoxLayout *mainLayout;
};

#endif // EDITORPREFERENCESTAB_H
