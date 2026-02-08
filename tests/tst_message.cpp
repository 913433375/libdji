#include "tst_message.h"
#include "dji/crc.h"
#include <QDebug>
#include <QtEndian>
#include <QtTest>

using namespace dji;

void TestMessage::testSerialize() {

    Message msg;
    msg.interface = {ComponentID::App, ComponentID::WiFiGroundStation}; // 0x02, 0x07
    msg.msgId = static_cast<MessageID>(0x0400);
    msg.msgType = MessageType{MessageTypeFlagResponse | MessageTypeFlagAckRequired,
                              CommandSet::WiFi, static_cast<CommandID>(0x46)};
    msg.payload = QByteArray::fromHex("00");

    QByteArray serialized = msg.serialize();

    QCOMPARE(static_cast<uint8_t>(serialized[0]), 0x55);
    QCOMPARE(static_cast<uint8_t>(serialized[1]), 14);   // 11 + 1 + 2
    QCOMPARE(static_cast<uint8_t>(serialized[2]), 0x04); // Version 0x01

    QCOMPARE(static_cast<uint8_t>(serialized[4]), 0x02);
    QCOMPARE(static_cast<uint8_t>(serialized[5]), 0x07);

    // Big-endian msgId
    QCOMPARE(static_cast<uint8_t>(serialized[6]), 0x04);
    QCOMPARE(static_cast<uint8_t>(serialized[7]), 0x00);

    QCOMPARE(static_cast<uint8_t>(serialized[8]), 0xC0);
    QCOMPARE(static_cast<uint8_t>(serialized[9]), 0x07);
    QCOMPARE(static_cast<uint8_t>(serialized[10]), 0x46);

    QCOMPARE(static_cast<uint8_t>(serialized[11]), 0x00); // payload

    uint8_t expectedHeaderCrc = dji::crc8(serialized.left(3));
    QCOMPARE(static_cast<uint8_t>(serialized[3]), expectedHeaderCrc);

    QByteArray fullNoCrc = serialized.left(serialized.size() - 2);
    uint16_t expectedFullCrc = dji::crc16(fullNoCrc);

    uint16_t actualFullCrc = qFromLittleEndian<uint16_t>(
        reinterpret_cast<const uchar *>(serialized.right(2).constData()));
    QCOMPARE(actualFullCrc, expectedFullCrc);
}

void TestMessage::testParse() {

    Message originalMsg;
    originalMsg.interface = {ComponentID::App, ComponentID::WiFiGroundStation};
    originalMsg.msgId = static_cast<MessageID>(0x0400);
    originalMsg.msgType = MessageType{MessageTypeFlagResponse | MessageTypeFlagAckRequired,
                                      CommandSet::WiFi, static_cast<CommandID>(0x46)};
    originalMsg.payload = QByteArray::fromHex("00");

    QByteArray validData = originalMsg.serialize();

    bool ok = false;
    Message msg = Message::parse(validData, &ok);

    QVERIFY(ok);
    QCOMPARE(msg.interface.sender, originalMsg.interface.sender);
    QCOMPARE(msg.interface.receiver, originalMsg.interface.receiver);
    QCOMPARE(msg.msgId, originalMsg.msgId);
    QCOMPARE(msg.msgType, originalMsg.msgType);
    QCOMPARE(msg.payload, originalMsg.payload);
}

void TestMessage::testPackString() {
    QString s = "Hello";
    QByteArray packed = packString(s);

    QCOMPARE(packed.size(), 6);
    uint8_t len = static_cast<uint8_t>(packed[0]);
    QCOMPARE(len, 5);
    QCOMPARE(packed.mid(1), QByteArray("Hello"));
}

void TestMessage::testPackURL() {
    QString s = "http://example.com";
    QByteArray packed = packURL(s);

    QCOMPARE(packed.size(), 2 + s.size());
    uint8_t len = static_cast<uint8_t>(packed[0]);
    QCOMPARE(len, s.size());
    QCOMPARE(packed.mid(2), s.toUtf8());
}
