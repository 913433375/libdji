#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothDeviceInfo>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>
#include <QTimer>
#include <iostream>

#include "dji/constants.h"
#include "dji/device.h"
#include "dji/device_flow.h"
#include "dji/device_manager.h"
#include "dji/interface_camera.h"
#include "dji/interface_goggles.h"
#include "dji/interface_remote_controller.h"
#include "dji/interface_video_transmission.h"
#include "dji/interface_wifi.h"
#include "dji/wifi_controller.h"

class DemoController : public QObject {
    Q_OBJECT
public:
    DemoController(dji::DeviceFlow *flow, int scanTimeoutMs, QObject *parent = nullptr)
        : QObject(parent), m_flow(flow), m_scanTimeout(scanTimeoutMs > 0 ? scanTimeoutMs : 30000) {
        m_manager = new dji::DeviceManager(nullptr, this);
        connect(m_manager, &dji::DeviceManager::log, this, &DemoController::onLog);
        connect(m_manager, &dji::DeviceManager::error, this, &DemoController::onError);
        connect(m_manager, &dji::DeviceManager::finished, this, &DemoController::onFinished);
        connect(m_manager, &dji::DeviceManager::deviceChanged, this, [this]() {
            if (m_manager->device() && !m_started) {
                m_started = true;
                if (m_flow) {
                    m_manager->runFlow(m_manager->device(), m_flow);
                } else {
                    // Logic for scan only
                    std::cout
                        << "Found device: "
                        << m_manager->device()->deviceInfo().name().toStdString() << " ("
                        << m_manager->device()->deviceInfo().address().toString().toStdString()
                        << ")" << std::endl;
                    QCoreApplication::quit();
                }
            }
        });
    }

    void start(const dji::DiscoveryOptions &discOpts) {
        m_manager->startDiscovery(discOpts);
        QTimer::singleShot(m_scanTimeout, this, [this]() {
            if (!m_manager->device()) {
                fail("Discovery timed out without matching device");
            }
        });
    }

private slots:
    void onLog(const QString &message) {
        qInfo().noquote() << message;
    }

    void onError(const QString &message) {
        fail(message);
    }

    void onFinished(dji::Device *dev, bool success) {
        Q_UNUSED(dev);
        if (success) {
            qInfo() << "Operation completed successfully";
            QCoreApplication::quit();
        } else {
            fail("Operation failed");
        }
    }

private:
    void fail(const QString &message) {
        if (m_failed)
            return;
        m_failed = true;
        qCritical().noquote() << message;
        QCoreApplication::exit(1);
    }

    dji::DeviceManager *m_manager = nullptr;
    dji::DeviceFlow *m_flow = nullptr;
    int m_scanTimeout = 30000;
    bool m_failed = false;
    bool m_started = false;
};

void printUsage() {
    std::cout << "Usage: dji_demo <subcommand> [options]" << std::endl;
    std::cout << "Subcommands:" << std::endl;
    std::cout << "  ble   BLE-based commands" << std::endl;
    std::cout << "  wifi  WiFi-based commands" << std::endl;
}

void printBleUsage() {
    std::cout << "Usage: dji_demo ble <command> [options]" << std::endl;
    std::cout << "Commands:" << std::endl;
    std::cout << "  scan" << std::endl;
    std::cout << "  connect-wifi-and-start-streaming --wifi-ssid <ssid> --wifi-psk <psk> "
                 "--rtmp-url <url> [--resolution <res>] [--fps <fps>] [--bitrate-kbps <kbps>]"
              << std::endl;
    std::cout << "  stop-streaming-and-disconnect-wifi" << std::endl;
    std::cout << "  camera-ap-info" << std::endl;
    std::cout << "  rtmp-broadcast --url <url> [--disable]" << std::endl;
    std::cout << "  firmware-version" << std::endl;
    std::cout << "  battery-info" << std::endl;
    std::cout << "  fcc-enable [--disable]" << std::endl;
    std::cout << "  set-goggles-mode --mode <usb|normal>" << std::endl;
    std::cout << "  remote-controller-simulator [--right-h <val>] [--right-v <val>] [--left-h "
                 "<val>] [--left-v <val>]"
              << std::endl;
}

void printWifiUsage() {
    std::cout << "Usage: dji_demo wifi <command> [options]" << std::endl;
    std::cout << "Commands:" << std::endl;
    std::cout << "  start-video" << std::endl;
    std::cout << "  fcc-enable" << std::endl;
    std::cout << "  rtmp-broadcast --url <url> [--disable]" << std::endl;
}

