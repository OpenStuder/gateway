#pragma once
#include "xcom485idevice.h"

class XCom485iXtender: public XCom485iDevice {
  public:
    enum Model {
        Invalid,
        Multicast,
        XTH,
        XTM,
        XTS
    };

    explicit XCom485iXtender(Model model, quint8 modbusAddress, XCom485iModbusAccess* modbusAccess);

    static Model model(quint8 modbusAddress, XCom485iModbusAccess& access);

    static QString toString(Model model);
};
