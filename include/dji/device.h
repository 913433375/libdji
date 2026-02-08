/**
 * @file device.h
 * @brief Base class for DJI devices, providing BLE communication and interface access.
 */

#ifndef DJI_DEVICE_H
#define DJI_DEVICE_H

#include "dji/message.h"
#include <QBluetoothDeviceInfo>
#include <QLowEnergyController>
#include <QLowEnergyService>
#include <QObject>

namespace dji {

class InterfaceWiFi;
class InterfaceCamera;
class InterfaceConfig;
class InterfaceBattery;
class InterfaceFlightController;
class InterfaceGoggles;
class InterfaceRemoteController;
class InterfaceVideoTransmission;

class Device : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString name READ name NOTIFY nameChanged)
    Q_PROPERTY(QString address READ address NOTIFY addressChanged)
    Q_PROPERTY(DeviceType deviceType READ deviceType NOTIFY deviceTypeChanged)
    Q_PROPERTY(bool isConnected READ isConnected NOTIFY connected)
    Q_PROPERTY(bool isInitialized READ isInitialized NOTIFY initialized)

public:
    explicit Device(QObject *parent = nullptr);
    explicit Device(const QBluetoothDeviceInfo &info, DeviceType type, QObject *parent = nullptr);
    ~Device() override;

    virtual void connectToDevice();
    virtual void disconnectFromDevice();

    virtual void sendMessage(const Message &msg, bool noResponse = true);
    virtual void sendACK(const Message &msg);
    virtual void sendRawPairing(const QByteArray &data);

    QString name() const {
        return m_deviceInfo.name();
    }
    QString address() const {
        return m_deviceInfo.address().toString();
    }

    DeviceType deviceType() const {
        return m_deviceType;
    }
    void setDeviceType(DeviceType t) {
        if (m_deviceType != t) {
            m_deviceType = t;
            emit deviceTypeChanged();
        }
    }

    QBluetoothDeviceInfo deviceInfo() const {
        return m_deviceInfo;
    }

    virtual bool isConnected() const;
    virtual bool isInitialized() const;

    InterfaceWiFi *wifi() {
        return m_wifi;
    }
    InterfaceCamera *camera() {
        return m_camera;
    }
    InterfaceConfig *config() {
        return m_config;
    }
    InterfaceBattery *battery() {
        return m_battery;
    }
    InterfaceFlightController *flightController() {
        return m_flightController;
    }
    InterfaceGoggles *goggles() {
        return m_goggles;
    }
    InterfaceRemoteController *remoteController() {
        return m_remoteController;
    }
    InterfaceVideoTransmission *videoTransmission() {
        return m_videoTransmission;
    }

signals:
    void connected();
    void disconnected();
    void initialized();
    void errorOccurred(const QString &message);
    void log(const QString &message);

    void messageReceived(const Message &msg);
    void nameChanged();
    void addressChanged();
    void deviceTypeChanged();

private slots:
    void onControllerConnected();
    void onControllerDisconnected();
    void onControllerError(QLowEnergyController::Error error);
    void onServiceDiscovered(const QBluetoothUuid &newService);
    void onServiceDiscoveryFinished();
    void onServiceStateChanged(QLowEnergyService::ServiceState newState);
    void onCharacteristicChanged(const QLowEnergyCharacteristic &c, const QByteArray &value);
    void onCharacteristicWriteFinished(const QLowEnergyCharacteristic &c, const QByteArray &value);
    void onCharacteristicReadFinished(const QLowEnergyCharacteristic &c, const QByteArray &value);

protected:
    void discoverCharacteristics();
    void receiveNotification(const QByteArray &data);
    void simulateIncomingData(const QByteArray &data) {
        receiveNotification(data);
    }

    InterfaceWiFi *m_wifi;
    InterfaceCamera *m_camera;
    InterfaceConfig *m_config;
    InterfaceBattery *m_battery;
    InterfaceFlightController *m_flightController;
    InterfaceGoggles *m_goggles;
    InterfaceRemoteController *m_remoteController;
    InterfaceVideoTransmission *m_videoTransmission;
    DeviceType m_deviceType = DeviceType::Unknown;

    QBluetoothDeviceInfo m_deviceInfo;
    QLowEnergyController *m_controller = nullptr;
    QLowEnergyService *m_service = nullptr;

    QLowEnergyCharacteristic m_charReceiver;
    QLowEnergyCharacteristic m_charSender;
    QLowEnergyCharacteristic m_charPairingRequestor;

    bool m_initialized = false;
    QByteArray m_receiveBuffer;
};

} // namespace dji

#endif
