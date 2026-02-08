#ifndef DJI_CONSTANTS_H
#define DJI_CONSTANTS_H

#include <cstdint>

namespace dji {

enum class DeviceType : uint8_t {
    Unknown = 0,
    OsmoPocket3 = 0x2A,
    OsmoAction4 = 0x2E,
    OsmoAction5Pro = 0x2F, // Differentiated
    Undefined = 0xFF
};

enum class ComponentID : uint8_t {
    Camera = 0x01,
    App = 0x02,
    Gimbal = 0x03,
    FlightController = 0x04,
    WiFiAir = 0x05,
    RemoteController = 0x06,
    WiFiGroundStation = 0x07,
    VideoTransmission = 0x08,
    Battery = 0x09,
    Gimbal2 = 0x0e,
    Vision = 0x11,
    Goggles = 0x17,
    Pairer = 0x88,
};

struct InterfaceID {
    uint8_t sender;
    uint8_t receiver;

    constexpr InterfaceID(uint8_t s = 0, uint8_t r = 0) : sender(s), receiver(r) {
    }
    constexpr InterfaceID(ComponentID s, ComponentID r)
        : sender(static_cast<uint8_t>(s)), receiver(static_cast<uint8_t>(r)) {
    }

    constexpr bool operator==(const InterfaceID &other) const {
        return sender == other.sender && receiver == other.receiver;
    }
    constexpr bool operator!=(const InterfaceID &other) const {
        return !(*this == other);
    }
    constexpr bool matches(const InterfaceID &other) const {
        return (sender == other.sender && receiver == other.receiver) ||
               (sender == other.receiver && receiver == other.sender);
    }
};

using InterfaceInfo = InterfaceID;

static constexpr InterfaceID InterfaceIDAppToCamera = {0x02, 0x01};
static constexpr InterfaceID InterfaceIDAppToVideoTransmission = {0x02, 0x08};
static constexpr InterfaceID InterfaceIDVideoTransmissionToApp = {0x08, 0x02};
static constexpr InterfaceID InterfaceIDAppToWiFiGroundStation = {0x02, 0x07};
static constexpr InterfaceID InterfaceIDAppToPairer = {0x02, 0x88};
static constexpr InterfaceID InterfaceIDAppToRemoteController = {0x02, 0x06};
static constexpr InterfaceID InterfaceIDAppToGoggles = {0x02, 0x17};
static constexpr InterfaceID InterfaceIDAppToBattery = {0x02, 0x09};
static constexpr InterfaceID InterfaceIDAppToFlightController = {0x02, 0x04};
static constexpr InterfaceID InterfaceIDAppToApp = {0x02, 0x02};

enum class CommandSet : uint8_t {
    Core = 0x00,
    Info = 0x01,
    Camera = 0x02,
    FlightController = 0x03,
    Gimbal = 0x04,
    RemoteController = 0x06,
    WiFi = 0x07,
    Config = 0x08,
    Vision = 0x0a,
    Battery = 0x0d,
};

enum class CommandID : uint8_t {
    GetVersion = 0x01,
    GetDeviceType = 0x01,
    ConfigureStreaming = 0x78,
    PairingStage2 = 0x32,
    PairingStarted = 0x80,
    SetPairingPIN = 0x45,
    PairingPINApproved = 0x46,
    ConnectToWiFi = 0x47,
    PrepareToLiveStream = 0xE1,
    StartStopStreaming = 0x8E,
    WiFiScanReport = 0xAC,
    FCCSupport = 0xDE,
    GogglesModeToggle = 0x06,
    GogglesMode = 0x3D,
    StartScanningWiFi = 0xAB,
    CameraAPInfo = 0x07,
    CameraAPPSK = 0x0E,
};

enum class MessageID : uint16_t {
    PairingStarted = 0x7911,
    SetPairingPIN = 0x72AA,
    PairingPINApproved = 0x72AA,
    PairingStage1 = 0x0400,
    PairingStage2 = 0x74AA,
    PrepareToLiveStreamStage1 = 0xFEAB,
    PrepareToLiveStreamStage2 = 0xFFAB,
    StartScanningWiFi = 0x8EBB,
    ConnectToWiFi = 0x98BB,
    ConfigureStreaming = 0xB3BB,
    StartStreaming = 0xB4BB,
    StopStreaming = 0xB5BB,
    AppIdentifier = 0xC994,
    FCCEnable = 0x0001,
    CameraAPInfo = 0x76AA,
};

struct MessageType {
    uint8_t flags;
    CommandSet cmdSet;
    CommandID cmdID;

