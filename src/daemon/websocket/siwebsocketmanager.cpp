#include "siwebsocketmanager.h"
#include "siwebsocketconnection.h"
#include <QHostAddress>

SIWebSocketManager::SIWebSocketManager(SIDeviceAccessManager* deviceAccessManager, QObject* parent):
    QObject(parent), server_("SIGateway", QWebSocketServer::NonSecureMode), deviceAccessManager_(deviceAccessManager) {
    connect(&server_, &QWebSocketServer::newConnection, this, &SIWebSocketManager::onNewConnection_);
}

bool SIWebSocketManager::listen(quint16 port) {
    return server_.listen(QHostAddress::Any, port);
}

void SIWebSocketManager::onNewConnection_() {
    while (server_.hasPendingConnections()) {
        auto* webSocket = server_.nextPendingConnection();
        new SIWebSocketConnection(webSocket, deviceAccessManager_, this);
    }
}