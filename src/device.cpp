/**
 * @file device.cpp
 * @brief Implementation of the DJI device base class and BLE service discovery.
 */

#include "dji/device.h"
#include "dji/interface_battery.h"
#include "dji/interface_camera.h"
#include "dji/interface_config.h"
#include "dji/interface_flight_controller.h"
#include "dji/interface_goggles.h"
#include "dji/interface_remote_controller.h"
#include "dji/interface_video_transmission.h"
#include "dji/interface_wifi.h"
#include <QDebug>
#include <QTimer>

namespace dji {

static const uint16_t characteristicIDReceiver = 0x002D;
static const uint16_t characteristicIDPairingRequestor = 0x002E;
static const uint16_t characteristicIDSender = 0x0030;

Device::Device(QObject *parent) : QObject(parent) {
    m_wifi = new InterfaceWiFi(this);
    m_camera = new InterfaceCamera(this);
    m_config = new InterfaceConfig(this);
    m_battery = new InterfaceBattery(this);
    m_flightController = new InterfaceFlightController(this);
    m_goggles = new InterfaceGoggles(this);
    m_remoteController = new InterfaceRemoteController(this);
    m_videoTransmission = new InterfaceVideoTransmission(this);

    connect(m_wifi, &InterfaceWiFi::log, this, &Device::log);
    connect(m_wifi, &InterfaceWiFi::error, this, &Device::errorOccurred);
    connect(m_camera, &InterfaceCamera::log, this, &Device::log);
    connect(m_camera, &InterfaceCamera::error, this, &Device::errorOccurred);
    connect(m_config, &InterfaceConfig::log, this, &Device::log);
    connect(m_config, &InterfaceConfig::error, this, &Device::errorOccurred);
    connect(m_battery, &InterfaceBattery::log, this, &Device::log);
    connect(m_battery, &InterfaceBattery::error, this, &Device::errorOccurred);
    connect(m_flightController, &InterfaceFlightController::log, this, &Device::log);
    connect(m_flightController, &InterfaceFlightController::error, this, &Device::errorOccurred);
    connect(m_goggles, &InterfaceGoggles::log, this, &Device::log);
    connect(m_goggles, &InterfaceGoggles::error, this, &Device::errorOccurred);
    connect(m_remoteController, &InterfaceRemoteController::log, this, &Device::log);
    connect(m_videoTransmission, &InterfaceVideoTransmission::log, this, &Device::log);
    connect(m_remoteController, &InterfaceRemoteController::error, this, &Device::errorOccurred);
    connect(m_videoTransmission, &InterfaceVideoTransmission::error, this, &Device::errorOccurred);

    connect(this, &Device::messageReceived, m_wifi, &InterfaceWiFi::handleMessage);
    connect(this, &Device::messageReceived, m_camera, &InterfaceCamera::handleMessage);
    connect(this, &Device::messageReceived, m_config, &InterfaceConfig::handleMessage);
    connect(this, &Device::messageReceived, m_battery, &InterfaceBattery::handleMessage);
    connect(this, &Device::messageReceived, m_flightController,
            &InterfaceFlightController::handleMessage);
    connect(this, &Device::messageReceived, m_goggles, &InterfaceGoggles::handleMessage);
    connect(this, &Device::messageReceived, m_remoteController,
            &InterfaceRemoteController::handleMessage);
    connect(this, &Device::messageReceived, m_videoTransmission,
            &InterfaceVideoTransmission::handleMessage);
}

Device::Device(const QBluetoothDeviceInfo &info, DeviceType type, QObject *parent)
    : QObject(parent), m_deviceInfo(info), m_deviceType(type) {
    m_wifi = new InterfaceWiFi(this);
    m_camera = new InterfaceCamera(this);
    m_config = new InterfaceConfig(this);
    m_battery = new InterfaceBattery(this);
    m_flightController = new InterfaceFlightController(this);
    m_goggles = new InterfaceGoggles(this);
    m_remoteController = new InterfaceRemoteController(this);
    m_videoTransmission = new InterfaceVideoTransmission(this);

    connect(m_wifi, &InterfaceWiFi::log, this, &Device::log);
    connect(m_wifi, &InterfaceWiFi::error, this, &Device::errorOccurred);
    connect(m_camera, &InterfaceCamera::log, this, &Device::log);
    connect(m_camera, &InterfaceCamera::error, this, &Device::errorOccurred);
    connect(m_config, &InterfaceConfig::log, this, &Device::log);
    connect(m_config, &InterfaceConfig::error, this, &Device::errorOccurred);
    connect(m_battery, &InterfaceBattery::log, this, &Device::log);
    connect(m_battery, &InterfaceBattery::error, this, &Device::errorOccurred);
    connect(m_flightController, &InterfaceFlightController::log, this, &Device::log);
    connect(m_flightController, &InterfaceFlightController::error, this, &Device::errorOccurred);
    connect(m_goggles, &InterfaceGoggles::log, this, &Device::log);
    connect(m_goggles, &InterfaceGoggles::error, this, &Device::errorOccurred);
    connect(m_remoteController, &InterfaceRemoteController::log, this, &Device::log);
    connect(m_videoTransmission, &InterfaceVideoTransmission::log, this, &Device::log);
    connect(m_remoteController, &InterfaceRemoteController::error, this, &Device::errorOccurred);
    connect(m_videoTransmission, &InterfaceVideoTransmission::error, this, &Device::errorOccurred);

    connect(this, &Device::messageReceived, m_wifi, &InterfaceWiFi::handleMessage);
    connect(this, &Device::messageReceived, m_camera, &InterfaceCamera::handleMessage);
    connect(this, &Device::messageReceived, m_config, &InterfaceConfig::handleMessage);
    connect(this, &Device::messageReceived, m_battery, &InterfaceBattery::handleMessage);
    connect(this, &Device::messageReceived, m_flightController,
            &InterfaceFlightController::handleMessage);
    connect(this, &Device::messageReceived, m_goggles, &InterfaceGoggles::handleMessage);
    connect(this, &Device::messageReceived, m_remoteController,
            &InterfaceRemoteController::handleMessage);
    connect(this, &Device::messageReceived, m_videoTransmission,
            &InterfaceVideoTransmission::handleMessage);
}

Device::~Device() {
    if (m_controller) {
        m_controller->disconnectFromDevice();
        delete m_controller;
    }
}

void Device::connectToDevice() {
    if (m_controller) {
        m_controller->disconnectFromDevice();
        delete m_controller;
    }

    m_controller = QLowEnergyController::createCentral(m_deviceInfo, this);
    connect(m_controller, &QLowEnergyController::connected, this, &Device::onControllerConnected);
    connect(m_controller, &QLowEnergyController::disconnected, this,
            &Device::onControllerDisconnected);
    connect(m_controller, &QLowEnergyController::errorOccurred, this, &Device::onControllerError);
    connect(m_controller, &QLowEnergyController::serviceDiscovered, this,
            &Device::onServiceDiscovered);
    connect(m_controller, &QLowEnergyController::discoveryFinished, this,
            &Device::onServiceDiscoveryFinished);

    emit log("[DJI-BLE] " + QString("Connecting to %1 (%2)")
                                .arg(m_deviceInfo.name(), m_deviceInfo.address().toString()));
    m_controller->connectToDevice();
}

void Device::disconnectFromDevice() {
    if (m_controller) {
        m_controller->disconnectFromDevice();
    }
}

void Device::onControllerConnected() {
    emit log("[DJI-BLE] "
             "Controller connected. Discovering services...");
    m_controller->discoverServices();
}

void Device::onControllerDisconnected() {
    emit log("[DJI-BLE] "
             "Controller disconnected");
    emit disconnected();
    m_initialized = false;
}

void Device::onControllerError(QLowEnergyController::Error error) {
    emit errorOccurred("[DJI-BLE] " + QString("Controller error: %1").arg(error));
}

void Device::onServiceDiscovered(const QBluetoothUuid &newService) {

    Q_UNUSED(newService);
}

void Device::onServiceDiscoveryFinished() {
    emit log("[DJI-BLE] "
             "Service discovery finished. Searching for DJI characteristics...");
    discoverCharacteristics();
}

void Device::discoverCharacteristics() {
    auto services = m_controller->services();
    for (auto serviceUuid : services) {
        QLowEnergyService *service = m_controller->createServiceObject(serviceUuid, this);
        if (!service)
            continue;

        connect(service, &QLowEnergyService::stateChanged, this, &Device::onServiceStateChanged);
        connect(service, &QLowEnergyService::characteristicChanged, this,
                &Device::onCharacteristicChanged);
        connect(service, &QLowEnergyService::characteristicWritten, this,
                &Device::onCharacteristicWriteFinished);
        connect(service, &QLowEnergyService::characteristicRead, this,
                &Device::onCharacteristicReadFinished);

        service->discoverDetails();
    }
}

void Device::onServiceStateChanged(QLowEnergyService::ServiceState newState) {
    if (newState != QLowEnergyService::RemoteServiceDiscovered)
        return;

    QLowEnergyService *service = qobject_cast<QLowEnergyService *>(sender());
    if (!service)
        return;

    emit log("[DJI-BLE] " + QString("Service %1 discovered with %2 characteristics")
                                .arg(service->serviceUuid().toString())
                                .arg(service->characteristics().size()));

    const QList<QLowEnergyCharacteristic> chars = service->characteristics();
    for (const QLowEnergyCharacteristic &c : chars) {
        QString props;
        if (c.properties() & QLowEnergyCharacteristic::Read)
            props += "Read ";
        if (c.properties() & QLowEnergyCharacteristic::Write)
            props += "Write ";
        if (c.properties() & QLowEnergyCharacteristic::WriteNoResponse)
            props += "WriteNoResp ";
        if (c.properties() & QLowEnergyCharacteristic::Notify)
            props += "Notify ";
        if (c.properties() & QLowEnergyCharacteristic::Indicate)
            props += "Indicate ";
        emit log("[DJI-BLE] " +
                 QString("Characteristic: %1 Properties: %2").arg(c.uuid().toString()).arg(props));

        bool isReceiver = false;
        bool isSender = false;
        bool isWiFi = false;

        if (c.uuid() == QBluetoothUuid(static_cast<uint16_t>(characteristicIDReceiver))) {
            isReceiver = true;
        } else if (c.uuid() == QBluetoothUuid(static_cast<uint16_t>(characteristicIDSender))) {
            isSender = true;
        } else if (c.uuid() ==
                   QBluetoothUuid(static_cast<uint16_t>(characteristicIDPairingRequestor))) {
            isWiFi = true;
        }

        if (isReceiver) {
            m_charReceiver = c;

            QLowEnergyDescriptor desc =
                c.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration);
            if (desc.isValid()) {
                service->writeDescriptor(desc, QByteArray::fromHex("0100"));
            }
            emit log("[DJI-BLE] " +
                     QString("Found Receiver characteristic: %1").arg(c.uuid().toString()));
        } else if (isSender) {
            m_charSender = c;
            emit log("[DJI-BLE] " +
                     QString("Found Sender characteristic: %1").arg(c.uuid().toString()));
        } else if (isWiFi) {
            m_charPairingRequestor = c;
            emit log("[DJI-BLE] " +
                     QString("Found WiFiRequestor characteristic: %1").arg(c.uuid().toString()));
        }
    }

