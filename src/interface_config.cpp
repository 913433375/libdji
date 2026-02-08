#include "dji/interface_config.h"
#include "dji/device.h"
#include <QDebug>

namespace dji {

InterfaceConfig::InterfaceConfig(Device *device) : m_device(device) {
}

void InterfaceConfig::setImageStabilization(ImageStabilization v) {
    emit log("[DJI-BLE] " + QString("Setting image stabilization to %1").arg(static_cast<int>(v)));
    sendMessageSetImageStabilization(v);
}

void InterfaceConfig::handleMessage(const Message &msg) {
    if (msg.interface == InterfaceIDAppToCamera && msg.msgType == MessageTypeStartStopStreaming) {
        emit log("[DJI-BLE] " +
                 QString("Received configurer result: %1").arg(QString(msg.payload.toHex())));
    }
}

void InterfaceConfig::sendMessageSetImageStabilization(ImageStabilization v) {

    QByteArray payload;
    payload.append(QByteArray::fromHex("0101"));
    payload.append(static_cast<char>(deviceTypeToStabilizationByte(m_device->deviceType())));
    payload.append(QByteArray::fromHex("0001"));
    payload.append(static_cast<char>(v));

    Message msg;
    msg.interface = interfaceID();
    msg.msgId = static_cast<MessageID>(0);
    msg.msgType = MessageTypeStartStopStreaming;
    msg.payload = payload;

    m_device->sendMessage(msg, true);
}

} // namespace dji
