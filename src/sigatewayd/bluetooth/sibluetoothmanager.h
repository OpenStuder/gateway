#pragma once
#include <QObject>
#include "siabstractbluetoothprotocol.h"
#include "../deviceaccess/sideviceaccessmanager.h"
#include <siuserauthorizer.h>

class QLowEnergyController;
class SIDeviceAccessManager;
class QLowEnergyCharacteristic;
class QLowEnergyService;

  class SIBluetoothManager: public QObject {
  Q_OBJECT

  public:
    explicit SIBluetoothManager(SIDeviceAccessManager* deviceAccessManager, SIUserAuthorizer* userAuthorizer, QObject* parent = nullptr);
    ~SIBluetoothManager() override;

      const QString& peripheralName() const {
          return peripheralName_;
      }

      void setPeripheralName(const QString& name) {
          peripheralName_ = name;
      }

  public slots:
    void startAdvertise();

  private slots:
    void onCharacteristicChanged_(const QLowEnergyCharacteristic& characteristic, const QByteArray& value);
    void onDisconnected_();
    void onDeviceMessageReceived_(const QString& deviceAccessID, const SIDeviceMessage& message);

  private:
    void sendFrame_(const SIBluetoothProtocolFrame& frame);

    QString peripheralName_;
    QLowEnergyController* peripheral_;
    QLowEnergyService* service_;

    SIAbstractBluetoothProtocol* protocol_ = nullptr;

    SIDeviceAccessManager* deviceAccessManager_;
    SIUserAuthorizer* userAuthorizer_;
};