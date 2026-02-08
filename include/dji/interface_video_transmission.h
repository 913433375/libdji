#ifndef DJI_INTERFACE_VIDEO_TRANSMISSION_H
#define DJI_INTERFACE_VIDEO_TRANSMISSION_H

#include "dji/message.h"
#include <QObject>

namespace dji {

class Device;

class InterfaceVideoTransmission : public QObject {
    Q_OBJECT
public:
    explicit InterfaceVideoTransmission(Device *device);

    InterfaceID interfaceID() const {
        return InterfaceIDAppToVideoTransmission;
    }

    void prepareToLiveStream();
    void startLiveStream(Resolution resolution, uint16_t bitrateKbps, FPS fps,
                         const QString &rtmpURL);
    void stopLiveStream();
    void configureLiveStream(Resolution resolution, uint16_t bitrateKbps, FPS fps,
                             const QString &rtmpURL);
    void configureRTMP(bool enabled, const QString &url);

    void handleMessage(const Message &msg);

private:
    Device *m_device;
    int m_prepareStage = 0;

signals:
    void error(const QString &message);
    void log(const QString &message);
    void prepareToLiveStreamComplete();
    void startLiveStreamComplete();
    void stopLiveStreamComplete();
    void configureLiveStreamComplete();
    void configureRTMPComplete();
    void liveStreamResult(int batteryPercentage);
};

} // namespace dji

#endif
