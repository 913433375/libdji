#ifndef DJI_INTERFACE_CAMERA_H
#define DJI_INTERFACE_CAMERA_H

#include "dji/message.h"
#include <QObject>

namespace dji {

class Device;

class InterfaceCamera : public QObject {
    Q_OBJECT
public:
    explicit InterfaceCamera(Device *device);

    InterfaceID interfaceID() const {
        return InterfaceIDAppToCamera;
    }

    void setFCCEnable(bool enable);
    void setImageStabilization(ImageStabilization stabilization);
    void getBatteryInfo();
    void getVersion();

    void handleMessage(const Message &msg);

signals:
    void error(const QString &message);
    void log(const QString &message);
    void imageStabilizationChanged(ImageStabilization stabilization);
    void batteryInfoReceived(const QByteArray &data);
    void versionReceived(const QString &version);

private:
    Device *m_device;
};

} // namespace dji

#endif
