#pragma once
#include "sipropertyid.h"
#include "sipropertytype.h"
#include "siaccesslevel.h"
#include "sipropertyflags.h"
#include "sijsonflags.h"
#include <QString>
#include <QJsonObject>
#include <utility>
#include <memory>

class SIProperty {
  public:
    SIProperty();
    SIProperty(SIPropertyID id, SIPropertyType type, SIAccessLevel accessLevel, SIPropertyFlags flags, const QString& description, const QString& unit);
    SIProperty(SIPropertyID id, SIPropertyType type, SIAccessLevel accessLevel, SIPropertyFlags flags, const QString& description, const std::initializer_list<std::pair<int, const char*>>& enumValues);

    SIPropertyID id() const;

    SIPropertyType type() const;

    SIAccessLevel accessLevel() const;

    SIPropertyFlags flags() const;
    bool isFlagSet(SIPropertyFlag flag) const;

    const QString& description() const;

    const QString& unit() const;

    QJsonObject jsonDescription(SIJsonFlags flags) const;

  private:
    struct Private;
    std::shared_ptr<Private> private_;
};
