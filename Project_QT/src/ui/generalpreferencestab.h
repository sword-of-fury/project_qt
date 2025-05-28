#ifndef GENERALPREFERENCESTAB_H
#define GENERALPREFERENCESTAB_H

#include <QWidget>
#include <QCheckBox>
#include <QSpinBox>
#include <QGroupBox> // For RadioBox equivalent
#include <QRadioButton>
#include <QLabel> // For grid layout labels

class QVBoxLayout; // Forward declaration
class QHBoxLayout; // Forward declaration
class QGridLayout; // Forward declaration

class GeneralPreferencesTab : public QWidget
{
    Q_OBJECT

public:
    explicit GeneralPreferencesTab(QWidget *parent = nullptr);
    ~GeneralPreferencesTab();

    void loadSettings();
    void applySettings();

private:
    QCheckBox *showWelcomeDialogChkBox;
    QCheckBox *alwaysMakeBackupChkBox;
    QCheckBox *updateCheckOnStartupChkBox;
    QCheckBox *onlyOneInstanceChkBox;
    QCheckBox *enableTilesetEditingChkBox;
    QCheckBox *autoSelectRawChkBox;

    QCheckBox *autosaveChkBox;
    QSpinBox *autosaveIntervalSpin;

    QSpinBox *undoSizeSpin;
    QSpinBox *undoMemSizeSpin;
    QSpinBox *workerThreadsSpin;
    QSpinBox *replaceSizeSpin;

    QGroupBox *positionFormatGroupBox;
    QRadioButton *positionFormatRadio0;
    QRadioButton *positionFormatRadio1;
    QRadioButton *positionFormatRadio2;
    QRadioButton *positionFormatRadio3;
    QRadioButton *positionFormatRadio4;

    QVBoxLayout *mainLayout;
    QHBoxLayout *autosaveLayout;
    QGridLayout *gridLayout;
    QVBoxLayout *positionFormatLayout;
};

#endif // GENERALPREFERENCESTAB_H
