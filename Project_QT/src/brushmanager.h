#ifndef BRUSHMANAGER_H
#define BRUSHMANAGER_H

#include <QObject>
#include <QMap>
#include "brush.h"

class BrushManager : public QObject
{
    Q_OBJECT

public:
    static BrushManager& getInstance();
    ~BrushManager();

    Brush* createBrush(Brush::Type type, QObject* parent = nullptr);
    Brush* getBrush(Brush::Type type) const;
    QList<Brush*> getAllBrushes() const;
    
    QString getTypeName(Brush::Type type) const;
    QIcon getTypeIcon(Brush::Type type) const;

signals:
    void brushCreated(Brush* brush);
    void brushChanged(Brush* brush);

private:
    BrushManager(QObject* parent = nullptr);
    static BrushManager* instance;

    QMap<Brush::Type, Brush*> m_brushes;
    QMap<Brush::Type, QString> m_typeNames;
    QMap<Brush::Type, QIcon> m_typeIcons;

    void initializeTypeNames();
    void initializeTypeIcons();
};

#endif // BRUSHMANAGER_H
