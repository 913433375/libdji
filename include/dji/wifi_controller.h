#ifndef DJI_WIFI_CONTROLLER_H
#define DJI_WIFI_CONTROLLER_H

#include "dji/message.h"
#include <QHostAddress>
#include <QObject>
#include <QUdpSocket>

namespace dji {

class WiFiController : public QObject {
    Q_OBJECT
public:
    explicit WiFiController(const QHostAddress &addr, QObject *parent = nullptr);

    void sendHandshake();
    void sendVideoHandshake();
    void sendFCCEnable(bool enable);
    void sendRTMPBroadcast(const QString &url);

private:
    QByteArray wrapMessage(uint8_t whType, const QByteArray &metadata, const QByteArray &payload);

    QUdpSocket *m_udp;
    QHostAddress m_addr;
};

} // namespace dji

#endif // DJI_WIFI_CONTROLLER_H
