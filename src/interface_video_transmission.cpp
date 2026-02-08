#include "dji/interface_video_transmission.h"
#include "dji/device.h"
#include <QDebug>
#include <QtEndian>

namespace dji {

InterfaceVideoTransmission::InterfaceVideoTransmission(Device *device) : m_device(device) {
}

void InterfaceVideoTransmission::prepareToLiveStream() {
    emit log("[DJI-BLE] Preparing to live stream...");
    m_prepareStage = 1;
    Message msg;
    msg.interface = interfaceID();
    msg.msgId = MessageID::PrepareToLiveStreamStage1;
    msg.msgType = MessageTypePrepareToLiveStream;
    msg.payload.append(static_cast<char>(0x1A));
    m_device->sendMessage(msg, true);
}

void InterfaceVideoTransmission::startLiveStream(Resolution resolution, uint16_t bitrateKbps,
                                                 FPS fps, const QString &rtmpURL) {
    emit log("[DJI-BLE] Starting live stream...");
    Message msg;
    msg.interface = interfaceID();
    msg.msgId = MessageID::StartStreaming;
    msg.msgType = MessageTypeStartStopStreaming;
    msg.payload = QByteArray::fromHex("01011A000101");
    m_device->sendMessage(msg, true);
}

void InterfaceVideoTransmission::stopLiveStream() {
    emit log("[DJI-BLE] Stopping live stream...");
    Message msg;
    msg.interface = interfaceID();
    msg.msgId = MessageID::StopStreaming;
    msg.msgType = MessageTypeStartStopStreaming;
    msg.payload = QByteArray::fromHex("01011A000102");
    m_device->sendMessage(msg, true);
}

void InterfaceVideoTransmission::configureLiveStream(Resolution resolution, uint16_t bitrateKbps,
                                                     FPS fps, const QString &rtmpURL) {
    emit log("[DJI-BLE] Configuring live stream...");
    Message msg;
    msg.interface = interfaceID();
    msg.msgId = MessageID::ConfigureStreaming;
    msg.msgType = MessageTypeConfigureStreaming;

    QByteArray payload;
    payload.append(static_cast<char>(0x00));

    // Device Type fixed bytes
    switch (m_device->deviceType()) {
    case DeviceType::OsmoAction5Pro:
        payload.append(static_cast<char>(0x2E));
        break;
    default:
        payload.append(static_cast<char>(0x2A));
        break;
    }

    payload.append(static_cast<char>(0x00));
    payload.append(static_cast<char>(static_cast<uint8_t>(resolution)));

    uint16_t brLE = qToLittleEndian<uint16_t>(bitrateKbps);
    payload.append(reinterpret_cast<const char *>(&brLE), 2);

    payload.append(static_cast<char>(0x02));
    payload.append(static_cast<char>(0x00));
    payload.append(static_cast<char>(static_cast<uint8_t>(fps)));
    payload.append(static_cast<char>(0x00));
    payload.append(static_cast<char>(0x00));
    payload.append(static_cast<char>(0x00));
    payload.append(packURL(rtmpURL));

    msg.payload = payload;
    m_device->sendMessage(msg, true);
}

void InterfaceVideoTransmission::configureRTMP(bool enabled, const QString &url) {
    emit log("[DJI-BLE] Configuring RTMP broadcast...");
    Message msg;
    msg.interface = InterfaceIDAppToCamera;
    msg.msgType = MessageTypeOsmoBroadcastConfig;

    QByteArray payload;
    payload.append(enabled ? 1 : 0);
    payload.append(2); // BroadcastPlatformRTMP
    payload.append(packURL(url));
    msg.payload = payload;

    m_device->sendMessage(msg, true);
}

void InterfaceVideoTransmission::handleMessage(const Message &msg) {
    if (!interfaceID().matches(msg.interface) && !InterfaceIDAppToCamera.matches(msg.interface))
        return;

    if (msg.msgType == MessageTypePrepareToLiveStreamResult) {
        if (m_prepareStage == 1) {
            m_prepareStage = 2;
            Message stage2;
            stage2.interface = interfaceID();
            stage2.msgType = MessageTypeStartStopStreaming;
            stage2.msgId = MessageID::StartStreaming;
            stage2.payload = QByteArray::fromHex("00011C00");
            m_device->sendMessage(stage2, false);
        }
    } else if (msg.msgType == MessageTypeStartStopStreamingResult) {
        if (m_prepareStage == 2) {
            m_prepareStage = 0;
            emit prepareToLiveStreamComplete();
        } else {
            emit startLiveStreamComplete();
        }
    } else if (msg.msgType == MessageTypeConfigureStreamingResult) {
        emit configureLiveStreamComplete();
    } else if (msg.msgType == MessageTypeOsmoBroadcastConfigResult) {
        emit configureRTMPComplete();
    }
}

} // namespace dji
