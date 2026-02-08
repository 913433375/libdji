#ifndef TST_CONNECT_FLOW_H
#define TST_CONNECT_FLOW_H

#include "dji/device.h"
#include "dji/message.h"
#include <QObject>

class MockDevice : public dji::Device {
    Q_OBJECT
public:
    using dji::Device::Device;
    void simulateIncomingData(const QByteArray &data);

    void connectToDevice() override {
        m_initialized = true;
        emit initialized();
        emit connected();
    }

    void sendMessage(const dji::Message &msg, bool noResponse = true) override;
    void sendACK(const dji::Message &msg) override;
    void sendRawPairing(const QByteArray &data) override;
    void handleSentMessage(const dji::Message &msg);
};

class TestConnectWifiAndStreaming : public QObject {
    Q_OBJECT
private slots:
    void testFullFlow();
};

#endif
