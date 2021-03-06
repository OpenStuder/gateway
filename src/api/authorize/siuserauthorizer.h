#pragma once
#include <siaccesslevel.h>
#include <QString>
#include <QMap>

class SIUserAuthorizer {
  public:
    SIUserAuthorizer() = default;
    SIUserAuthorizer(const SIUserAuthorizer&) = delete;
    SIUserAuthorizer& operator =(const SIUserAuthorizer&) = delete;
    virtual ~SIUserAuthorizer() = default;

    inline SIAccessLevel authorizeUser(const QString& username, const QString& password) const {
        return authorizeUser_(username, password);
    }

  private:
    virtual SIAccessLevel authorizeUser_(const QString& username, const QString& password) const = 0;
};
