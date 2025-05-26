#include "binaryfile.h"

BinaryFile::BinaryFile() : stream(&file) {
    stream.setByteOrder(QDataStream::LittleEndian);
}

BinaryFile::~BinaryFile() {
    close();
}

bool BinaryFile::open(const QString& filename, QIODevice::OpenMode mode) {
    file.setFileName(filename);
    if (!file.open(mode)) {
        lastError = file.errorString();
        return false;
    }
    return true;
}

void BinaryFile::close() {
    if (file.isOpen()) {
        file.close();
    }
}

bool BinaryFile::isOpen() const {
    return file.isOpen();
}

QString BinaryFile::errorString() const {
    return lastError;
}

bool BinaryFile::readU8(quint8& value) {
    if (!isOpen()) return false;
    stream >> value;
    return stream.status() == QDataStream::Ok;
}

bool BinaryFile::readU16(quint16& value) {
    if (!isOpen()) return false;
    stream >> value;
    return stream.status() == QDataStream::Ok;
}

bool BinaryFile::readU32(quint32& value) {
    if (!isOpen()) return false;
    stream >> value;
    return stream.status() == QDataStream::Ok;
}

bool BinaryFile::readString(QString& value) {
    if (!isOpen()) return false;
    quint16 length;
    if (!readU16(length)) return false;
    
    QByteArray data;
    data.resize(length);
    if (stream.readRawData(data.data(), length) != length) {
        return false;
    }
    
    value = QString::fromUtf8(data);
    return true;
}

bool BinaryFile::readRaw(char* buffer, qint64 size) {
    if (!isOpen()) return false;
    return stream.readRawData(buffer, size) == size;
}

bool BinaryFile::writeU8(quint8 value) {
    if (!isOpen()) return false;
    stream << value;
    return stream.status() == QDataStream::Ok;
}

bool BinaryFile::writeU16(quint16 value) {
    if (!isOpen()) return false;
    stream << value;
    return stream.status() == QDataStream::Ok;
}

bool BinaryFile::writeU32(quint32 value) {
    if (!isOpen()) return false;
    stream << value;
    return stream.status() == QDataStream::Ok;
}

bool BinaryFile::writeString(const QString& value) {
    if (!isOpen()) return false;
    QByteArray utf8 = value.toUtf8();
    if (!writeU16(utf8.size())) return false;
    return stream.writeRawData(utf8.constData(), utf8.size()) == utf8.size();
}

bool BinaryFile::writeRaw(const char* buffer, qint64 size) {
    if (!isOpen()) return false;
    return stream.writeRawData(buffer, size) == size;
}

qint64 BinaryFile::pos() const {
    return file.pos();
}

bool BinaryFile::seek(qint64 pos) {
    return file.seek(pos);
}

qint64 BinaryFile::size() const {
    return file.size();
} 