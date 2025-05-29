#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QDialog>

// Forward declaration if not using .ui file and QLabel/QPushButton are private members
QT_BEGIN_NAMESPACE
class QLabel;
class QPushButton;
QT_END_NAMESPACE

class AboutDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AboutDialog(QWidget *parent = nullptr);
    ~AboutDialog();

private:
    // If not using .ui file, declare UI elements here
    QLabel *infoTextLabel;
    QPushButton *okButton;
};

#endif // ABOUTDIALOG_H
