#ifndef DJI_INTERFACE_REMOTE_CONTROLLER_H
#define DJI_INTERFACE_REMOTE_CONTROLLER_H

#include "dji/message.h"
#include <QObject>

namespace dji {

class Device;

class InterfaceRemoteController : public QObject {
    Q_OBJECT
public:
    explicit InterfaceRemoteController(Device *device);

    void sendSimulatorData(const RemoteControllerSimulatorData &data);

    void handleMessage(const Message &msg);

signals:
    void error(const QString &message);
    void log(const QString &message);

private:
    Device *m_device;
};

} // namespace dji

#endif
