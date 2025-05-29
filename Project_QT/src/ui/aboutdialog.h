#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QDialog>

// Forward declarations for Qt classes to minimize include dependencies in headers
class QLabel;
class QPushButton;
class QVBoxLayout;
class QHBoxLayout;
class QDialogButtonBox; // Added for standard button box

class AboutDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AboutDialog(QWidget* parent = nullptr);
    ~AboutDialog(); // Good practice to have a destructor

private slots:
    void showLicense();

private:
    // UI Elements
    QLabel* titleLabel; // For "[AppName] - [Version]"
    QLabel* mainTextLabel; // For the main body of about text
    // QPushButton* okButton; // Will be part of QDialogButtonBox
    // QPushButton* licenseButton; // Will be part of QDialogButtonBox
    QDialogButtonBox* buttonBox;

    // Layouts (optional to declare as members if only used in constructor)
    // QVBoxLayout* mainLayout;
    // QHBoxLayout* buttonLayout; // Will be managed by QDialogButtonBox
};

#endif // ABOUTDIALOG_H
