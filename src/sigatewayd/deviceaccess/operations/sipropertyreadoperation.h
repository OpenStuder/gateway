#pragma once
#include "siabstractoperation.h"
#include <siglobalpropertyid.h>
#include <sistatus.h>
#include <QVariant>

class SIPropertyReadOperation final: public SIAbstractOperation {
  Q_OBJECT

  public:
    explicit SIPropertyReadOperation(SIGlobalPropertyID id, QObject* parent = nullptr);
    SIPropertyReadOperation(const SIPropertyReadOperation&) = delete;
    SIPropertyReadOperation& operator =(const SIPropertyReadOperation&) = delete;

    inline const SIGlobalPropertyID& id() const {
        return id_;
    }

    inline const QVariant& value() const {
        return value_;
    }

    inline SIStatus status() const {
        return status_;
    }

  private:
    SIStatus execute_(SIDeviceAccessRegistry* deviceAccessRegistry) override;

    SIGlobalPropertyID id_;
    QVariant value_;
    SIStatus status_ = SIStatus::InProgress;
};
