#ifndef GOTOPOSITIONDIALOG_H
#define GOTOPOSITIONDIALOG_H

#include <QDialog>
#include <QSpinBox>
#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>
#include "map.h"

class GotoPositionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GotoPositionDialog(QWidget* parent = nullptr, Map* map = nullptr);
    ~GotoPositionDialog();

    // Get the selected position
    QPoint getPosition() const;
    int getLayer() const;

private slots:
    void onOkClicked();
    void onCancelClicked();
    void onCurrentPositionClicked();

private:
    void setupUi();

    Map* map;

    // UI elements
    QSpinBox* xSpinBox;
    QSpinBox* ySpinBox;
    QSpinBox* zSpinBox;
    QPushButton* currentPositionButton;
    QDialogButtonBox* buttonBox;
};

#endif // GOTOPOSITIONDIALOG_H
