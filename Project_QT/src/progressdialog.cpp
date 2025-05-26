#include "progressdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QApplication>

ProgressDialog::ProgressDialog(const QString& title, 
                             const QString& labelText,
                             const QString& cancelButtonText,
                             QWidget* parent)
    : QDialog(parent)
    , isCanceled(false)
    , autoCloseEnabled(true)
    , autoResetEnabled(true)
{
    // Set window properties
    setWindowTitle(title);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowModality(Qt::ApplicationModal);
    
    // Create layout
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // Create label
    label = new QLabel(labelText, this);
    mainLayout->addWidget(label);
    
    // Create progress bar
    progressBar = new QProgressBar(this);
    progressBar->setRange(0, 100);
    progressBar->setValue(0);
    progressBar->setTextVisible(true);
    mainLayout->addWidget(progressBar);
    
    // Create cancel button if text is provided
    if (!cancelButtonText.isEmpty()) {
        QHBoxLayout* buttonLayout = new QHBoxLayout();
        cancelButton = new QPushButton(cancelButtonText, this);
        connect(cancelButton, &QPushButton::clicked, this, &ProgressDialog::onCancelClicked);
        buttonLayout->addStretch();
        buttonLayout->addWidget(cancelButton);
        mainLayout->addLayout(buttonLayout);
    } else {
        cancelButton = nullptr;
    }
    
    // Set size
    resize(300, 100);
}

ProgressDialog::~ProgressDialog()
{
}

void ProgressDialog::setRange(int minimum, int maximum)
{
    progressBar->setRange(minimum, maximum);
}

void ProgressDialog::setValue(int value)
{
    progressBar->setValue(value);
    
    // Process events to update UI
    QApplication::processEvents();
    
    // Check if we've reached the maximum
    if (value >= progressBar->maximum()) {
        if (autoCloseEnabled) {
            hide();
        }
        
        if (autoResetEnabled) {
            reset();
        }
    }
}

void ProgressDialog::setLabelText(const QString& text)
{
    label->setText(text);
}

void ProgressDialog::setModal(bool modal)
{
    setWindowModality(modal ? Qt::ApplicationModal : Qt::NonModal);
}

bool ProgressDialog::wasCanceled() const
{
    return isCanceled;
}

void ProgressDialog::setAutoClose(bool autoClose)
{
    autoCloseEnabled = autoClose;
}

void ProgressDialog::setAutoReset(bool autoReset)
{
    autoResetEnabled = autoReset;
}

void ProgressDialog::reset()
{
    progressBar->reset();
    isCanceled = false;
}

void ProgressDialog::setMaximum()
{
    setValue(progressBar->maximum());
}

void ProgressDialog::onCancelClicked()
{
    isCanceled = true;
    emit canceled();
    hide();
}
