#pragma once

#include <QObject>
#include <QString>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTimer>
#include <memory>
#include <string>
#include "wallet/api/wallet2_api.h"

// Forward declarations
namespace zmq {
    class context_t;
    class socket_t;
}

namespace qsf {

// Use NetworkType from wallet API

class ZmqRpcClient : public QObject
{
    Q_OBJECT

public:
    explicit ZmqRpcClient(QObject *parent = nullptr);
    ~ZmqRpcClient();

    // Connection management
    bool connect(const QString &address, uint16_t port);
    bool connect(qsf::NetworkType networkType);
    bool connectToAny(const QStringList &hosts, uint16_t port);
    bool connectUsingConfigured(const QStringList &zmqEndpoints, uint16_t defaultPort);
    bool connectUri(const QString &uri);
    void disconnect();
    bool isConnected() const;

    // RPC methods
    QJsonObject callMethod(const QString &method, const QJsonObject &params = QJsonObject());
    
    // Mining-specific methods
    QJsonObject getMiningStatus();
    QJsonObject startMining(const QString &address, uint32_t threads = 1, bool background = false);
    QJsonObject stopMining();
    QJsonObject getInfo();

    // Error handling
    QString getLastError() const;
    void clearLastError();

signals:
    void connected();
    void disconnected();
    void error(const QString &error);

private:
    struct ZmqContext;
    std::unique_ptr<ZmqContext> m_context;
    QString m_lastError;
    bool m_connected;
    
    // Reconnection support
    QTimer* m_reconnectTimer;
    bool m_connectInProgress = false;
    QString m_lastAddress;
    uint16_t m_lastPort;
    int m_reconnectAttempts;
    static const int MAX_RECONNECT_ATTEMPTS = 5;

    // Helper methods
    QJsonObject sendRequest(const QJsonObject &request);
    QString formatZmqAddress(const QString &address, uint16_t port);
    uint16_t getZmqPort(qsf::NetworkType networkType);
    void scheduleReconnect();
    void attemptReconnect();

    // DNS seed helpers
    QStringList resolveSeedLabelTxt(const QString &seedLabel);
    QStringList expandEndpointToken(const QString &token, uint16_t defaultPort);
};

} // namespace qsf
