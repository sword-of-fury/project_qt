#ifndef XMLFILE_H
#define XMLFILE_H

#include <QObject>
#include <QDomDocument>
#include <QDomElement>
#include <QString>
#include <QList>
#include <QMap>

struct Spawn {
    QString name;
    QPoint position;
    int radius;
    QList<QPair<int, int>> creatures; // ID, ilość
};

struct House {
    QString name;
    QPoint position;
    int size;
    int rent;
    QString owner;
    QList<QPoint> doors;
    QList<QPoint> beds;
};

class XMLFile : public QObject {
    Q_OBJECT

public:
    explicit XMLFile(QObject* parent = nullptr);
    ~XMLFile();

    bool loadSpawns(const QString& filename);
    bool saveSpawns(const QString& filename);
    bool loadHouses(const QString& filename);
    bool saveHouses(const QString& filename);

    QList<Spawn> getSpawns() const { return spawns; }
    QList<House> getHouses() const { return houses; }

    void addSpawn(const Spawn& spawn);
    void removeSpawn(const QString& name);
    void addHouse(const House& house);
    void removeHouse(const QString& name);

private:
    bool parseSpawnElement(const QDomElement& element, Spawn& spawn);
    bool parseHouseElement(const QDomElement& element, House& house);
    QDomElement createSpawnElement(QDomDocument& doc, const Spawn& spawn);
    QDomElement createHouseElement(QDomDocument& doc, const House& house);

    QList<Spawn> spawns;
    QList<House> houses;
};

#endif // XMLFILE_H 