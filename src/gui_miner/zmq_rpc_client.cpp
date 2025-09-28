#include "zmq_rpc_client.h"
#include "main_window.h" // For NetworkType enum
#include "cryptonote_config.h"
#include "common/dns_utils.h"

#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTimer>
#include <QProcessEnvironment>

// ZMQ includes
#include <zmq.h>

namespace qsf {

struct ZmqRpcClient::ZmqContext {
    void* context;
    void* socket;
    
    ZmqContext() : context(zmq_ctx_new()), socket(nullptr) {}
    ~ZmqContext() {
        if (socket) {
            zmq_close(socket);
        }
        if (context) {
            zmq_ctx_destroy(context);
        }
    }
};

ZmqRpcClient::ZmqRpcClient(QObject *parent)
    : QObject(parent)
    , m_context(std::make_unique<ZmqContext>())
    , m_connected(false)
{
}

ZmqRpcClient::~ZmqRpcClient()
{
    disconnect();
}

bool ZmqRpcClient::connect(const QString &address, uint16_t port)
{
    try {
        disconnect();
        
        QString zmqAddress = formatZmqAddress(address, port);
        qDebug() << "Connecting to ZMQ RPC at:" << zmqAddress;
        
        m_context->socket = zmq_socket(m_context->context, ZMQ_REQ);
        if (!m_context->socket) {
            m_lastError = QString("Failed to create ZMQ socket: %1").arg(zmq_strerror(zmq_errno()));
            qDebug() << "ZMQ socket creation failed:" << m_lastError;
            emit error(m_lastError);
            return false;
        }
        
        // Set socket options
        int timeout = 5000; // 5 seconds
        zmq_setsockopt(m_context->socket, ZMQ_RCVTIMEO, &timeout, sizeof(timeout));
        zmq_setsockopt(m_context->socket, ZMQ_SNDTIMEO, &timeout, sizeof(timeout));
        
        // Connect to the server
        int result = zmq_connect(m_context->socket, zmqAddress.toStdString().c_str());
        if (result != 0) {
            m_lastError = QString("ZMQ connection error: %1").arg(zmq_strerror(zmq_errno()));
            qDebug() << "ZMQ connection failed:" << m_lastError;
            emit error(m_lastError);
            zmq_close(m_context->socket);
            m_context->socket = nullptr;
            return false;
        }
        
        m_connected = true;
        clearLastError();
        
        qDebug() << "Successfully connected to ZMQ RPC";
        emit connected();
        return true;
        
    } catch (const std::exception &e) {
        m_lastError = QString("Connection error: %1").arg(e.what());
        qDebug() << "Connection failed:" << m_lastError;
        emit error(m_lastError);
        return false;
    }
}

bool ZmqRpcClient::connect(qsf::NetworkType networkType)
{
    uint16_t port = getZmqPort(networkType);
    // Prefer DNS seed list by default; allow override via env vars
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    QString host = env.value("QSF_ZMQ_HOST");
    QString portStr = env.value("QSF_ZMQ_PORT");
    if (!portStr.isEmpty()) {
        bool ok = false; int p = portStr.toInt(&ok);
        if (ok && p > 0 && p < 65536) port = static_cast<uint16_t>(p);
    }
    if (!host.isEmpty()) {
        return connect(host, port);
    }

    QStringList hosts;
    switch (networkType) {
        case qsf::TESTNET:
            hosts << "seeds.qsfnetwork.com";
            break;
        case qsf::STAGENET:
            hosts << "seeds.qsfcoin.network";
            break;
        case qsf::MAINNET:
        default:
            hosts << "seeds.qsfchain.com"
                  << "seed2.qsfchain.com"
                  << "seeds.qsfcoin.com"
                  << "seeds.qsfcoin.network"
                  << "seeds.qsfcoin.org"
                  << "seeds.qsfnetwork.co";
            break;
    }
    return connectToAny(hosts, port);
}

void ZmqRpcClient::disconnect()
{
    if (m_context->socket) {
        zmq_close(m_context->socket);
        m_context->socket = nullptr;
    }
    
    if (m_connected) {
        m_connected = false;
        emit disconnected();
    }
}

bool ZmqRpcClient::isConnected() const
{
    return m_connected && m_context->socket;
}

QJsonObject ZmqRpcClient::callMethod(const QString &method, const QJsonObject &params)
{
    if (!isConnected()) {
        m_lastError = "Not connected to ZMQ RPC server";
        emit this->error(m_lastError);
        return QJsonObject();
    }
    
    QJsonObject request;
    request["jsonrpc"] = "2.0";
    request["id"] = "0";
    request["method"] = method;
    request["params"] = params;
    
    return sendRequest(request);
}

QJsonObject ZmqRpcClient::getMiningStatus()
{
    return callMethod("mining_status");
}

QJsonObject ZmqRpcClient::startMining(const QString &address, uint32_t threads, bool background)
{
    QJsonObject params;
    params["miner_address"] = address;
    params["threads_count"] = static_cast<int>(threads);
    params["do_background_mining"] = background;
    params["ignore_battery"] = false;
    // Allow mining on restricted RPC too
    params["restricted"] = false;
    
    return callMethod("start_mining", params);
}

QJsonObject ZmqRpcClient::stopMining()
{
    return callMethod("stop_mining");
}

QJsonObject ZmqRpcClient::getInfo()
{
    return callMethod("get_info");
}

QString ZmqRpcClient::getLastError() const
{
    return m_lastError;
}

void ZmqRpcClient::clearLastError()
{
    m_lastError.clear();
}

QJsonObject ZmqRpcClient::sendRequest(const QJsonObject &request)
{
    try {
        QJsonDocument doc(request);
        QString jsonStr = doc.toJson(QJsonDocument::Compact);
        
        qDebug() << "Sending ZMQ request:" << jsonStr;
        
        // Send request
        zmq_msg_t requestMsg;
        zmq_msg_init_size(&requestMsg, jsonStr.length());
        memcpy(zmq_msg_data(&requestMsg), jsonStr.toStdString().c_str(), jsonStr.length());
        
        int sendResult = zmq_msg_send(&requestMsg, m_context->socket, 0);
        zmq_msg_close(&requestMsg);
        
        if (sendResult == -1) {
            m_lastError = QString("Failed to send ZMQ message: %1").arg(zmq_strerror(zmq_errno()));
            emit this->error(m_lastError);
            return QJsonObject();
        }
        
        // Receive response
        zmq_msg_t responseMsg;
        zmq_msg_init(&responseMsg);
        
        int recvResult = zmq_msg_recv(&responseMsg, m_context->socket, 0);
        
        if (recvResult == -1) {
            m_lastError = QString("Failed to receive ZMQ message: %1").arg(zmq_strerror(zmq_errno()));
            zmq_msg_close(&responseMsg);
            emit this->error(m_lastError);
            return QJsonObject();
        }
        
        QString responseStr = QString::fromUtf8(static_cast<const char*>(zmq_msg_data(&responseMsg)), zmq_msg_size(&responseMsg));
        zmq_msg_close(&responseMsg);
        
        qDebug() << "Received ZMQ response:" << responseStr;
        
        QJsonDocument responseDoc = QJsonDocument::fromJson(responseStr.toUtf8());
        if (responseDoc.isNull()) {
            m_lastError = "Invalid JSON response from ZMQ RPC server";
            emit this->error(m_lastError);
            return QJsonObject();
        }
        
        QJsonObject response = responseDoc.object();
        
        // Check for error
        if (response.contains("error") && !response["error"].isNull()) {
            QJsonObject error = response["error"].toObject();
            m_lastError = QString("RPC error: %1").arg(error["message"].toString());
            emit this->error(m_lastError);
        }
        
        // Return the JSON-RPC result object when present, since GUI code expects that shape
        if (response.contains("result") && response["result"].isObject()) {
            return response["result"].toObject();
        }
        return response;
        
    } catch (const std::exception &e) {
        m_lastError = QString("Communication error: %1").arg(e.what());
        qDebug() << "Communication failed:" << m_lastError;
        emit this->error(m_lastError);
        return QJsonObject();
    }
}

QString ZmqRpcClient::formatZmqAddress(const QString &address, uint16_t port)
{
    return QString("tcp://%1:%2").arg(address).arg(port);
}

uint16_t ZmqRpcClient::getZmqPort(qsf::NetworkType networkType)
{
    switch (networkType) {
        case qsf::TESTNET:
            return ::config::testnet::ZMQ_RPC_DEFAULT_PORT; // 28072
        case qsf::STAGENET:
            return ::config::stagenet::ZMQ_RPC_DEFAULT_PORT; // 38072
        case qsf::MAINNET:
        default:
            return ::config::ZMQ_RPC_DEFAULT_PORT; // 18072
    }
}

bool ZmqRpcClient::connectUri(const QString &uri)
{
    try {
        disconnect();
        m_context->socket = zmq_socket(m_context->context, ZMQ_REQ);
        if (!m_context->socket) {
            m_lastError = QString("Failed to create ZMQ socket: %1").arg(zmq_strerror(zmq_errno()));
            emit error(m_lastError);
            return false;
        }
        int timeout = 5000;
        zmq_setsockopt(m_context->socket, ZMQ_RCVTIMEO, &timeout, sizeof(timeout));
        zmq_setsockopt(m_context->socket, ZMQ_SNDTIMEO, &timeout, sizeof(timeout));
        const QByteArray u = uri.toUtf8();
        if (zmq_connect(m_context->socket, u.constData()) != 0) {
            m_lastError = QString("ZMQ connection error: %1").arg(zmq_strerror(zmq_errno()));
            emit error(m_lastError);
            zmq_close(m_context->socket);
            m_context->socket = nullptr;
            return false;
        }
        m_connected = true;
        clearLastError();
        emit connected();
        return true;
    } catch (const std::exception &e) {
        m_lastError = QString("Connection error: %1").arg(e.what());
        emit error(m_lastError);
        return false;
    }
}

bool ZmqRpcClient::connectToAny(const QStringList &hosts, uint16_t port)
{
    // Try each endpoint until one succeeds
    for (const QString &h : hosts) {
        if (connect(h, port)) {
            return true;
        }
    }
    // Final fallback: try localhost (in case user runs local daemon with public ZMQ)
    connect("127.0.0.1", port);
    return isConnected();
}

bool ZmqRpcClient::connectUsingConfigured(const QStringList &zmqEndpoints, uint16_t defaultPort)
{
    if (!zmqEndpoints.isEmpty()) {
        // Each endpoint can be host or host:port
        QStringList expanded;
        QStringList seeds;
        for (const QString &e : zmqEndpoints) {
            if (e.startsWith("_seed._tcp.")) { seeds << e; continue; }
            for (const QString &token : expandEndpointToken(e, defaultPort)) {
                const int idx = token.lastIndexOf(':');
                if (idx > 0 && idx < token.size()-1) {
                    bool ok=false; const int p = token.mid(idx+1).toInt(&ok);
                    const QString host = token.left(idx);
                    if (ok && p>0 && p<65536) {
                        if (connect(host, static_cast<uint16_t>(p))) return true;
                    } else {
                        if (connect(host, defaultPort)) return true;
                    }
                } else {
                    if (connect(token, defaultPort)) return true;
                }
            }
        }
        // If none of the explicit hosts worked, resolve seeds (blocking) last, off the UI thread recommended; here as fallback
        for (const QString &seed : seeds) {
            for (const QString &token : expandEndpointToken(seed, defaultPort)) {
                const int idx = token.lastIndexOf(':');
                if (idx > 0 && idx < token.size()-1) {
                    bool ok=false; const int p = token.mid(idx+1).toInt(&ok);
                    const QString host = token.left(idx);
                    if (ok && p>0 && p<65536) {
                        if (connect(host, static_cast<uint16_t>(p))) return true;
                    } else {
                        if (connect(host, defaultPort)) return true;
                    }
                } else {
                    if (connect(token, defaultPort)) return true;
                }
            }
        }
        return false;
    }
    return false;
}

QStringList ZmqRpcClient::resolveSeedLabelTxt(const QString &seedLabel)
{
    QStringList out;
    try {
        bool avail=false, valid=false;
        std::vector<std::string> recs = tools::DNSResolver::instance().get_txt_record(seedLabel.toStdString(), avail, valid);
        // Be lenient: accept TXT records even if DNSSEC is not available/valid
        for (const auto &s : recs) {
            QString q = QString::fromStdString(s);
            if (!q.isEmpty()) out << q;
        }
    } catch (...) {
    }
    // Fallback: if no TXT entries, try stripping the seed service prefix and use host:defaultPort later
    if (out.isEmpty() && seedLabel.startsWith("_seed._tcp.")) {
        const QString baseHost = seedLabel.mid(QString("_seed._tcp.").length());
        if (!baseHost.isEmpty()) out << baseHost;
    }
    return out;
}

QStringList ZmqRpcClient::expandEndpointToken(const QString &token, uint16_t defaultPort)
{
    // Accept forms:
    //  - tcp://host:port
    //  - host:port
    //  - host
    //  - _seed._tcp.domain (TXT records of "ip:port" entries)
    if (token.startsWith("tcp://")) return QStringList{ token }; // already URI
    if (token.startsWith("_seed._tcp.")) {
        QStringList txt = resolveSeedLabelTxt(token);
        QStringList out;
        for (const QString &entry : txt) {
            // Expect "IP:port" or "host:port" in TXT. If only IP, append port.
            if (entry.contains(':')) {
                out << entry.trimmed();
            } else if (!entry.trimmed().isEmpty()) {
                out << QString("%1:%2").arg(entry.trimmed()).arg(defaultPort);
            }
        }
        return out;
    }
    // host or host:port
    return QStringList{ token };
}

} // namespace qsf
