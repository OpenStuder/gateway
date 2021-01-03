#include "sideviceaccess.h"
#include "sidevice.h"
#include <QJsonArray>

struct SIDeviceAccess::Private_ {
    QString id;
    mutable SIJsonFlags cachedJsonFlags = SIJsonFlag::None;
    mutable QJsonObject cachedJsonDescription;
};

SIDeviceAccess::SIDeviceAccess(const QString& id): priv_(new Private_) {
    priv_->id = id;
}

SIDeviceAccess::~SIDeviceAccess() = default;

const QString& SIDeviceAccess::id() const {
    return priv_->id;
}

QVector<SIDeviceMessage> SIDeviceAccess::retrievePendingDeviceMessages() const {
    QVector<SIDeviceMessage> messages;
    retrievePendingDeviceMessages_(messages);
    return messages;
}

int SIDeviceAccess::enumerateDevices() {

    // Clear JSON description cache,
    priv_->cachedJsonDescription = {};

    // Populate device list.
    QVector<SIDevice*> devices;
    for (const auto& child: children()) {
        devices.append(qobject_cast<SIDevice*>(child));
    }

    // Let the driver enumerate the devices.
    if (enumerateDevices_(devices)) {

        // Remove all devices from the devices list that are already present.
        // TODO: might not be needed, moving to the same parent again is no problem.
        for (const auto& child: children()) {
            devices.removeOne(qobject_cast<SIDevice*>(child));
        }

        // Set parent for all enumerated devices.
        for (auto* device: devices) {
            device->setParent(this);
        }

        // Return the number of children.
        return children().count();
    } else {

        // Return error.
        return -1;
    }
}

int SIDeviceAccess::deviceCount() const {
    return children().count();
}

QPointer<SIDevice> SIDeviceAccess::device(int index) const {
    if (index < children().count()) {
        return qobject_cast<SIDevice*>(children()[index]);
    } else {
        return nullptr;
    }
}

QPointer<SIDevice> SIDeviceAccess::device(const QString& id) const {
    for (auto* child: children()) {
        auto* device = qobject_cast<SIDevice*>(child);
        if (device->id() == id) {
            return device;
        }
    }
    return nullptr;
}

const QJsonObject& SIDeviceAccess::jsonDescription(SIJsonFlags flags) const {
    if (priv_->cachedJsonFlags != flags) {
        priv_->cachedJsonDescription = {};
    }
    priv_->cachedJsonFlags = flags;

    if (priv_->cachedJsonDescription.isEmpty()) {
        priv_->cachedJsonDescription["id"] = id();
        if (flags.testFlag(SIJsonFlag::IncludeAccessDetails)) {
            QJsonArray devs;
            for (auto* child: children()) {
                auto* device = qobject_cast<SIDevice*>(child);
                devs.append(device->jsonDescription(flags));
            }
            priv_->cachedJsonDescription["devices"] = devs;
            completeJsonDescription_(priv_->cachedJsonDescription, flags);
        }
    }
    return priv_->cachedJsonDescription;
}

void SIDeviceAccess::retrievePendingDeviceMessages_(QVector<SIDeviceMessage>& messages) const {
    Q_UNUSED(messages)
}

void SIDeviceAccess::completeJsonDescription_(QJsonObject& object, SIJsonFlags flags) const {
    Q_UNUSED(object)
    Q_UNUSED(flags)
}