int main(int argc, char **argv) {
    QCoreApplication app(argc, argv);
    QStringList args = app.arguments();

    if (args.size() < 2) {
        printUsage();
        return 0;
    }

    QString subcmd = args.at(1);
    if (subcmd == "ble") {
        if (args.size() < 3) {
            printBleUsage();
            return 0;
        }

        QString blecmd = args.at(2);
        dji::DeviceFlow *flow = nullptr;

        QCommandLineParser parser;
        parser.addHelpOption();

        QCommandLineOption filterAddrOpt("filter-device-addr", "Filter by device address substring",
                                         "addr");
        parser.addOption(filterAddrOpt);
        QCommandLineOption scanTimeoutOpt("scan-timeout", "Scan timeout seconds", "seconds", "30");
        parser.addOption(scanTimeoutOpt);

        if (blecmd == "connect-wifi-and-start-streaming") {
            QCommandLineOption ssidOpt("wifi-ssid", "WiFi SSID", "ssid");
            QCommandLineOption pskOpt("wifi-psk", "WiFi PSK", "psk");
            QCommandLineOption rtmpOpt("rtmp-url", "RTMP URL", "url");
            QCommandLineOption resOpt("resolution", "Resolution (480p, 720p, 1080p)", "res",
                                      "1080p");
            QCommandLineOption fpsOpt("fps", "FPS (24, 25, 30)", "fps", "30");
            QCommandLineOption bitrateOpt("bitrate-kbps", "Bitrate in Kbps", "kbps", "6000");

            parser.addOption(ssidOpt);
            parser.addOption(pskOpt);
            parser.addOption(rtmpOpt);
            parser.addOption(resOpt);
            parser.addOption(fpsOpt);
            parser.addOption(bitrateOpt);

            parser.process(app);

            dji::StreamingOptions opts;
            opts.ssid = parser.value(ssidOpt);
            opts.psk = parser.value(pskOpt);
            opts.rtmpUrl = parser.value(rtmpOpt);
            opts.bitrateKbps = parser.value(bitrateOpt).toUInt();

            QString resStr = parser.value(resOpt);
            if (resStr == "480p")
                opts.resolution = dji::Resolution::Res480p;
            else if (resStr == "720p")
                opts.resolution = dji::Resolution::Res720p;
            else
                opts.resolution = dji::Resolution::Res1080p;

            int fpsVal = parser.value(fpsOpt).toInt();
            if (fpsVal == 24)
                opts.fps = dji::FPS::FPS24;
            else if (fpsVal == 25)
                opts.fps = dji::FPS::FPS25;
            else
                opts.fps = dji::FPS::FPS30;

            if (opts.ssid.isEmpty() || opts.rtmpUrl.isEmpty()) {
                parser.showHelp(1);
            }

            flow = new dji::StreamingStarter(opts);
        } else if (blecmd == "stop-streaming-and-disconnect-wifi") {
            parser.process(app);
            flow = new dji::StreamingStopper();
        } else if (blecmd == "camera-ap-info") {
            parser.process(app);
            flow = new dji::CameraAPInfoFlow();
        } else if (blecmd == "rtmp-broadcast") {
            QCommandLineOption urlOpt("url", "RTMP URL", "url");
            QCommandLineOption disableOpt("disable", "Disable RTMP broadcast");
            parser.addOption(urlOpt);
            parser.addOption(disableOpt);
            parser.process(app);
            QString url = parser.value(urlOpt);
            bool enabled = !parser.isSet(disableOpt);
            flow = new dji::GenericActionFlow([enabled, url](dji::Device *dev) {
                dev->videoTransmission()->configureRTMP(enabled, url);
            });
        } else if (blecmd == "firmware-version") {
            parser.process(app);
            flow = new dji::GenericActionFlow([](dji::Device *dev) {
                QObject::connect(
                    dev->camera(), &dji::InterfaceCamera::versionReceived, [](const QString &ver) {
                        std::cout << "Firmware version: " << ver.toStdString() << std::endl;
                    });
                dev->camera()->getVersion();
            });
        } else if (blecmd == "battery-info") {
            parser.process(app);
            flow = new dji::GenericActionFlow([](dji::Device *dev) {
                QObject::connect(dev->camera(), &dji::InterfaceCamera::batteryInfoReceived,
                                 [](const QByteArray &data) {
                                     std::cout << "Battery info: " << data.toHex().toStdString()
                                               << std::endl;
                                 });
                dev->camera()->getBatteryInfo();
            });
        } else if (blecmd == "fcc-enable") {
            QCommandLineOption disableOpt("disable", "Disable FCC mode");
            parser.addOption(disableOpt);
            parser.process(app);
            bool enable = !parser.isSet(disableOpt);
            flow = new dji::GenericActionFlow(
                [enable](dji::Device *dev) { dev->camera()->setFCCEnable(enable); });
        } else if (blecmd == "set-goggles-mode") {
            QCommandLineOption modeOpt("mode", "Goggles mode (usb, normal)", "mode");
            parser.addOption(modeOpt);
            parser.process(app);
            QString modeStr = parser.value(modeOpt);
            dji::GogglesMode mode = dji::GogglesMode::Normal;
            if (modeStr == "usb")
                mode = dji::GogglesMode::V3;

            flow = new dji::GenericActionFlow(
                [mode](dji::Device *dev) { dev->goggles()->setGogglesMode(mode); });
        } else if (blecmd == "remote-controller-simulator") {
            QCommandLineOption rhOpt("right-h", "Right stick horizontal", "val", "1024");
            QCommandLineOption rvOpt("right-v", "Right stick vertical", "val", "1024");
            QCommandLineOption lhOpt("left-h", "Left stick horizontal", "val", "1024");
            QCommandLineOption lvOpt("left-v", "Left stick vertical", "val", "1024");
            parser.addOption(rhOpt);
            parser.addOption(rvOpt);
            parser.addOption(lhOpt);
            parser.addOption(lvOpt);
            parser.process(app);

            dji::RemoteControllerSimulatorData data;
            data.rightStickHorizontal = parser.value(rhOpt).toUInt();
            data.rightStickVertical = parser.value(rvOpt).toUInt();
            data.leftStickHorizontal = parser.value(lhOpt).toUInt();
            data.leftStickVertical = parser.value(lvOpt).toUInt();

            flow = new dji::GenericActionFlow(
                [data](dji::Device *dev) { dev->remoteController()->sendSimulatorData(data); });
        } else if (blecmd == "scan") {
            parser.process(app);
            // flow NULL leads to scan logic in DemoController
        } else {
            printBleUsage();
            return 1;
        }

        dji::DiscoveryOptions discOpts;
        discOpts.deviceAddrFilter = parser.value(filterAddrOpt);
        int scanTimeout = parser.value(scanTimeoutOpt).toInt() * 1000;

        DemoController controller(flow, scanTimeout);
        controller.start(discOpts);
    } else if (subcmd == "wifi") {
        if (args.size() < 3) {
            printWifiUsage();
            return 0;
        }

        QString wificmd = args.at(2);
        QCommandLineParser parser;
        parser.addHelpOption();
        QCommandLineOption addrOpt("addr", "Device IP address", "addr", "192.168.2.1");
        parser.addOption(addrOpt);

        if (wificmd == "start-video") {
            parser.process(app);
            dji::WiFiController ctrl(QHostAddress(parser.value(addrOpt)));
            ctrl.sendHandshake();
            ctrl.sendVideoHandshake();
            std::cout << "WiFi start-video sequence sent" << std::endl;
            return 0;
        } else if (wificmd == "fcc-enable") {
            parser.process(app);
            dji::WiFiController ctrl(QHostAddress(parser.value(addrOpt)));
            ctrl.sendFCCEnable(true);
            std::cout << "WiFi fcc-enable command sent" << std::endl;
            return 0;
        } else if (wificmd == "rtmp-broadcast") {
            QCommandLineOption urlOpt("url", "RTMP URL", "url");
            QCommandLineOption disableOpt("disable", "Disable RTMP broadcast");
            parser.addOption(urlOpt);
            parser.addOption(disableOpt);
            parser.process(app);

            dji::WiFiController ctrl(QHostAddress(parser.value(addrOpt)));
            ctrl.sendRTMPBroadcast(parser.value(urlOpt));
            std::cout << "WiFi rtmp-broadcast command sent" << std::endl;
            return 0;
        } else {
            printWifiUsage();
            return 1;
        }
    } else {
        printUsage();
        return 0;
    }

    return app.exec();
}

#include "dji_demo.moc"
