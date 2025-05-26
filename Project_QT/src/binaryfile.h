#ifndef BINARYFILE_H
#define BINARYFILE_H

#include <QFile>
#include <QString>
#include <QByteArray>
#include <QDataStream>
#include <QIODevice>

class BinaryFile {
public:
    BinaryFile();
    virtual ~BinaryFile();

    bool open(const QString& filename, QIODevice::OpenMode mode);
    void close();
    bool isOpen() const;
    QString errorString() const;

    // Podstawowe operacje odczytu
    bool readU8(quint8& value);
    bool readU16(quint16& value);
    bool readU32(quint32& value);
    bool readString(QString& value);
    bool readRaw(char* buffer, qint64 size);

    // Podstawowe operacje zapisu
    bool writeU8(quint8 value);
    bool writeU16(quint16 value);
    bool writeU32(quint32 value);
    bool writeString(const QString& value);
    bool writeRaw(const char* buffer, qint64 size);

    // Operacje na pozycji w pliku
    qint64 pos() const;
    bool seek(qint64 pos);
    qint64 size() const;

protected:
    QFile file;
    QDataStream stream;
    QString lastError;
};

#endif // BINARYFILE_H 