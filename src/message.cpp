#include "dji/message.h"
#include "dji/crc.h"
#include <QDebug>
#include <QtEndian>

namespace dji {

QByteArray packString(const QString &s) {
    QByteArray res;
    QByteArray utf8 = s.toUtf8();
    uint8_t len = static_cast<uint8_t>(utf8.length());
    res.append(static_cast<char>(len));
    res.append(utf8);
    return res;
}

QByteArray packURL(const QString &s) {
    QByteArray res;
    QByteArray utf8 = s.toUtf8();
    uint16_t len = qToLittleEndian<uint16_t>(static_cast<uint16_t>(utf8.length()));
    res.append(reinterpret_cast<const char *>(&len), 2);
    res.append(utf8);
    return res;
}

QString unpackStringU16BE(const QByteArray &data) {
    if (data.size() < 2)
        return QString();
    uint16_t len = qFromBigEndian<uint16_t>(reinterpret_cast<const uchar *>(data.data()));
    if (data.size() < 2 + len)
        return QString();
    return QString::fromUtf8(data.mid(2, len));
}

uint8_t deviceTypeToByte(DeviceType t) {
    switch (t) {
    case DeviceType::OsmoAction5Pro:
        return 0x2E;
    default:
        return 0x2A;
    }
}

uint8_t deviceTypeToStabilizationByte(DeviceType t) {
    switch (t) {
    case DeviceType::OsmoAction5Pro:
        return 0x1A;
    default:
        return 0x08;
    }
}

DeviceType identifyDeviceType(const QByteArray &manufacturerData) {
    if (manufacturerData.size() < 4)
        return DeviceType::Undefined;
    uint8_t typeByte = static_cast<uint8_t>(manufacturerData[2]);
    switch (typeByte) {
    case 0x15:
        return DeviceType::OsmoAction5Pro;
    case 0x14:
        return DeviceType::OsmoAction4;
    case 0x20:
        return DeviceType::OsmoPocket3;
    default:
        return DeviceType::Unknown;
    }
}

QByteArray Message::serialize() const {
    const uint16_t totalLengthHeadersAndTail = 13;
    if (payload.size() > (1023 - totalLengthHeadersAndTail)) {
        qCritical() << "Payload too long:" << payload.size();
    }

    QByteArray buf;
    buf.append(static_cast<char>(0x55));
    uint16_t length = static_cast<uint16_t>(payload.size() + totalLengthHeadersAndTail);
    buf.append(static_cast<char>(length & 0xFF));
    buf.append(static_cast<char>((0x01 << 2) | ((length >> 8) & 0x03)));

    buf.append(static_cast<char>(crc8(buf)));

    buf.append(static_cast<char>(interface.sender));
    buf.append(static_cast<char>(interface.receiver));

    uint16_t idBE = qToBigEndian(static_cast<uint16_t>(msgId));
    buf.append(reinterpret_cast<const char *>(&idBE), 2);

    buf.append(static_cast<char>(msgType.flags));
    buf.append(static_cast<char>(msgType.cmdSet));
    buf.append(static_cast<char>(msgType.cmdID));

    buf.append(payload);

    uint16_t fullCrc = crc16(buf);

    uint16_t fullCrcLE = qToLittleEndian(fullCrc);
    buf.append(reinterpret_cast<const char *>(&fullCrcLE), 2);

    return buf;
}

Message Message::parse(const QByteArray &data, bool *ok, bool *needMore) {
    if (ok)
        *ok = false;
    if (needMore)
        *needMore = false;

    if (data.size() < 3) {
        if (needMore)
            *needMore = true;
        return {};
    }

    if (static_cast<uint8_t>(data[0]) != 0x55) {
        qWarning() << "Invalid magic:" << static_cast<uint8_t>(data[0]);
        return {};
    }

    uint16_t length =
        (static_cast<uint8_t>(data[1])) | ((static_cast<uint8_t>(data[2]) & 0x03) << 8);

    if (length > data.size()) {
        if (needMore)
            *needMore = true;
        return {};
    }

    if (data.size() < 13) {
        if (needMore)
            *needMore = true;
        return {};
    }

    uint8_t version = static_cast<uint8_t>(data[2]) >> 2;
    if (version != 0x01) {
        qWarning() << "Invalid version:" << version;
        return {};
    }

    uint8_t headerCRC = static_cast<uint8_t>(data[3]);
    QByteArray header = data.left(3);
    if (crc8(header) != headerCRC) {
        qWarning() << "Header CRC mismatch";
        return {};
    }

    QByteArray msgWithoutCRC = data.left(length - 2);
    QByteArray providedCRCBytes = data.mid(length - 2, 2);
    uint16_t providedCRC =
        qFromLittleEndian<uint16_t>(reinterpret_cast<const uchar *>(providedCRCBytes.data()));

    if (crc16(msgWithoutCRC) != providedCRC) {
        qWarning() << "Full CRC mismatch";
        return {};
    }

    Message msg;
    msg.interface.sender = static_cast<uint8_t>(data[4]);
    msg.interface.receiver = static_cast<uint8_t>(data[5]);
    msg.msgId = static_cast<MessageID>(
        qFromBigEndian<uint16_t>(reinterpret_cast<const uchar *>(data.data() + 6)));
    msg.msgType.flags = static_cast<uint8_t>(data[8]);
    msg.msgType.cmdSet = static_cast<CommandSet>(static_cast<uint8_t>(data[9]));
    msg.msgType.cmdID = static_cast<CommandID>(static_cast<uint8_t>(data[10]));
    msg.payload = data.mid(11, length - 13);

    if (ok)
        *ok = true;
    return msg;
}

} // namespace dji
