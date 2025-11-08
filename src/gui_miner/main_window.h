// Copyright (c) 2024, QuantumSafeFoundation
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

#include <QMainWindow>
#include <QTimer>
#include <QThread>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <memory>
#include "mining_worker.h"
#include "quantum_safe_widget.h"
#include "zmq_rpc_client.h"
#include "wallet/api/wallet2_api.h"

QT_BEGIN_NAMESPACE
class QLabel;
class QLineEdit;
class QPushButton;
class QComboBox;
class QSpinBox;
class QTextEdit;
class QProgressBar;
class QGroupBox;
class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QTabWidget;
class QWidget;
class QTextBrowser;
class QProcess;
QT_END_NAMESPACE

namespace qsf
{
  // Use the NetworkType from wallet API
  class GuiWalletManager;

  struct NetworkConfig
  {
    QString name;
    QString daemonUrl;
    QString poolUrl;
    uint16_t rpcPort;
    uint16_t p2pPort;
    QStringList seedNodes;  // List of seed node addresses for each network
  };

  // Forward declaration
  class WalletManager;

  class MainWindow : public QMainWindow
  {
    Q_OBJECT

  public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

  private slots:
    void onStartDaemon();
    void onStopDaemon();
    void onStartMining();
    void onStopMining();
    void onOpenWallet();
    void onGenerateWallet();
    void onRecoverWallet();
    void onNetworkChanged(int index);
    void onMiningModeChanged(const QString& mode);
    void onMiningUpdate(double hashRate, uint64_t acceptedShares, uint64_t rejectedShares);
    void onMiningError(const QString& error);
    void onUpdateStatistics();
    void onUpdateMiningStatus();
    void onCheckServerStatus();
    bool safeZmqConnect(const QString& address, uint16_t port);
    void onServerStatusReply(QNetworkReply* reply);
    void onDaemonStatusChanged(bool running);
    void onQuickSend();
    void onQuickReceive();
    void onRescanWallet();
    
    // Wallet Manager slots
    void onWalletOpened(const QString& address);
    void onWalletClosed();
    void onBalanceUpdated(const QString& balance);
    void onWalletError(const QString& error);

  signals:
    void daemonCrashed();

  private:
    void setupUI();
    void setupSettingsTab();
    void connectSignals();
    void updateMiningStatus(bool mining);
    void loadSettings();
    void saveSettings();
    void checkDaemonStatus();
    void checkAlternativeLocalPorts();
    void startMiningWithDaemon(const QString& daemonUrl = QString());
    void startMiningStatusMonitoring();
    void stopMiningStatusMonitoring();
    void updatePeerCount();
    void updatePeerCountHttp();
    void updateDaemonStatus(bool running);
    void checkLocalDaemonReady();
    bool detectAndHandleExistingDaemon();
    bool tryConnectToExistingDaemon();
    bool autoStartLocalDaemon();
    bool tryMiningOnDaemon(const QString& daemonUrl);
    void ensureLocalConfigExists();
    QString generateDefaultConfig();
    QString generateMinerGuiConfig();
    void connectToRemoteDaemon();
    void setupNetworkConfigs();
    void loadMinerConfigFromFile();
    void applyMinerConfigToUi();
    void setupHeader();
    void setupOverviewTab();
    void setupMiningTab();
    void setupQuantumSafeTab();
    void updateNetworkConfig();
    void generateNewWallet();
    QString getNetworkDaemonUrl() const;
    QString getNetworkPoolUrl() const;
    void onCopyAddress();
    void onShowPrivateKey();
    void updateWalletBalance();
    void refreshWalletBalance();
    void showTransactionHistory();
    void sweepAllBalance();
    void createNewSubaddress();

    // New stand-alone mining methods
    bool startStandaloneMining();
    void stopStandaloneMining();
    bool findFreePorts(int& rpcPort, int& zmqPort, int& p2pPort);
    bool startLocalDaemonWithMining();
    bool startLocalDaemonWithoutMining();
    void restartDaemonWithMining();
    void restartDaemonWithoutMining();
    bool isPortAvailable(int port);
    QString generateLocalConfig();
    void updateMiningControls();
    void handleDaemonOutput();
    void handleDaemonError(int error);
    void handleDaemonFinished(int exitCode, int exitStatus);
    bool checkDaemonMiningSupport(const QString& daemonUrl);
    void startDirectMining();
    void stopDirectMining();

    // UI Components
    QWidget* m_centralWidget;
    QVBoxLayout* m_mainLayout;
    QTabWidget* m_tabWidget;
    
    // Header
    QLabel* m_statusLabel;
    
