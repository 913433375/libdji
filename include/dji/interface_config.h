#ifndef DJI_INTERFACE_CONFIG_H
#define DJI_INTERFACE_CONFIG_H

#include "dji/message.h"
#include <QByteArray>
#include <QObject>

namespace dji {

class Device;

class InterfaceConfig : public QObject {
    Q_OBJECT
public:
    explicit InterfaceConfig(Device *device);

    InterfaceID interfaceID() const {
        return InterfaceIDAppToCamera;
    }

    void setImageStabilization(ImageStabilization v);
    void handleMessage(const Message &msg);

signals:
    void log(const QString &message);
    void error(const QString &message);
    void imageStabilizationSet();

private:
    Device *m_device;

    void sendMessageSetImageStabilization(ImageStabilization v);
};

} // namespace dji

#endif
