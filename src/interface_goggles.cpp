#include "dji/interface_goggles.h"
#include "dji/device.h"

namespace dji {

InterfaceGoggles::InterfaceGoggles(Device *device) : m_device(device) {
}

void InterfaceGoggles::setGogglesMode(GogglesMode mode) {
    Message msg;
    msg.interface = InterfaceIDAppToGoggles;
    msg.msgType = MessageTypeGogglesMode;
    msg.payload.append(static_cast<char>(mode));
    m_device->sendMessage(msg);
}

void InterfaceGoggles::toggleGogglesMode() {
    Message msg;
    msg.interface = InterfaceIDAppToCamera;
    msg.msgType = MessageTypeGogglesModeToggle;
    m_device->sendMessage(msg);
}

void InterfaceGoggles::handleMessage(const Message &msg) {
    Q_UNUSED(msg);
}

} // namespace dji
