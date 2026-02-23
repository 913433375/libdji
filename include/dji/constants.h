#ifndef DJI_CONSTANTS_H
#define DJI_CONSTANTS_H

#include <cstdint>
#include <string>

namespace dji {

// ============================================================================
// BLE GATT Constants
// ============================================================================

// BLE characteristic handles for Osmo Action/Pocket/Drone devices.
// These are ATT handles, not UUIDs.
static constexpr uint16_t BLECharHandleReceiver = 0x002D;         // Notifications: device -> app
static constexpr uint16_t BLECharHandlePairingRequestor = 0x002E; // Write: pairing initiation
static constexpr uint16_t BLECharHandleSender = 0x0030;           // Write: app -> device

// BLE service/characteristic short UUIDs for Osmo Mobile gimbals (OM2/OM3/OM4/OM SE).
static constexpr uint16_t BLEServiceUUIDOsmoMobile = 0xFFF0;
static constexpr uint16_t BLECharUUIDOsmoMobileNotify = 0xFFF4;    // Notifications: device -> host
static constexpr uint16_t BLECharUUIDOsmoMobileWrite = 0xFFF5;     // Write: host -> device
static constexpr uint16_t BLECharUUIDOsmoMobileReadWrite = 0xFFF3; // Read/Write (auxiliary)

// BLE MTU negotiated with DJI devices.
static constexpr uint16_t BLEMTU = 517;

// BLE manufacturer advertisement data prefix identifying DJI devices.
static constexpr uint8_t DJIMagic[2] = {0xAA, 0x08};

// Default pairing PIN code. The user must confirm this PIN on the device screen.
static constexpr const char *DefaultPINCode = "5160";

// ============================================================================
// Device Types
// ============================================================================

// DeviceType identifies the model of a DJI device.
// For Osmo Action/Pocket: determined from BLE manufacturer data bytes [2:4].
// For libdji internal use: uses device-specific bytes for streaming config.
enum class DeviceType : uint8_t {
    Unknown = 0,
    OsmoPocket3 = 0x2A,
    OsmoAction3 = 0x2C,
    OsmoAction4 = 0x2E,
    OsmoAction5Pro = 0x2F,
    Undefined = 0xFF
};

// DeviceFamily represents the broad category of a device.
enum class DeviceFamily : uint8_t {
    Unknown = 0,
    OsmoAction,  // Uses BLE handles 0x002D/0x002E/0x0030
    OsmoPocket,  // Uses BLE handles 0x002D/0x002E/0x0030
    OsmoMobile,  // Uses BLE service UUID FFF0 with FFF4/FFF5
    Drone,       // Uses BLE handles similar to Osmo Action
    Mic,         // DJI Mic products
};

// Returns the device family for a given device type.
inline DeviceFamily deviceTypeFamily(DeviceType t) {
    switch (t) {
    case DeviceType::OsmoAction3:
    case DeviceType::OsmoAction4:
    case DeviceType::OsmoAction5Pro:
        return DeviceFamily::OsmoAction;
    case DeviceType::OsmoPocket3:
        return DeviceFamily::OsmoPocket;
    default:
        return DeviceFamily::Unknown;
    }
}

// Returns true if the device supports image stabilization setting via BLE.
inline bool deviceTypeSupportsImageStabilization(DeviceType t) {
    return t == DeviceType::OsmoAction4 || t == DeviceType::OsmoAction5Pro;
}

// Returns true if the device supports RTMP livestreaming via BLE.
inline bool deviceTypeSupportsRTMPStreaming(DeviceType t) {
    auto family = deviceTypeFamily(t);
    return family == DeviceFamily::OsmoAction || family == DeviceFamily::OsmoPocket;
}

// ============================================================================
// Component IDs (DUML addressing, bytes [4:5] in packet)
// ============================================================================

enum class ComponentID : uint8_t {
    Camera = 0x01,            // Primary imaging unit
    App = 0x02,               // Mobile app (DJI MIMO, DJI Fly)
    Gimbal = 0x03,            // Primary gimbal stabilizer
    FlightController = 0x04,  // Aircraft flight controller
    WiFiAir = 0x05,           // Air-side wireless module
    RemoteController = 0x06,  // Physical remote controller
    WiFiGroundStation = 0x07, // Ground-side wireless module
    VideoTransmission = 0x08, // OFDM video transmission link
    Battery = 0x09,           // Smart battery management system
    ESC = 0x0C,               // Electronic speed controller
    Gimbal2 = 0x0e,           // Secondary gimbal
    Vision = 0x11,            // Mono/binocular vision system
    Goggles = 0x17,           // FPV Goggles
    Payload = 0x21,           // External payload device
    CameraExtension = 0x28,   // Camera extension module
    Pairer = 0x88,            // BLE pairing service
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

// Pre-defined interface pairs for common communication paths.
static constexpr InterfaceID InterfaceIDAppToApp = {0x02, 0x02};
static constexpr InterfaceID InterfaceIDAppToCamera = {0x02, 0x01};
static constexpr InterfaceID InterfaceIDAppToGimbal = {0x02, 0x03};
static constexpr InterfaceID InterfaceIDAppToFlightController = {0x02, 0x04};
static constexpr InterfaceID InterfaceIDAppToRemoteController = {0x02, 0x06};
static constexpr InterfaceID InterfaceIDAppToWiFiGroundStation = {0x02, 0x07};
static constexpr InterfaceID InterfaceIDAppToVideoTransmission = {0x02, 0x08};
static constexpr InterfaceID InterfaceIDAppToBattery = {0x02, 0x09};
static constexpr InterfaceID InterfaceIDAppToGoggles = {0x02, 0x17};
static constexpr InterfaceID InterfaceIDAppToPairer = {0x02, 0x88};
static constexpr InterfaceID InterfaceIDFlightControllerToApp = {0x04, 0x02};
static constexpr InterfaceID InterfaceIDCameraExtensionToApp = {0x28, 0x02};
static constexpr InterfaceID InterfaceIDCameraToApp = {0x01, 0x02};
static constexpr InterfaceID InterfaceIDVideoTransmissionToApp = {0x08, 0x02};
static constexpr InterfaceID InterfaceIDBatteryToApp = {0x09, 0x02};

// ============================================================================
// Command Sets (byte [9] in DUML packet)
// ============================================================================

enum class CommandSet : uint8_t {
    Core = 0x00,             // Core/General: heartbeat, serial number, FCC, pairing stage 2
    Info = 0x01,             // Device info: product ID, firmware version
    Camera = 0x02,           // Camera: photo, record, goggles mode, broadcast, streaming
    FlightController = 0x03, // Flight control: stick data, motor, goggles mode
    Gimbal = 0x04,           // Gimbal: status, keep alive, rotation control
    GimbalControl = 0x05,    // Gimbal control: rotation commands (OM2/OM3/OM4)
    RemoteController = 0x06, // Remote controller: simulator data
    WiFi = 0x07,             // WiFi/Pairing: AP info, PIN, scan, connect
    Config = 0x08,           // Config/DM36X: streaming configuration
    Vision = 0x0a,           // Vision: obstacle avoidance
    Battery = 0x0d,          // Battery: status, dynamic info
    Bluetooth = 0x12,        // Bluetooth: BLE name, MAC, broadcast, iBeacon UUIDs
    WirelessMic = 0x5b,      // Audio/Wireless Mic: DJI Mic pairing, parameters
    App = 0xee,              // App-level commands
};

// ============================================================================
// Command IDs (byte [10] in DUML packet, meaning depends on CommandSet)
// ============================================================================

enum class CommandID : uint8_t {
    // Core (Set 0x00)
    GetSerialNum = 0x0A,
    Heartbeat = 0x2B,
    PairingStage2 = 0x32,
    ParameterPush = 0x99,
    FCCSupport = 0xDE,

