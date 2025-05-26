#include "helpsystem.h"
#include <QDebug>
#include <QDesktopServices>
#include <QDialog>
#include <QVBoxLayout>
#include <QTextBrowser>
#include <QPushButton>
#include <QLabel>

HelpSystem* HelpSystem::instance = nullptr;

HelpSystem& HelpSystem::getInstance()
{
    if (!instance) {
        instance = new HelpSystem();
    }
    return *instance;
}

HelpSystem::HelpSystem(QObject* parent)
    : QObject(parent)
{
    initializeHelpContents();
    initializeDocumentationUrls();
    initializeTooltips();
    initializeShortcutHelp();
}

HelpSystem::~HelpSystem()
{
    if (instance == this) {
        instance = nullptr;
    }
}

void HelpSystem::initializeHelpContents()
{
    // General help
    m_helpContents["general"] = tr(
        "<h2>IdlersMapEditor</h2>"
        "<p>IdlersMapEditor is a powerful map editor for creating and editing game maps.</p>"
        "<p>Use the tools panel on the left to select different editing tools, layers, and items.</p>"
        "<p>The main view in the center shows your map. You can zoom in/out using the mouse wheel.</p>"
        "<p>The status bar at the bottom shows information about the current position and selected item.</p>"
    );

    // Tools help
    m_helpContents["tools"] = tr(
        "<h2>Tools</h2>"
        "<p>The following tools are available:</p>"
        "<ul>"
        "<li><b>Pencil Tool</b>: Draw individual tiles on the map.</li>"
        "<li><b>Eraser Tool</b>: Remove tiles from the map.</li>"
        "<li><b>Fill Tool</b>: Fill an area with the selected tile.</li>"
        "<li><b>Selection Tool</b>: Select an area of the map for copying, cutting, or pasting.</li>"
        "</ul>"
    );

    // Layers help
    m_helpContents["layers"] = tr(
        "<h2>Layers</h2>"
        "<p>Maps are organized into layers:</p>"
        "<ul>"
        "<li><b>Ground Layer</b>: The base layer for terrain.</li>"
        "<li><b>Items Layer</b>: Objects placed on the ground.</li>"
        "<li><b>Creatures Layer</b>: NPCs and monsters.</li>"
        "</ul>"
        "<p>You can show/hide layers by clicking the eye icon next to each layer.</p>"
    );

    // Items help
    m_helpContents["items"] = tr(
        "<h2>Items</h2>"
        "<p>Items are objects that can be placed on the map.</p>"
        "<p>Select an item from the items panel and use the pencil tool to place it on the map.</p>"
        "<p>Items have properties such as walkable, stackable, etc.</p>"
    );

    // File operations help
    m_helpContents["file"] = tr(
        "<h2>File Operations</h2>"
        "<p>The following file operations are available:</p>"
        "<ul>"
        "<li><b>New Map</b>: Create a new empty map.</li>"
        "<li><b>Open Map</b>: Open an existing map file.</li>"
        "<li><b>Save Map</b>: Save the current map.</li>"
        "<li><b>Save Map As</b>: Save the current map with a new name.</li>"
        "<li><b>Export Map</b>: Export the map to different formats (OTBM, JSON, XML).</li>"
        "<li><b>Import Map</b>: Import a map from different formats.</li>"
        "</ul>"
    );

    // Keyboard shortcuts help
    m_helpContents["shortcuts"] = tr(
        "<h2>Keyboard Shortcuts</h2>"
        "<p>The following keyboard shortcuts are available:</p>"
        "<ul>"
        "<li><b>Ctrl+N</b>: New map</li>"
        "<li><b>Ctrl+O</b>: Open map</li>"
        "<li><b>Ctrl+S</b>: Save map</li>"
        "<li><b>Ctrl+Shift+S</b>: Save map as</li>"
        "<li><b>Ctrl+Z</b>: Undo</li>"
        "<li><b>Ctrl+Y</b>: Redo</li>"
        "<li><b>Ctrl+X</b>: Cut selection</li>"
        "<li><b>Ctrl+C</b>: Copy selection</li>"
        "<li><b>Ctrl+V</b>: Paste selection</li>"
        "<li><b>Delete</b>: Delete selection</li>"
        "<li><b>Ctrl+A</b>: Select all</li>"
        "<li><b>Ctrl+D</b>: Deselect all</li>"
        "<li><b>Ctrl++</b>: Zoom in</li>"
        "<li><b>Ctrl+-</b>: Zoom out</li>"
        "<li><b>Ctrl+0</b>: Reset zoom</li>"
        "<li><b>F1</b>: Show help</li>"
        "</ul>"
    );
}

void HelpSystem::initializeDocumentationUrls()
{
    // Local documentation
    m_documentationUrls["user_manual"] = QUrl("file:///docs/user_manual.html");
    m_documentationUrls["api_reference"] = QUrl("file:///docs/api_reference.html");
    m_documentationUrls["tutorials"] = QUrl("file:///docs/tutorials.html");

    // Online documentation
    m_documentationUrls["online_manual"] = QUrl("https://idlersmapeditor.example.com/manual");
    m_documentationUrls["online_tutorials"] = QUrl("https://idlersmapeditor.example.com/tutorials");
    m_documentationUrls["forum"] = QUrl("https://idlersmapeditor.example.com/forum");
}

