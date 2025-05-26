#ifndef MAPPROPERTIESDIALOG_H
#define MAPPROPERTIESDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QGridLayout>
#include <QDialogButtonBox>
#include "map.h"

class MapPropertiesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MapPropertiesDialog(QWidget* parent = nullptr, Map* map = nullptr);
    ~MapPropertiesDialog();

    // Get the updated map properties
    QString getDescription() const;
    QString getSpawnFile() const;
    QString getHouseFile() const;

private slots:
    void onOkClicked();
    void onCancelClicked();
    void onResetClicked();

private:
    void setupUi();
    void loadMapProperties();
    void saveMapProperties();

    Map* map;

    // UI elements
    QLineEdit* descriptionEdit;
    QSpinBox* widthSpinBox;
    QSpinBox* heightSpinBox;
    QLineEdit* spawnFileEdit;
    QLineEdit* houseFileEdit;
    QPushButton* browseSpawnButton;
    QPushButton* browseHouseButton;
    QDialogButtonBox* buttonBox;
};

#endif // MAPPROPERTIESDIALOG_H
