#include "sidaemon.h"
#include <sistoragedriver.h>
#include <sidevice.h>
#include <sideviceaccess.h>
#include <sideviceaccessregistry.h>
#include "deviceaccess/sideviceaccessmanager.h"
#include "deviceaccess/sisequentialpropertymanager.h"
#include "websocket/siwebsocketmanager.h"
#include "bluetooth/sibluetoothmanager.h"
#include <QCommandLineParser>
#include <QFile>
#include <QLoggingCategory>
#include <QSettings>
#include <QElapsedTimer>
#include <memory>

using namespace std;

Q_LOGGING_CATEGORY(DAEMON, "daemon", QtInfoMsg)

class StdOutSubscriber: public SIDeviceAccessManager::PropertySubscriber {
  public:
    inline StdOutSubscriber(): out_(stdout) {}

  private:
    void propertyChanged(SIGlobalPropertyID id, const QVariant& value) override {
        out_ << id.propertyID() << " = " << value.toString() << endl;
    }

    QTextStream out_;
};

SIDaemon::SIDaemon(int argc, char** argv): QCoreApplication(argc, argv) {
    setApplicationName("sigatewayd");
}

SIDaemon::~SIDaemon() = default;

bool SIDaemon::initialize() {
    // Parse command line options.
    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    QCommandLineOption configurationFileOption {{"c", "config"}, "Use configuration file <file> instead of default \"/etc/sigateway.conf\"", "file"};
    configurationFileOption.setDefaultValue("/etc/sigatewayd.conf");
    parser.addOption(configurationFileOption);
    parser.process(*this);

    // Ensure that configuration file exists.
    auto configurationFileLocation = parser.value(configurationFileOption);
    if (!QFile::exists(configurationFileLocation)) {
        qCCritical(DAEMON) << "Configuration file" << configurationFileLocation << "does not exist";
        return false;
    }
    qCInfo(DAEMON) << "Using configuration file" << configurationFileLocation;

    // Load configuration file.
    QSettings settings(parser.value(configurationFileOption), QSettings::IniFormat);
    if (settings.status() != QSettings::NoError) {
        qCritical(DAEMON) << "Configuration file" << configurationFileLocation << "is malformed";
        return false;
    }

    // Load driver search paths.
    settings.beginGroup("Gateway");
    auto driverSearchPaths = settings.value("driverSearchPaths", "/var/lib/sigatewayd/drivers").toString().split(" ");
    settings.endGroup();
    qCInfo(DAEMON) << "Driver search locations are" << driverSearchPaths;

    // Load Storage driver.
    settings.beginGroup("Storage");
    auto storageDriverName = settings.value("driver", "SQLite").toString();
    auto* storageDriver = SIStorageDriver::loadStorageDriver(driverSearchPaths, storageDriverName);
    if (storageDriver == nullptr) {
        qCritical() << "Unable to load storage driver" << storageDriverName;
        return false;
    }
    storage_.reset(storageDriver->createStorageInstance(filteredChildSettings_(settings, {"driver"})));
    if (storage_ == nullptr) {
        qCritical() << "Unable to instantiate storage driver" << storageDriverName;
        return false;
    }
    qCInfo(DAEMON) << "Successfully loaded storage driver" << storageDriverName;
    settings.endGroup();

    // Load and instantiate configured device access drivers.
    qCInfo(DAEMON) << "Loading device access drivers and instantiating objects...";
    for (const auto& group: filteredChildGroups_(settings, {"Gateway", "Storage", "WebSocket", "Bluetooth"})) {
        settings.beginGroup(group);
        auto deviceAccessDriverName = settings.value("driver").toString();
        if (!SIDeviceAccessRegistry::loadDeviceAccessDriver(driverSearchPaths, deviceAccessDriverName)) {
            qCritical() << "  - Error loading device access driver" << deviceAccessDriverName;
        } else {

        }if (!SIDeviceAccessRegistry::sharedRegistry().instantiateDeviceAccess(deviceAccessDriverName, group, filteredChildSettings_(settings, {"driver"}))) {
            qCritical() << "  - Error creating device access" << group;
        }
        qCInfo(DAEMON) << "  - Successfully loaded and instantiated" << group << "with driver" << deviceAccessDriverName;
        settings.endGroup();
    }

    // Enumerate all devices on startup.
    qCInfo(DAEMON) << "Enumerating all devices...";
    QElapsedTimer elapsedTimer;
    elapsedTimer.start();
    auto deviceCount = SIDeviceAccessRegistry::sharedRegistry().enumerateDevices();
    if (deviceCount >= 0) {
        qCInfo(DAEMON) << "  - Enumerated" << deviceCount << "devices in" << elapsedTimer.elapsed() << "ms.";
    } else {
        qCCritical(DAEMON) << "Failed to enumerate devices";
    }

    // Create property manager and start polling timer.
    int propertyPollInterval = settings.value("Gateway/propertyPollInterval", 60000).toInt();
    deviceAccessManager_ = new SISequentialPropertyManager(this);
    deviceAccessManager_->startPropertyPolling(propertyPollInterval);

    // Create web socket manager.
    settings.beginGroup("WebSocket");
    if (settings.value("enabled", false).toBool()) {
        webSocketManager_ = new SIWebSocketManager(deviceAccessManager_, this);
        if (!webSocketManager_->listen(settings.value("port", 1987).toUInt())) {
            qCCritical(DAEMON) << "Failed to start web socket listening";
            delete webSocketManager_;
        }
    }
    settings.endGroup();

    // Create bluetooth manager.
    settings.beginGroup("Bluetooth");
    if (settings.value("enabled", false).toBool()) {
        bluetoothManager_ = new SIBluetoothManager(deviceAccessManager_, this);
        bluetoothManager_->setPeripheralName(settings.value("name", "SIGateway").toString());
        bluetoothManager_->startAdvertise();
    }
    settings.endGroup();

    return true;
}

QVariantMap SIDaemon::filteredChildSettings_(const QSettings& settings, const QStringList& exclude) {
    QVariantMap filtered;
    for (const auto& key: settings.childKeys()) {
        if (!exclude.contains(key)) {
            filtered[key] = settings.value(key);
        }
    }
    return filtered;
}

QStringList SIDaemon::filteredChildGroups_(const QSettings& settings, const QStringList& exclude) {
    QStringList filtered;
    for (const auto& group: settings.childGroups()) {
        if (!exclude.contains(group)) {
            filtered << group;
        }
    }
    return filtered;
}