void HelpSystem::initializeTooltips()
{
    // Tool tooltips
    m_tooltips["pencil_tool"] = tr("Pencil Tool: Draw individual tiles on the map");
    m_tooltips["eraser_tool"] = tr("Eraser Tool: Remove tiles from the map");
    m_tooltips["fill_tool"] = tr("Fill Tool: Fill an area with the selected tile");
    m_tooltips["selection_tool"] = tr("Selection Tool: Select an area of the map");

    // Layer tooltips
    m_tooltips["ground_layer"] = tr("Ground Layer: Base terrain layer");
    m_tooltips["items_layer"] = tr("Items Layer: Objects placed on the ground");
    m_tooltips["creatures_layer"] = tr("Creatures Layer: NPCs and monsters");

    // Action tooltips
    m_tooltips["new_map"] = tr("Create a new empty map");
    m_tooltips["open_map"] = tr("Open an existing map file");
    m_tooltips["save_map"] = tr("Save the current map");
    m_tooltips["save_map_as"] = tr("Save the current map with a new name");
    m_tooltips["export_map"] = tr("Export the map to different formats");
    m_tooltips["import_map"] = tr("Import a map from different formats");
}

void HelpSystem::initializeShortcutHelp()
{
    // File actions
    m_shortcutHelp["new_map"] = tr("Ctrl+N");
    m_shortcutHelp["open_map"] = tr("Ctrl+O");
    m_shortcutHelp["save_map"] = tr("Ctrl+S");
    m_shortcutHelp["save_map_as"] = tr("Ctrl+Shift+S");

    // Edit actions
    m_shortcutHelp["undo"] = tr("Ctrl+Z");
    m_shortcutHelp["redo"] = tr("Ctrl+Y");
    m_shortcutHelp["cut"] = tr("Ctrl+X");
    m_shortcutHelp["copy"] = tr("Ctrl+C");
    m_shortcutHelp["paste"] = tr("Ctrl+V");
    m_shortcutHelp["delete"] = tr("Delete");
    m_shortcutHelp["select_all"] = tr("Ctrl+A");
    m_shortcutHelp["deselect_all"] = tr("Ctrl+D");

    // View actions
    m_shortcutHelp["zoom_in"] = tr("Ctrl++");
    m_shortcutHelp["zoom_out"] = tr("Ctrl+-");
    m_shortcutHelp["zoom_reset"] = tr("Ctrl+0");

    // Help actions
    m_shortcutHelp["show_help"] = tr("F1");
}

QString HelpSystem::getHelpContent(const QString& topic) const
{
    return m_helpContents.value(topic.toLower(), m_helpContents.value("general"));
}

QStringList HelpSystem::getAllTopics() const
{
    return m_helpContents.keys();
}

bool HelpSystem::hasHelpFor(const QString& topic) const
{
    return m_helpContents.contains(topic.toLower());
}

QUrl HelpSystem::getDocumentationUrl(const QString& section) const
{
    return m_documentationUrls.value(section.toLower(), m_documentationUrls.value("user_manual"));
}

QStringList HelpSystem::getAllDocumentationSections() const
{
    return m_documentationUrls.keys();
}

QString HelpSystem::getTooltip(const QString& widget) const
{
    return m_tooltips.value(widget.toLower());
}

QString HelpSystem::getShortcutHelp(const QString& action) const
{
    return m_shortcutHelp.value(action.toLower());
}

void HelpSystem::showHelpDialog(const QString& topic)
{
    QDialog* helpDialog = new QDialog();
    helpDialog->setWindowTitle(tr("IdlersMapEditor Help"));
    helpDialog->setMinimumSize(600, 400);

    QVBoxLayout* layout = new QVBoxLayout(helpDialog);

    QTextBrowser* textBrowser = new QTextBrowser(helpDialog);
    textBrowser->setOpenExternalLinks(true);
    textBrowser->setHtml(getHelpContent(topic));

    QHBoxLayout* topicsLayout = new QHBoxLayout();
    QLabel* topicsLabel = new QLabel(tr("Topics:"), helpDialog);
    topicsLayout->addWidget(topicsLabel);

    for (const QString& t : getAllTopics()) {
        QPushButton* topicButton = new QPushButton(t, helpDialog);
        connect(topicButton, &QPushButton::clicked, [this, textBrowser, t]() {
            textBrowser->setHtml(getHelpContent(t));
        });
        topicsLayout->addWidget(topicButton);
    }

    QPushButton* closeButton = new QPushButton(tr("Close"), helpDialog);
    connect(closeButton, &QPushButton::clicked, helpDialog, &QDialog::accept);

    layout->addLayout(topicsLayout);
    layout->addWidget(textBrowser);
    layout->addWidget(closeButton, 0, Qt::AlignRight);

    helpDialog->setLayout(layout);
    helpDialog->setAttribute(Qt::WA_DeleteOnClose);
    helpDialog->show();
}

void HelpSystem::showDocumentation(const QString& section)
{
    QUrl url = getDocumentationUrl(section);
    QDesktopServices::openUrl(url);
}
