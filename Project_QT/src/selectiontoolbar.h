#ifndef SELECTIONTOOLBAR_H
#define SELECTIONTOOLBAR_H

#include <QToolBar>
#include <QAction>
#include <QSpinBox>
#include <QComboBox>
#include <QLabel>

class MapView;
class SelectionBrush;

class SelectionToolbar : public QToolBar
{
    Q_OBJECT

public:
    explicit SelectionToolbar(const QString& title, QWidget* parent = nullptr);
    ~SelectionToolbar();

    void setMapView(MapView* view);
    void setSelectionBrush(SelectionBrush* brush);

private slots:
    void onMoveSelection();
    void onCopySelection();
    void onRotateSelection();
    void onFlipHorizontally();
    void onFlipVertically();
    void onDeleteSelection();
    void onSelectionModeChanged(int index);
    void onSelectionChanged(const QRect& selection);
    void onSelectionCleared();

private:
    void setupActions();
    void updateActionStates(bool hasSelection);

    MapView* mapView;
    SelectionBrush* selectionBrush;

    // Actions
    QAction* moveAction;
    QAction* copyAction;
    QAction* rotateAction;
    QAction* flipHAction;
    QAction* flipVAction;
    QAction* deleteAction;

    // Widgets
    QComboBox* selectionModeCombo;
    QSpinBox* offsetXSpin;
    QSpinBox* offsetYSpin;
    QComboBox* rotationCombo;
    QLabel* selectionInfoLabel;
};

#endif // SELECTIONTOOLBAR_H
