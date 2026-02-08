#include "dji/device_flow.h"
#include "dji/device.h"
#include "dji/interface_camera.h"
#include "dji/interface_video_transmission.h"
#include "dji/interface_wifi.h"

namespace dji {

StreamingStarter::StreamingStarter(const StreamingOptions &options, QObject *parent)
    : DeviceFlow(parent), m_options(options) {
}

void StreamingStarter::start(Device *dev) {
    m_device = dev;

    connect(dev, &Device::initialized, this, &StreamingStarter::onInitialized);
    connect(dev->wifi(), &InterfaceWiFi::pairingComplete, this,
            &StreamingStarter::onPairingComplete);
    connect(dev->wifi(), &InterfaceWiFi::wifiConnected, this, &StreamingStarter::onWifiConnected);
    connect(dev->wifi(), &InterfaceWiFi::error, this, &StreamingStarter::onError);

    connect(dev->videoTransmission(), &InterfaceVideoTransmission::prepareToLiveStreamComplete,
            this, &StreamingStarter::onPrepareComplete);
    connect(dev->videoTransmission(), &InterfaceVideoTransmission::startLiveStreamComplete, this,
            &StreamingStarter::onStartComplete);
    connect(dev->videoTransmission(), &InterfaceVideoTransmission::configureLiveStreamComplete,
            this, &StreamingStarter::onConfigureComplete);
    connect(dev->videoTransmission(), &InterfaceVideoTransmission::error, this,
            &StreamingStarter::onError);

    if (dev->isInitialized()) {
        onInitialized();
    } else if (!dev->isConnected()) {
        emit log(QString("[DJI-BLE] Flow: Connecting to device %1...")
                     .arg(dev->deviceInfo().address().toString()));
        dev->connectToDevice();
    }
}

void StreamingStarter::stop() {
    if (m_device) {
        m_device->videoTransmission()->stopLiveStream();
    }
}

void StreamingStarter::onInitialized() {
    emit log(QString("[DJI-BLE] Flow: Device %1 initialized. Starting pairing...")
                 .arg(m_device->deviceInfo().address().toString()));
    m_device->wifi()->pair();
}

void StreamingStarter::onPairingComplete() {
    emit log(QString("[DJI-BLE] Flow: Pairing complete for %1. Preparing to live stream...")
                 .arg(m_device->deviceInfo().address().toString()));
    m_device->videoTransmission()->prepareToLiveStream();
}

void StreamingStarter::onPrepareComplete() {
    emit log(QString("[DJI-BLE] Flow: Prepare complete for %1. Connecting to WiFi %2...")
                 .arg(m_device->deviceInfo().address().toString(), m_options.ssid));
    m_device->wifi()->connectToWiFi(m_options.ssid, m_options.psk);
}

void StreamingStarter::onWifiConnected() {
    emit log(QString("[DJI-BLE] Flow: WiFi connected for %1. Configuring live stream...")
                 .arg(m_device->deviceInfo().address().toString()));
    m_device->videoTransmission()->configureLiveStream(m_options.resolution, m_options.bitrateKbps,
                                                       m_options.fps, m_options.rtmpUrl);
}

void StreamingStarter::onConfigureComplete() {
    emit log(QString("[DJI-BLE] Flow: Configuration complete for %1. Starting live stream...")
                 .arg(m_device->deviceInfo().address().toString()));
    m_device->videoTransmission()->startLiveStream(m_options.resolution, m_options.bitrateKbps,
                                                   m_options.fps, m_options.rtmpUrl);
}

void StreamingStarter::onStartComplete() {
    emit log(QString("[DJI-BLE] Flow: Live stream started for %1.")
                 .arg(m_device->deviceInfo().address().toString()));
    emit finished(m_device, true);
}

void StreamingStarter::onError(const QString &msg) {
    emit log(QString("[DJI-BLE] Flow error: %1").arg(msg));
    emit finished(m_device, false);
}

GenericActionFlow::GenericActionFlow(Action action, QObject *parent)
    : DeviceFlow(parent), m_action(std::move(action)) {
}

void GenericActionFlow::start(Device *dev) {
    m_device = dev;
    connect(dev, &Device::initialized, this, &GenericActionFlow::onInitialized);
    connect(dev->wifi(), &InterfaceWiFi::pairingComplete, this,
            &GenericActionFlow::onPairingComplete);
    connect(dev->wifi(), &InterfaceWiFi::error, this, &GenericActionFlow::onError);

    if (dev->isInitialized()) {
        onInitialized();
    } else if (!dev->isConnected()) {
        emit log(QString("[DJI-BLE] Flow: Connecting to device %1...")
                     .arg(dev->deviceInfo().address().toString()));
        dev->connectToDevice();
    }
}

void GenericActionFlow::onInitialized() {
    emit log(QString("[DJI-BLE] Flow: Device %1 initialized. Starting pairing...")
                 .arg(m_device->deviceInfo().address().toString()));
    m_device->wifi()->pair();
}

void GenericActionFlow::onPairingComplete() {
    emit log(QString("[DJI-BLE] Flow: Pairing complete for %1. Executing action...")
                 .arg(m_device->deviceInfo().address().toString()));
    m_action(m_device);
    emit finished(m_device, true);
}

void GenericActionFlow::onError(const QString &msg) {
    emit log(QString("[DJI-BLE] Flow error: %1").arg(msg));
    emit finished(m_device, false);
}

StreamingStopper::StreamingStopper(QObject *parent) : DeviceFlow(parent) {
}

void StreamingStopper::start(Device *dev) {
    m_device = dev;
    connect(dev, &Device::initialized, this, &StreamingStopper::onInitialized);
    connect(dev->wifi(), &InterfaceWiFi::pairingComplete, this,
            &StreamingStopper::onPairingComplete);
    connect(dev->videoTransmission(), &InterfaceVideoTransmission::startLiveStreamComplete, this,
            &StreamingStopper::onFinishedAction);
    connect(dev->wifi(), &InterfaceWiFi::error, this, &StreamingStopper::onError);
    connect(dev->videoTransmission(), &InterfaceVideoTransmission::error, this,
            &StreamingStopper::onError);

    if (dev->isInitialized()) {
        onInitialized();
    } else if (!dev->isConnected()) {
        dev->connectToDevice();
    }
}

void StreamingStopper::onInitialized() {
    m_device->wifi()->pair();
}

void StreamingStopper::onPairingComplete() {
    m_device->videoTransmission()->stopLiveStream();
}

void StreamingStopper::onFinishedAction() {
    emit finished(m_device, true);
}

void StreamingStopper::onError(const QString &msg) {
    emit log(QString("[DJI-BLE] Flow error: %1").arg(msg));
    emit finished(m_device, false);
}

CameraAPInfoFlow::CameraAPInfoFlow(QObject *parent) : DeviceFlow(parent) {
}

void CameraAPInfoFlow::start(Device *dev) {
    m_device = dev;
    connect(dev, &Device::initialized, this, &CameraAPInfoFlow::onInitialized);
    connect(dev->wifi(), &InterfaceWiFi::pairingComplete, this,
            &CameraAPInfoFlow::onPairingComplete);
    connect(dev->wifi(), &InterfaceWiFi::cameraAPInfoReceived, this,
            &CameraAPInfoFlow::onInfoReceived);
    connect(dev->wifi(), &InterfaceWiFi::error, this, &CameraAPInfoFlow::onError);

    if (dev->isInitialized()) {
        onInitialized();
    } else if (!dev->isConnected()) {
        dev->connectToDevice();
    }
}

void CameraAPInfoFlow::onInitialized() {
    m_device->wifi()->pair();
}

void CameraAPInfoFlow::onPairingComplete() {
    m_device->wifi()->getCameraAPInfo();
}

void CameraAPInfoFlow::onInfoReceived(const QString &ssid, const QString &psk) {
    emit log(QString("Camera AP Info: SSID='%1', PSK='%2'").arg(ssid, psk));
    emit finished(m_device, true);
}

void CameraAPInfoFlow::onError(const QString &msg) {
    emit log(QString("[DJI-BLE] Flow error: %1").arg(msg));
    emit finished(m_device, false);
}

} // namespace dji
