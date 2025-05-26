#ifndef PROPERTYEDITOR_H
#define PROPERTYEDITOR_H

#include <QWidget>
#include <QMap>
#include <QString>
#include <QVariant>

/**
 * @brief Base class for property editors
 * 
 * This class provides a common interface for all property editors.
 * Specific property editors (TilePropertyEditor, ItemPropertyEditor, etc.)
 * should inherit from this class.
 */
class PropertyEditor : public QWidget
{
    Q_OBJECT

public:
    explicit PropertyEditor(QWidget* parent = nullptr);
    virtual ~PropertyEditor();

    /**
     * @brief Set a property value
     * @param name Property name
     * @param value Property value
     */
    virtual void setProperty(const QString& name, const QVariant& value);

    /**
     * @brief Get a property value
     * @param name Property name
     * @return Property value
     */
    virtual QVariant getProperty(const QString& name) const;

    /**
     * @brief Clear all properties
     */
    virtual void clearProperties();

    /**
     * @brief Apply changes to the edited object
     */
    virtual void applyChanges() = 0;

signals:
    /**
     * @brief Signal emitted when a property changes
     * @param name Property name
     * @param value New property value
     */
    void propertyChanged(const QString& name, const QVariant& value);

protected:
    QMap<QString, QVariant> properties;
};

#endif // PROPERTYEDITOR_H