    // Info (Set 0x01)
    GetProductID = 0x0D,
    GetVersion = 0x1E,

    // Camera (Set 0x02)
    TakePhoto = 0x01,
    TakeRecord = 0x02,
    GogglesModeToggle = 0x06,
    OsmoBroadcastConfig = 0x08,
    VideoStreamSubscribe = 0x3C,
    VideoStreamUnsubscribe = 0x3D,
    PairingStarted = 0x80,
    StartStopStreaming = 0x8E,
    PrepareToLiveStream = 0xE1,

    // Flight Control (Set 0x03)
    FlightStickData = 0x02,
    MotorControl = 0x21,
    GogglesMode = 0x3D,

    // Gimbal (Set 0x04)
    GimbalStatus = 0x05,
    KeepAlive = 0x27,

    // Gimbal Control (Set 0x05)
    GimbalRotation = 0x01,
    GimbalBattery = 0x06,

    // Remote Controller (Set 0x06)
    RCSimulatorData = 0x24,

    // WiFi (Set 0x07)
    CameraAPInfo = 0x07,
    CameraAPPSK = 0x0E,
    SetPairingPIN = 0x45,
    PairingPINApproved = 0x46,
    ConnectToWiFi = 0x47,
    StartScanningWiFi = 0xAB,
    WiFiScanReport = 0xAC,

