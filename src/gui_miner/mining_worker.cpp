// Copyright (c) 2024, The QSF Quantum-Safe Coin Project
//
// All rights reserved.
//
// See license header in headers.

#include "mining_worker.h"
#include "zmq_rpc_client.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>
#include <QUrl>
#include <QEventLoop>
#include <QDateTime>
#include <QCoreApplication>
#include <QDebug>

#include "cryptonote_config.h"
#include "main_window.h" // For NetworkType enum

namespace qsf
{
  namespace
  {
    static QString rpcBaseUrl(qsf::NetworkType networkType = qsf::MAINNET)
    {
      // Use localhost with correct RPC port based on network type
      uint16_t port;
      switch (networkType) {
        case qsf::TESTNET:
          port = ::config::testnet::RPC_DEFAULT_PORT; // 28071
          break;
        case qsf::STAGENET:
          port = ::config::stagenet::RPC_DEFAULT_PORT; // 38071
          break;
        case qsf::MAINNET:
        default:
          port = ::config::RPC_DEFAULT_PORT; // 18071
          break;
      }
      return QString("http://127.0.0.1:%1").arg(port);
    }
  }

  MiningWorker::MiningWorker(QObject *parent)
    : QObject(parent)
    , m_mining(false)
    , m_miningTimer(new QTimer(this))
    , m_zmqClient(std::make_unique<ZmqRpcClient>(this))
    , m_currentHashRate(0.0)
    , m_sharesSubmitted(0)
  {
    // Connect ZMQ client signals
    connect(m_zmqClient.get(), &ZmqRpcClient::connected, this, &MiningWorker::onZmqConnected);
    connect(m_zmqClient.get(), &ZmqRpcClient::disconnected, this, &MiningWorker::onZmqDisconnected);
    connect(m_zmqClient.get(), &ZmqRpcClient::error, this, &MiningWorker::onZmqError);
    
    // Connect mining timer
    connect(m_miningTimer, &QTimer::timeout, this, &MiningWorker::onMiningTimer);
    m_miningTimer->setInterval(1000); // Update every second
  }

  MiningWorker::~MiningWorker()
  {
    stopMining();
  }

  void MiningWorker::setConfig(const MiningConfig &config)
  {
    QMutexLocker locker(&m_mutex);
    m_config = config;
    
    // Connect to ZMQ RPC using the network type from config
    if (m_zmqClient->isConnected()) {
      m_zmqClient->disconnect();
    }
    
    if (!m_zmqClient->connect(config.networkType)) {
      qDebug() << "Failed to connect to ZMQ RPC:" << m_zmqClient->getLastError();
    }
  }

  void MiningWorker::setDaemonUrl(const QString &daemonUrl)
  {
    QMutexLocker locker(&m_mutex);
    m_config.daemonUrl = daemonUrl.toStdString();
  }

  void MiningWorker::setWalletAddress(const QString &walletAddress)
  {
    QMutexLocker locker(&m_mutex);
    m_config.walletAddress = walletAddress.toStdString();
  }

  void MiningWorker::setThreads(uint32_t threads)
  {
    QMutexLocker locker(&m_mutex);
    m_config.threads = threads;
  }

  void MiningWorker::startMining()
  {
    QMutexLocker locker(&m_mutex);
    
    if (m_mining.load()) {
      qDebug() << "Mining already in progress";
      return;
    }
    
    // Start mining via HTTP JSON-RPC (daemon RPC), not ZMQ
    QString daemonUrl = QString::fromStdString(m_config.daemonUrl);
    if (daemonUrl.isEmpty()) daemonUrl = rpcBaseUrl(m_config.networkType);
    if (!daemonUrl.endsWith("/json_rpc")) daemonUrl += "/json_rpc";
    
    QString walletAddress = QString::fromStdString(m_config.walletAddress);
    if (walletAddress.isEmpty()) {
      QString error = "No wallet address specified for mining";
      qDebug() << error;
      emit this->error(error);
      return;
    }
    
    qDebug() << "Starting mining via HTTP RPC...";
    qDebug() << "Wallet address:" << walletAddress;
    qDebug() << "Threads:" << m_config.threads;
    // Compose HTTP RPC request to /start_mining
    QNetworkAccessManager nam;
    if (daemonUrl.endsWith("/json_rpc")) {
      daemonUrl.chop(QString("/json_rpc").size());
    }
    if (!daemonUrl.endsWith("/start_mining")) {
      if (!daemonUrl.endsWith('/')) daemonUrl += '/';
      daemonUrl += "start_mining";
    }
    QNetworkRequest req{QUrl(daemonUrl)};
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QJsonObject params;
    params["miner_address"] = walletAddress;
    params["threads_count"] = static_cast<int>(m_config.threads);
    params["do_background_mining"] = false;
    params["ignore_battery"] = false;
    QJsonDocument doc(params);
    QEventLoop loop;
    QNetworkReply* reply = nam.post(req, doc.toJson(QJsonDocument::Compact));
    QObject::connect(&nam, &QNetworkAccessManager::finished, &loop, &QEventLoop::quit);
    QTimer::singleShot(5000, &loop, &QEventLoop::quit);
    loop.exec();
    if (!reply || reply->error() != QNetworkReply::NoError) {
      QString err = reply ? reply->errorString() : QString("RPC request failed");
      emit this->error(QString("Failed to start mining: %1").arg(err));
      if (reply) reply->deleteLater();
      return;
    }
    const QByteArray body = reply->readAll();
    reply->deleteLater();
    QJsonParseError perr{};
    QJsonDocument rdoc = QJsonDocument::fromJson(body, &perr);
    if (perr.error != QJsonParseError::NoError || rdoc.isNull()) {
      emit this->error("Invalid JSON response from daemon");
      return;
    }
    const QJsonObject result = rdoc.object();
    if (result.contains("error") && !result["error"].isNull()) {
      const auto e = result["error"].toObject();
      emit this->error(QString("Failed to start mining: %1").arg(e.value("message").toString()));
      return;
    }
    m_mining.store(true);
    m_miningTimer->start();
    m_sharesSubmitted = 0;
    emit miningStarted();
  }

