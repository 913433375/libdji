#include "dji/interface_battery.h"
#include "dji/message.h"

namespace dji {

InterfaceBattery::InterfaceBattery(Device *device) : m_device(device) {
}

void InterfaceBattery::handleMessage(const Message &msg) {
    if (msg.msgType == MessageTypeBatteryStatus) {
        int percentage = -1;
        if (msg.payload.size() >= 21) {
            percentage = static_cast<uint8_t>(msg.payload[20]);
        } else if (msg.payload.size() >= 13) {
            percentage = static_cast<uint8_t>(msg.payload[12]);
        }

        if (percentage != -1 && percentage != m_percentage) {
            m_percentage = percentage;
            emit batteryPercentageChanged(m_percentage);
        }
    }
}

} // namespace dji
