#ifndef DJI_INTERFACE_BATTERY_H
#define DJI_INTERFACE_BATTERY_H

#include "dji/message.h"
#include <QObject>

namespace dji {

class Device;

class InterfaceBattery : public QObject {
    Q_OBJECT
    Q_PROPERTY(int percentage READ percentage NOTIFY batteryPercentageChanged)
public:
    explicit InterfaceBattery(Device *device);

    void handleMessage(const Message &msg);

    int percentage() const {
        return m_percentage;
    }

signals:
    void batteryPercentageChanged(int percentage);
    void error(const QString &message);
    void log(const QString &message);

private:
    Device *m_device;
    int m_percentage = 0;
};

} // namespace dji

#endif
