#ifndef DJI_MESSAGE_H
#define DJI_MESSAGE_H

#include "dji/constants.h"
#include <QByteArray>
#include <QString>
#include <cstdint>

namespace dji {

struct Message {
    InterfaceID interface = {2, 2}; // Default App to App
    MessageID msgId = static_cast<MessageID>(0);
    MessageType msgType = {0x00, CommandSet::Core, CommandID::GetVersion}; // Default Request
    QByteArray payload;

    QByteArray serialize() const;
    static Message parse(const QByteArray &data, bool *ok = nullptr, bool *needMore = nullptr);
};

QByteArray packString(const QString &s);
QByteArray packURL(const QString &s);
QString unpackStringU16BE(const QByteArray &data);
uint8_t deviceTypeToByte(DeviceType t);
uint8_t deviceTypeToStabilizationByte(DeviceType t);
DeviceType identifyDeviceType(const QByteArray &manufacturerData);

} // namespace dji

#endif // DJI_MESSAGE_H