    // Overview Tab
    QLabel* m_balanceLabel;
    QLabel* m_walletStatusLabel;
    QLabel* m_hashrateLabel;
    QLabel* m_unlockedBalanceLabel;
    QLabel* m_lockedBalanceLabel;
    QLabel* m_connectionLabel;
    QLabel* m_blockHeightLabel;
    QLabel* m_networkHashrateLabel;
    QLabel* m_peerCountLabel;
    QPushButton* m_generateWalletBtn;
    QPushButton* m_recoverWalletBtn;
    QPushButton* m_copyAddressBtn;
    QPushButton* m_showHistoryBtn;
    QPushButton* m_rescanSpentBtn;
    QPushButton* m_sweepAllBtn;
    QPushButton* m_createSubaddressBtn;
    QPushButton* m_showPrivateKeyBtn;
    QPushButton* m_rescanWalletBtn;
    QTextBrowser* m_walletAddressDisplay;
    QLabel* m_networkLabel;
    
    // Network Configuration
    QComboBox* m_networkCombo;
    qsf::NetworkType m_currentNetwork;
    QMap<qsf::NetworkType, NetworkConfig> m_networkConfigs;
    
    // Mining Configuration
    QComboBox* m_miningModeCombo;
    QLineEdit* m_poolAddressEdit;
    QLineEdit* m_daemonUrlEdit;
    QLineEdit* m_walletAddressEdit;
    QSpinBox* m_threadsSpinBox;
    QPushButton* m_startMiningBtn;
    QPushButton* m_stopMiningBtn;
    QLabel* m_poolAddressLabel;
    
    // Daemon Management
    QPushButton* m_startDaemonBtn;
    QPushButton* m_stopDaemonBtn;
    QLabel* m_daemonStatusLabel;
    QProcess* m_daemonProcess;
    
    // Quantum-Safe Keys
    QTextEdit* m_generatedKeysDisplay;
    QLabel* m_quantumKeysStatusLabel;
    
    // Mining Statistics
    QLabel* m_hashRateLabel;
    QLabel* m_acceptedSharesLabel;
    QLabel* m_rejectedSharesLabel;
    QLabel* m_uptimeLabel;
    QLabel* m_difficultyLabel;
    QLabel* m_blockRewardLabel;
    
    // Mining statistics tracking
    uint64_t m_realAcceptedShares;
    uint64_t m_realRejectedShares;
    uint64_t m_daemonMiningStartTime;
    uint64_t m_lastBlockHeight;
    double m_currentBlockReward;
    
    // Mining Log
    QTextEdit* m_miningLog;
    
    // Mining Worker
    std::unique_ptr<MiningWorker> m_miningWorker;
    QThread* m_miningThread;
    
    // ZMQ RPC Client
    std::unique_ptr<ZmqRpcClient> m_zmqClient;
    bool m_zmqConnecting;
    
    // Wallet Manager (QSF GUI approach)
    GuiWalletManager* m_walletManager;
    
    // Update Timer
    QTimer* m_updateTimer;
    
    // Network Manager for server status checking
    QNetworkAccessManager* m_networkManager;
    QTimer* m_serverStatusTimer;
    
    // Wallet State
    QString m_walletPrivateKey;
    bool m_hasWallet;
    QString m_walletPassword;
    
    // State
    bool m_isMining;
    double m_currentHashRate;

    // Daemon and mining state
    bool m_daemonRunning;
    bool m_miningActive;
    QProcess* m_localDaemonProcess;
    QString m_daemonUrl;
    QString m_walletAddress;
    QString m_miningWalletAddress;
    int m_miningThreads;
    qint64 m_startTime;
    QTimer* m_miningStatusTimer;
    QTimer* m_peerCountTimer;

    // Miner config from file
    QStringList m_customZmqEndpoints;
    int m_configuredThreads = 0; // 0 means auto
    QString m_configuredDaemonUrl; // optional override for daemon URL

    // New stand-alone mining state
    int m_localRpcPort;
    int m_localZmqPort;
    int m_localP2pPort;
    QString m_localConfigPath;
    bool m_standaloneMode;
    bool m_daemonSupportsMiningRpc;
    QTimer* m_daemonHealthTimer;
    QString m_daemonPath;
    
    // Daemon state management
    bool m_daemonStartInProgress;
    int m_daemonRetryCount;
    
    // Wallet process management
    QProcess* m_activeWalletProcess;
    QTimer* m_walletCooldownTimer;
    
  private:
    // Helper methods for mining statistics
    double calculateCurrentBlockReward(uint64_t height, uint64_t alreadyGeneratedCoins);
    void updateMiningStatistics();
    void resetMiningStatistics();
    void unblockLocalhost();
  };
} 