    static constexpr MessageType request(CommandSet s, CommandID i, uint8_t extraFlags = 0x40) {
        return {extraFlags, s, i};
    }
    static constexpr MessageType response(CommandSet s, CommandID i, uint8_t extraFlags = 0x80) {
        return {extraFlags, s, i};
    }
    static constexpr MessageType notification(CommandSet s, CommandID i) {
        return {0x00, s, i};
    }

    constexpr bool operator==(const MessageType &other) const {
        return flags == other.flags && cmdSet == other.cmdSet && cmdID == other.cmdID;
    }
    constexpr bool operator!=(const MessageType &other) const {
        return !(*this == other);
    }
};

static constexpr uint8_t MessageTypeFlagRequest = 0x00;
static constexpr uint8_t MessageTypeFlagAckRequired = 0x40;
static constexpr uint8_t MessageTypeFlagResponse = 0x80;

// Pairing
static constexpr MessageType MessageTypePairingStarted =
    MessageType::notification(CommandSet::Camera, CommandID::PairingStarted);
static constexpr MessageType MessageTypeSetPairingPIN =
    MessageType::request(CommandSet::WiFi, CommandID::SetPairingPIN);
static constexpr MessageType MessageTypePairingPINApproved =
    MessageType::request(CommandSet::WiFi, CommandID::PairingPINApproved);
static constexpr MessageType MessageTypePairingStatus =
    MessageType::response(CommandSet::WiFi, CommandID::SetPairingPIN, 0xC0);
static constexpr MessageType MessageTypePairingStage1 =
    MessageType::response(CommandSet::WiFi, CommandID::PairingPINApproved, 0xC0);
static constexpr MessageType MessageTypePairingStage2 =
    MessageType::request(CommandSet::Core, CommandID::PairingStage2);

// WiFi
static constexpr MessageType MessageTypeConnectToWiFi =
    MessageType::request(CommandSet::WiFi, CommandID::ConnectToWiFi);
static constexpr MessageType MessageTypeConnectToWiFiResult =
    MessageType::response(CommandSet::WiFi, CommandID::ConnectToWiFi, 0xC0);
static constexpr MessageType MessageTypeFCCSupport =
    MessageType::request(CommandSet::Core, CommandID::FCCSupport);
static constexpr MessageType MessageTypeFCCSupportResult =
    MessageType::response(CommandSet::Core, CommandID::FCCSupport, 0xC0);
static constexpr MessageType MessageTypeCameraAPInfo =
    MessageType::request(CommandSet::WiFi, CommandID::CameraAPInfo);
static constexpr MessageType MessageTypeCameraAPInfoResultSSID =
    MessageType::response(CommandSet::WiFi, CommandID::CameraAPInfo, 0xC0);
static constexpr MessageType MessageTypeCameraAPPSK =
    MessageType::request(CommandSet::WiFi, CommandID::CameraAPPSK);
static constexpr MessageType MessageTypeCameraAPInfoResultPSK =
    MessageType::response(CommandSet::WiFi, CommandID::CameraAPPSK, 0xC0);
static constexpr MessageType MessageTypeWiFiScanReport =
    MessageType::request(CommandSet::WiFi, CommandID::WiFiScanReport);
static constexpr MessageType MessageTypeStartScanningWiFi =
    MessageType::request(CommandSet::WiFi, CommandID::StartScanningWiFi);
static constexpr MessageType MessageTypeStartScanningWiFiResult =
    MessageType::response(CommandSet::WiFi, CommandID::StartScanningWiFi, 0xC0);

// Video Transmission
static constexpr MessageType MessageTypePrepareToLiveStream =
    MessageType::request(CommandSet::Camera, CommandID::PrepareToLiveStream);
static constexpr MessageType MessageTypePrepareToLiveStreamResult =
    MessageType::response(CommandSet::Camera, CommandID::PrepareToLiveStream, 0xC0);
static constexpr MessageType MessageTypeStartStopStreaming =
    MessageType::request(CommandSet::Camera, CommandID::StartStopStreaming);
static constexpr MessageType MessageTypeStartStopStreamingResult =
    MessageType::response(CommandSet::Camera, CommandID::StartStopStreaming);

// Streaming Config
static constexpr MessageType MessageTypeConfigureStreaming =
    MessageType::request(CommandSet::Config, CommandID::ConfigureStreaming);
static constexpr MessageType MessageTypeConfigureStreamingResult =
    MessageType::response(CommandSet::Config, CommandID::ConfigureStreaming, 0xC0);

// Camera/Other
static constexpr MessageType MessageTypeOsmoBroadcastConfig =
    MessageType::request(CommandSet::Camera, static_cast<CommandID>(0x08));
static constexpr MessageType MessageTypeOsmoBroadcastConfigResult =
    MessageType::response(CommandSet::Camera, static_cast<CommandID>(0x08));
static constexpr MessageType MessageTypeBatteryStatus =
    MessageType::notification(CommandSet::Battery, static_cast<CommandID>(0x02));
static constexpr MessageType MessageTypeStreamingStatus =
    MessageType::notification(CommandSet::Battery, static_cast<CommandID>(0x02));
static constexpr MessageType MessageTypeGetVersion =
    MessageType::request(CommandSet::Core, CommandID::GetVersion);
static constexpr MessageType MessageTypeGetVersionResponse =
    MessageType::response(CommandSet::Core, CommandID::GetVersion);
static constexpr MessageType MessageTypeGetBatteryInfo =
    MessageType::request(CommandSet::Camera, static_cast<CommandID>(0xF4));
static constexpr MessageType MessageTypeGogglesModeToggle =
    MessageType::request(CommandSet::Config, CommandID::GogglesModeToggle);
static constexpr MessageType MessageTypeGogglesMode =
    MessageType::request(CommandSet::FlightController, CommandID::GogglesMode);
static constexpr MessageType MessageTypeRemoteControllerSimulatorData =
    MessageType::notification(CommandSet::RemoteController, static_cast<CommandID>(0x24));

// Generic
static constexpr MessageType MessageTypeMaybeStatus =
    MessageType::notification(CommandSet::Camera, static_cast<CommandID>(0xFF));
static constexpr MessageType MessageTypeKeepAlive =
    MessageType::notification(CommandSet::Core, CommandID::GetDeviceType);

enum class Resolution : uint8_t {
    Res480p = 0x47,
    Res720p = 0x04,
    Res1080p = 0x0A,
};

enum class FPS : uint8_t {
    FPS24 = 0x01,
    FPS25 = 0x02,
    FPS30 = 0x03,
};

enum class ImageStabilization : uint8_t {
    None = 0,
    RockSteady = 1,
    RockSteadyPlus = 2,
};

enum class GogglesMode : uint8_t {
    Normal = 0,
    V3 = 1,
};

struct RemoteControllerSimulatorData {
    uint16_t rightStickHorizontal;
    uint16_t rightStickVertical;
    uint16_t leftStickVertical;
    uint16_t leftStickHorizontal;
    uint32_t buttons;
};

} // namespace dji

#endif // DJI_CONSTANTS_H
