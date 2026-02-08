#ifndef DJI_INTERFACE_WIFI_H
#define DJI_INTERFACE_WIFI_H

#include "dji/message.h"
#include <QByteArray>
#include <QObject>

namespace dji {

class Device;

class InterfaceWiFi : public QObject {
    Q_OBJECT
public:
    enum class State { Idle, WaitingForStatus, WaitingForApproval, Finalizing };

    explicit InterfaceWiFi(Device *device);

    InterfaceID interfaceID() const {
        return InterfaceIDAppToWiFiGroundStation;
    }

    void pair();
    void connectToWiFi(const QString &ssid, const QString &psk);
    void startScanningWiFi();
    void getCameraAPInfo();
    void handleMessage(const Message &msg);

    State state() const {
        return m_state;
    }

signals:
    void pairingComplete();
    void wifiConnected();
    void wifiScanReport(const QByteArray &report);
    void cameraAPInfoReceived(const QString &ssid, const QString &psk);
    void error(const QString &message);
    void log(const QString &message);

private:
    Device *m_device;
    State m_state = State::Idle;
    QString m_cameraSSID;
    QString m_cameraPSK;

    void sendRequestStartPairing();
    void sendMessageSetPairingPIN(const QString &pinCode);
    void sendMessagePairingStage1();
    void sendMessagePairingStage2();

    void sendMessageStartScanningWiFi();
    void sendMessageConnectToWiFi(const QString &ssid, const QString &psk);
};

} // namespace dji

#endif