  void MiningWorker::stopMining()
  {
    QMutexLocker locker(&m_mutex);
    
    if (!m_mining.load()) {
      return;
    }
    
    m_miningTimer->stop();
    
    // Stop mining via HTTP RPC /stop_mining
    QString daemonUrl = QString::fromStdString(m_config.daemonUrl);
    if (daemonUrl.isEmpty()) daemonUrl = rpcBaseUrl(m_config.networkType);
    if (daemonUrl.endsWith("/json_rpc")) {
      daemonUrl.chop(QString("/json_rpc").size());
    }
    if (!daemonUrl.endsWith("/stop_mining")) {
      if (!daemonUrl.endsWith('/')) daemonUrl += '/';
      daemonUrl += "stop_mining";
    }
    QNetworkAccessManager nam;
    QNetworkRequest req{QUrl(daemonUrl)};
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QJsonObject rpc; // empty body is acceptable, but send empty JSON
    QEventLoop loop;
    QNetworkReply* reply = nam.post(req, QJsonDocument(rpc).toJson(QJsonDocument::Compact));
    QObject::connect(&nam, &QNetworkAccessManager::finished, &loop, &QEventLoop::quit);
    QTimer::singleShot(5000, &loop, &QEventLoop::quit);
    loop.exec();
    if (reply) reply->deleteLater();
    
    m_mining.store(false);
    qDebug() << "Mining stopped";
    emit miningStopped();
  }

  bool MiningWorker::isMining() const
  {
    return m_mining.load();
  }

  void MiningWorker::onMiningTimer()
  {
    if (!m_mining.load()) {
      return;
    }
    
    updateMiningStatus();
  }

  void MiningWorker::onZmqConnected()
  {
    qDebug() << "Connected to ZMQ RPC server";
  }

  void MiningWorker::onZmqDisconnected()
  {
    qDebug() << "Disconnected from ZMQ RPC server";
    if (m_mining.load()) {
      emit error("Lost connection to ZMQ RPC server");
      stopMining();
    }
  }

  void MiningWorker::onZmqError(const QString &error)
  {
    qDebug() << "ZMQ RPC error:" << error;
    emit this->error(error);
  }

  void MiningWorker::updateMiningStatus()
  {
    // Query mining status via HTTP RPC /mining_status
    QString daemonUrl = QString::fromStdString(m_config.daemonUrl);
    if (daemonUrl.isEmpty()) daemonUrl = rpcBaseUrl(m_config.networkType);
    if (daemonUrl.endsWith("/json_rpc")) {
      daemonUrl.chop(QString("/json_rpc").size());
    }
    if (!daemonUrl.endsWith("/mining_status")) {
      if (!daemonUrl.endsWith('/')) daemonUrl += '/';
      daemonUrl += "mining_status";
    }
    QNetworkAccessManager nam;
    QNetworkRequest req{QUrl(daemonUrl)};
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QJsonObject rpc; // empty JSON body
    QEventLoop loop;
    QNetworkReply* reply = nam.post(req, QJsonDocument(rpc).toJson(QJsonDocument::Compact));
    QObject::connect(&nam, &QNetworkAccessManager::finished, &loop, &QEventLoop::quit);
    QTimer::singleShot(5000, &loop, &QEventLoop::quit);
    loop.exec();
    if (!reply || reply->error() != QNetworkReply::NoError) {
      QString err = reply ? reply->errorString() : QString("RPC request failed");
      qDebug() << "mining_status error:" << err;
      emit hashRateUpdated(0.0);
      if (reply) reply->deleteLater();
      return;
    }
    const QByteArray body = reply->readAll();
    reply->deleteLater();
    QJsonDocument rdoc = QJsonDocument::fromJson(body);
    if (rdoc.isNull()) return;
    QJsonObject result = rdoc.object();
    
    if (result.contains("error") && !result["error"].isNull()) {
      qDebug() << "Error getting mining status:" << result["error"].toObject()["message"].toString();
      return;
    }
    
    // Support both JSON-RPC-wrapped and direct HTTP RPC responses
    QJsonObject status;
    if (result.contains("result") && result.value("result").isObject()) {
      status = result.value("result").toObject();
    } else {
      status = result;
    }
    if (!status.isEmpty()) {
      
      if (status.contains("active")) {
        bool active = status["active"].toBool();
        if (!active && m_mining.load()) {
          qDebug() << "Mining stopped by daemon";
          stopMining();
          return;
        }
      }

      if (status.contains("speed")) {
        double speed = status["speed"].toDouble();
        if (speed != m_currentHashRate) {
          m_currentHashRate = speed;
          emit hashRateUpdated(speed);
          qDebug() << "Hash rate updated:" << speed << "H/s";
        }
      } else {
        // If no speed in response, emit a default hash rate to show activity
        emit hashRateUpdated(0.0);
      }
      
      if (status.contains("address")) {
        QString address = status["address"].toString();
        qDebug() << "Mining to address:" << address;
      }
    }
  }

  void MiningWorker::submitShare()
  {
    // This would be called when a share is found
    // For now, we'll just increment the counter
    m_sharesSubmitted++;
    emit sharesSubmitted(m_sharesSubmitted);
  }

} // namespace qsf 