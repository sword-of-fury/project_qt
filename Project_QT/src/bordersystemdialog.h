#ifndef BORDERSYSTEMDIALOG_H
#define BORDERSYSTEMDIALOG_H

#include <QDialog>
#include <QTableWidget>
#include <QPushButton>
#include <QCheckBox>
#include <QSpinBox>
#include <QComboBox>
#include <QLabel>

class BorderSystem;

/**
 * @brief Dialog for configuring the Automagic Border System
 * 
 * This dialog allows users to configure the Automagic Border System by
 * defining which tiles should have borders and which tiles should be used
 * as border tiles.
 */
class BorderSystemDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param borderSystem Pointer to the border system to configure
     * @param parent Parent widget
     */
    explicit BorderSystemDialog(BorderSystem* borderSystem, QWidget* parent = nullptr);
    ~BorderSystemDialog();

private slots:
    void onAddMapping();
    void onRemoveMapping();
    void onEnableToggled(bool checked);
    void onApply();
    void onOk();
    void onCancel();

private:
    void setupUi();
    void loadMappings();
    void saveMappings();

    BorderSystem* borderSystem;
    
    // UI elements
    QTableWidget* mappingsTable;
    QPushButton* addButton;
    QPushButton* removeButton;
    QCheckBox* enableCheckBox;
    QPushButton* applyButton;
    QPushButton* okButton;
    QPushButton* cancelButton;
};

#endif // BORDERSYSTEMDIALOG_H
