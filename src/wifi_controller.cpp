#include "dji/wifi_controller.h"
#include <QDebug>
#include <QtEndian>

namespace dji {

WiFiController::WiFiController(const QHostAddress &addr, QObject *parent)
    : QObject(parent), m_addr(addr) {
    m_udp = new QUdpSocket(this);
}

void WiFiController::sendHandshake() {
    const QByteArray MetadataInitial = QByteArray::fromHex("00000000");
    const QByteArray PayloadAppIdentifier = QByteArray::fromHex("94c9");

    Message msg;
    msg.interface = InterfaceIDAppToApp;
    msg.msgType =
        MessageType::request(CommandSet::Core, static_cast<CommandID>(0x01)); // GetVersion/AppID?
    msg.msgId = MessageID::AppIdentifier;
    msg.payload = PayloadAppIdentifier;

    QByteArray duml = msg.serialize();
    QByteArray pkt = wrapMessage(0, MetadataInitial, duml);
    m_udp->writeDatagram(pkt, m_addr, 9004);
}

void WiFiController::sendVideoHandshake() {
    const QByteArray MetadataVideo = QByteArray::fromHex("00000000");
    const QByteArray PayloadHandshakeRMVT = QByteArray::fromHex("524d5674000100010000000000000000");

    QByteArray pkt = wrapMessage(1, MetadataVideo, PayloadHandshakeRMVT);
    m_udp->writeDatagram(pkt, m_addr, 9004);
}

void WiFiController::sendFCCEnable(bool enable) {
    Message msg;
    msg.interface = InterfaceIDAppToApp;
    msg.msgType = MessageTypeFCCSupport;
    msg.msgId = MessageID::FCCEnable;
    msg.payload.append(static_cast<char>(enable ? 0x01 : 0x00));

    QByteArray duml = msg.serialize();
    QByteArray pkt = wrapMessage(0, QByteArray::fromHex("04100202"), duml);
    m_udp->writeDatagram(pkt, m_addr, 9004);
}

void WiFiController::sendRTMPBroadcast(const QString &url) {
    Message msg;
    msg.interface = InterfaceIDAppToCamera;
    msg.msgType = MessageTypeOsmoBroadcastConfig;
    msg.msgId = MessageID::StartStreaming;

    QByteArray payload;
    payload.append(static_cast<char>(0x01)); // Enable
    payload.append(static_cast<char>(0x02)); // Platform RTMP
    payload.append(packURL(url));
    msg.payload = payload;

    QByteArray duml = msg.serialize();
    QByteArray pkt = wrapMessage(0, QByteArray::fromHex("04100201"), duml);
    m_udp->writeDatagram(pkt, m_addr, 9004);
}

QByteArray WiFiController::wrapMessage(uint8_t whType, const QByteArray &metadata,
                                       const QByteArray &payload) {
    QByteArray pkt;
    uint16_t len = 20 + payload.size();
    pkt.append(static_cast<char>(len & 0xFF));
    pkt.append(static_cast<char>((whType << 4) | ((len >> 8) & 0x0F)));
    pkt.append(static_cast<char>(0x47));
    pkt.append(static_cast<char>(0xA8));
    pkt.append(metadata);
    while (pkt.size() < 20)
        pkt.append(static_cast<char>(0x00));
    pkt.append(payload);
    return pkt;
}

} // namespace dji
