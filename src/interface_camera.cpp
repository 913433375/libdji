#include "dji/interface_camera.h"
#include "dji/device.h"

namespace dji {

InterfaceCamera::InterfaceCamera(Device *device) : m_device(device) {
}

void InterfaceCamera::setFCCEnable(bool enable) {
    Message msg;
    msg.interface = interfaceID();
    msg.msgType = MessageTypeFCCSupport;
    msg.payload.append(enable ? 0x01 : 0x00);
    m_device->sendMessage(msg);
}

void InterfaceCamera::setImageStabilization(ImageStabilization stabilization) {
    Message msg;
    msg.interface = interfaceID();
    msg.msgType = MessageTypeStartStopStreaming;

    QByteArray payload;
    payload.append(static_cast<char>(0x01));
    payload.append(static_cast<char>(0x01));
    payload.append(static_cast<char>(0x1A));
    payload.append(static_cast<char>(0x00));
    payload.append(static_cast<char>(0x01));
    payload.append(static_cast<char>(static_cast<uint8_t>(stabilization)));
    m_device->sendMessage(msg);
}

void InterfaceCamera::getBatteryInfo() {
    Message msg;
    msg.interface = interfaceID();
    msg.msgType = MessageTypeGetBatteryInfo;
    m_device->sendMessage(msg);
}

void InterfaceCamera::getVersion() {
    Message msg;
    msg.interface = interfaceID();
    msg.msgType = MessageTypeGetVersion;
    m_device->sendMessage(msg);
}

void InterfaceCamera::handleMessage(const Message &msg) {
    if (msg.interface != interfaceID() &&
        msg.interface.receiver != static_cast<uint8_t>(ComponentID::App))
        return;

    if (msg.msgType == MessageTypeGetVersionResponse) {
        emit versionReceived(QString::fromUtf8(msg.payload));
    }
}

} // namespace dji
