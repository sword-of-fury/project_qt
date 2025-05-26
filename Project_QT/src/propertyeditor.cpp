#include "propertyeditor.h"

PropertyEditor::PropertyEditor(QWidget* parent)
    : QWidget(parent)
{
}

PropertyEditor::~PropertyEditor()
{
}

void PropertyEditor::setProperty(const QString& name, const QVariant& value)
{
    properties[name] = value;
    emit propertyChanged(name, value);
}

QVariant PropertyEditor::getProperty(const QString& name) const
{
    return properties.value(name);
}

void PropertyEditor::clearProperties()
{
    properties.clear();
}