    if (m_charReceiver.isValid() && m_charSender.isValid() && m_charPairingRequestor.isValid()) {
        if (!m_initialized) {
            m_initialized = true;
            m_service = service;

            emit log("[DJI-BLE] "
                     "Device initialized. All characteristics found.");
            emit connected();
            emit initialized();
        }
    }
}

void Device::onCharacteristicChanged(const QLowEnergyCharacteristic &c, const QByteArray &value) {
    if (c.uuid() == m_charReceiver.uuid()) {
        receiveNotification(value);
    }
}

void Device::onCharacteristicWriteFinished(const QLowEnergyCharacteristic &c,
                                           const QByteArray &value) {
    Q_UNUSED(c);
    Q_UNUSED(value);
}

void Device::onCharacteristicReadFinished(const QLowEnergyCharacteristic &c,
                                          const QByteArray &value) {
    Q_UNUSED(c);
    Q_UNUSED(value);
}

void Device::receiveNotification(const QByteArray &data) {
    emit log("[DJI-BLE] " + QString("Received notification: %1").arg(QString(data.toHex())));
    m_receiveBuffer.append(data);

    while (!m_receiveBuffer.isEmpty()) {
        bool ok = false;
        bool needMore = false;
        Message msg = Message::parse(m_receiveBuffer, &ok, &needMore);

        if (needMore) {
            return;
        }

        if (!ok) {
            emit log("[DJI-BLE] " + QString("Failed to parse incoming message, clearing buffer: %1")
                                        .arg(QString(m_receiveBuffer.toHex())));
            m_receiveBuffer.clear();
            return;
        }

        uint16_t length = (static_cast<uint8_t>(m_receiveBuffer[1])) |
                          ((static_cast<uint8_t>(m_receiveBuffer[2]) & 0x03) << 8);
        m_receiveBuffer.remove(0, length);

        if (static_cast<uint8_t>(msg.msgType.flags) &
            static_cast<uint8_t>(MessageTypeFlagAckRequired)) {
            emit log("[DJI-BLE] Sending ACK for message");
            sendACK(msg);
        }

        emit log(
            "[DJI-BLE] " +
            QString("Parsed message: interface=%1->%2 id=0x%3 type=flags:0x%4,set:0x%5,id:0x%6")
                .arg(static_cast<uint8_t>(msg.interface.sender), 0, 16)
                .arg(static_cast<uint8_t>(msg.interface.receiver), 0, 16)
                .arg(static_cast<uint16_t>(msg.msgId), 0, 16)
                .arg(static_cast<uint8_t>(msg.msgType.flags), 0, 16)
                .arg(static_cast<uint8_t>(msg.msgType.cmdSet), 0, 16)
                .arg(static_cast<uint8_t>(msg.msgType.cmdID), 0, 16));
        emit messageReceived(msg);
    }
}

