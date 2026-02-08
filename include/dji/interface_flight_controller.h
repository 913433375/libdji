#ifndef DJI_INTERFACE_FLIGHT_CONTROLLER_H
#define DJI_INTERFACE_FLIGHT_CONTROLLER_H

#include "dji/message.h"
#include <QObject>

namespace dji {

class Device;

class InterfaceFlightController : public QObject {
    Q_OBJECT
public:
    explicit InterfaceFlightController(Device *device);

    void handleMessage(const Message &msg);

signals:
    void error(const QString &message);
    void log(const QString &message);

private:
    Device *m_device;
};

} // namespace dji

#endif
