#include "tst_connect_flow.h"
#include "dji/device_manager.h"
#include "dji/interface_camera.h"
#include "dji/interface_video_transmission.h"
#include "dji/interface_wifi.h"
#include <QDebug>
#include <QSignalSpy>
#include <QTimer>
#include <QtTest>

void MockDevice::simulateIncomingData(const QByteArray &data) {
    qDebug().noquote() << "RECV_HEX:" << data.toHex().toUpper();
    receiveNotification(data);
}

void MockDevice::sendMessage(const dji::Message &msg, bool noResponse) {
    Q_UNUSED(noResponse);
    qDebug().noquote() << "SENT_HEX:" << msg.serialize().toHex().toUpper();
    handleSentMessage(msg);
}

void MockDevice::sendACK(const dji::Message &msg) {
    dji::Message ack;
    ack.interface = dji::InterfaceID(msg.interface.receiver, msg.interface.sender);
    ack.msgId = msg.msgId;
    ack.msgType = dji::MessageType::response(msg.msgType.cmdSet, msg.msgType.cmdID);
    ack.payload = QByteArray::fromHex("00");
    qDebug().noquote() << "SENT_HEX:" << ack.serialize().toHex().toUpper();
}

void MockDevice::sendRawPairing(const QByteArray &data) {
    qDebug().noquote() << "SENT_HEX:" << data.toHex().toUpper();
}

void MockDevice::handleSentMessage(const dji::Message &msg) {
    dji::Message resp;
    resp.interface = msg.interface;
    resp.msgId = msg.msgId;
    bool shouldRespond = false;

    if (msg.msgType == dji::MessageTypeSetPairingPIN) {
        resp.payload = QByteArray::fromHex("0001");
        resp.msgType = dji::MessageTypePairingStatus;
        shouldRespond = true;
    } else if (msg.msgType == dji::MessageTypePairingPINApproved) {
        resp.msgType = dji::MessageTypePairingStatus;
        resp.payload = QByteArray::fromHex("0001");
        shouldRespond = true;
    } else if (msg.msgType == dji::MessageTypePrepareToLiveStream) {
        resp.msgType = dji::MessageTypePrepareToLiveStreamResult;
        resp.payload = QByteArray::fromHex("00");
        shouldRespond = true;
    } else if (msg.msgType == dji::MessageTypeConfigureStreaming) {
        resp.msgType = dji::MessageTypeConfigureStreamingResult;
        resp.payload = QByteArray::fromHex("00");
        shouldRespond = true;
    } else if (msg.msgType == dji::MessageTypeStartStopStreaming) {
        resp.msgType = dji::MessageTypeStartStopStreamingResult;
        resp.payload = QByteArray::fromHex("00");
        shouldRespond = true;
        if (static_cast<uint16_t>(msg.msgId) ==
                static_cast<uint16_t>(dji::MessageID::StartStreaming) &&
            msg.payload.size() >= 1 && static_cast<uint8_t>(msg.payload[0]) == 0x01) {
            QTimer::singleShot(100, [this, msg]() {
                dji::Message status;
                status.interface = dji::InterfaceID(0x00, 0x00);
                status.msgId = static_cast<dji::MessageID>(0x0000);
                status.msgType = dji::MessageTypeBatteryStatus;
                status.payload = QByteArray(21, 0);
                status.payload[20] = 100;
                simulateIncomingData(status.serialize());
            });
        }
    } else if (msg.msgType == dji::MessageTypeConnectToWiFi) {
        resp.msgType = dji::MessageTypeConnectToWiFiResult;
        resp.payload = QByteArray::fromHex("0000");
        shouldRespond = true;
    }

    if (shouldRespond) {
        QTimer::singleShot(10, [this, resp]() { simulateIncomingData(resp.serialize()); });
    }
}

void TestConnectWifiAndStreaming::testFullFlow() {
    MockDevice device;
    struct HackDevice : public dji::Device {
        using dji::Device::m_initialized;
    };
    ((HackDevice *)&device)->m_initialized = true;

    QSignalSpy spyReceiver(&device, &dji::Device::messageReceived);

    // Initial status
    dji::Message initStatus;
    initStatus.interface = dji::InterfaceID(0x00, 0x00);
    initStatus.msgId = static_cast<dji::MessageID>(0x0000);
    initStatus.msgType = dji::MessageTypeBatteryStatus;
    initStatus.payload = QByteArray(21, 0);
    initStatus.payload[20] = 100;
    device.simulateIncomingData(initStatus.serialize());

    dji::DeviceManager manager(&device);
    QSignalSpy spyFinished(&manager, &dji::DeviceManager::finished);

    dji::StreamingOptions opts;
    opts.ssid = "test-ssid";
    opts.psk = "test-psk";
    opts.rtmpUrl = "rtmp://test/live";
    opts.bitrateKbps = 6000;
    opts.fps = dji::FPS::FPS30;
    opts.resolution = dji::Resolution::Res1080p;

    manager.connectToWiFiAndStartStreaming(&device, opts);

    QVERIFY(spyFinished.wait(5000));
    QEventLoop waitLoop;
    QTimer::singleShot(200, &waitLoop, &QEventLoop::quit);
    waitLoop.exec();
}