    // Config (Set 0x08)
    ConfigureStreaming = 0x78,

    // Battery (Set 0x0D)
    BatteryStatus = 0x02,
    GetBatteryInfo = 0x03,

    // Bluetooth (Set 0x12)
    BLEGetName = 0x07,
    BLESetName = 0x08,
    GetMACAddress = 0x0C,
    BroadcastControl = 0x10,
    GetIBeaconUUID = 0x21,
    AudioDeviceMgmt = 0x65,

    // Wireless Mic (Set 0x5B)
    MicParameter = 0x01,
    MicPassword = 0x09,
    MicCapability = 0x12,
    MicHIDUpgrade = 0x35,
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
    MessageType::request(CommandSet::Info, CommandID::GetVersion);
static constexpr MessageType MessageTypeGetVersionResponse =
    MessageType::response(CommandSet::Info, CommandID::GetVersion);
static constexpr MessageType MessageTypeGetBatteryInfo =
    MessageType::request(CommandSet::Battery, CommandID::GetBatteryInfo);
static constexpr MessageType MessageTypeGogglesModeToggle =
    MessageType::request(CommandSet::Config, CommandID::GogglesModeToggle);
static constexpr MessageType MessageTypeGogglesMode =
    MessageType::request(CommandSet::FlightController, CommandID::GogglesMode);
static constexpr MessageType MessageTypeRemoteControllerSimulatorData =
    MessageType::notification(CommandSet::RemoteController, static_cast<CommandID>(0x24));

// Gimbal
static constexpr MessageType MessageTypeGimbalStatus =
    MessageType::notification(CommandSet::Gimbal, CommandID::GimbalStatus);
static constexpr MessageType MessageTypeKeepAlive =
    MessageType::notification(CommandSet::Gimbal, CommandID::KeepAlive);
static constexpr MessageType MessageTypeGimbalRotation =
    MessageType::request(CommandSet::GimbalControl, CommandID::GimbalRotation);
static constexpr MessageType MessageTypeGimbalBattery =
    MessageType::notification(CommandSet::GimbalControl, CommandID::GimbalBattery);

// Core
static constexpr MessageType MessageTypeGetSerialNum =
    MessageType::request(CommandSet::Core, CommandID::GetSerialNum);
static constexpr MessageType MessageTypeHeartbeat =
    MessageType::request(CommandSet::Core, CommandID::Heartbeat);
static constexpr MessageType MessageTypeParameterPush =
    MessageType::notification(CommandSet::Core, CommandID::ParameterPush);

// Bluetooth (Set 0x12)
static constexpr MessageType MessageTypeBLEGetName =
    MessageType::request(CommandSet::Bluetooth, CommandID::BLEGetName);
static constexpr MessageType MessageTypeBLEGetNameResult =
    MessageType::response(CommandSet::Bluetooth, CommandID::BLEGetName, 0xC0);
static constexpr MessageType MessageTypeBLESetName =
    MessageType::request(CommandSet::Bluetooth, CommandID::BLESetName);
static constexpr MessageType MessageTypeGetMACAddress =
    MessageType::request(CommandSet::Bluetooth, CommandID::GetMACAddress);
static constexpr MessageType MessageTypeGetMACAddressResult =
    MessageType::response(CommandSet::Bluetooth, CommandID::GetMACAddress, 0xC0);
static constexpr MessageType MessageTypeBroadcastControl =
    MessageType::request(CommandSet::Bluetooth, CommandID::BroadcastControl);
static constexpr MessageType MessageTypeGetIBeaconUUID =
    MessageType::request(CommandSet::Bluetooth, CommandID::GetIBeaconUUID);
static constexpr MessageType MessageTypeGetIBeaconUUIDResult =
    MessageType::response(CommandSet::Bluetooth, CommandID::GetIBeaconUUID, 0xC0);
static constexpr MessageType MessageTypeAudioDeviceMgmt =
    MessageType::request(CommandSet::Bluetooth, CommandID::AudioDeviceMgmt);

// Wireless Mic (Set 0x5B)
static constexpr MessageType MessageTypeMicParameter =
    MessageType::request(CommandSet::WirelessMic, CommandID::MicParameter);
static constexpr MessageType MessageTypeMicPassword =
    MessageType::request(CommandSet::WirelessMic, CommandID::MicPassword);
static constexpr MessageType MessageTypeMicCapability =
    MessageType::request(CommandSet::WirelessMic, CommandID::MicCapability);
static constexpr MessageType MessageTypeMicHIDUpgrade =
    MessageType::request(CommandSet::WirelessMic, CommandID::MicHIDUpgrade);

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
    Off = 0x00,
    RockSteady = 0x01,
    HorizonSteady = 0x02,
    RockSteadyPlus = 0x03,
    HorizonBalancing = 0x04,
};

enum class GogglesMode : uint8_t {
    Normal = 0,
    USB = 1,
};

// Remote Controller simulator data.
// Stick values range: min=364, center=1024, max=1684.
struct RemoteControllerSimulatorData {
    uint16_t rightStickHorizontal = 1024;
    uint16_t rightStickVertical = 1024;
    uint16_t leftStickVertical = 1024;
    uint16_t leftStickHorizontal = 1024;
    uint32_t buttons = 0;

