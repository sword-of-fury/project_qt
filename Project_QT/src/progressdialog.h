#ifndef PROGRESSDIALOG_H
#define PROGRESSDIALOG_H

#include <QDialog>
#include <QProgressBar>
#include <QLabel>
#include <QPushButton>
#include <QTimer>

/**
 * @brief A dialog that shows progress for long-running operations
 * 
 * This class provides a simple progress dialog with a progress bar,
 * status label, and optional cancel button. It can be used for
 * file operations, map generation, and other time-consuming tasks.
 */
class ProgressDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param title Dialog title
     * @param labelText Initial label text
     * @param cancelButtonText Text for cancel button (empty for no cancel button)
     * @param parent Parent widget
     */
    explicit ProgressDialog(const QString& title, 
                           const QString& labelText = QString(),
                           const QString& cancelButtonText = QString(),
                           QWidget* parent = nullptr);
    ~ProgressDialog();

    /**
     * @brief Set the range of the progress bar
     * @param minimum Minimum value
     * @param maximum Maximum value
     */
    void setRange(int minimum, int maximum);

    /**
     * @brief Set the current progress value
     * @param value Current progress value
     */
    void setValue(int value);

    /**
     * @brief Set the label text
     * @param text New label text
     */
    void setLabelText(const QString& text);

    /**
     * @brief Set whether the dialog is modal
     * @param modal True for modal, false for non-modal
     */
    void setModal(bool modal);

    /**
     * @brief Check if the operation was canceled
     * @return True if canceled, false otherwise
     */
    bool wasCanceled() const;

    /**
     * @brief Auto-close the dialog when progress reaches maximum
     * @param autoClose True to auto-close, false to keep open
     */
    void setAutoClose(bool autoClose);

    /**
     * @brief Auto-reset the progress bar when progress reaches maximum
     * @param autoReset True to auto-reset, false to keep at maximum
     */
    void setAutoReset(bool autoReset);

signals:
    /**
     * @brief Signal emitted when the cancel button is clicked
     */
    void canceled();

public slots:
    /**
     * @brief Reset the progress bar to minimum
     */
    void reset();

    /**
     * @brief Set the progress to the maximum value
     */
    void setMaximum();

private slots:
    void onCancelClicked();

private:
    QProgressBar* progressBar;
    QLabel* label;
    QPushButton* cancelButton;
    bool isCanceled;
    bool autoCloseEnabled;
    bool autoResetEnabled;
};

#endif // PROGRESSDIALOG_H