void Device::sendMessage(const Message &msg, bool noResponse) {
    if (!m_initialized || !m_service || !m_charSender.isValid()) {
        emit errorOccurred("Cannot send message: Device not initialized");
        return;
    }

    QByteArray data = msg.serialize();
    QLowEnergyService::WriteMode mode =
        noResponse ? QLowEnergyService::WriteWithoutResponse : QLowEnergyService::WriteWithResponse;
    m_service->writeCharacteristic(m_charSender, data, mode);
}

void Device::sendACK(const Message &msg) {
    Message ack;
    ack.interface = {msg.interface.receiver, msg.interface.sender}; // Swap for ACK
    ack.msgId = msg.msgId;
    ack.msgType = MessageType::response(msg.msgType.cmdSet, msg.msgType.cmdID);
    ack.payload = QByteArray::fromHex("00");
    sendMessage(ack, true);
}

void Device::sendRawPairing(const QByteArray &data) {
    if (!m_initialized || !m_service || !m_charPairingRequestor.isValid()) {
        emit errorOccurred(
            "Cannot send pairing request: Device not initialized or characteristic not found");
        return;
    }

    emit log("[DJI-BLE] " + QString("Sending raw pairing (writing to characteristic 0x2E): %1")
                                .arg(QString(data.toHex())));
    m_service->writeCharacteristic(m_charPairingRequestor, data,
                                   QLowEnergyService::WriteWithoutResponse);
}

bool Device::isConnected() const {
    return m_controller && m_controller->state() == QLowEnergyController::ConnectedState;
}

bool Device::isInitialized() const {
    return m_initialized;
}

} // namespace dji
