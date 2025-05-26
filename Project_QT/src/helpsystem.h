#ifndef HELPSYSTEM_H
#define HELPSYSTEM_H

#include <QObject>
#include <QString>
#include <QMap>
#include <QUrl>

class HelpSystem : public QObject
{
    Q_OBJECT

public:
    static HelpSystem& getInstance();
    ~HelpSystem();

    // Help topics
    QString getHelpContent(const QString& topic) const;
    QStringList getAllTopics() const;
    bool hasHelpFor(const QString& topic) const;

    // Documentation
    QUrl getDocumentationUrl(const QString& section) const;
    QStringList getAllDocumentationSections() const;

    // Quick help
    QString getTooltip(const QString& widget) const;
    QString getShortcutHelp(const QString& action) const;

    // Show help dialog
    void showHelpDialog(const QString& topic = QString());
    void showDocumentation(const QString& section = QString());

private:
    HelpSystem(QObject* parent = nullptr);
    static HelpSystem* instance;

    QMap<QString, QString> m_helpContents;
    QMap<QString, QUrl> m_documentationUrls;
    QMap<QString, QString> m_tooltips;
    QMap<QString, QString> m_shortcutHelp;

    void initializeHelpContents();
    void initializeDocumentationUrls();
    void initializeTooltips();
    void initializeShortcutHelp();
};

#endif // HELPSYSTEM_H
