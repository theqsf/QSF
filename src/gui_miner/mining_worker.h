// Copyright (c) 2024, The QSF Quantum-Safe Coin Project
// 
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without modification, are
// permitted provided that the following conditions are met:
// 
// 1. Redistributions of source code must retain the above copyright notice, this list of
//    conditions and the following disclaimer.
// 
// 2. Redistributions in binary form must reproduce the above copyright notice, this list of
//    conditions and the following disclaimer in the documentation and/or other
//    materials provided with the distribution.
// 
// 3. Neither the name of the copyright holder nor the names of its contributors may be
//    used to endorse or promote products derived from this software without specific
//    prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
// THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
// STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
// THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#pragma once

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QTimer>
#include <QString>
#include <atomic>
#include <string>
#include <vector>
#include <memory>
#include "wallet/api/wallet2_api.h"

// Forward declarations
namespace qsf {
  // Use NetworkType from wallet API
  class ZmqRpcClient;
}

namespace qsf
{
  enum class MiningMode
  {
    POOL_MINING,
    SOLO_MINING
  };

  enum class MiningAlgorithm
  {
    RANDOMX,
    CRYPTONIGHT
  };

  struct MiningConfig
  {
    MiningMode mode;
    MiningAlgorithm algorithm;
    qsf::NetworkType networkType;  // Add network type
    std::string poolAddress;
    std::string daemonUrl;
    std::string walletAddress;
    uint32_t threads;
  };

  class MiningWorker : public QObject
  {
    Q_OBJECT

  public:
    explicit MiningWorker(QObject *parent = nullptr);
    ~MiningWorker();

    void setConfig(const MiningConfig &config);
    void setDaemonUrl(const QString &daemonUrl);
    void setWalletAddress(const QString &walletAddress);
    void setThreads(uint32_t threads);
    void startMining();
    void stopMining();
    bool isMining() const;

  signals:
    void miningStarted();
    void miningStopped();
    void hashRateUpdated(double hashRate);
    void sharesSubmitted(int shares);
    void error(const QString &error);

  private slots:
    void onMiningTimer();
    void onZmqConnected();
    void onZmqDisconnected();
    void onZmqError(const QString &error);

  private:
    MiningConfig m_config;
    std::atomic<bool> m_mining;
    QTimer *m_miningTimer;
    QMutex m_mutex;
    
    // ZMQ RPC client
    std::unique_ptr<ZmqRpcClient> m_zmqClient;
    
    // Mining statistics
    double m_currentHashRate;
    int m_sharesSubmitted;
    
    void updateMiningStatus();
    void submitShare();
  };

} // namespace qsf 