    static constexpr uint16_t StickMin = 364;
    static constexpr uint16_t StickCenter = 1024;
    static constexpr uint16_t StickMax = 1684;
};

// Gimbal rotation command for Osmo Mobile gimbals.
// Angles/speeds are in 0.1 degree units.
enum class GimbalRotationMode : uint8_t {
    Relative = 0x04, // Move relative to current position
    Absolute = 0x05, // Move to absolute position
    Speed = 0x80,    // Continuous rotation at specified speed
};

struct GimbalRotation {
    int16_t yaw = 0;
    int16_t roll = 0;
    int16_t pitch = 0;
    GimbalRotationMode mode = GimbalRotationMode::Speed;
    uint8_t time = 0;
};

// WiFi scan result from a WiFi scan report.
struct WiFiScanResult {
    std::string ssid;
    int8_t rssi = 0;
    uint8_t security = 0;
    uint8_t channel = 0;
};

// Battery status from a battery notification.
struct BatteryStatus {
    int8_t capacity = -128; // Percentage 0-100, or -128 if undefined
};

// Firmware version information.
struct FirmwareVersion {
    std::string raw;
};

// BLE wrapper layer magic bytes.
static constexpr uint8_t BLEWrapperMagicDUML = 0xEE;
static constexpr uint8_t BLEWrapperMagicVideo = 0x02;
static constexpr uint8_t BLEWrapperMagicHandshake = 0x00;

} // namespace dji

#endif // DJI_CONSTANTS_H
