#include "dji/interface_remote_controller.h"
#include "dji/device.h"
#include <QtEndian>

namespace dji {

InterfaceRemoteController::InterfaceRemoteController(Device *device) : m_device(device) {
}

void InterfaceRemoteController::sendSimulatorData(const RemoteControllerSimulatorData &data) {
    Message msg;
    msg.interface = InterfaceIDAppToRemoteController;
    msg.msgType = MessageTypeRemoteControllerSimulatorData;

    QByteArray payload(38, 0);
    qToLittleEndian<uint16_t>(data.rightStickHorizontal,
                              reinterpret_cast<uchar *>(payload.data() + 0));
    qToLittleEndian<uint16_t>(data.rightStickVertical,
                              reinterpret_cast<uchar *>(payload.data() + 2));
    qToLittleEndian<uint16_t>(data.leftStickVertical,
                              reinterpret_cast<uchar *>(payload.data() + 4));
    qToLittleEndian<uint16_t>(data.leftStickHorizontal,
                              reinterpret_cast<uchar *>(payload.data() + 6));
    qToLittleEndian<uint32_t>(data.buttons, reinterpret_cast<uchar *>(payload.data() + 8));
    // Bytes 12-37 are reserved (zero-filled)

    msg.payload = payload;
    m_device->sendMessage(msg);
}

void InterfaceRemoteController::handleMessage(const Message &msg) {
    Q_UNUSED(msg);
}

} // namespace dji
