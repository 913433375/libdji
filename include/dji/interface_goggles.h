#ifndef DJI_INTERFACE_GOGGLES_H
#define DJI_INTERFACE_GOGGLES_H

#include "dji/message.h"
#include <QObject>

namespace dji {

class Device;

class InterfaceGoggles : public QObject {
    Q_OBJECT
public:
    explicit InterfaceGoggles(Device *device);

    void setGogglesMode(GogglesMode mode);
    void toggleGogglesMode();

    void handleMessage(const Message &msg);

signals:
    void error(const QString &message);
    void log(const QString &message);

private:
    Device *m_device;
};

} // namespace dji

#endif
