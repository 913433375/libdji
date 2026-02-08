#ifndef DJI_DEVICE_FLOW_H
#define DJI_DEVICE_FLOW_H

#include "dji/device_manager.h"
#include <QObject>
#include <functional>

namespace dji {

class Device;

/**
 * @brief Abstract base class for device operation flows.
 */
class DeviceFlow : public QObject {
    Q_OBJECT
public:
    explicit DeviceFlow(QObject *parent = nullptr) : QObject(parent) {
    }
    virtual ~DeviceFlow() = default;

    virtual void start(Device *dev) = 0;
    virtual void stop() {
    }

signals:
    void finished(Device *dev, bool success);
    void log(const QString &msg);
};

/**
 * @brief Concrete flow for pairing, connecting to WiFi, and starting a live stream.
 */
class StreamingStarter : public DeviceFlow {
    Q_OBJECT
public:
    explicit StreamingStarter(const StreamingOptions &options, QObject *parent = nullptr);

    void start(Device *dev) override;
    void stop() override;

private slots:
    void onInitialized();
    void onPairingComplete();
    void onWifiConnected();
    void onPrepareComplete();
    void onConfigureComplete();
    void onStartComplete();
    void onError(const QString &msg);

private:
    Device *m_device = nullptr;
    StreamingOptions m_options;
};

/**
 * @brief Generic flow that pairs with the device and then executes a callback.
 */
class GenericActionFlow : public DeviceFlow {
    Q_OBJECT
public:
    using Action = std::function<void(Device *)>;
    explicit GenericActionFlow(Action action, QObject *parent = nullptr);

    void start(Device *dev) override;

private slots:
    void onInitialized();
    void onPairingComplete();
    void onError(const QString &msg);

private:
    Device *m_device = nullptr;
    Action m_action;
};

/**
 * @brief Concrete flow for stopping a live stream.
 */
class StreamingStopper : public DeviceFlow {
    Q_OBJECT
public:
    explicit StreamingStopper(QObject *parent = nullptr);

    void start(Device *dev) override;

private slots:
    void onInitialized();
    void onPairingComplete();
    void onFinishedAction();
    void onError(const QString &msg);

private:
    Device *m_device = nullptr;
};

class CameraAPInfoFlow : public DeviceFlow {
    Q_OBJECT
public:
    explicit CameraAPInfoFlow(QObject *parent = nullptr);

    void start(Device *dev) override;

private slots:
    void onInitialized();
    void onPairingComplete();
    void onInfoReceived(const QString &ssid, const QString &psk);
    void onError(const QString &msg);

private:
    Device *m_device = nullptr;
};

} // namespace dji

#endif // DJI_DEVICE_FLOW_H
