#include "dji/interface_flight_controller.h"
#include "dji/device.h"

namespace dji {

InterfaceFlightController::InterfaceFlightController(Device *device) : m_device(device) {
}

void InterfaceFlightController::handleMessage(const Message &msg) {
    // Handle responses or status updates if any
    Q_UNUSED(msg);
}

} // namespace dji
