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

#include "main_window.h"
#include "mining_worker.h"
#include "quantum_safe_widget.h"
#include "wallet_manager.h"
#include <QApplication>
#include <QGroupBox>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QTextEdit>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QTimer>
#include <QSettings>
#include <QMessageBox>
#include <QFileDialog>
#include <QTime>
#include <QDate>
#include <QDateTime>
#include <QTabWidget>
#include <QFrame>
#include <QProgressBar>
#include <QStyle>
#include <QFont>
#include <QFontDatabase>
#include <QPixmap>
#include <QPainter>
#include <QIcon>
#include <QSplitter>
#include <QScrollArea>
#include <QTextBrowser>
#include <QClipboard>
#include <QInputDialog>
#include <QDialog>
#include <QCryptographicHash>
#include <QByteArray>
#include <QRandomGenerator>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QProcess>
#include <QDebug> // Added for qDebug
#include <QCoreApplication> // Added for QCoreApplication::applicationDirPath()
#include <QFile> // Added for QFile::exists()
#include <QFileInfo>
#include <QDir>
#include <QTextStream>
#include <QEventLoop>
#include <QStandardPaths>
#include <QShortcut>
#include <QTcpServer>
#include <QHostAddress>
#include <QRegExp>
#include <QProcess>

#include "wallet/api/wallet2_api.h"
#include "crypto/quantum_safe.h"
#include "cryptonote_config.h"
#include "QrCode.hpp"

namespace qsf
{

  MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_isMining(false)
    , m_currentHashRate(0.0)
    , m_realAcceptedShares(0)
    , m_realRejectedShares(0)
    , m_daemonMiningStartTime(0)
    , m_lastBlockHeight(0)
    , m_currentBlockReward(0.0)
    , m_hasWallet(false)
    , m_daemonRunning(false)
    , m_miningActive(false)
    , m_localDaemonProcess(nullptr)
    , m_daemonUrl("http://127.0.0.1:18071")
    , m_walletAddress("")
    , m_miningThreads(1)
    , m_startTime(0)
    , m_miningStatusTimer(nullptr)
    , m_peerCountTimer(nullptr)
    , m_configuredThreads(0)
    , m_configuredDaemonUrl("")
    , m_currentNetwork(qsf::MAINNET)
    , m_localRpcPort(38171)
    , m_localZmqPort(38172)
    , m_localP2pPort(38170)
    , m_localConfigPath("")
    , m_standaloneMode(true)
    , m_daemonSupportsMiningRpc(false)
    , m_daemonHealthTimer(nullptr)
    , m_daemonPath("")
    , m_walletManager(new GuiWalletManager(this))
    , m_daemonRetryCount(0)
  {
    // Find daemon path (platform-aware)
    QString daemonName;
#ifdef Q_OS_WIN
    daemonName = "qsf.exe";
#else
    daemonName = "qsf";
#endif

    QStringList possiblePaths = {
      QCoreApplication::applicationDirPath() + "/" + daemonName,
      QCoreApplication::applicationDirPath() + "/../" + daemonName
#ifndef Q_OS_WIN
      ,
      "/home/qsf/quantumsafefoundation/build/bin/" + daemonName,
      "/usr/local/bin/" + daemonName,
      "/usr/bin/" + daemonName
#endif
    };

    for (const QString& path : possiblePaths) {
      if (QFile::exists(path)) {
        m_daemonPath = path;
        qDebug() << "Found daemon at:" << path;
        break;
      }
    }

    if (m_daemonPath.isEmpty()) {
      qDebug() << "Warning: QSF daemon not found in standard locations";
      qDebug() << "Searched paths:";
      for (const QString& path : possiblePaths) {
        qDebug() << "  -" << path;
      }
    }
    
    // Generate local config path - prefer default config that works when run manually
    #ifdef Q_OS_WIN
        QString baseDir = QString::fromWCharArray(_wgetenv(L"PROGRAMDATA"));
        if (baseDir.isEmpty()) baseDir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
        baseDir = QDir::toNativeSeparators(baseDir) + "\\quantumsafefoundation";
    #else
        QString baseDir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/.quantumsafefoundation";
    #endif
    
    // Prefer using the default config file if it exists (same one used when running manually)
    QString defaultConfig = baseDir + "/qsf.conf";
    if (m_currentNetwork == qsf::MAINNET) {
        // Use default config if it exists, otherwise use local config
        m_localConfigPath = QFile::exists(defaultConfig) ? defaultConfig : (baseDir + "/qsf.local.conf");
    } else {
        m_localConfigPath = baseDir + "/testnet/qsf.testnet.conf";
    }
    
    setupUI();
    
    // Set window icon (platform-specific)
    // Try multiple paths for the window icon
    QStringList windowIconPaths = {
        QCoreApplication::applicationDirPath() + "/qsf_icon.png",  // Check build directory first
        ":/icons/qsf_icon.png",  // Then check embedded resources
        ":/icons/qsf_icon.ico",
        QCoreApplication::applicationDirPath() + "/qsf_icon.ico"
    };
    
    bool windowIconSet = false;
    for (const QString& path : windowIconPaths) {
        QIcon icon(path);
        if (!icon.isNull()) {
            setWindowIcon(icon);
            windowIconSet = true;
            qDebug() << "Window icon loaded from:" << path;
            break;
        }
    }
    
    if (!windowIconSet) {
        qDebug() << "Failed to load window icon from all paths";
    }
    
    // Set WM_CLASS for proper dock integration on Linux
#ifndef Q_OS_WIN
    setProperty("WM_CLASS", "qsf-gui-miner");
    setWindowTitle("QSF Quantum-Safe Miner");
#endif
    
    // Use standard ports for better compatibility
    m_localRpcPort = 18071;
    m_localZmqPort = 18072;
    m_localP2pPort = 18070;
    setupNetworkConfigs();
    loadSettings();
    connectSignals();
    
    // Initialize daemon process
    m_daemonProcess = new QProcess(this);
    
    // Force reset daemon status to ensure clean state
    m_daemonRunning = false;
    m_daemonStartInProgress = false;
    m_activeWalletProcess = nullptr;
    m_walletCooldownTimer = new QTimer(this);
    m_walletCooldownTimer->setSingleShot(true);
    m_walletCooldownTimer->setInterval(3000); // 3 second cooldown between wallet operations
    
    // Production: connect to ZMQ at startup using current network (defaults to Mainnet)
    QTimer::singleShot(0, this, [this]() {
      // Initialize ZMQ client lazily and connect
      if (!m_zmqClient) {
        m_zmqClient = std::make_unique<ZmqRpcClient>(this);
        connect(m_zmqClient.get(), &ZmqRpcClient::connected, [this]() {
          m_miningLog->append("[INFO] ✅ Connected to ZMQ RPC server (startup)");
          m_connectionLabel->setText("Connected (ZMQ)");
          m_connectionLabel->setStyleSheet("color: #00d4aa; font-weight: bold;");
          // Consider quantum-safe features active when ZMQ control channel is up
          m_daemonRunning = true;
          m_daemonStatusLabel->setText("✅ Running");
          m_daemonStatusLabel->setStyleSheet("color: #00d4aa; font-weight: bold;");
          m_generatedKeysDisplay->setText(
            "🔐 Quantum-Safe Signatures Active\n"
            "==============================\n\n"
            "✅ XMSS signatures: Automatically generated\n"
            "✅ SPHINCS+ signatures: Automatically generated\n"
            "✅ Dual enforcement: Always active\n"
            "✅ Block validation: Automatic\n"
            "✅ Mining integration: Active\n\n"
            "Status: Connected to daemon via ZMQ\n"
            "All blocks will have dual quantum-safe signatures"
          );
        });
        connect(m_zmqClient.get(), &ZmqRpcClient::disconnected, [this]() {
          m_miningLog->append("[INFO] ⚠️ Disconnected from ZMQ RPC server");
        });
        connect(m_zmqClient.get(), &ZmqRpcClient::error, [this](const QString &err) {
          m_miningLog->append("[ERROR] ❌ ZMQ RPC error: " + err);
        });
      }
      m_miningLog->append("[INFO] 🔌 Connecting to ZMQ for current network...");
      if (!m_customZmqEndpoints.isEmpty()) {
        m_zmqClient->connectUsingConfigured(m_customZmqEndpoints, config::ZMQ_RPC_DEFAULT_PORT);
      } else {
        m_zmqClient->connect(m_currentNetwork);
      }
    });
    
    // Initial server status check
    onCheckServerStatus();
    
    // Initialize timers
    m_updateTimer = new QTimer(this);
    m_serverStatusTimer = new QTimer(this);
    m_miningStatusTimer = new QTimer(this);
    m_peerCountTimer = new QTimer(this);
    m_daemonHealthTimer = new QTimer(this);
    
    // Set up timer connections
    connect(m_updateTimer, &QTimer::timeout, this, &MainWindow::onUpdateStatistics);
    connect(m_serverStatusTimer, &QTimer::timeout, this, &MainWindow::onCheckServerStatus);
    connect(m_miningStatusTimer, &QTimer::timeout, this, &MainWindow::onUpdateMiningStatus);
    connect(m_peerCountTimer, &QTimer::timeout, this, &MainWindow::updatePeerCount);
    connect(m_daemonHealthTimer, &QTimer::timeout, this, &MainWindow::checkDaemonStatus);
    
    // Start timers
    m_updateTimer->start(5000);  // Update every 5 seconds
#ifdef Q_OS_WIN
    m_miningStatusTimer->start(4000);  // Windows: poll less frequently to reduce UI pressure
    m_serverStatusTimer->start(45000); // Windows: slower server status checks
    m_peerCountTimer->start(15000);    // Windows: slower peer count updates
    m_daemonHealthTimer->start(20000); // Windows: slower daemon health checks
#else
    m_miningStatusTimer->start(2000);  // Update mining status every 2 seconds
    m_serverStatusTimer->start(30000);  // Check server status every 30 seconds
    m_peerCountTimer->start(10000);  // Update peer count every 10 seconds
    m_daemonHealthTimer->start(15000);  // Check daemon health every 15 seconds
#endif
    
    // Initialize network manager
    m_networkManager = new QNetworkAccessManager(this);
    
    // Initialize ZMQ client
    m_zmqClient = std::make_unique<ZmqRpcClient>();
    
    // Connect ZMQ client signals
    connect(m_zmqClient.get(), &ZmqRpcClient::connected, this, [this]() {
      m_miningLog->append("[INFO] 🔗 Connected to ZMQ RPC");
    });
    connect(m_zmqClient.get(), &ZmqRpcClient::disconnected, this, [this]() {
      m_miningLog->append("[WARNING] 🔌 Disconnected from ZMQ RPC");
    });
    connect(m_zmqClient.get(), &ZmqRpcClient::error, this, [this](const QString& error) {
      m_miningLog->append("[ERROR] ZMQ: " + error);
    });
    
    // Try to connect to local daemon ZMQ immediately if daemon is running
    QTimer::singleShot(2000, [this]() {
      if (m_daemonRunning && m_zmqClient && !m_zmqClient->isConnected()) {
        m_miningLog->append("[INFO] 🔗 Attempting ZMQ connection to local daemon...");
        bool connected = m_zmqClient->connect("127.0.0.1", 18072);
        if (connected) {
          m_miningLog->append("[INFO] ✅ Connected to local daemon ZMQ");
        } else {
          m_miningLog->append("[WARNING] Failed to connect to local ZMQ, will retry...");
          // Retry local connection after a delay instead of falling back to remote
          QTimer::singleShot(3000, [this]() {
            if (m_zmqClient && !m_zmqClient->isConnected()) {
              m_miningLog->append("[INFO] 🔄 Retrying ZMQ connection to local daemon...");
              bool connected = m_zmqClient->connect("127.0.0.1", 18072);
              if (connected) {
                m_miningLog->append("[INFO] ✅ Connected to local daemon ZMQ on retry");
              } else {
                m_miningLog->append("[WARNING] Still failed to connect to local ZMQ, using remote endpoints");
                m_zmqClient->connect(qsf::MAINNET);
              }
            }
          });
        }
      }
    });
    
    // Initialize mining worker
    m_miningWorker = std::make_unique<MiningWorker>();
    m_miningThread = new QThread(this);
    m_miningWorker->moveToThread(m_miningThread);
    
    connect(m_miningWorker.get(), &MiningWorker::hashRateUpdated, this, [this](double hashRate) {
      onMiningUpdate(hashRate, 0, 0);
    });
    connect(m_miningWorker.get(), &MiningWorker::error, this, &MainWindow::onMiningError);
    connect(m_miningThread, &QThread::started, m_miningWorker.get(), &MiningWorker::startMining);
    connect(m_miningWorker.get(), &MiningWorker::miningStopped, m_miningThread, &QThread::quit);
    
    // Check initial daemon status and auto-start if needed
    QTimer::singleShot(1500, [this]() {
      // First do a quick synchronous check for running daemon process (Unix/Linux only)
      bool daemonProcessFound = false;
#ifndef Q_OS_WIN
      QProcess checkProcess;
      checkProcess.start("pgrep", QStringList() << "-f" << "qsf.*18071|qsf.*18072|qsf.*18070");
      checkProcess.waitForFinished(1000);
      daemonProcessFound = (checkProcess.exitCode() == 0 && 
                           !checkProcess.readAllStandardOutput().trimmed().isEmpty());
      
      if (daemonProcessFound) {
        // Daemon process found on Linux, verify it's responding
        m_miningLog->append("[INFO] 🔍 Daemon process detected, verifying connection...");
        checkDaemonStatus();
        return;
      }
#endif
      
      // No daemon process found (or on Windows), auto-start one
      // onStartDaemon() will first try to connect to existing daemon, then start if needed
      if (!m_daemonStartInProgress) {
        m_miningLog->append("[INFO] 🔍 No daemon detected. Auto-starting local daemon...");
        onStartDaemon();
      }
    });
    
    // Add daemon crash recovery
    connect(this, &MainWindow::daemonCrashed, this, [this]() {
      m_miningLog->append("[ERROR] 🚨 Daemon crashed! Attempting recovery...");
      m_daemonRunning = false;
      onDaemonStatusChanged(false);
      
      // Wait a bit then try to restart
      QTimer::singleShot(5000, [this]() {
        m_miningLog->append("[INFO] 🔄 Attempting to restart daemon...");
        onStartDaemon();
      });
    });
    
    // Set window properties
    setWindowTitle("QSF Quantum-Safe GUI Miner v2.0");
    setMinimumSize(1000, 700);
    // Try to set window icon again with fallback paths
    QStringList fallbackIconPaths = {
        QCoreApplication::applicationDirPath() + "/qsf_icon.png",  // Check build directory first
        ":/icons/qsf_icon.png",  // Then check embedded resources
        ":/icons/qsf_icon.ico",
        QCoreApplication::applicationDirPath() + "/qsf_icon.ico"
    };
    
    for (const QString& path : fallbackIconPaths) {
        QIcon icon(path);
        if (!icon.isNull()) {
            setWindowIcon(icon);
            break;
        }
    }
    
    // Set dark mode style
    setStyleSheet(R"(
      QMainWindow {
        background-color: #1a1a1a;
        color: #ffffff;
      }
      QTabWidget::pane {
        border: 1px solid #404040;
        background-color: #2d2d2d;
        border-radius: 8px;
      }
      QTabBar::tab {
        background-color: #404040;
        color: #ffffff;
        padding: 12px 20px;
        margin-right: 2px;
        border-top-left-radius: 8px;
        border-top-right-radius: 8px;
        font-weight: bold;
      }
      QTabBar::tab:selected {
        background-color: #2d2d2d;
        border-bottom: 2px solid #00d4aa;
      }
      QTabBar::tab:hover {
        background-color: #505050;
      }
      QGroupBox {
        font-weight: bold;
        border: 2px solid #404040;
        border-radius: 8px;
        margin-top: 10px;
        padding-top: 10px;
        background-color: #2d2d2d;
        color: #ffffff;
      }
      QGroupBox::title {
        subcontrol-origin: margin;
        left: 10px;
        padding: 0 5px 0 5px;
        color: #00d4aa;
        background-color: #2d2d2d;
      }
      QPushButton {
        background-color: #00d4aa;
        color: #1a1a1a;
        border: none;
        padding: 10px 20px;
        border-radius: 6px;
        font-weight: bold;
      }
      QPushButton:hover {
        background-color: #00b894;
      }
      QPushButton:pressed {
        background-color: #00a085;
      }
      QPushButton:disabled {
        background-color: #404040;
        color: #666666;
      }
      QLineEdit, QComboBox, QSpinBox {
        padding: 8px;
        border: 2px solid #404040;
        border-radius: 6px;
        background-color: #1a1a1a;
        color: #ffffff;
      }
      QLineEdit:focus, QComboBox:focus, QSpinBox:focus {
        border-color: #00d4aa;
      }
      QLineEdit::placeholder, QComboBox::placeholder {
        color: #666666;
      }
      QComboBox::drop-down {
        border: none;
        background-color: #404040;
      }
      QComboBox::down-arrow {
        image: none;
        border-left: 5px solid transparent;
        border-right: 5px solid transparent;
        border-top: 5px solid #ffffff;
      }
      QComboBox QAbstractItemView {
        background-color: #2d2d2d;
        color: #ffffff;
        border: 1px solid #404040;
        selection-background-color: #00d4aa;
        selection-color: #1a1a1a;
      }
      QSpinBox::up-button, QSpinBox::down-button {
        background-color: #404040;
        border: none;
        border-radius: 2px;
      }
      QSpinBox::up-button:hover, QSpinBox::down-button:hover {
        background-color: #505050;
      }
      QLabel {
        color: #ffffff;
      }
      QTextEdit, QTextBrowser {
        border: 2px solid #404040;
        border-radius: 6px;
        background-color: #1a1a1a;
        color: #ffffff;
        padding: 8px;
      }
      QScrollBar:vertical {
        background-color: #2d2d2d;
        width: 12px;
        border-radius: 6px;
      }
      QScrollBar::handle:vertical {
        background-color: #404040;
        border-radius: 6px;
        min-height: 20px;
      }
      QScrollBar::handle:vertical:hover {
        background-color: #505050;
      }
      QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
        height: 0px;
      }
      QScrollBar:horizontal {
        background-color: #2d2d2d;
        height: 12px;
        border-radius: 6px;
      }
      QScrollBar::handle:horizontal {
        background-color: #404040;
        border-radius: 6px;
        min-width: 20px;
      }
      QScrollBar::handle:horizontal:hover {
        background-color: #505050;
      }
      QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {
        width: 0px;
      }
      
      /* Dark mode for popups and dialogs */
      QMessageBox {
        background-color: #2d2d2d;
        color: #ffffff;
      }
      QMessageBox QLabel {
        color: #ffffff;
        background-color: #2d2d2d;
      }
      QMessageBox QPushButton {
        background-color: #404040;
        color: #ffffff;
        border: 1px solid #505050;
        border-radius: 4px;
        padding: 8px 16px;
        min-width: 80px;
      }
      QMessageBox QPushButton:hover {
        background-color: #505050;
        border-color: #606060;
      }
      QMessageBox QPushButton:pressed {
        background-color: #303030;
      }
      
      /* Input dialogs */
      QInputDialog {
        background-color: #2d2d2d;
        color: #ffffff;
      }
      QInputDialog QLabel {
        color: #ffffff;
        background-color: #2d2d2d;
      }
      QInputDialog QLineEdit {
        background-color: #1a1a1a;
        color: #ffffff;
        border: 1px solid #404040;
        border-radius: 4px;
        padding: 6px;
      }
      QInputDialog QLineEdit:focus {
        border-color: #606060;
      }
      
      /* File dialogs */
      QFileDialog {
        background-color: #2d2d2d;
        color: #ffffff;
      }
      QFileDialog QLabel {
        color: #ffffff;
        background-color: #2d2d2d;
      }
      QFileDialog QLineEdit {
        background-color: #1a1a1a;
        color: #ffffff;
        border: 1px solid #404040;
        border-radius: 4px;
        padding: 6px;
      }
      QFileDialog QTreeView {
        background-color: #1a1a1a;
        color: #ffffff;
        border: 1px solid #404040;
        border-radius: 4px;
      }
      QFileDialog QTreeView::item:selected {
        background-color: #404040;
      }
      QFileDialog QPushButton {
        background-color: #404040;
        color: #ffffff;
        border: 1px solid #505050;
        border-radius: 4px;
        padding: 6px 12px;
      }
      QFileDialog QPushButton:hover {
        background-color: #505050;
      }
    )");
  }

  MainWindow::~MainWindow()
  {
    if (m_isMining)
    {
      onStopMining();
    }
    saveSettings();
    
    // Clean up daemon process safely
    if (m_localDaemonProcess) {
      m_localDaemonProcess->disconnect();
      m_localDaemonProcess->kill();
      m_localDaemonProcess->waitForFinished(3000);
      m_localDaemonProcess->deleteLater();
      m_localDaemonProcess = nullptr;
    }
    
    // Clean up wallet manager
    if (m_walletManager) {
      m_walletManager->closeWallet();
      delete m_walletManager;
      m_walletManager = nullptr;
    }
  }

  void MainWindow::setupNetworkConfigs()
  {
    // Mainnet configuration - prefer local daemon for RPC; ZMQ used for mining templates
    NetworkConfig mainnet;
    mainnet.name = "Mainnet";
    // RPC fallback should be local by default
    mainnet.daemonUrl = "http://127.0.0.1:18071";
    mainnet.poolUrl = "";
    mainnet.rpcPort = 18071;
    mainnet.p2pPort = 18070;
    mainnet.seedNodes = QStringList() 
      << "seeds.qsfchain.com:18070";     // Use DNS seeds only
    m_networkConfigs[qsf::MAINNET] = mainnet;

    // Force mainnet only - removed testnet and stagenet configurations
    m_currentNetwork = qsf::MAINNET;
  }

  void MainWindow::setupUI()
  {
    m_centralWidget = new QWidget(this);
    setCentralWidget(m_centralWidget);
    
    m_mainLayout = new QVBoxLayout(m_centralWidget);
    m_mainLayout->setSpacing(10);
    m_mainLayout->setContentsMargins(20, 20, 20, 20);
    
    // Header with logo and title
    setupHeader();
    
    // Main tab widget
    m_tabWidget = new QTabWidget(this);
    m_mainLayout->addWidget(m_tabWidget);
    
    // Create tabs
    setupOverviewTab();
    setupMiningTab();
    setupQuantumSafeTab();
    setupSettingsTab();
    
    // Force reset daemon status to ensure clean state
    m_daemonRunning = false;
    onDaemonStatusChanged(false);
    m_miningLog->append("[INFO] 🔄 Daemon status reset to ensure clean startup");
  }

  void MainWindow::setupHeader()
  {
    QHBoxLayout* headerLayout = new QHBoxLayout();
    
    // Logo with proper scaling
    QLabel* logoLabel = new QLabel(this);
    QPixmap logoPixmap;
    
    // Try multiple paths for the logo
    QStringList logoPaths = {
        ":/icons/qsf_icon.png",
        QCoreApplication::applicationDirPath() + "/qsf_icon.ico",
        QCoreApplication::applicationDirPath() + "/qsf_icon.png",
        QCoreApplication::applicationDirPath() + "/../src/gui_miner/icons/qsf_icon.png",
        QCoreApplication::applicationDirPath() + "/../../src/gui_miner/icons/qsf_icon.png",
        QDir::homePath() + "/QSF/src/gui_miner/icons/qsf_icon.png",
        QDir::homePath() + "/quantumsafefoundation/src/gui_miner/icons/qsf_icon.png",
        "/home/mb/QSF/src/gui_miner/icons/qsf_icon.png",
        "qsf_icon.png"
    };
    
    bool logoLoaded = false;
    for (const QString& path : logoPaths) {
        logoPixmap = QPixmap(path);
        if (!logoPixmap.isNull()) {
            logoPixmap = logoPixmap.scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            logoLabel->setPixmap(logoPixmap);
            qDebug() << "QSF Logo loaded successfully from:" << path;
            logoLoaded = true;
            break;
        }
    }
    
    if (!logoLoaded) {
        logoLabel->setText("QSF");
        logoLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #ff6b35; margin-right: 15px;");
        qDebug() << "QSF Logo failed to load from all paths, using text fallback";
    } else {
        // Ensure the pixmap is valid before setting it
        if (!logoPixmap.isNull()) {
            logoLabel->setPixmap(logoPixmap);
        } else {
            logoLabel->setText("QSF");
            logoLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #ff6b35; margin-right: 15px;");
        }
    }
    headerLayout->addWidget(logoLabel);
    
    // Title and subtitle
    QVBoxLayout* titleLayout = new QVBoxLayout();
    QLabel* titleLabel = new QLabel("QSF Quantum-Safe Wallet", this);
    titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #ffffff; margin: 0;");
    QLabel* subtitleLabel = new QLabel("Quantum-Resistant Cryptocurrency", this);
    subtitleLabel->setStyleSheet("font-size: 14px; color: #cccccc; margin: 0;");
    
    titleLayout->addWidget(titleLabel);
    titleLayout->addWidget(subtitleLabel);
    headerLayout->addLayout(titleLayout);
    
    headerLayout->addStretch();
    
    // Network selector - Force mainnet only
    QHBoxLayout* networkLayout = new QHBoxLayout();
    networkLayout->addWidget(new QLabel("Network:"));
    m_networkCombo = new QComboBox(this);
    m_networkCombo->addItem("Mainnet");
    // Removed testnet option to fix connection issues
    m_networkCombo->setCurrentIndex(0); // Default to mainnet
    m_networkCombo->setEnabled(false); // Disable network switching
    networkLayout->addWidget(m_networkCombo);
    headerLayout->addLayout(networkLayout);
    
    // Status indicator
    m_statusLabel = new QLabel("Ready", this);
    m_statusLabel->setStyleSheet("padding: 8px 16px; background-color: #00d4aa; color: #1a1a1a; border-radius: 20px; font-weight: bold;");
    headerLayout->addWidget(m_statusLabel);
    
    m_mainLayout->addLayout(headerLayout);
  }

  void MainWindow::setupOverviewTab()
  {
    QWidget* overviewWidget = new QWidget();
    QVBoxLayout* overviewLayout = new QVBoxLayout(overviewWidget);
    
    // Network status
    QGroupBox* networkGroup = new QGroupBox("Network Status", overviewWidget);
    QGridLayout* networkLayout = new QGridLayout(networkGroup);
    
    networkLayout->addWidget(new QLabel("Network:"), 0, 0);
    m_networkLabel = new QLabel("Mainnet", networkGroup);
    m_networkLabel->setStyleSheet("color: #00d4aa; font-weight: bold;");
    networkLayout->addWidget(m_networkLabel, 0, 1);
    
    networkLayout->addWidget(new QLabel("Connection:"), 1, 0);
    m_connectionLabel = new QLabel("Connected", networkGroup);
    m_connectionLabel->setStyleSheet("color: #00d4aa; font-weight: bold;");
    networkLayout->addWidget(m_connectionLabel, 1, 1);
    
    networkLayout->addWidget(new QLabel("Block Height:"), 2, 0);
    m_blockHeightLabel = new QLabel("0", networkGroup);
    networkLayout->addWidget(m_blockHeightLabel, 2, 1);
    
    networkLayout->addWidget(new QLabel("Network Hashrate:"), 3, 0);
    m_networkHashrateLabel = new QLabel("0 H/s", networkGroup);
    networkLayout->addWidget(m_networkHashrateLabel, 3, 1);
    
    networkLayout->addWidget(new QLabel("Peer Connections:"), 4, 0);
    
    // Create a horizontal layout for peer count and refresh button
    QHBoxLayout* peerCountLayout = new QHBoxLayout();
    m_peerCountLabel = new QLabel("0", networkGroup);
    m_peerCountLabel->setStyleSheet("color: #00d4aa; font-weight: bold;");
    
    QPushButton* refreshPeerBtn = new QPushButton("🔄", networkGroup);
    refreshPeerBtn->setToolTip("Refresh peer count");
    refreshPeerBtn->setMaximumWidth(30);
    refreshPeerBtn->setStyleSheet("QPushButton { background-color: #404040; color: #ffffff; border: none; border-radius: 3px; padding: 2px; } QPushButton:hover { background-color: #505050; }");
    
    peerCountLayout->addWidget(m_peerCountLabel);
    peerCountLayout->addWidget(refreshPeerBtn);
    peerCountLayout->addStretch();
    
    networkLayout->addLayout(peerCountLayout, 4, 1);
    
    // Connect refresh button to update peer count
    connect(refreshPeerBtn, &QPushButton::clicked, [this]() {
      m_miningLog->append("[INFO] Manual peer count refresh requested");
      updatePeerCount();
    });
    
    overviewLayout->addWidget(networkGroup);
    
    // Wallet section
    QGroupBox* walletGroup = new QGroupBox("Wallet", overviewWidget);
    QVBoxLayout* walletLayout = new QVBoxLayout(walletGroup);
    
    // Balance section
    QHBoxLayout* balanceLayout = new QHBoxLayout();
    balanceLayout->addWidget(new QLabel("Balance:"));
    m_balanceLabel = new QLabel("0.00000000 QSF", walletGroup);
    m_balanceLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #00d4aa;");
    balanceLayout->addWidget(m_balanceLabel);
    
    // Add wallet status indicator
    m_walletStatusLabel = new QLabel("❌ No Daemon", walletGroup);
    m_walletStatusLabel->setStyleSheet("color: #ff6b6b; font-weight: bold; font-size: 12px;");
    balanceLayout->addWidget(m_walletStatusLabel);
    
    // Add refresh button for balance
    QPushButton* refreshBalanceBtn = new QPushButton("🔄", walletGroup);
    refreshBalanceBtn->setToolTip("Refresh wallet balance");
    refreshBalanceBtn->setMaximumWidth(30);
    refreshBalanceBtn->setStyleSheet("QPushButton { background-color: #404040; color: #ffffff; border: none; border-radius: 3px; padding: 2px; } QPushButton:hover { background-color: #505050; }");
    balanceLayout->addWidget(refreshBalanceBtn);

    balanceLayout->addStretch();
    
    // Connect refresh button
    connect(refreshBalanceBtn, &QPushButton::clicked, [this]() {
      m_miningLog->append("[INFO] Manual balance refresh requested");
      updateWalletBalance();
    });
    
    walletLayout->addLayout(balanceLayout);
    
    // Hashrate section
    QHBoxLayout* hashrateLayout = new QHBoxLayout();
    hashrateLayout->addWidget(new QLabel("Hashrate:"));
    m_hashrateLabel = new QLabel("0.00 H/s", walletGroup);
    m_hashrateLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #ff6b35;");
    hashrateLayout->addWidget(m_hashrateLabel);
    hashrateLayout->addStretch();
    walletLayout->addLayout(hashrateLayout);
    
    // Connect hashrate updates to overview tab
    // This will be handled in the update functions
    
    // Wallet address display
    QHBoxLayout* addressLayout = new QHBoxLayout();
    addressLayout->addWidget(new QLabel("Address:"));
    m_walletAddressDisplay = new QTextBrowser(walletGroup);
    m_walletAddressDisplay->setMaximumHeight(60);
    m_walletAddressDisplay->setText("No wallet generated");
    m_walletAddressDisplay->setReadOnly(true);
    addressLayout->addWidget(m_walletAddressDisplay);
    walletLayout->addLayout(addressLayout);
    
    // Wallet actions
    QHBoxLayout* walletBtnLayout = new QHBoxLayout();
    m_generateWalletBtn = new QPushButton("Generate New Wallet", walletGroup);
    m_generateWalletBtn->setVisible(true);
    m_recoverWalletBtn = new QPushButton("Recover Wallet", walletGroup);
    m_recoverWalletBtn->setVisible(true);
    m_copyAddressBtn = new QPushButton("Copy Address", walletGroup);
    m_showPrivateKeyBtn = new QPushButton("Show Private Key", walletGroup);
    m_rescanWalletBtn = new QPushButton("Rescan Wallet", walletGroup);
    m_copyAddressBtn->setEnabled(false);
    m_rescanWalletBtn->setEnabled(false);
    m_showPrivateKeyBtn->setEnabled(false);
    m_rescanWalletBtn->setEnabled(false);
    
    walletBtnLayout->addWidget(m_generateWalletBtn);
    walletBtnLayout->addWidget(m_recoverWalletBtn);
    walletBtnLayout->addWidget(m_copyAddressBtn);
    walletBtnLayout->addWidget(m_rescanWalletBtn);
    walletBtnLayout->addWidget(m_showPrivateKeyBtn);
    walletLayout->addLayout(walletBtnLayout);
    
    overviewLayout->addWidget(walletGroup);
    
    // Quick actions
    QGroupBox* actionsGroup = new QGroupBox("Quick Actions", overviewWidget);
    QHBoxLayout* actionsLayout = new QHBoxLayout(actionsGroup);
    
    QPushButton* sendBtn = new QPushButton("Send QSF", actionsGroup);
    QPushButton* receiveBtn = new QPushButton("Receive QSF", actionsGroup);
    QPushButton* mineBtn = new QPushButton("Start Mining", actionsGroup);
    
    actionsLayout->addWidget(sendBtn);
    actionsLayout->addWidget(receiveBtn);
    actionsLayout->addWidget(mineBtn);
    
    overviewLayout->addWidget(actionsGroup);

    // Wire quick actions
    connect(mineBtn, &QPushButton::clicked, [this]() {
      m_tabWidget->setCurrentIndex(1); // Mining tab
      onStartMining();
    });
    connect(sendBtn, &QPushButton::clicked, this, &MainWindow::onQuickSend);
    connect(receiveBtn, &QPushButton::clicked, this, &MainWindow::onQuickReceive);
    
    m_tabWidget->addTab(overviewWidget, "Overview");
  }

  void MainWindow::setupMiningTab()
  {
    // Scrollable mining tab container
    QScrollArea* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setStyleSheet("QScrollArea { background-color: #1a1a1a; } QScrollArea > QWidget > QWidget { background-color: #1a1a1a; }");
    QWidget* miningWidget = new QWidget();
    QVBoxLayout* miningLayout = new QVBoxLayout(miningWidget);
    
    // Daemon Management
    QGroupBox* daemonGroup = new QGroupBox("Daemon Management", miningWidget);
    QGridLayout* daemonLayout = new QGridLayout(daemonGroup);
    daemonLayout->setContentsMargins(14, 14, 14, 14);
    daemonLayout->setHorizontalSpacing(12);
    daemonLayout->setVerticalSpacing(8);
    
    // Daemon Status
    daemonLayout->addWidget(new QLabel("Daemon Status:"), 0, 0);
    m_daemonStatusLabel = new QLabel("❌ Stopped", daemonGroup);
    m_daemonStatusLabel->setStyleSheet("color: #ff6b6b; font-weight: bold;");
    daemonLayout->addWidget(m_daemonStatusLabel, 0, 1);
    
    // Daemon Controls
    m_startDaemonBtn = new QPushButton("Start Daemon", daemonGroup);
    m_stopDaemonBtn = new QPushButton("Stop Daemon", daemonGroup);
    m_stopDaemonBtn->setEnabled(false);
    daemonLayout->addWidget(m_startDaemonBtn, 1, 0);
    daemonLayout->addWidget(m_stopDaemonBtn, 1, 1);
    
    miningLayout->addWidget(daemonGroup);
    
    // Mining configuration
    QGroupBox* configGroup = new QGroupBox("Mining Configuration", miningWidget);
    QGridLayout* configLayout = new QGridLayout(configGroup);
    configLayout->setHorizontalSpacing(12);
    configLayout->setVerticalSpacing(8);
    configLayout->setColumnStretch(0, 0);
    configLayout->setColumnStretch(1, 1);
    configLayout->setContentsMargins(14, 14, 14, 14);
    
    // Mining Mode
    configLayout->addWidget(new QLabel("Mining Mode:"), 0, 0);
    m_miningModeCombo = new QComboBox(miningWidget);
    m_miningModeCombo->addItem("Solo Mining");
    m_miningModeCombo->addItem("Pool Mining");
    configLayout->addWidget(m_miningModeCombo, 0, 1);
    
    // Daemon URL
    configLayout->addWidget(new QLabel("Daemon URL:"), 1, 0);
    m_daemonUrlEdit = new QLineEdit(miningWidget);
    m_daemonUrlEdit->setPlaceholderText("http://127.0.0.1:18071");
    configLayout->addWidget(m_daemonUrlEdit, 1, 1);
    
    // Pool Address (initially hidden)
    m_poolAddressLabel = new QLabel("Pool Address:", miningWidget);
    m_poolAddressEdit = new QLineEdit(miningWidget);
    m_poolAddressEdit->setPlaceholderText("stratum+tcp://pool.qsfcoin.com:3333");
    configLayout->addWidget(m_poolAddressLabel, 2, 0);
    configLayout->addWidget(m_poolAddressEdit, 2, 1);
    m_poolAddressLabel->setVisible(false);
    m_poolAddressEdit->setVisible(false);
    
    // Wallet Address
    configLayout->addWidget(new QLabel("Wallet Address:"), 3, 0);
    m_walletAddressEdit = new QLineEdit(miningWidget);
    m_walletAddressEdit->setPlaceholderText("Paste your QSF wallet address from qsf-wallet-cli");
    configLayout->addWidget(m_walletAddressEdit, 3, 1);
    
    // Mining Threads
    configLayout->addWidget(new QLabel("Mining Threads:"), 4, 0);
    m_threadsSpinBox = new QSpinBox(miningWidget);
    m_threadsSpinBox->setRange(1, 32);
#ifdef Q_OS_WIN
    // Default to a conservative number of threads on Windows to avoid UI stalls
    int hw = QThread::idealThreadCount();
    int defaultThreads = qMax(1, hw / 2);
    m_threadsSpinBox->setValue(defaultThreads);
#else
    m_threadsSpinBox->setValue(QThread::idealThreadCount());
#endif
    configLayout->addWidget(m_threadsSpinBox, 4, 1);
    
    // Algorithm Info (Read-only)
    configLayout->addWidget(new QLabel("Algorithm:"), 5, 0);
    QLabel* algorithmLabel = new QLabel("RandomX (Quantum-Safe)", miningWidget);
    algorithmLabel->setStyleSheet("color: #00d4aa; font-weight: bold;");
    configLayout->addWidget(algorithmLabel, 5, 1);
    
    miningLayout->addWidget(configGroup);
    
    // Mining controls
    QGroupBox* controlsGroup = new QGroupBox("Mining Controls", miningWidget);
    QHBoxLayout* controlsLayout = new QHBoxLayout(controlsGroup);
    controlsLayout->setSpacing(12);
    controlsLayout->setContentsMargins(14, 14, 14, 14);
    
    m_startMiningBtn = new QPushButton("Start Mining", controlsGroup);
    m_stopMiningBtn = new QPushButton("Stop Mining", controlsGroup);
    m_stopMiningBtn->setEnabled(false);
    
    controlsLayout->addWidget(m_startMiningBtn);
    controlsLayout->addWidget(m_stopMiningBtn);
    controlsLayout->addStretch();
    
    miningLayout->addWidget(controlsGroup);
    
    // Mining statistics
    QGroupBox* statsGroup = new QGroupBox("Mining Statistics", miningWidget);
    QGridLayout* statsLayout = new QGridLayout(statsGroup);
    statsLayout->setHorizontalSpacing(12);
    statsLayout->setVerticalSpacing(6);
    statsLayout->setColumnStretch(0, 0);
    statsLayout->setColumnStretch(1, 1);
    statsLayout->setContentsMargins(14, 14, 14, 14);
    
    // Hash Rate
    statsLayout->addWidget(new QLabel("Hash Rate:"), 0, 0);
    m_hashRateLabel = new QLabel("0.00 H/s", statsGroup);
    statsLayout->addWidget(m_hashRateLabel, 0, 1);
    
    // Accepted Shares
    statsLayout->addWidget(new QLabel("Accepted Shares:"), 1, 0);
    m_acceptedSharesLabel = new QLabel("0", statsGroup);
    statsLayout->addWidget(m_acceptedSharesLabel, 1, 1);
    
    // Rejected Shares
    statsLayout->addWidget(new QLabel("Rejected Shares:"), 2, 0);
    m_rejectedSharesLabel = new QLabel("0", statsGroup);
    statsLayout->addWidget(m_rejectedSharesLabel, 2, 1);
    
    // Uptime
    statsLayout->addWidget(new QLabel("Uptime:"), 3, 0);
    m_uptimeLabel = new QLabel("00:00:00", statsGroup);
    statsLayout->addWidget(m_uptimeLabel, 3, 1);
    
    // Difficulty
    statsLayout->addWidget(new QLabel("Difficulty:"), 4, 0);
    m_difficultyLabel = new QLabel("1.00K", statsGroup);
    statsLayout->addWidget(m_difficultyLabel, 4, 1);
    
    // Block Reward removed (was inaccurate); row omitted intentionally
    
    miningLayout->addWidget(statsGroup);
    
    // Mining Log
    QGroupBox* logGroup = new QGroupBox("Mining Log", miningWidget);
    QVBoxLayout* logLayout = new QVBoxLayout(logGroup);
    logLayout->setContentsMargins(14, 14, 14, 14);
    
    m_miningLog = new QTextEdit(miningWidget);
    m_miningLog->setMaximumHeight(300);
    m_miningLog->setReadOnly(true);
    m_miningLog->setPlaceholderText("Mining activity will appear here...");
    m_miningLog->setStyleSheet("QTextEdit { background-color: #1a1a1a; border: 1px solid #404040; border-radius: 4px; font-family: monospace; color: #ffffff; }");
    logLayout->addWidget(m_miningLog);
    
    miningLayout->addWidget(logGroup);
    
    // Add generous spacing between sections
    miningLayout->setSpacing(18);
    miningLayout->setContentsMargins(16, 16, 16, 16);
    scrollArea->setWidget(miningWidget);
    m_tabWidget->addTab(scrollArea, "Mining");
  }

  void MainWindow::setupQuantumSafeTab()
  {
    QWidget* quantumWidget = new QWidget();
    QVBoxLayout* quantumLayout = new QVBoxLayout(quantumWidget);
    
    // Quantum-safe keys section
    QGroupBox* keysGroup = new QGroupBox("Quantum-Safe Signatures (Automatic)", quantumWidget);
    QVBoxLayout* keysLayout = new QVBoxLayout(keysGroup);
    
    // Status indicator
    QHBoxLayout* statusLayout = new QHBoxLayout();
    m_quantumKeysStatusLabel = new QLabel("✅ Quantum-safe signatures handled automatically by daemon", quantumWidget);
    m_quantumKeysStatusLabel->setStyleSheet("color: #00d4aa; font-weight: bold;");
    statusLayout->addWidget(m_quantumKeysStatusLabel);
    statusLayout->addStretch();
    keysLayout->addLayout(statusLayout);
    
    // Algorithm Info (Read-only)
    QLabel* algoInfoLabel = new QLabel("🔒 Dual Algorithm: XMSS + SPHINCS+ (Both Required)", quantumWidget);
    algoInfoLabel->setStyleSheet("color: #00d4aa; font-weight: bold; padding: 10px; background-color: #404040; border-radius: 6px;");
    keysLayout->addWidget(algoInfoLabel);
    
    // Automatic signature info
    QLabel* autoInfoLabel = new QLabel("🔄 Automatic Quantum-Safe Signatures", quantumWidget);
    autoInfoLabel->setStyleSheet("color: #00d4aa; font-weight: bold; padding: 10px; background-color: #2a2a2a; border-radius: 6px;");
    keysLayout->addWidget(autoInfoLabel);
    
    // Generated Keys Display (Read-only, shows current status)
    m_generatedKeysDisplay = new QTextEdit(quantumWidget);
    m_generatedKeysDisplay->setMaximumHeight(150);
    m_generatedKeysDisplay->setReadOnly(true);
    m_generatedKeysDisplay->setPlaceholderText("Quantum-safe signatures are automatically generated by the daemon...\n\n✅ XMSS signatures: Automatically generated\n✅ SPHINCS+ signatures: Automatically generated\n✅ Dual enforcement: Always active\n✅ Block validation: Automatic");
    m_generatedKeysDisplay->setStyleSheet("QTextEdit { background-color: #1a1a1a; border: 1px solid #404040; border-radius: 4px; font-family: monospace; color: #00d4aa; }");
    keysLayout->addWidget(m_generatedKeysDisplay);
    
    quantumLayout->addWidget(keysGroup);
    
    // Quantum-safe information
    QGroupBox* infoGroup = new QGroupBox("Quantum-Safe Information", quantumWidget);
    QVBoxLayout* infoLayout = new QVBoxLayout(infoGroup);
    
    QLabel* infoText = new QLabel(
      "🔒 QSF uses DUAL quantum-resistant signature schemes for maximum security:\n\n"
      "• XMSS (eXtended Merkle Signature Scheme): Stateful hash-based signatures\n"
      "• SPHINCS+: Stateless hash-based signatures\n\n"
      "⚠️  IMPORTANT: BOTH quantum-safe algorithms are MANDATORY!\n"
      "This dual approach provides maximum protection against both classical and quantum computers.\n\n"
      "✅ All blocks are automatically validated with dual quantum-safe signatures\n"
      "✅ Mining automatically generates dual quantum-safe signatures\n"
      "✅ Network consensus enforces dual quantum-safe requirements\n"
      "✅ No manual key generation required - daemon handles everything automatically"
    );
    infoText->setWordWrap(true);
    infoText->setStyleSheet("padding: 10px; background-color: #404040; border-radius: 6px; color: #ffffff;");
    infoLayout->addWidget(infoText);
    
    quantumLayout->addWidget(infoGroup);
    
    m_tabWidget->addTab(quantumWidget, "Quantum-Safe");
  }

  void MainWindow::setupSettingsTab()
  {
    QWidget* settingsWidget = new QWidget();
    QVBoxLayout* settingsLayout = new QVBoxLayout(settingsWidget);
    
    // General settings
    QGroupBox* generalGroup = new QGroupBox("General Settings", settingsWidget);
    QGridLayout* generalLayout = new QGridLayout(generalGroup);
    
    generalLayout->addWidget(new QLabel("Default Mining Mode:"), 0, 0);
    QComboBox* defaultMiningMode = new QComboBox(settingsWidget);
    defaultMiningMode->addItem("Solo Mining");
    defaultMiningMode->addItem("Pool Mining");
    generalLayout->addWidget(defaultMiningMode, 0, 1);
    
    generalLayout->addWidget(new QLabel("Algorithm:"), 1, 0);
    QLabel* defaultAlgorithmLabel = new QLabel("RandomX (Quantum-Safe) - Fixed", settingsWidget);
    defaultAlgorithmLabel->setStyleSheet("color: #00d4aa; font-weight: bold;");
    generalLayout->addWidget(defaultAlgorithmLabel, 1, 1);
    
    settingsLayout->addWidget(generalGroup);
    
    // Network settings
    QGroupBox* networkGroup = new QGroupBox("Network Settings", settingsWidget);
    QGridLayout* networkLayout = new QGridLayout(networkGroup);
    
    networkLayout->addWidget(new QLabel("Default Daemon URL:"), 0, 0);
    QLineEdit* defaultDaemonUrl = new QLineEdit(settingsWidget);
    // Production default: HTTP fallback to seeds; ZMQ used primarily
    defaultDaemonUrl->setText("http://127.0.0.1:18071");
    networkLayout->addWidget(defaultDaemonUrl, 0, 1);
    
    networkLayout->addWidget(new QLabel("Default Pool URL:"), 1, 0);
    QLineEdit* defaultPoolUrl = new QLineEdit(settingsWidget);
    defaultPoolUrl->setText("stratum+tcp://pool.qsfcoin.com:3333");
    networkLayout->addWidget(defaultPoolUrl, 1, 1);
    
    settingsLayout->addWidget(networkGroup);
    
    // About section
    QGroupBox* aboutGroup = new QGroupBox("About QSF", settingsWidget);
    QVBoxLayout* aboutLayout = new QVBoxLayout(aboutGroup);
    
    QLabel* aboutText = new QLabel(
      "QSF Quantum-Safe Coin v2.0\n\n"
      "A quantum-resistant cryptocurrency combining RandomX proof-of-work mining "
      "with XMSS/SPHINCS+ quantum-safe signatures.\n\n"
      "© 2024 QSF Coin Project\n"
      "All rights reserved."
    );
    aboutText->setAlignment(Qt::AlignCenter);
    aboutText->setStyleSheet("padding: 20px;");
    aboutLayout->addWidget(aboutText);
    
    settingsLayout->addWidget(aboutGroup);
    
    settingsLayout->addStretch();
    
    m_tabWidget->addTab(settingsWidget, "Settings");
  }

  void MainWindow::connectSignals()
  {
    connect(m_startMiningBtn, &QPushButton::clicked, this, &MainWindow::onStartMining);
    connect(m_stopMiningBtn, &QPushButton::clicked, this, &MainWindow::onStopMining);
    connect(m_generateWalletBtn, &QPushButton::clicked, this, &MainWindow::onGenerateWallet);
    connect(m_recoverWalletBtn, &QPushButton::clicked, this, &MainWindow::onRecoverWallet);
    // Add open wallet action if a button/menu exists (fallback: bind to Ctrl+O)
    QShortcut* openWalletShortcut = new QShortcut(QKeySequence("Ctrl+O"), this);
    connect(openWalletShortcut, &QShortcut::activated, this, &MainWindow::onOpenWallet);
    connect(m_copyAddressBtn, &QPushButton::clicked, this, &MainWindow::onCopyAddress);
    connect(m_showPrivateKeyBtn, &QPushButton::clicked, this, &MainWindow::onShowPrivateKey);
    connect(m_rescanWalletBtn, &QPushButton::clicked, this, &MainWindow::onRescanWallet);
    
    // Connect network change - Disabled to force mainnet only
    // connect(m_networkCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
    //         this, &MainWindow::onNetworkChanged);
    
    // Connect mining mode change to show/hide appropriate fields
    connect(m_miningModeCombo, QOverload<const QString &>::of(&QComboBox::currentTextChanged),
            this, &MainWindow::onMiningModeChanged);
    
    // Connect daemon management
    connect(m_startDaemonBtn, &QPushButton::clicked, this, &MainWindow::onStartDaemon);
    connect(m_stopDaemonBtn, &QPushButton::clicked, this, &MainWindow::onStopDaemon);
    
    // Initialize field visibility
    onMiningModeChanged(m_miningModeCombo->currentText());
    // Force mainnet network selection - disabled network switching
    m_networkCombo->setCurrentIndex(0);
    m_currentNetwork = qsf::MAINNET;
    
    // Don't auto-check daemon status - let user control it manually
    // QTimer::singleShot(1000, this, &MainWindow::checkDaemonStatus);
    
    // Connect wallet manager signals
    connect(m_walletManager, &GuiWalletManager::walletOpened, this, &MainWindow::onWalletOpened);
    connect(m_walletManager, &GuiWalletManager::walletClosed, this, &MainWindow::onWalletClosed);
    connect(m_walletManager, &GuiWalletManager::balanceUpdated, this, &MainWindow::onBalanceUpdated);
    connect(m_walletManager, &GuiWalletManager::error, this, &MainWindow::onWalletError);
    // Interactive password prompts are disabled; password is handled programmatically for rescan commands
    
    // Enable auto-refresh for wallet balance (every 15 seconds)
    m_walletManager->setAutoRefresh(true, 15000);
    
    // Force initial wallet refresh after a short delay
    QTimer::singleShot(2000, [this]() {
      if (m_walletManager && m_walletManager->hasWallet()) {
        m_walletManager->refreshBalance();
      }
    });
  }

  void MainWindow::onOpenWallet()
  {
    QString defaultDir = QDir::homePath() + "/.quantumsafefoundation";
    QDir().mkpath(defaultDir);
    QString walletPath = QFileDialog::getOpenFileName(this, "Open Wallet File", defaultDir, "Wallet Files (*)");
    if (walletPath.isEmpty()) return;

    bool ok = false;
    QString password = QInputDialog::getText(this, "Wallet Password", "Enter wallet password:", QLineEdit::Password, "", &ok);
    if (!ok) return;

    if (!m_walletManager) return;

    // Open the wallet explicitly so we know exactly which one
    if (m_walletManager->openWallet(walletPath, password)) {
      m_hasWallet = true;
      m_walletAddressEdit->clear();
      m_miningLog->append("[INFO] ✅ Wallet opened: " + walletPath);

      // Persist path and remember for next run
      QSettings s("QSFCoin", "QuantumSafeWallet");
      s.setValue("wallet_path", walletPath);

      // If daemon is running, notify wallet manager to connect/refresh
      if (m_daemonRunning) {
        m_walletManager->setDaemonAddress("127.0.0.1:18071");
        m_walletManager->onDaemonStatusChanged(true);
      }
    } else {
      QMessageBox::critical(this, "Wallet Error", "Failed to open wallet.");
    }
  }

  void MainWindow::checkDaemonStatus()
  {
#ifdef Q_OS_WIN
    // On Windows with detached process, we can't check process state directly
    // Just verify via network connection
#else
    // First check if our local daemon process is still running
    if (m_localDaemonProcess && m_localDaemonProcess->state() == QProcess::Running) {
      m_daemonRunning = true;
      onDaemonStatusChanged(true);
      m_miningLog->append("[DEBUG] 🔍 Local daemon process is running");
      return;
    }
    
    // Check if daemon process crashed
    if (m_localDaemonProcess && m_localDaemonProcess->state() == QProcess::NotRunning && m_daemonRunning) {
      m_miningLog->append("[ERROR] 🚨 Local daemon process stopped unexpectedly!");
      m_miningLog->append("[ERROR] 🚨 Exit code: " + QString::number(m_localDaemonProcess->exitCode()));
      m_miningLog->append("[ERROR] 🚨 Exit status: " + QString::number(m_localDaemonProcess->exitStatus()));
      emit daemonCrashed();
      return;
    }
#endif
    
    // Check if daemon is actually responding via HTTP
    if (m_daemonRunning) {
      QNetworkAccessManager* nam = new QNetworkAccessManager(this);
      QNetworkRequest req(QUrl("http://127.0.0.1:18071/json_rpc"));
      req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
      req.setTransferTimeout(5000); // 5 second timeout
      
      QJsonObject request;
      request["jsonrpc"] = "2.0";
      request["id"] = "0";
      request["method"] = "get_info";
      request["params"] = QJsonObject();
      
      QNetworkReply* reply = nam->post(req, QJsonDocument(request).toJson());
      connect(reply, &QNetworkReply::finished, [this, reply, nam]() {
        if (reply->error() == QNetworkReply::NoError) {
          // Daemon is responding, update status
          if (!m_daemonRunning) {
            m_daemonRunning = true;
            onDaemonStatusChanged(true);
            m_miningLog->append("[INFO] 🔄 Daemon status corrected - daemon is actually running");
          }
        } else {
          // Daemon is not responding
          if (m_daemonRunning) {
            m_miningLog->append("[WARNING] Daemon not responding via HTTP, updating status to stopped.");
            m_daemonRunning = false;
            onDaemonStatusChanged(false);
          }
        }
        reply->deleteLater();
        nam->deleteLater();
      });
    }
    
    // Check if there's already a QSF daemon process running - improved detection
    bool foundExternalDaemon = false;
    QProcess checkProcess;
    checkProcess.start("pgrep", QStringList() << "-f" << "qsf.*daemon|qsf.*rpc|qsf.*18071|qsf.*18081");
    checkProcess.waitForFinished(1000);
    
    if (checkProcess.exitCode() == 0) {
      QString output = checkProcess.readAllStandardOutput().trimmed();
      if (!output.isEmpty()) {
        // Verify the daemon is actually responding before claiming it's running
        QString testUrl = "http://127.0.0.1:18071";
        QNetworkAccessManager* testNam = new QNetworkAccessManager(this);
        QNetworkRequest testRequest(QUrl(testUrl + "/json_rpc"));
        testRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        
        QJsonObject testJson;
        testJson["jsonrpc"] = "2.0";
        testJson["id"] = "0";
        testJson["method"] = "get_info";
        testJson["params"] = QJsonObject();
        
        QJsonDocument testDoc(testJson);
        QByteArray testData = testDoc.toJson();
        
        QNetworkReply* testReply = testNam->post(testRequest, testData);
        connect(testReply, &QNetworkReply::finished, [this, testReply, testNam, output, &foundExternalDaemon]() {
          if (testReply->error() == QNetworkReply::NoError) {
            foundExternalDaemon = true;
            m_daemonRunning = true;
            onDaemonStatusChanged(true);
            m_miningLog->append("[INFO] 🔍 Detected external QSF daemon process running and responding (PID: " + output + ")");
          } else {
            m_miningLog->append("[INFO] ⚠️ Found daemon process but it's not responding. Clearing daemon status.");
            m_daemonRunning = false;
            onDaemonStatusChanged(false);
          }
          testReply->deleteLater();
          testNam->deleteLater();
        });
        return; // Exit early since we're doing async check
      }
    }
    
    // Check if local daemon is running on default ports (127.0.0.1)
    QString localDaemonUrl = "http://127.0.0.1:" + QString::number(m_networkConfigs[m_currentNetwork].rpcPort);
    QNetworkAccessManager* nam = new QNetworkAccessManager(this);
    QNetworkRequest request(QUrl(localDaemonUrl + "/json_rpc"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    QJsonObject jsonObj;
    jsonObj["jsonrpc"] = "2.0";
    jsonObj["id"] = "0";
    jsonObj["method"] = "get_info";
    jsonObj["params"] = QJsonObject();
    
    QJsonDocument doc(jsonObj);
    QByteArray data = doc.toJson();
    
    QNetworkReply* reply = nam->post(request, data);
    connect(reply, &QNetworkReply::finished, [this, reply, nam, localDaemonUrl, data, foundExternalDaemon]() {
      if (reply->error() == QNetworkReply::NoError) {
        // Local daemon is running on default ports
        onDaemonStatusChanged(true);
        m_miningLog->append("[INFO] ✅ Detected local daemon running at " + localDaemonUrl);
        
        if (foundExternalDaemon) {
          m_miningLog->append("[INFO] ℹ️ External daemon detected - GUI will connect to existing daemon");
          m_miningLog->append("[INFO] 💡 Use 'Stop Daemon' to stop the external daemon if needed");
        }
        
        // Update daemon URL to use local daemon
        m_daemonUrlEdit->setText(localDaemonUrl);
        
        // Update connection status
        m_connectionLabel->setText("Connected");
        m_connectionLabel->setStyleSheet("color: #00d4aa; font-weight: bold;");
        
        // Also check if it's a quantum-safe daemon
        QJsonDocument responseDoc = QJsonDocument::fromJson(reply->readAll());
        if (responseDoc.isObject()) {
          QJsonObject response = responseDoc.object();
          if (response.contains("result")) {
            QJsonObject result = response["result"].toObject();
            if (result.contains("version")) {
              QString version = result["version"].toString();
              if (version.contains("Quantum Safe") || version.contains("QSF")) {
                m_miningLog->append("[INFO] 🔒 Confirmed: Local daemon is QSF quantum-safe enabled");
                m_miningLog->append("[INFO] 💡 You can now start mining - daemon is ready!");
              }
            }
            // Update block height if available
            if (result.contains("height")) {
              int height = result["height"].toInt();
              m_blockHeightLabel->setText(QString::number(height));
            }
          }
        }
      } else {
        // Local daemon is not running on default ports
        if (foundExternalDaemon) {
          // External daemon is running but not responding on expected port
          m_miningLog->append("[INFO] ⚠️ External daemon detected but not responding on port " + QString::number(m_networkConfigs[m_currentNetwork].rpcPort));
          m_miningLog->append("[INFO] 💡 The daemon might be using different ports or configuration");
        } else {
          onDaemonStatusChanged(false);
          m_miningLog->append("[INFO] ℹ️ No local daemon detected at " + localDaemonUrl);
        }
        
        // Update connection status
        m_connectionLabel->setText("Disconnected");
        m_connectionLabel->setStyleSheet("color: #ff6b6b; font-weight: bold;");
        
        // Check if there's a daemon running on alternative local ports
        checkAlternativeLocalPorts();
        
        // Check if remote daemon is available as fallback
        QString remoteDaemonUrl = getNetworkDaemonUrl();
        if (remoteDaemonUrl != localDaemonUrl) {
          QNetworkRequest remoteRequest(QUrl(remoteDaemonUrl + "/json_rpc"));
          remoteRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
          
          QNetworkReply* remoteReply = nam->post(remoteRequest, data);
          connect(remoteReply, &QNetworkReply::finished, [this, remoteReply, remoteDaemonUrl]() {
            if (remoteReply->error() == QNetworkReply::NoError) {
              m_miningLog->append("[INFO] 🌐 Remote daemon available at " + remoteDaemonUrl + " (can use for mining)");
              m_daemonUrlEdit->setText(remoteDaemonUrl);
              m_miningLog->append("[INFO] 💡 You can start mining using the remote daemon");
              
              // Update connection status for remote daemon
              m_connectionLabel->setText("Connected (Remote)");
              m_connectionLabel->setStyleSheet("color: #00d4aa; font-weight: bold;");
            } else {
              m_miningLog->append("[INFO] 🌐 Remote daemon also unavailable at " + remoteDaemonUrl);
              m_miningLog->append("[INFO] 🚀 Click 'Start Daemon' to start a local quantum-safe daemon");
              m_miningLog->append("[INFO] 💡 Or manually enter a daemon URL in the Daemon URL field");
            }
            remoteReply->deleteLater();
          });
        }
      }
      reply->deleteLater();
      nam->deleteLater();
    });
  }

  void MainWindow::checkAlternativeLocalPorts()
  {
    // Check common alternative local ports for daemons - Force mainnet only
    QList<int> alternativePorts;
    alternativePorts << 18081 << 18082 << 18083;  // Alternative mainnet ports only
    
    for (int port : alternativePorts) {
      QString altUrl = "http://127.0.0.1:" + QString::number(port);
      QNetworkAccessManager* altNam = new QNetworkAccessManager(this);
      QNetworkRequest altRequest(QUrl(altUrl + "/json_rpc"));
      altRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
      
      QJsonObject altJsonObj;
      altJsonObj["jsonrpc"] = "2.0";
      altJsonObj["id"] = "0";
      altJsonObj["method"] = "get_info";
      altJsonObj["params"] = QJsonObject();
      
      QJsonDocument altDoc(altJsonObj);
      QByteArray altData = altDoc.toJson();
      
      QNetworkReply* altReply = altNam->post(altRequest, altData);
      connect(altReply, &QNetworkReply::finished, [this, altReply, altNam, altUrl, port]() {
        if (altReply->error() == QNetworkReply::NoError) {
          m_miningLog->append("[INFO] 🔍 Found daemon running on alternative port " + QString::number(port));
          m_miningLog->append("[INFO] 💡 You can use this daemon for mining: " + altUrl);
          m_daemonUrlEdit->setText(altUrl);
          
          // Update connection status
          m_connectionLabel->setText("Connected (Alt Port)");
          m_connectionLabel->setStyleSheet("color: #00d4aa; font-weight: bold;");
          
          // If we found a daemon, update the status
          onDaemonStatusChanged(true);
        }
        altReply->deleteLater();
        altNam->deleteLater();
      });
    }
  }

  void MainWindow::onGenerateWallet()
  {
  // Ask for wallet save path - Use proper QSF data directory structure
  QString defaultDir;
  if (m_currentNetwork == qsf::MAINNET) {
    defaultDir = QDir::homePath() + "/.quantumsafefoundation/wallets";
  } else if (m_currentNetwork == qsf::TESTNET) {
    defaultDir = QDir::homePath() + "/.quantumsafefoundation/testnet/wallets";
  } else { // STAGENET
    defaultDir = QDir::homePath() + "/.quantumsafefoundation/stagenet/wallets";
  }
    QDir().mkpath(defaultDir);
    QString walletPath = QFileDialog::getSaveFileName(this, "Create Wallet File", defaultDir + "/qsf-wallet", "Wallet Files (*)");
    if (walletPath.isEmpty()) return;

    // Ask for password
    bool ok1 = false;
    QString password = QInputDialog::getText(this, "Wallet Password", "Enter a strong password:", QLineEdit::Password, "", &ok1);
    if (!ok1) return;
    bool ok2 = false;
    QString confirm = QInputDialog::getText(this, "Confirm Password", "Re-enter password:", QLineEdit::Password, "", &ok2);
    if (!ok2 || confirm != password)
    {
      QMessageBox::warning(this, "Password Mismatch", "Passwords do not match.");
      return;
    }

  // Create wallet via libwallet using the currently selected network
    auto mgr = qsf::WalletManagerFactory::getWalletManager();
  qsf::NetworkType net = m_currentNetwork;
    qsf::Wallet* w = mgr->createWallet(walletPath.toStdString(), password.toStdString(), std::string("English"), net);
    if (!w)
    {
      QMessageBox::critical(this, "Wallet Error", "Failed to create wallet (null).");
      return;
    }

    int st = w->status();
    if (st != qsf::Wallet::Status_Ok)
    {
      QString err = QString::fromStdString(w->errorString());
      mgr->closeWallet(w, false);
      QMessageBox::critical(this, "Wallet Error", "Failed to create wallet: " + err);
      return;
    }

    // Initialize daemon address from UI (host:port)
    const QString daemonUrl = m_daemonUrlEdit->text().trimmed();
    if (!daemonUrl.isEmpty())
      w->init(daemonUrl.toStdString());

    // Persist wallet to disk
    w->store(walletPath.toStdString());

    // Gather details
    const std::string addr = w->address();
    const std::string seed = w->seed();
    const std::string spend = w->secretSpendKey();
    const std::string view = w->secretViewKey();

    // Close wallet to avoid file locks
    mgr->closeWallet(w, true);

    // Update UI state
    m_walletAddress = QString::fromStdString(addr);
    m_walletPrivateKey = QString::fromStdString(spend);
    m_hasWallet = true;
    m_walletAddressDisplay->setText(m_walletAddress);
    m_walletAddressDisplay->setStyleSheet("background-color: #1a1a1a; border: 1px solid #404040; padding: 8px; border-radius: 4px; font-family: monospace; color: #ffffff;");
    m_walletAddressEdit->setText(m_walletAddress);
    m_copyAddressBtn->setEnabled(true);
    m_rescanWalletBtn->setEnabled(true);
    m_showPrivateKeyBtn->setEnabled(true);

    // Save to settings
    QSettings settings("QSFCoin", "QuantumSafeWallet");
    settings.setValue("wallet_address_saved", m_walletAddress);
    settings.setValue("wallet_private_key", m_walletPrivateKey);
    settings.setValue("wallet_path", walletPath);

    // Show details
    QString info = QString(
      "Wallet created successfully!\n\n"
      "Path: %1\n"
      "Network: %2\n\n"
      "Address:\n%3\n\n"
      "Seed phrase:\n%4\n\n"
      "Secret spend key:\n%5\n\n"
      "Secret view key:\n%6\n")
      .arg(walletPath)
      .arg(m_networkConfigs[m_currentNetwork].name)
      .arg(m_walletAddress)
      .arg(QString::fromStdString(seed))
      .arg(QString::fromStdString(spend))
      .arg(QString::fromStdString(view));

    QMessageBox::information(this, "Wallet Created", info);

    // Initialize the WalletManager with the new wallet for balance tracking
    if (m_walletManager) {
      m_walletManager->openWallet(walletPath, password);
      // Force rescan from block height 0 for new wallets
      m_walletManager->rescanBlockchainFromZero();
      m_miningLog->append("[INFO] 🔄 Wallet opened in WalletManager, forcing automatic rescan from block 0");
      
      // Also trigger rescan when daemon becomes available
      QTimer::singleShot(2000, [this]() {
        if (m_walletManager && m_daemonRunning) {
          m_walletManager->onDaemonStatusChanged(true);
        }
      });
    }

    // Ask the user if they want to start mining now (avoid auto-start)
    auto res = QMessageBox::question(this,
                                     "Start Mining?",
                                     "Your wallet has been created and will automatically rescan from block 0.\n\nDo you want to start mining now?",
                                     QMessageBox::Yes | QMessageBox::No,
                                     QMessageBox::No);
    if (res == QMessageBox::Yes) {
      // Ensure daemon is running first
      if (!m_daemonRunning) {
        onStartDaemon();
      }
      // Do not actually start mining until daemon is confirmed and wallet is opened
      QSettings settings("QSFCoin", "QuantumSafeWallet");
      settings.setValue("mining_wallet_address", m_walletAddress);
      m_walletAddressEdit->setText(m_walletAddress);
      m_miningLog->append("[INFO] ⛏️ Mining will start once daemon is ready and wallet is opened");
    }
  }

  // Wallet Manager slot implementations
  void MainWindow::onWalletOpened(const QString& address)
  {
    m_walletAddress = address;
    m_hasWallet = true;
    m_walletAddressDisplay->setText(address);
    m_walletAddressDisplay->setStyleSheet("background-color: #1a1a1a; border: 1px solid #404040; padding: 8px; border-radius: 4px; font-family: monospace; color: #ffffff;");
    m_walletAddressEdit->setText(address);
    m_copyAddressBtn->setEnabled(true);
    m_rescanWalletBtn->setEnabled(true);
    m_showPrivateKeyBtn->setEnabled(true);
    m_miningLog->append("[INFO] ✅ Wallet opened: " + address);
    // Ensure wallet talks to the same daemon URL as GUI
    if (m_walletManager && m_daemonUrlEdit) {
      const QString daemonUrl = m_daemonUrlEdit->text().trimmed();
      if (!daemonUrl.isEmpty()) {
        // Accept both http://host:port and host:port; extract host:port
        QString hostPort = daemonUrl;
        if (hostPort.startsWith("http://")) hostPort = hostPort.mid(QString("http://").length());
        if (hostPort.startsWith("https://")) hostPort = hostPort.mid(QString("https://").length());
        // Strip path if any
        int slash = hostPort.indexOf('/');
        if (slash > 0) hostPort = hostPort.left(slash);
        m_walletManager->setDaemonAddress(hostPort);
      }
    }
  }

  void MainWindow::onWalletClosed()
  {
    m_hasWallet = false;
    m_walletAddress.clear();
    m_walletAddressDisplay->setText("No wallet loaded");
    m_walletAddressDisplay->setStyleSheet("background-color: #1a1a1a; border: 1px solid #404040; padding: 8px; border-radius: 4px; font-family: monospace; color: #666666;");
    m_walletAddressEdit->clear();
    m_copyAddressBtn->setEnabled(false);
    m_rescanWalletBtn->setEnabled(false);
    m_showPrivateKeyBtn->setEnabled(false);
    m_balanceLabel->setText("0.00000000 QSF");
    m_miningLog->append("[INFO] ℹ️ Wallet closed");
  }

  void MainWindow::onBalanceUpdated(const QString& balance)
  {
    m_balanceLabel->setText(balance + " QSF");
    m_miningLog->append("[INFO] 💰 Balance updated: " + balance + " QSF");
    
    // Check if balance is 0 and daemon is not running
    if (balance == "0.000000000000" || balance == "0" || balance.isEmpty()) {
      if (!m_daemonRunning) {
        m_miningLog->append("[WARNING] ⚠️ Balance is 0 - this may be because no daemon is running");
        m_miningLog->append("[INFO] 💡 Click 'Start Daemon' to sync with the blockchain");
      } else {
        m_miningLog->append("[INFO] ℹ️ Balance is 0 - wallet is synced but no transactions found");
      }
    }
    
    // Enable auto-refresh for wallet balance
    if (m_walletManager) {
      m_walletManager->setAutoRefresh(true, 10000); // Refresh every 10 seconds
    }
  }

  void MainWindow::onWalletError(const QString& error)
  {
    m_miningLog->append("[ERROR] ❌ Wallet error: " + error);
    
    // Handle daemon connection errors specifically
    if (error.contains("cannot connect to daemon", Qt::CaseInsensitive) ||
        error.contains("daemon is not started", Qt::CaseInsensitive)) {
      m_miningLog->append("[INFO] 💡 Please start the daemon first using 'Start Daemon' button");
      m_miningLog->append("[INFO] 💡 Once daemon is running, wallet will automatically reconnect");
      
      // Don't show error dialog for daemon connection issues
      return;
    }
    
    // Show error dialog for other issues
    QMessageBox::warning(this, "Wallet Error", error);
    
    // Offer retry if password is invalid or wallet failed to start
    if (error.contains("Invalid wallet password", Qt::CaseInsensitive) ||
        error.contains("Failed to start wallet process", Qt::CaseInsensitive)) {
      QSettings settings("QSFCoin", "QuantumSafeWallet");
      const QString walletPath = settings.value("wallet_path", "").toString();
      if (!walletPath.isEmpty() && m_walletManager) {
        bool ok = false;
        QString password = QInputDialog::getText(this, "Wallet Password", "Re-enter wallet password:", QLineEdit::Password, "", &ok);
        if (ok && !password.isEmpty()) {
          m_walletManager->openWallet(walletPath, password);
        }
      }
    }
  }

  void MainWindow::generateNewWallet()
  {
    onGenerateWallet();
  }

  void MainWindow::onRecoverWallet()
  {
    // Ask for wallet save path
    QString defaultDir;
    if (m_currentNetwork == qsf::MAINNET) {
      defaultDir = QDir::homePath() + "/.quantumsafefoundation/wallets";
    } else if (m_currentNetwork == qsf::TESTNET) {
      defaultDir = QDir::homePath() + "/.quantumsafefoundation/testnet/wallets";
    } else {
      defaultDir = QDir::homePath() + "/.quantumsafefoundation/stagenet/wallets";
    }
    QDir().mkpath(defaultDir);
    
    QString walletPath = QFileDialog::getSaveFileName(this, "Recover Wallet File", defaultDir + "/qsf-wallet-recovered", "Wallet Files (*)");
    if (walletPath.isEmpty()) return;

    // Ask for password
    bool ok1 = false;
    QString password = QInputDialog::getText(this, "Wallet Password", "Enter a password for the recovered wallet:", QLineEdit::Password, "", &ok1);
    if (!ok1) return;
    
    bool ok2 = false;
    QString confirm = QInputDialog::getText(this, "Confirm Password", "Re-enter password:", QLineEdit::Password, "", &ok2);
    if (!ok2 || confirm != password) {
      QMessageBox::warning(this, "Password Mismatch", "Passwords do not match.");
      return;
    }

    // Ask for mnemonic seed
    QDialog* dialog = new QDialog(this);
    dialog->setWindowTitle("Enter Mnemonic Seed");
    dialog->setModal(true);
    
    QVBoxLayout* layout = new QVBoxLayout(dialog);
    QLabel* label = new QLabel("Enter your 25-word mnemonic seed phrase:", dialog);
    QTextEdit* mnemonicEdit = new QTextEdit(dialog);
    mnemonicEdit->setPlaceholderText("Enter 25 words separated by spaces...");
    mnemonicEdit->setMinimumHeight(100);
    
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    QPushButton* okBtn = new QPushButton("Recover", dialog);
    QPushButton* cancelBtn = new QPushButton("Cancel", dialog);
    buttonLayout->addWidget(okBtn);
    buttonLayout->addWidget(cancelBtn);
    
    layout->addWidget(label);
    layout->addWidget(mnemonicEdit);
    layout->addLayout(buttonLayout);
    
    connect(okBtn, &QPushButton::clicked, dialog, &QDialog::accept);
    connect(cancelBtn, &QPushButton::clicked, dialog, &QDialog::reject);
    
    if (dialog->exec() != QDialog::Accepted) {
      delete dialog;
      return;
    }
    
    QString mnemonic = mnemonicEdit->toPlainText().trimmed();
    delete dialog;
    
    if (mnemonic.isEmpty()) {
      QMessageBox::warning(this, "Invalid Seed", "Mnemonic seed cannot be empty.");
      return;
    }

    // Ask for restore height (optional, default to 0 which scans from beginning)
    bool ok3 = false;
    QString heightStr = QInputDialog::getText(this, "Restore Height", 
      "Enter block height to restore from (0 to scan from beginning):", 
      QLineEdit::Normal, "0", &ok3);
    
    uint64_t restoreHeight = 0;
    if (ok3 && !heightStr.isEmpty()) {
      restoreHeight = heightStr.toULongLong(&ok3);
      if (!ok3) {
        QMessageBox::warning(this, "Invalid Height", "Restore height must be a valid number.");
        return;
      }
    }

    // Show progress
    QMessageBox::information(this, "Recovering Wallet", 
      "Recovering wallet from seed phrase. This may take a few moments...");

    // Use wallet manager to recover
    bool success = m_walletManager->recoverWallet(password, walletPath, mnemonic, restoreHeight);
    
    if (!success) {
      QMessageBox::critical(this, "Recovery Failed", "Failed to recover wallet: " + mnemonic);
      return;
    }

    // Update UI
    m_hasWallet = true;
    m_walletAddress = m_walletManager->getAddress();
    m_walletAddressDisplay->setText(m_walletAddress);
    m_copyAddressBtn->setEnabled(true);
    m_rescanWalletBtn->setEnabled(true);
    m_showPrivateKeyBtn->setEnabled(true);
    
    // Save wallet path
    QSettings settings("QSFCoin", "QuantumSafeWallet");
    settings.setValue("wallet_path", walletPath);
    
    QMessageBox::information(this, "Wallet Recovered", 
      "Wallet successfully recovered!\n\nAddress: " + m_walletAddress + 
      "\n\nRescanning blockchain to update balance...");
  }

  void MainWindow::onCopyAddress()
  {
    if (m_hasWallet) {
        QApplication::clipboard()->setText(m_walletAddress);
        QMessageBox::information(this, "Address Copied", "Wallet address copied to clipboard!");
    }
  }

  void MainWindow::onShowPrivateKey()
  {
    if (!m_hasWallet) {
      QMessageBox::warning(this, "No Wallet", "Please create a wallet first.");
      return;
    }

    // Load wallet path to fetch seed and keys for display
    QSettings settings("QSFCoin", "QuantumSafeWallet");
    const QString walletPath = settings.value("wallet_path", "").toString();
    if (walletPath.isEmpty()) {
      QMessageBox::information(this, "Wallet", "Address:\n" + m_walletAddress + "\n\nSecret spend key:\n" + m_walletPrivateKey);
      return;
    }

    // Ask for password to open
    bool ok = false;
    QString password = QInputDialog::getText(this, "Wallet Password", "Enter wallet password:", QLineEdit::Password, "", &ok);
    if (!ok) return;

    auto mgr = qsf::WalletManagerFactory::getWalletManager();
    qsf::NetworkType net = qsf::MAINNET;  // Force mainnet
    qsf::Wallet* w = mgr->openWallet(walletPath.toStdString(), password.toStdString(), net);
    if (!w || w->status() != qsf::Wallet::Status_Ok) {
      QMessageBox::critical(this, "Wallet Error", "Failed to open wallet: " + QString::fromStdString(w ? w->errorString() : std::string("unknown")));
      if (w) mgr->closeWallet(w, false);
      return;
    }

    const std::string seed = w->seed();
    const std::string spend = w->secretSpendKey();
    const std::string view = w->secretViewKey();
    mgr->closeWallet(w, false);

    QString details = QString(
      "Address:\n%1\n\nSeed phrase:\n%2\n\nSecret spend key:\n%3\n\nSecret view key:\n%4")
      .arg(m_walletAddress)
      .arg(QString::fromStdString(seed))
      .arg(QString::fromStdString(spend))
      .arg(QString::fromStdString(view));

    QMessageBox::information(this, "Wallet Secrets", details);
  }

  void MainWindow::onRescanWallet()
  {
    if (!m_hasWallet) {
      QMessageBox::warning(this, "No Wallet", "Please create a wallet first.");
      return;
    }

    if (!m_daemonRunning) {
      QMessageBox::warning(this, "Daemon Not Running", "Please start the daemon first before rescanning the wallet.");
      return;
    }

    if (!m_walletManager) {
      QMessageBox::warning(this, "Wallet Manager", "Wallet manager is not available.");
      return;
    }

    // Ask for confirmation
    auto res = QMessageBox::question(this,
                                     "Rescan Wallet",
                                     "This will rescan the wallet from block height 0, which may take some time.\n\n"
                                     "This is useful when:\n"
                                     "• The wallet's refresh-from-block-height setting is higher than the daemon's height\n"
                                     "• You want to ensure all transactions are detected\n\n"
                                     "Continue?",
                                     QMessageBox::Yes | QMessageBox::No,
                                     QMessageBox::No);
    
    if (res == QMessageBox::Yes) {
      m_miningLog->append("[INFO] 🔄 Manual wallet rescan requested from block height 0");
      m_walletManager->rescanBlockchainFromZero();
    }
  }

  void MainWindow::onNetworkChanged(int index)
  {
    qsf::NetworkType newNetwork = static_cast<qsf::NetworkType>(index);
    if (newNetwork == m_currentNetwork)
      return;

    m_currentNetwork = newNetwork;
    
    // Keep the configured daemon URL - don't override with seed node info
    const NetworkConfig& config = m_networkConfigs[m_currentNetwork];
    qDebug() << "Network changed to" << config.name << "- Using configured daemon URL:" << config.daemonUrl;
    
    updateNetworkConfig();
    
    // Clear existing wallet when switching networks
    if (m_hasWallet) {
      m_hasWallet = false;
      m_walletAddress.clear();
      m_walletPrivateKey.clear();
      m_walletAddressDisplay->setText("No wallet generated");
      m_walletAddressDisplay->setStyleSheet("background-color: #1a1a1a; border: 1px solid #404040; padding: 8px; border-radius: 4px; font-family: monospace; color: #666666;");
      m_copyAddressBtn->setEnabled(false);
    m_rescanWalletBtn->setEnabled(false);
      m_showPrivateKeyBtn->setEnabled(false);
      m_miningLog->append(QString("[INFO] Switched to %1 network - wallet cleared").arg(m_networkConfigs[m_currentNetwork].name));
    }
    
    // Reconnect ZMQ on network change
    if (m_zmqClient) {
      m_miningLog->append("[INFO] 🔌 Reconnecting ZMQ for network change...");
      m_zmqClient->disconnect();
      if (!m_customZmqEndpoints.isEmpty()) {
        m_zmqClient->connectUsingConfigured(m_customZmqEndpoints, config::ZMQ_RPC_DEFAULT_PORT);
      } else {
        m_zmqClient->connect(m_currentNetwork);
      }
    }

    // Check server status (HTTP fallback) for new network
    onCheckServerStatus();
    
    // Save network selection
    saveSettings();
  }

  void MainWindow::updateNetworkConfig()
  {
    const NetworkConfig& config = m_networkConfigs[m_currentNetwork];
    
    // Update network label
    m_networkLabel->setText(config.name);
    
    // Update daemon URL
    m_daemonUrlEdit->setText(config.daemonUrl);
    
    // Update pool URL (if present)
    m_poolAddressEdit->setText(config.poolUrl);
    
    // Update mining log
    m_miningLog->append("[INFO] Switched to " + config.name + " network");
    m_miningLog->append("[INFO] Daemon URL: " + config.daemonUrl);
    if (!m_customZmqEndpoints.isEmpty()) {
      m_miningLog->append("[INFO] ZMQ endpoints from config:");
      for (const auto &ep : m_customZmqEndpoints) {
        m_miningLog->append("[INFO]   - " + ep);
      }
    }
  }

  QString MainWindow::getNetworkDaemonUrl() const
  {
    return m_networkConfigs[m_currentNetwork].daemonUrl;
  }

  QString MainWindow::getNetworkPoolUrl() const
  {
    return m_networkConfigs[m_currentNetwork].poolUrl;
  }

  void MainWindow::onCheckServerStatus()
  {
    // Check for local daemon first, then fall back to remote if needed
    checkDaemonStatus();
  }

  void MainWindow::onServerStatusReply(QNetworkReply* reply)
  {
    bool serverOnline = false;
    QString statusText = "Disconnected";
    QString statusColor = "#dc3545"; // Red
    
    if (reply->error() == QNetworkReply::NoError) {
      QByteArray data = reply->readAll();
      QJsonParseError parseError;
      QJsonDocument jsonResponse = QJsonDocument::fromJson(data, &parseError);
      
      if (parseError.error == QJsonParseError::NoError) {
        QJsonObject response = jsonResponse.object();
        if (response.contains("result")) {
          serverOnline = true;
          statusText = "Connected";
          statusColor = "#28a745"; // Green
          
          // Update block height if available
          QJsonObject result = response["result"].toObject();
          if (result.contains("height")) {
            int height = result["height"].toInt();
            m_blockHeightLabel->setText(QString::number(height));
          }
        }
      }
    } else {
      // Log specific error for debugging
      qDebug() << "Server status check failed:" << reply->errorString();
      qDebug() << "URL:" << reply->url().toString();
    }
    
    // Update connection status
    m_connectionLabel->setText(statusText);
    if (serverOnline) {
      m_connectionLabel->setStyleSheet("color: #00d4aa; font-weight: bold;");
    } else {
      m_connectionLabel->setStyleSheet("color: #ff6b6b; font-weight: bold;");
    }
    
    // Update mining log
    if (serverOnline) {
      m_miningLog->append("[INFO] Server status: Online");
    } else {
      m_miningLog->append("[WARNING] Server status: Offline - " + reply->errorString());
    }
    
    reply->deleteLater();
  }

  void MainWindow::onStartMining()
  {
    // Prevent multiple rapid clicks
    if (m_isMining || m_miningActive) {
      m_miningLog->append("[WARNING] Mining is already in progress");
      return;
    }
    
    // Reset mining statistics when starting
    resetMiningStatistics();
    
    // Disable button immediately to prevent multiple clicks
    m_startMiningBtn->setEnabled(false);
    
    // Get wallet address
    QString walletAddress = m_walletAddressEdit->text().trimmed();
    if (walletAddress.isEmpty()) {
      m_miningLog->append("[ERROR] ❌ No wallet address specified for mining");
      QMessageBox::critical(this, "Mining Error", "Please enter a wallet address for mining");
      m_startMiningBtn->setEnabled(true); // Re-enable button
      return;
    }
    
    m_walletAddress = walletAddress;
    m_miningThreads = m_configuredThreads > 0 ? m_configuredThreads : m_threadsSpinBox->value();
    
    m_miningLog->append("[INFO] 🚀 Starting stand-alone mining...");
    m_miningLog->append("[INFO] 📍 Wallet address: " + walletAddress);
    m_miningLog->append("[INFO] 🔧 Threads: " + QString::number(m_miningThreads));
    
    // Force ZMQ reconnection to local daemon before starting mining
    if (m_zmqClient && m_zmqClient->isConnected()) {
      m_miningLog->append("[INFO] 🔄 Reconnecting ZMQ to local daemon for mining...");
      m_zmqClient->disconnect();
      QTimer::singleShot(500, [this]() {
        bool connected = m_zmqClient->connect("127.0.0.1", 18072);
        if (connected) {
          m_miningLog->append("[INFO] ✅ Connected to local daemon ZMQ for mining");
        } else {
          m_miningLog->append("[WARNING] Failed to connect to local ZMQ, will retry...");
          // Retry local connection after a delay
          QTimer::singleShot(1000, [this]() {
            if (m_zmqClient && !m_zmqClient->isConnected()) {
              m_miningLog->append("[INFO] 🔄 Retrying ZMQ connection to local daemon for mining...");
              bool connected = m_zmqClient->connect("127.0.0.1", 18072);
              if (connected) {
                m_miningLog->append("[INFO] ✅ Connected to local daemon ZMQ for mining on retry");
              } else {
                m_miningLog->append("[WARNING] Still failed to connect to local ZMQ, mining stats may not update");
              }
            }
          });
        }
      });
    }
    
    // Use the new stand-alone mining approach
    if (startStandaloneMining()) {
      m_miningActive = true;
      updateMiningStatus(true);
      updateMiningControls();
    } else {
      m_miningLog->append("[ERROR] ❌ Failed to start mining");
      QMessageBox::critical(this, "Mining Error", "Failed to start mining. Check the logs for details.");
    }
  }

  void MainWindow::startMiningWithDaemon(const QString& daemonUrl)
  {
    // Get wallet address
    QString walletAddress = m_walletAddressEdit->text().trimmed();
    if (walletAddress.isEmpty()) {
      m_miningLog->append("[ERROR] ❌ No wallet address specified for mining");
      QMessageBox::critical(this, "Mining Error", "Please enter a wallet address for mining");
      return;
    }
    
    // Get thread count
    int threads = m_configuredThreads > 0 ? m_configuredThreads : m_threadsSpinBox->value();
    m_miningLog->append("[INFO] 🚀 Starting mining via HTTP JSON-RPC...");
    m_miningLog->append("[INFO] 📍 Wallet address: " + walletAddress);
    m_miningLog->append("[INFO] 🔧 Threads: " + QString::number(threads));

    QNetworkAccessManager* nam = new QNetworkAccessManager(this);
    QNetworkRequest req(QUrl(daemonUrl + "/json_rpc"));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QJsonObject rpc; rpc["jsonrpc"] = "2.0"; rpc["id"] = "0"; rpc["method"] = "start_mining";
    QJsonObject p; p["miner_address"] = walletAddress; p["threads_count"] = threads; p["do_background_mining"] = false; p["ignore_battery"] = false; rpc["params"] = p;
    QNetworkReply* r = nam->post(req, QJsonDocument(rpc).toJson());
    connect(r, &QNetworkReply::finished, [this, r, nam, daemonUrl, walletAddress, threads]() {
      QByteArray payload = r->readAll();
      r->deleteLater(); nam->deleteLater();
      QJsonDocument jd = QJsonDocument::fromJson(payload);
      if (!jd.isObject()) { QMessageBox::critical(this, "Mining Error", "Invalid response from daemon."); return; }
      QJsonObject obj = jd.object();
      if (obj.contains("error") && obj["error"].isObject()) {
        QString msg = obj["error"].toObject()["message"].toString();
        m_miningLog->append("[ERROR] ❌ HTTP start_mining failed: " + msg);
        // If daemon does not expose mining RPC, try alternative approaches
        if (msg.contains("Method not found", Qt::CaseInsensitive) || msg.contains("Not found", Qt::CaseInsensitive)) {
          m_miningLog->append("[INFO] ℹ️ Daemon does not support start_mining RPC, trying alternative approach...");
          
          // Try to use standalone mining instead of daemon mining
          if (startStandaloneMining()) {
            m_miningLog->append("[INFO] ✅ Started standalone mining successfully");
            m_startTime = QDateTime::currentSecsSinceEpoch();
            updateMiningStatus(true);
            return;
          }
          
          // If standalone mining fails, offer to start local daemon
          if (tryConnectToExistingDaemon()) {
            QMessageBox::information(this, "Mining Not Supported",
                                     "The connected daemon (" + daemonUrl + ") does not expose start_mining via JSON-RPC.\n\n"
                                     "Standalone mining is not available. You can mine by starting the daemon with --start-mining and --mining-threads from the command line, or switch to a local daemon managed by the GUI.");
            return;
          }
          
          // Require a wallet address before offering to auto-start
          if (walletAddress.isEmpty()) {
            QMessageBox::warning(this, "Wallet Required", "Please create or enter a wallet address before starting mining.");
            return;
          }
          
          auto res = QMessageBox::question(this,
                                            "Start Local Daemon?",
                                            "The current daemon does not support mining via RPC.\n\n"
                                            "Would you like to start a local daemon with mining enabled now?",
                                            QMessageBox::Yes | QMessageBox::No,
                                            QMessageBox::Yes);
          if (res == QMessageBox::Yes) {
            m_walletAddress = walletAddress;
            m_miningThreads = threads;
            if (!autoStartLocalDaemon()) {
              QMessageBox::critical(this, "Mining Error", "Could not start local daemon for mining. Check if another daemon is running.");
            }
          }
        } else {
          QMessageBox::critical(this, "Mining Error", msg);
        }
        return;
      }
      m_miningLog->append("[INFO] ✅ Mining started via HTTP JSON-RPC");
      m_startTime = QDateTime::currentSecsSinceEpoch();
      updateMiningStatus(true);
      
      // Start the mining worker to monitor hash rate
      if (m_miningWorker && m_miningThread) {
        m_miningWorker->setDaemonUrl(daemonUrl);
        m_miningWorker->setWalletAddress(walletAddress);
        m_miningWorker->setThreads(threads);
        m_miningThread->start();
      }
    });
  }

  void MainWindow::onStopMining()
  {
    m_miningLog->append("[INFO] 🛑 Stopping mining...");
    
    // Use the new stand-alone mining approach
    stopStandaloneMining();
    
    m_isMining = false;
    m_miningActive = false;
    updateMiningStatus(false);
    updateMiningControls();
  }

  void MainWindow::onMiningModeChanged(const QString& mode)
  {
    if (mode == "Pool Mining")
    {
      m_poolAddressLabel->setVisible(true);
      m_poolAddressEdit->setVisible(true);
      m_daemonUrlEdit->setVisible(false);
      // Find the daemon URL label safely
      QGridLayout* layout = qobject_cast<QGridLayout*>(m_daemonUrlEdit->parentWidget()->layout());
      if (layout) {
        for (int i = 0; i < layout->rowCount(); ++i) {
          QLayoutItem* item = layout->itemAtPosition(i, 0);
          if (item && item->widget()) {
            QLabel* label = qobject_cast<QLabel*>(item->widget());
            if (label && label->text() == "Daemon URL:") {
              label->setVisible(false);
              break;
            }
          }
        }
      }
    }
    else // Solo Mining
    {
      m_poolAddressLabel->setVisible(false);
      m_poolAddressEdit->setVisible(false);
      m_daemonUrlEdit->setVisible(true);
      // Find the daemon URL label safely
      QGridLayout* layout = qobject_cast<QGridLayout*>(m_daemonUrlEdit->parentWidget()->layout());
      if (layout) {
        for (int i = 0; i < layout->rowCount(); ++i) {
          QLayoutItem* item = layout->itemAtPosition(i, 0);
          if (item && item->widget()) {
            QLabel* label = qobject_cast<QLabel*>(item->widget());
            if (label && label->text() == "Daemon URL:") {
              label->setVisible(true);
              break;
            }
          }
        }
      }
    }
  }

  void MainWindow::onMiningUpdate(double hashRate, uint64_t acceptedShares, uint64_t rejectedShares)
  {
    m_currentHashRate = hashRate;
    m_realAcceptedShares = acceptedShares;
    m_realRejectedShares = rejectedShares;
  }

  void MainWindow::onMiningError(const QString& error)
  {
    m_miningLog->append("[ERROR] " + error);
    QMessageBox::critical(this, "Mining Error", error);
  }

  void MainWindow::onUpdateStatistics()
  {
    // Check if daemon is actually responding (even if status says it's not)
    if (!m_daemonRunning) {
      QNetworkAccessManager* nam = new QNetworkAccessManager(this);
      QUrl url("http://127.0.0.1:18071/json_rpc");
      QNetworkRequest req(url);
      req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
      
      QJsonObject rpc;
      rpc["jsonrpc"] = "2.0";
      rpc["id"] = "0";
      rpc["method"] = "get_info";
      rpc["params"] = QJsonObject();
      
      QByteArray data = QJsonDocument(rpc).toJson();
      
      QNetworkReply* reply = nam->post(req, data);
      connect(reply, &QNetworkReply::finished, [this, reply, nam]() {
        if (reply->error() == QNetworkReply::NoError) {
          // Daemon is actually responding, update status
          m_daemonRunning = true;
          onDaemonStatusChanged(true);
          m_miningLog->append("[INFO] 🔄 Daemon status corrected - daemon is actually running");
          
          // Parse response to get mining info
          QJsonDocument responseDoc = QJsonDocument::fromJson(reply->readAll());
          if (responseDoc.isObject()) {
            QJsonObject response = responseDoc.object();
            if (response.contains("result")) {
              QJsonObject result = response["result"].toObject();
              if (result.contains("info")) {
                QJsonObject info = result["info"].toObject();
                if (info.contains("height")) {
                  int height = info["height"].toInt();
                  m_blockHeightLabel->setText(QString::number(height));
                  m_miningLog->append(QString("[INFO] 📊 Block height: %1").arg(height));
                }
                // Update difficulty
                if (info.contains("difficulty")) {
                  double difficulty = info["difficulty"].toDouble();
                  m_difficultyLabel->setText(QString::number(difficulty / 1000, 'f', 2) + "K");
                }
                // Block reward display removed
              }
            }
          }
        }
        reply->deleteLater();
        nam->deleteLater();
      });
    }
    
    // Update wallet balance if we have a wallet (but not too frequently to avoid spam)
    static int balanceUpdateCounter = 0;
    balanceUpdateCounter++;
    if (m_hasWallet && !m_walletAddress.isEmpty() && balanceUpdateCounter >= 3) {
      updateWalletBalance();
      balanceUpdateCounter = 0;
    }
    
    // Update mining statistics
    if (m_isMining && m_startTime > 0)
    {
      uint64_t uptime = QDateTime::currentSecsSinceEpoch() - m_startTime;
      QTime time(0, 0);
      time = time.addSecs(uptime);
      m_uptimeLabel->setText(time.toString("hh:mm:ss"));
    }
    
    // Update mining statistics from daemon via ZMQ
    if (m_zmqClient && m_zmqClient->isConnected()) {
      QJsonObject result = m_zmqClient->getInfo();
      if (!result.isEmpty()) {
        if (result.contains("info")) {
          QJsonObject info = result["info"].toObject();
          
          // Update block height
          if (info.contains("height")) {
            int height = info["height"].toInt();
            m_blockHeightLabel->setText(QString::number(height));
            static int lastHeight = -1;
            if (m_hasWallet && height != lastHeight) {
              lastHeight = height;
              refreshWalletBalance();
            }
          }
          
          // Update difficulty
          if (info.contains("difficulty")) {
            double difficulty = info["difficulty"].toDouble();
            m_difficultyLabel->setText(QString::number(difficulty / 1000, 'f', 2) + "K");
          }
          
          // Update network hashrate (calculated from difficulty)
          if (info.contains("difficulty")) {
            double difficulty = info["difficulty"].toDouble();
            double networkHashrate = difficulty * 60.0; // Assuming 60 second block time
            if (networkHashrate > 1000000000) {
              m_networkHashrateLabel->setText(QString::number(networkHashrate / 1000000000, 'f', 2) + " GH/s");
            } else if (networkHashrate > 1000000) {
              m_networkHashrateLabel->setText(QString::number(networkHashrate / 1000000, 'f', 2) + " MH/s");
            } else if (networkHashrate > 1000) {
              m_networkHashrateLabel->setText(QString::number(networkHashrate / 1000, 'f', 2) + " KH/s");
            } else {
              m_networkHashrateLabel->setText(QString::number(networkHashrate, 'f', 2) + " H/s");
            }
          }
          
          // Block reward is now calculated dynamically in updateMiningStatistics()
        }
      }
      
      // Also get mining status if we're mining
      if (m_isMining) {
        QJsonObject miningStatus = m_zmqClient->getMiningStatus();
        if (!miningStatus.isEmpty()) {
          double speed = miningStatus["speed"].toDouble();
          
          if (speed > 0 && speed != m_currentHashRate) {
            m_currentHashRate = speed;
            m_miningLog->append(QString("[INFO] Hash rate updated: %1 H/s").arg(speed, 0, 'f', 2));
          }
        }
      }
    }
    else if (m_isMining)
    {
      // Fallback: try to connect to ZMQ if we're mining but not connected
      if (!m_zmqConnecting) {
        m_miningLog->append("[INFO] Attempting to reconnect to ZMQ for mining statistics...");
        safeZmqConnect("127.0.0.1", 18072); // Default ZMQ port
      }
    }
    
    // Update hash rate display in mining tab
    if (m_currentHashRate > 1000000)
      m_hashRateLabel->setText(QString::number(m_currentHashRate / 1000000, 'f', 2) + " MH/s");
    else if (m_currentHashRate > 1000)
      m_hashRateLabel->setText(QString::number(m_currentHashRate / 1000, 'f', 2) + " KH/s");
    else
      m_hashRateLabel->setText(QString::number(m_currentHashRate, 'f', 2) + " H/s");
    
    // Also update hashrate in overview tab
    if (m_hashrateLabel) {
      if (m_currentHashRate > 1000000)
        m_hashrateLabel->setText(QString::number(m_currentHashRate / 1000000, 'f', 2) + " MH/s");
      else if (m_currentHashRate > 1000)
        m_hashrateLabel->setText(QString::number(m_currentHashRate / 1000, 'f', 2) + " KH/s");
      else
        m_hashrateLabel->setText(QString::number(m_currentHashRate, 'f', 2) + " H/s");
    }
    
    // Update mining statistics with real data
    updateMiningStatistics();
    
    // Update uptime
    if (m_uptimeLabel && m_isMining && m_startTime > 0) {
      uint64_t uptime = QDateTime::currentSecsSinceEpoch() - m_startTime;
      QTime time(0, 0);
      time = time.addSecs(uptime);
      m_uptimeLabel->setText(time.toString("hh:mm:ss"));
    }
    
    // Block reward display removed
    
    // Update peer count more frequently (every 2 seconds instead of every 5 seconds)
    static int peerCountCounter = 0;
    peerCountCounter++;
    if (peerCountCounter >= 2) {
      updatePeerCount();
      peerCountCounter = 0;
    }
  }

  void MainWindow::onUpdateMiningStatus()
  {
    // Lightweight mining status updates only
    if (m_isMining && m_startTime > 0)
    {
      uint64_t uptime = QDateTime::currentSecsSinceEpoch() - m_startTime;
      QTime time(0, 0);
      time = time.addSecs(uptime);
      m_uptimeLabel->setText(time.toString("hh:mm:ss"));
    }
  }

  bool MainWindow::safeZmqConnect(const QString& address, uint16_t port)
  {
    if (m_zmqConnecting || !m_zmqClient) {
      return false;
    }
    if (m_zmqClient->isConnected()) {
      return true;
    }
    m_zmqConnecting = true;
    bool result = m_zmqClient->connect(address, port);
    m_zmqConnecting = false;
    return result;
  }

  void MainWindow::updateWalletBalance()
  {
    if (!m_walletManager || !m_walletManager->hasWallet()) {
      m_balanceLabel->setText("0.00000000 QSF");
      return;
    }
    
    // Check if daemon is running first
    if (!m_daemonRunning) {
      m_balanceLabel->setText("0.00000000 QSF");
      m_miningLog->append("[WARNING] ⚠️ Cannot refresh balance - no daemon running");
      m_miningLog->append("[INFO] 💡 Click 'Start Daemon' to sync wallet with blockchain");
      return;
    }
    
    // Force refresh the balance
    m_walletManager->refreshBalance();
    
    // Also try to get balance from current state
    QString currentBalance = m_walletManager->getBalance();
    if (!currentBalance.isEmpty()) {
      m_balanceLabel->setText(currentBalance + " QSF");
    }
  }

  void MainWindow::refreshWalletBalance() {
    if (!m_walletManager || !m_walletManager->hasWallet()) {
      return;
    }
    m_walletManager->refreshBalance();
  }

  void MainWindow::updateMiningStatus(bool isMining)
  {
    m_isMining = isMining;
    m_startMiningBtn->setEnabled(!isMining);
    m_stopMiningBtn->setEnabled(isMining);
    m_miningModeCombo->setEnabled(!isMining);
    m_poolAddressEdit->setEnabled(!isMining);
    m_walletAddressEdit->setEnabled(!isMining);
    m_threadsSpinBox->setEnabled(!isMining);
    // Algorithm is fixed to RandomX, no need to enable/disable
  }

  void MainWindow::loadSettings()
  {
    QSettings settings("QSFCoin", "QuantumSafeWallet");
    
    m_poolAddressEdit->setText(settings.value("pool_address", "").toString());
    m_daemonUrlEdit->setText(settings.value("daemon_url", "http://127.0.0.1:18071").toString());
    // Load mining wallet address (prefer saved mining address, fallback to general wallet address)
    QString miningAddress = settings.value("mining_wallet_address", "").toString();
    if (miningAddress.isEmpty()) {
        miningAddress = settings.value("wallet_address", "").toString();
    }
    m_walletAddressEdit->setText(miningAddress);
    {
      int saved = settings.value("threads", QThread::idealThreadCount()).toInt();
#ifdef Q_OS_WIN
      int hw = QThread::idealThreadCount();
      int cap = qMax(1, hw / 2);
      m_threadsSpinBox->setValue(qBound(1, saved, cap));
#else
      m_threadsSpinBox->setValue(saved);
#endif
    }
    m_miningModeCombo->setCurrentIndex(settings.value("mining_mode", 0).toInt()); // Default to Solo
    // Algorithm is fixed to RandomX, no need to load from settings
    // Signature algorithm is fixed to dual XMSS+SPHINCS, no need to load from settings
    
    // Force mainnet; hide network switching in miner UI
    m_networkCombo->setCurrentIndex(0);
    
    // Load wallet if exists
    m_walletAddress = settings.value("wallet_address_saved", "").toString();
    m_walletPrivateKey = settings.value("wallet_private_key", "").toString();
    m_hasWallet = !m_walletAddress.isEmpty();
    
    // Load mining wallet address (for daemon config)
    m_miningWalletAddress = settings.value("mining_wallet_address", "").toString();
    
    if (m_hasWallet) {
        m_walletAddressDisplay->setText(m_walletAddress);
        m_walletAddressDisplay->setStyleSheet("background-color: #f8f9fa; border: 1px solid #dee2e6; padding: 8px; border-radius: 4px; font-family: monospace;");
        m_walletAddressEdit->setText(m_walletAddress);
        m_copyAddressBtn->setEnabled(true);
    m_rescanWalletBtn->setEnabled(true);
        m_showPrivateKeyBtn->setEnabled(true);
        
        // Try to open the wallet in the wallet manager
        QString walletPath = settings.value("wallet_path", "").toString();
        if (!walletPath.isEmpty() && m_walletManager) {
            // Prompt for password and open wallet on startup for smooth UX
            bool ok = false;
            QString password = QInputDialog::getText(this, "Wallet Password", "Enter wallet password:", QLineEdit::Password, "", &ok);
            if (ok && !password.isEmpty()) {
                if (!m_walletManager->openWallet(walletPath, password)) {
                    m_miningLog->append("[ERROR] ❌ Failed to open wallet on startup");
                }
            } else {
                // Fallback: set path so user actions can trigger later
                m_walletManager->setWalletPath(walletPath);
            }
        }
    }

    // Load quantum-safe keys
    bool quantumKeysGenerated = settings.value("quantum_keys_generated", false).toBool();
    if (quantumKeysGenerated) {
      QString algorithm = settings.value("quantum_keys_algorithm", "XMSS (Recommended)").toString();
      QString publicKey = settings.value("quantum_keys_public", "").toString();
      QString privateKey = settings.value("quantum_keys_private", "").toString();
      
      if (!publicKey.isEmpty() && !privateKey.isEmpty()) {
        QString keysText = QString(
          "🔐 Quantum-Safe Keys Generated\n"
          "==============================\n\n"
          "Algorithm: %1\n\n"
          "Public Key:\n%2\n\n"
          "Private Key:\n%3\n\n"
          "⚠️  IMPORTANT: Save these keys securely!\n"
          "The private key is required for signing transactions."
        ).arg(algorithm, publicKey, privateKey);
        
        m_generatedKeysDisplay->setText(keysText);
        m_quantumKeysStatusLabel->setText("✅ Quantum-safe keys generated");
        m_quantumKeysStatusLabel->setStyleSheet("color: #28a745; font-weight: bold;");
        // Signature algorithm is fixed to dual XMSS+SPHINCS
      }
    }
  }

  void MainWindow::saveSettings()
  {
    QSettings settings("QSFCoin", "QuantumSafeWallet");
    
    settings.setValue("pool_address", m_poolAddressEdit->text());
    settings.setValue("daemon_url", m_daemonUrlEdit->text());
    settings.setValue("wallet_address", m_walletAddressEdit->text());
    settings.setValue("threads", m_threadsSpinBox->value());
    settings.setValue("mining_mode", m_miningModeCombo->currentIndex());
    settings.setValue("algorithm", "RandomX"); // Fixed to RandomX
    settings.setValue("signature_algorithm", "dual_xmss_sphincs"); // Fixed to dual XMSS+SPHINCS
    settings.setValue("network", m_networkCombo->currentIndex());
    
    // Save wallet
    if (m_hasWallet) {
        settings.setValue("wallet_address_saved", m_walletAddress);
        settings.setValue("wallet_private_key", m_walletPrivateKey);
    }
    
    // Save mining wallet address
    if (!m_walletAddressEdit->text().isEmpty()) {
        settings.setValue("mining_wallet_address", m_walletAddressEdit->text());
    }
  }
  
  // Daemon Management Slots
  void MainWindow::onStartDaemon()
  {
    m_miningLog->append("[INFO] 🚀 Starting QSF Standalone daemon management...");
    
    // Ensure local config exists
    ensureLocalConfigExists();
    
    // Strategy 1: Try to connect to existing daemon first (most common case)
    if (detectAndHandleExistingDaemon()) {
      m_miningLog->append("[INFO] ✅ Connected to existing local daemon");
      m_daemonUrlEdit->setText("http://127.0.0.1:18071");
      onDaemonStatusChanged(true);
      m_connectionLabel->setText("Connected (Local)");
      m_connectionLabel->setStyleSheet("color: #00d4aa; font-weight: bold;");
      return;
    }
    
    // Strategy 2: Start our own local daemon only if no existing daemon is working
    if (!m_localDaemonProcess || m_localDaemonProcess->state() == QProcess::NotRunning) {
      m_miningLog->append("[INFO] 🚀 Starting new local daemon...");
      if (autoStartLocalDaemon()) {
        m_miningLog->append("[INFO] ✅ Local daemon started successfully");
        m_miningLog->append("[INFO] 🎯 Full mining and wallet control available");
        m_daemonUrlEdit->setText("http://127.0.0.1:18071");
        return;
      } else {
        m_miningLog->append("[ERROR] ❌ Failed to start local daemon");
      }
    }
    
    // Strategy 3: Fall back to remote daemon (for mining only, wallet will be limited)
    m_miningLog->append("[WARNING] ⚠️ Could not start local daemon, falling back to remote connection");
    m_miningLog->append("[WARNING] ⚠️ Wallet features will be limited without local daemon");
    connectToRemoteDaemon();
  }

  void MainWindow::onStopDaemon()
  {
    m_miningLog->append("[INFO] 🛑 Stopping standalone daemon...");

    // Stop mining first
    if (m_isMining) {
      onStopMining();
    }

#ifdef Q_OS_WIN
    // On Windows, if we started the daemon detached, we need to find and kill it
    // Use taskkill to find qsf.exe processes listening on port 18071
    m_miningLog->append("[INFO] 🛑 Stopping daemon on Windows...");
    QProcess killProcess;
    killProcess.start("taskkill", QStringList() << "/F" << "/IM" << "qsf.exe");
    killProcess.waitForFinished(3000);
    if (killProcess.exitCode() == 0) {
      m_miningLog->append("[INFO] ✅ Daemon process stopped");
    } else {
      QString output = killProcess.readAllStandardOutput();
      QString error = killProcess.readAllStandardError();
      if (output.contains("not found") || error.contains("not found")) {
        m_miningLog->append("[INFO] ℹ️ No daemon process found to stop");
      } else {
        m_miningLog->append("[WARNING] ⚠️ Could not stop daemon: " + error);
        m_miningLog->append("[INFO] 💡 You may need to stop qsf.exe manually from Task Manager");
      }
    }
    
    // Clear process reference on Windows
    if (m_localDaemonProcess) {
      m_localDaemonProcess->deleteLater();
      m_localDaemonProcess = nullptr;
    }
#else
    // Stop only the daemon we started (avoid killing system-wide services)
    if (m_localDaemonProcess && m_localDaemonProcess->state() != QProcess::NotRunning) {
      m_miningLog->append("[INFO] 🛑 Terminating local daemon process...");
      m_localDaemonProcess->terminate();
      if (!m_localDaemonProcess->waitForFinished(5000)) {
        m_miningLog->append("[INFO] 🛑 Force killing local daemon process...");
        m_localDaemonProcess->kill();
        m_localDaemonProcess->waitForFinished(2000);
      }
      m_miningLog->append("[INFO] ✅ Local daemon process stopped");
      
      // Clear the process reference but don't delete it yet
      m_localDaemonProcess->disconnect();
      m_localDaemonProcess = nullptr;
    } else {
      m_miningLog->append("[INFO] ℹ️ No local daemon process owned by GUI");
    }
#endif
    
    // Update status
    m_daemonRunning = false;
    onDaemonStatusChanged(false);
    
    // Disconnect wallet from daemon
    if (m_walletManager) {
      m_walletManager->setDaemonAddress("");
      m_miningLog->append("[INFO] 🔗 Wallet disconnected from daemon");
      
      // Update wallet status indicator
      if (m_walletStatusLabel) {
        m_walletStatusLabel->setText("❌ No Daemon");
        m_walletStatusLabel->setStyleSheet("color: #ff6b6b; font-weight: bold; font-size: 12px;");
      }
    }
    
    // Also check if there are any external daemons running
    QProcess checkProcess;
    checkProcess.start("pgrep", QStringList() << "-f" << "qsf.*daemon|qsf.*rpc|qsf.*18071|qsf.*18081");
    checkProcess.waitForFinished(1000);
    
    if (checkProcess.exitCode() == 0) {
      QString output = checkProcess.readAllStandardOutput().trimmed();
      if (!output.isEmpty()) {
        m_miningLog->append("[INFO] ℹ️ External daemon processes still running (PIDs: " + output + ")");
        m_miningLog->append("[INFO] 💡 These are not controlled by this GUI instance");
      }
    }
  }

  void MainWindow::onQuickSend()
  {
    QDialog dlg(this);
    dlg.setWindowTitle("Quick Send");
    dlg.setFixedSize(420, 240);
    // Dark mode styling for dialog and controls
    dlg.setStyleSheet(
      "QDialog { background-color: #121212; color: #e0e0e0; }"
      "QLabel { color: #e0e0e0; }"
      "QLineEdit, QTextEdit { background-color: #1e1e1e; color: #ffffff; border: 1px solid #333333; padding: 6px; border-radius: 4px; }"
      "QPushButton { background-color: #2a2a2a; color: #ffffff; border: 1px solid #3a3a3a; padding: 6px 10px; border-radius: 4px; }"
      "QPushButton:hover { background-color: #333333; }"
      "QGroupBox { border: 1px solid #333333; margin-top: 12px; }"
      "QGroupBox::title { subcontrol-origin: margin; left: 8px; padding: 0 4px; }"
    );
    
    QVBoxLayout* layout = new QVBoxLayout(&dlg);
    
    QLabel* toLabel = new QLabel("To Address:");
    QLineEdit* toEdit = new QLineEdit();
    toEdit->setPlaceholderText("Enter recipient address");
    
    QLabel* amtLabel = new QLabel("Amount (QSF):");
    QLineEdit* amtEdit = new QLineEdit();
    amtEdit->setPlaceholderText("0.0");
    
    QHBoxLayout* btnLayout = new QHBoxLayout();
    QPushButton* send = new QPushButton("Send");
    QPushButton* cancel = new QPushButton("Cancel");
    
    btnLayout->addWidget(send);
    btnLayout->addWidget(cancel);
    
    layout->addWidget(toLabel);
    layout->addWidget(toEdit);
    layout->addWidget(amtLabel);
    layout->addWidget(amtEdit);
    layout->addLayout(btnLayout);
    
    QObject::connect(cancel, &QPushButton::clicked, &dlg, &QDialog::reject);
    QObject::connect(send,&QPushButton::clicked,&dlg,[&,this,toEdit,amtEdit](){
      const QString to = toEdit->text().trimmed();
      const QString amt = amtEdit->text().trimmed();
      if (!m_walletManager || !m_walletManager->hasWallet()) {
        QMessageBox::warning(&dlg, "Send", "No wallet loaded. Open or create a wallet first.");
        return;
      }
      if (to.isEmpty() || amt.isEmpty()) {
        QMessageBox::warning(&dlg, "Send", "Please enter recipient address and amount.");
        return;
      }
      QString txid, err;
      bool ok = m_walletManager->sendTransaction(to, amt, &txid, &err);
      if (!ok) {
        QMessageBox::critical(&dlg, "Send Failed", err.isEmpty() ? QString("Unknown error") : err);
        return;
      }
      QMessageBox::information(&dlg, "Send Success", txid.isEmpty() ? QString("Transaction submitted.") : QString("Transaction submitted.\nTXID: %1").arg(txid));
      dlg.accept();
    });
    
    dlg.exec();
  }

  void MainWindow::onQuickReceive()
  {
    QDialog dlg(this);
    dlg.setWindowTitle("Receive QSF");
    dlg.setMinimumSize(560, 700);

    // Dark mode styling for dialog and controls
    dlg.setStyleSheet(
      "QDialog { background-color: #121212; color: #e0e0e0; }"
      "QLabel { color: #e0e0e0; }"
      "QLineEdit, QTextEdit { background-color: #1e1e1e; color: #ffffff; border: 1px solid #333333; padding: 6px; border-radius: 4px; }"
      "QPushButton { background-color: #2a2a2a; color: #ffffff; border: 1px solid #3a3a3a; padding: 6px 10px; border-radius: 4px; }"
      "QPushButton:hover { background-color: #333333; }"
      "QGroupBox { border: 1px solid #333333; margin-top: 12px; }"
      "QGroupBox::title { subcontrol-origin: margin; left: 8px; padding: 0 4px; }"
    );

    QVBoxLayout* layout = new QVBoxLayout(&dlg);

    if (!m_hasWallet) {
      QLabel* lbl = new QLabel("No wallet yet. Generate or open one on the Overview tab.");
      lbl->setWordWrap(true);
      lbl->setAlignment(Qt::AlignCenter);
      layout->addWidget(lbl);
      QHBoxLayout* h = new QHBoxLayout();
      QPushButton* close = new QPushButton("Close");
      h->addStretch(); h->addWidget(close); h->addStretch();
      layout->addLayout(h);
      QObject::connect(close, &QPushButton::clicked, &dlg, &QDialog::accept);
      dlg.exec();
      return;
    }

    // Address and integrated address
    QLabel* addrLabel = new QLabel("Primary address:");
    QTextEdit* addrDisplay = new QTextEdit();
    addrDisplay->setReadOnly(true);
    addrDisplay->setFixedHeight(60);
    addrDisplay->setText(m_walletAddress);

    QHBoxLayout* addrBtnRow = new QHBoxLayout();
    QPushButton* copyAddrBtn = new QPushButton("Copy Address");
    QPushButton* makeIntegratedBtn = new QPushButton("Integrated Address…");
    addrBtnRow->addWidget(copyAddrBtn);
    addrBtnRow->addWidget(makeIntegratedBtn);

    // Request section
    QGroupBox* reqGroup = new QGroupBox("Payment request");
    QGridLayout* reqGrid = new QGridLayout(reqGroup);
    QLabel* amountLbl = new QLabel("Amount (QSF)");
    QLineEdit* amountEdit = new QLineEdit();
    QLabel* pidLbl = new QLabel("Payment ID (optional)");
    QLineEdit* pidEdit = new QLineEdit();
    QLabel* descLbl = new QLabel("Description (optional)");
    QLineEdit* descEdit = new QLineEdit();
    reqGrid->addWidget(amountLbl, 0, 0); reqGrid->addWidget(amountEdit, 0, 1);
    reqGrid->addWidget(pidLbl, 1, 0); reqGrid->addWidget(pidEdit, 1, 1);
    reqGrid->addWidget(descLbl, 2, 0); reqGrid->addWidget(descEdit, 2, 1);

    // QR code preview
    QLabel* qrLabel = new QLabel();
    qrLabel->setAlignment(Qt::AlignCenter);
    qrLabel->setMinimumSize(320, 320);
    qrLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    qrLabel->setStyleSheet("background-color: #1a1a1a; border: 1px solid #333333;");

    // Buttons
    QHBoxLayout* actions = new QHBoxLayout();
    QPushButton* genReqBtn = new QPushButton("Generate Request");
    QPushButton* copyUriBtn = new QPushButton("Copy URI");
    QPushButton* savePngBtn = new QPushButton("Save QR...");
    QPushButton* closeBtn = new QPushButton("Close");
    actions->addWidget(genReqBtn);
    actions->addWidget(copyUriBtn);
    actions->addWidget(savePngBtn);
    actions->addStretch();
    actions->addWidget(closeBtn);

    layout->addWidget(addrLabel);
    layout->addWidget(addrDisplay);
    layout->addLayout(addrBtnRow);
    layout->addWidget(reqGroup);
    layout->addWidget(qrLabel, 1);
    layout->addStretch();
    layout->addLayout(actions);

    // State holders
    QString currentAddress = m_walletAddress;
    QString currentUri;

    // Copy address
    QObject::connect(copyAddrBtn, &QPushButton::clicked, [this,&dlg,currentAddress]() {
      QApplication::clipboard()->setText(currentAddress);
      QMessageBox::information(&dlg, "Copied", "Address copied to clipboard.");
    });

    // Make integrated address
    QObject::connect(makeIntegratedBtn, &QPushButton::clicked, [this, &dlg, addrDisplay, &currentAddress]() {
      bool ok = false;
      QString pid = QInputDialog::getText(&dlg, "Integrated Address", "Payment ID (16 hex, leave empty for random):", QLineEdit::Normal, QString(), &ok);
      if (!ok) return;
      if (!m_walletManager) return;
      if (pid.isEmpty()) pid = QString::fromStdString(qsf::Wallet::genPaymentId());
      QString integrated = m_walletManager->makeIntegratedAddress(pid);
      if (integrated.isEmpty()) { QMessageBox::warning(&dlg, "Error", "Failed to create integrated address."); return; }
      currentAddress = integrated;
      addrDisplay->setText(integrated);
    });

    // Generate request and QR
    QObject::connect(genReqBtn, &QPushButton::clicked, [this,&dlg,amountEdit,pidEdit,descEdit,&currentAddress,qrLabel,&currentUri]() {
      if (!m_walletManager) return;
      QString err;
      currentUri = m_walletManager->makePaymentUri(currentAddress, pidEdit->text().trimmed(), amountEdit->text().trimmed(), descEdit->text().trimmed(), &err);
      if (currentUri.isEmpty()) { QMessageBox::warning(&dlg, "Request", err.isEmpty() ? QString("Failed to create request URI") : err); return; }
      // Build QR with qrcodegen
      try {
        const std::string data = currentUri.toStdString();
        qrcodegen::QrCode qr = qrcodegen::QrCode::encodeText(data.c_str(), qrcodegen::QrCode::Ecc::LOW);
        const int size = qr.getSize();
        const int border = 4;
        // Compute scale to fit current label size
        const int avail = qMin(qrLabel->width(), qrLabel->height());
        const int scale = qMax(2, (avail - border * 2) / qMax(21, size));
        const int imgSize = (size + border * 2) * scale;
        QImage img(imgSize, imgSize, QImage::Format_RGB32);
        img.fill(QColor("#121212"));
        QPainter p(&img);
        p.setPen(Qt::NoPen);
        p.setBrush(QBrush(Qt::white));
        for (int y = -border; y < size + border; ++y) {
          for (int x = -border; x < size + border; ++x) {
            bool on = (x >= 0 && x < size && y >= 0 && y < size) ? qr.getModule(x, y) : false;
            if (on) p.drawRect((x + border) * scale, (y + border) * scale, scale, scale);
          }
        }
        p.end();
        qrLabel->setPixmap(QPixmap::fromImage(img));
      } catch (const std::exception& e) {
        QMessageBox::warning(&dlg, "QR", QString("Failed to generate QR: %1").arg(e.what()));
      }
    });

    // Copy URI
    QObject::connect(copyUriBtn, &QPushButton::clicked, [this,&dlg,&currentUri]() {
      if (currentUri.isEmpty()) { QMessageBox::information(&dlg, "Copy URI", "Generate a request first."); return; }
      QApplication::clipboard()->setText(currentUri);
      QMessageBox::information(&dlg, "Copied", "Payment URI copied to clipboard.");
    });

    // Save QR as PNG
    QObject::connect(savePngBtn, &QPushButton::clicked, [this,&dlg,qrLabel]() {
      const QPixmap* pm = qrLabel->pixmap();
      if (!pm) { QMessageBox::information(&dlg, "Save QR", "Generate a request first."); return; }
      QString path = QFileDialog::getSaveFileName(&dlg, "Save QR Code", QDir::homePath() + "/qsf-request.png", "PNG Image (*.png)");
      if (path.isEmpty()) return;
      pm->save(path, "PNG");
      QMessageBox::information(&dlg, "Saved", "QR code saved.");
    });

    QObject::connect(closeBtn, &QPushButton::clicked, &dlg, &QDialog::accept);

    dlg.exec();
  }

  void MainWindow::updateDaemonStatus(bool running)
  {
    onDaemonStatusChanged(running);
  }

  void MainWindow::onDaemonStatusChanged(bool isRunning)
  {
    m_daemonRunning = isRunning;
    
    if (isRunning) {
      m_daemonStatusLabel->setText("✅ Running");
      m_daemonStatusLabel->setStyleSheet("color: #00d4aa; font-weight: bold;");
      m_startDaemonBtn->setEnabled(false);
      m_stopDaemonBtn->setEnabled(true);
      
      // Update wallet manager to use local daemon
      if (m_walletManager) {
        m_walletManager->setDaemonAddress("127.0.0.1:18071");
        m_walletManager->onDaemonStatusChanged(true);
        m_miningLog->append("[INFO] 🔗 Wallet connected to local daemon");
        
        // Update wallet status indicator
        if (m_walletStatusLabel) {
          m_walletStatusLabel->setText("✅ Connected");
          m_walletStatusLabel->setStyleSheet("color: #00d4aa; font-weight: bold; font-size: 12px;");
        }
      }
      
      // Enable mining only when a wallet is opened/ready
      bool walletReady = (m_walletManager && m_walletManager->hasWallet());
      m_startMiningBtn->setEnabled(walletReady);
      
      // Start peer count updates after a short delay
      QTimer::singleShot(1000, [this]() {
        updatePeerCount();
        m_miningLog->append("[INFO] Initial peer count update triggered");
      });
      
      // Force ZMQ to switch to the local daemon when it starts, even if already connected to seeds
      if (m_zmqClient) {
        m_miningLog->append("[INFO] 🔗 Switching ZMQ to local daemon (127.0.0.1:18072)...");
        m_zmqClient->disconnect();
        bool connected = m_zmqClient->connect("127.0.0.1", 18072);
        if (connected) {
          m_miningLog->append("[INFO] ✅ ZMQ connected to local daemon");
        } else {
          m_miningLog->append("[WARNING] Failed to connect to local ZMQ, trying network endpoints...");
          connected = m_zmqClient->connect(qsf::MAINNET);
          if (!connected) {
            m_miningLog->append("[WARNING] Failed to connect to any ZMQ endpoint, using HTTP fallback");
          }
        }
      }
      
      // Update the generated keys display with daemon status
      m_generatedKeysDisplay->setText(
        QString("🔐 Quantum-Safe Key Generation Status:\n\n"
        "✅ Daemon Running: Yes\n"
        "🌐 Network: %1\n"
        "🔗 Connection: Active\n"
        "⚡ Mining Ready: Yes\n"
        "💰 Wallet Ready: Yes\n\n"
        "💡 Your standalone daemon is ready for quantum-safe operations!").arg("Mainnet")
      );
    } else {
      m_daemonStatusLabel->setText("❌ Stopped");
      m_daemonStatusLabel->setStyleSheet("color: #ff6b6b; font-weight: bold;");
      m_startDaemonBtn->setEnabled(true);
      m_stopDaemonBtn->setEnabled(false);
      m_startMiningBtn->setEnabled(false);
      
      // Notify wallet manager that daemon stopped
      if (m_walletManager) {
        m_walletManager->onDaemonStatusChanged(false);
        m_walletManager->setDaemonAddress("");
      }
      
      // Disconnect ZMQ when daemon stops
      if (m_zmqClient && m_zmqClient->isConnected()) {
        m_zmqClient->disconnect();
      }
      
      m_generatedKeysDisplay->setText(
        QString("🔐 Quantum-Safe Key Generation Status:\n\n"
        "❌ Daemon Running: No\n"
        "🌐 Network: %1\n"
        "🔗 Connection: Inactive\n"
        "⚡ Mining Ready: No\n\n"
        "💡 Start the daemon to enable quantum-safe operations").arg("Mainnet")
      );
    }
  }

  void MainWindow::startMiningStatusMonitoring()
  {
    // Create a timer for mining status updates
    QTimer* statusTimer = new QTimer(this);
    connect(statusTimer, &QTimer::timeout, [this, statusTimer]() {
      // Check if ZMQ client is connected and mining is active
      if (m_zmqClient && m_zmqClient->isConnected()) {
        QJsonObject result = m_zmqClient->getMiningStatus();
        bool active = result["active"].toBool();
        double speed = result["speed"].toDouble();
        
        // Update mining status if it changed
        if (!active && m_isMining) {
          m_miningLog->append("[INFO] ⚠️ Mining stopped by daemon");
          updateMiningStatus(false);
        }
        
        // Update hash rate if it changed
        if (speed != m_currentHashRate) {
          m_currentHashRate = speed;
          m_hashRateLabel->setText(QString::number(speed, 'f', 2) + " H/s");
        }
        
        // Get additional mining statistics
        QJsonObject infoResult = m_zmqClient->getInfo();
        if (!infoResult.isEmpty() && infoResult.contains("info")) {
          QJsonObject info = infoResult["info"].toObject();
          
          // Update difficulty
          if (info.contains("difficulty")) {
            double difficulty = info["difficulty"].toDouble();
            m_difficultyLabel->setText(QString::number(difficulty / 1000, 'f', 2) + "K");
            m_miningLog->append(QString("[DEBUG] 📊 Difficulty updated: %1").arg(difficulty));
          }
          
          // Update block height
          if (info.contains("height")) {
            int height = info["height"].toInt();
            m_blockHeightLabel->setText(QString::number(height));
            m_miningLog->append(QString("[DEBUG] 📊 Block height updated: %1").arg(height));
          }
          
          // Update network hashrate (calculate from difficulty)
          if (info.contains("difficulty") && info.contains("target")) {
            double difficulty = info["difficulty"].toDouble();
            double target = info["target"].toDouble();
            if (target > 0) {
              double networkHashrate = difficulty / target; // Approximate network hashrate
              if (networkHashrate > 1000000000000) {
                m_networkHashrateLabel->setText(QString::number(networkHashrate / 1000000000000.0, 'f', 2) + " TH/s");
              } else if (networkHashrate > 1000000000) {
                m_networkHashrateLabel->setText(QString::number(networkHashrate / 1000000000.0, 'f', 2) + " GH/s");
              } else if (networkHashrate > 1000000) {
                m_networkHashrateLabel->setText(QString::number(networkHashrate / 1000000.0, 'f', 2) + " MH/s");
              } else {
                m_networkHashrateLabel->setText(QString::number(networkHashrate, 'f', 2) + " H/s");
              }
              m_miningLog->append(QString("[DEBUG] 📊 Network hashrate updated: %1").arg(networkHashrate));
            }
          }
          
          // Block reward is now calculated dynamically in updateMiningStatistics()
        } else {
          m_miningLog->append("[DEBUG] 📊 No mining stats data received from ZMQ");
        }
      } else {
        // Fallback to HTTP polling for mining status
        QNetworkAccessManager* nam = new QNetworkAccessManager(this);
        QNetworkRequest req(QUrl(m_daemonUrlEdit->text() + "/json_rpc"));
        req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        
        QJsonObject rpc;
        rpc["jsonrpc"] = "2.0";
        rpc["id"] = "0";
        rpc["method"] = "get_info";
        rpc["params"] = QJsonObject();
        
        QJsonDocument doc(rpc);
        QNetworkReply* r = nam->post(req, doc.toJson());
        connect(r,&QNetworkReply::finished,[this,r,nam](){
          if (r->error() == QNetworkReply::NoError) {
            QJsonDocument responseDoc = QJsonDocument::fromJson(r->readAll());
            if (responseDoc.isObject()) {
              QJsonObject response = responseDoc.object();
              if (response.contains("result")) {
                QJsonObject result = response["result"].toObject();
                if (result.contains("info")) {
                  QJsonObject info = result["info"].toObject();
                  
                  // Update difficulty
                  if (info.contains("difficulty")) {
                    double difficulty = info["difficulty"].toDouble();
                    m_difficultyLabel->setText(QString::number(difficulty / 1000, 'f', 2) + "K");
                  }
                  
                  // Update block height
                  if (info.contains("height")) {
                    int height = info["height"].toInt();
                    m_blockHeightLabel->setText(QString::number(height));
                  }
                  
                  // Update network hashrate (calculate from difficulty)
                  if (info.contains("difficulty") && info.contains("target")) {
                    double difficulty = info["difficulty"].toDouble();
                    double target = info["target"].toDouble();
                    if (target > 0) {
                      double networkHashrate = difficulty / target; // Approximate network hashrate
                      if (networkHashrate > 1000000000000) {
                        m_networkHashrateLabel->setText(QString::number(networkHashrate / 1000000000000.0, 'f', 2) + " TH/s");
                      } else if (networkHashrate > 1000000000) {
                        m_networkHashrateLabel->setText(QString::number(networkHashrate / 1000000000.0, 'f', 2) + " GH/s");
                      } else if (networkHashrate > 1000000) {
                        m_networkHashrateLabel->setText(QString::number(networkHashrate / 1000000.0, 'f', 2) + " MH/s");
                      } else {
                        m_networkHashrateLabel->setText(QString::number(networkHashrate, 'f', 2) + " H/s");
                      }
                    }
                  }
                  
                  // Block reward display removed
                }
              }
            }
          }
          r->deleteLater(); 
          nam->deleteLater(); 
        });
      }
    });
    
    statusTimer->start(2000); // Update every 2 seconds
  }

  void MainWindow::updatePeerCount()
  {
    m_miningLog->append("[DEBUG] Updating peer count via ZMQ...");
    
    // Try ZMQ first for real-time updates
    if (m_zmqClient && m_zmqClient->isConnected()) {
      QJsonObject result = m_zmqClient->getInfo();
      
      if (!result.isEmpty()) {
        QJsonObject info = result["info"].toObject();
        int incomingConnections = info["incoming_connections_count"].toInt();
        int outgoingConnections = info["outgoing_connections_count"].toInt();
        int totalConnections = incomingConnections + outgoingConnections;
        int height = info["height"].toInt();
        
        // Update peer count display
        QString peerText = QString("%1 (%2 in, %3 out)").arg(totalConnections).arg(incomingConnections).arg(outgoingConnections);
        m_peerCountLabel->setText(peerText);
        
        // Update connection status
        if (totalConnections > 0) {
          m_peerCountLabel->setStyleSheet("color: #00d4aa; font-weight: bold;");
          m_connectionLabel->setText("Connected");
          
          // Update daemon status if not already set
          if (!m_daemonRunning) {
            m_daemonStatusLabel->setText("Running");
            m_daemonRunning = true;
          }
          
          // Update block height
          if (height > 0) {
            m_blockHeightLabel->setText(QString::number(height));
          }
        } else {
          m_peerCountLabel->setStyleSheet("color: #ff6b6b; font-weight: bold;");
          m_connectionLabel->setText("Disconnected");
        }
        
        m_miningLog->append(QString("[DEBUG] ZMQ peer count: %1").arg(peerText));
        return;
      }
    }
    
    // Fallback to HTTP if ZMQ is not available
    updatePeerCountHttp();
  }

  void MainWindow::updatePeerCountHttp()
  {
    m_miningLog->append("[DEBUG] Updating peer count via HTTP...");
    
    QString daemonUrl = m_daemonUrlEdit->text();
    if (daemonUrl.isEmpty()) {
      m_peerCountLabel->setText("No URL");
      return;
    }
    
    QNetworkRequest request(QUrl(daemonUrl + "/json_rpc"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    QJsonObject rpc;
    rpc["jsonrpc"] = "2.0";
    rpc["id"] = "0";
    rpc["method"] = "get_info";
    rpc["params"] = QJsonObject();
    
    QJsonDocument doc(rpc);
    QByteArray data = doc.toJson();
    
    QNetworkReply* reply = m_networkManager->post(request, data);
    
    connect(reply, &QNetworkReply::finished, [this, reply]() {
      if (reply->error() != QNetworkReply::NoError) {
        m_peerCountLabel->setText("Error");
        m_miningLog->append(QString("[DEBUG] HTTP peer count error: %1").arg(reply->errorString()));
        reply->deleteLater();
        return;
      }
      
      QByteArray responseData = reply->readAll();
      reply->deleteLater();
      
      m_miningLog->append(QString("[DEBUG] HTTP response received, size: %1 bytes").arg(responseData.size()));
      
      if (responseData.isEmpty()) {
        m_peerCountLabel->setText("No Data");
        m_miningLog->append("[DEBUG] Empty HTTP response data");
        return;
      }
      
      QString responsePreview = QString::fromUtf8(responseData.left(100));
      m_miningLog->append(QString("[DEBUG] HTTP response preview: %1...").arg(responsePreview));
      
      QJsonParseError parseError;
      QJsonDocument responseDoc = QJsonDocument::fromJson(responseData, &parseError);
      
      if (parseError.error != QJsonParseError::NoError) {
        m_peerCountLabel->setText("Parse Error");
        m_miningLog->append(QString("[DEBUG] HTTP JSON parse error: %1").arg(parseError.errorString()));
        return;
      }
      
      QJsonObject response = responseDoc.object();
      m_miningLog->append(QString("[DEBUG] HTTP response object keys: %1").arg(response.keys().join(", ")));
      
      if (response.contains("result")) {
        QJsonObject result = response["result"].toObject();
        m_miningLog->append(QString("[DEBUG] HTTP result object keys: %1").arg(result.keys().join(", ")));
        
        if (result.contains("info")) {
          QJsonObject info = result["info"].toObject();
          int incomingConnections = info["incoming_connections_count"].toInt();
          int outgoingConnections = info["outgoing_connections_count"].toInt();
          int totalConnections = incomingConnections + outgoingConnections;
          int height = info["height"].toInt();
          
          m_miningLog->append(QString("[DEBUG] HTTP parsed connections: in=%1, out=%2, total=%3").arg(incomingConnections).arg(outgoingConnections).arg(totalConnections));
          
          QString peerText = QString("%1 (%2 in, %3 out)").arg(totalConnections).arg(incomingConnections).arg(outgoingConnections);
          
          if (totalConnections > 0) {
            m_peerCountLabel->setStyleSheet("color: #00d4aa; font-weight: bold;");
            
            // Update daemon status if not already set
            if (!m_daemonRunning) {
              m_daemonStatusLabel->setText("Running");
              m_miningLog->append("[INFO] Daemon status updated to Running");
            }
            
            m_connectionLabel->setText("Connected");
          } else {
            m_peerCountLabel->setStyleSheet("color: #ff6b6b; font-weight: bold;");
            m_connectionLabel->setText("Disconnected");
          }
          
          m_peerCountLabel->setText(peerText);
          m_miningLog->append(QString("[DEBUG] HTTP peer count updated: %1").arg(peerText));
          
          // Update block height if available
          if (height > 0) {
            m_blockHeightLabel->setText(QString::number(height));
            m_miningLog->append(QString("[DEBUG] HTTP block height updated: %1").arg(height));
          }
        } else {
          m_peerCountLabel->setText("RPC Error");
          m_miningLog->append("[DEBUG] HTTP RPC error in peer count update");
          
          if (response.contains("error")) {
            QJsonObject error = response["error"].toObject();
            m_miningLog->append(QString("[DEBUG] HTTP error details: %1").arg(QString::fromUtf8(QJsonDocument(error).toJson())));
          }
        }
      } else {
        m_peerCountLabel->setText("Unknown");
        m_miningLog->append("[DEBUG] Unknown HTTP response format");
      }
    });
    
    connect(reply, &QNetworkReply::errorOccurred, [this, reply](QNetworkReply::NetworkError error) {
      if (error == QNetworkReply::OperationCanceledError) {
        m_peerCountLabel->setText("Timeout");
      }
      reply->deleteLater();
    });
  }

  QString MainWindow::generateDefaultConfig() {
    return QString(
      "# QSF Daemon Configuration (Auto-generated)\n"
      "# This file was automatically created by the GUI miner\n\n"
      "# RPC Settings\n"
      "rpc-bind-ip=127.0.0.1\n"
      "rpc-bind-port=18071\n"
      "restricted-rpc=1\n"
      "\n"
      "# P2P Settings - must be public for peer connections\n"
      "p2p-bind-ip=0.0.0.0\n"
      "p2p-bind-port=18070\n"
      "public-node=1\n"
      "\n"
      "# ZMQ Settings for mining\n"
      "zmq-rpc-bind-ip=0.0.0.0\n"
      "zmq-rpc-bind-port=18072\n"
      "zmq-pub=tcp://0.0.0.0:18073\n"
      "\n"
      "# Logging\n"
      "log-level=1\n"
      "\n"
      "# Performance\n"
      "max-concurrency=1\n"
      "\n"
      "# Connection stability settings - more peers for better sync reliability\n"
      "out-peers=16\n"
      "in-peers=16\n"
      "limit-rate-up=8192\n"
      "limit-rate-down=32768\n"
      "\n"
      "# Blockchain sync settings\n"
      "block-sync-size=2048\n"
      "db-sync-mode=fast:async:250000000\n"
      "prune-blockchain=1\n"
      "\n"
      "# Network\n"
      "no-igd=1\n"
      "hide-my-port=0\n"
      "\n"
      "# Seed Nodes - priority connections for reliable sync\n"
      "add-priority-node=seeds.qsfchain.com:18070\n"
      "add-priority-node=seeds.qsfnetwork.co:18070\n"
      "add-priority-node=seeds.qsfcoin.org:18070\n"
      "add-priority-node=seeds.qsfcoin.com:18070\n"
    );
  }

  QString MainWindow::generateMinerGuiConfig() {
    return QString(
      "# QSF Miner GUI Configuration (Auto-generated)\n"
      "# This file is read by the GUI miner only\n\n"
      "# ZMQ endpoints should be tcp://host:port for immediate connectivity\n"
      "miner.zmq_endpoints=tcp://seeds.qsfchain.com:18072\n"
      "miner.zmq_endpoints=tcp://seeds.qsfnetwork.co:18072\n"
      "miner.zmq_endpoints=tcp://seeds.qsfcoin.org:18072\n"
      "miner.zmq_endpoints=tcp://seeds.qsfcoin.com:18072\n"
      "# Threads: 0=auto-detect\n"
      "miner.threads=0\n"
    );
  }

  void MainWindow::ensureLocalConfigExists() {
    // Use Windows-compatible config directory
#ifdef Q_OS_WIN
    QString configDir = QString::fromWCharArray(_wgetenv(L"PROGRAMDATA"));
    if (configDir.isEmpty()) configDir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    configDir = QDir::toNativeSeparators(configDir) + "\\quantumsafefoundation";
    QString daemonConf = m_localConfigPath.isEmpty() ? (QDir::toNativeSeparators(configDir + "\\qsf.local.conf")) : m_localConfigPath;
    QString minerConf = QDir::toNativeSeparators(configDir + "\\miner.conf");
#else
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/.quantumsafefoundation";
    QString daemonConf = m_localConfigPath.isEmpty() ? (configDir + "/qsf.local.conf") : m_localConfigPath;
    QString minerConf = configDir + "/miner.conf";
#endif
    // Use the same config path that the daemon will use (matches m_localConfigPath)
    
    QDir().mkpath(QFileInfo(daemonConf).absolutePath());
    
    // Check if default config exists and prefer it (may have user customizations)
#ifdef Q_OS_WIN
    QString defaultConf = QDir::toNativeSeparators(configDir + "\\qsf.conf");
#else
    QString defaultConf = configDir + "/qsf.conf";
#endif
    if (QFile::exists(defaultConf) && !QFile::exists(daemonConf)) {
      // Copy default config to local config location
      if (QFile::copy(defaultConf, daemonConf)) {
        if (m_miningLog) m_miningLog->append("[INFO] ✅ Using existing config from: " + defaultConf);
      } else {
        // If copy fails, generate new config
        QString content = generateDefaultConfig();
        QFile f(daemonConf);
        if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
          QTextStream s(&f);
          s << content;
          f.close();
          if (m_miningLog) m_miningLog->append("[INFO] ✅ Auto-generated daemon config: " + daemonConf);
        }
      }
    } else if (!QFile::exists(daemonConf)) {
      // No config exists, generate new one
      QString content = generateDefaultConfig();
      QFile f(daemonConf);
      if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream s(&f);
        s << content;
        f.close();
        if (m_miningLog) m_miningLog->append("[INFO] ✅ Auto-generated daemon config: " + daemonConf);
      }
    } else {
      // Config already exists, use it
      if (m_miningLog) m_miningLog->append("[INFO] ℹ️ Using existing daemon config: " + daemonConf);
    }
    if (!QFile::exists(minerConf)) {
      QString content = generateMinerGuiConfig();
      QFile f(minerConf);
      if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream s(&f);
        s << content;
        f.close();
        if (m_miningLog) m_miningLog->append("[INFO] ✅ Auto-generated miner GUI config: " + minerConf);
      }
    }
    // Load miner GUI config every run
    loadMinerConfigFromFile();
  }
  void MainWindow::loadMinerConfigFromFile() {
    // Use Windows-compatible config directory
#ifdef Q_OS_WIN
    QString configDir = QString::fromWCharArray(_wgetenv(L"PROGRAMDATA"));
    if (configDir.isEmpty()) configDir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    configDir = QDir::toNativeSeparators(configDir) + "\\quantumsafefoundation";
    QString configPath = QDir::toNativeSeparators(configDir + "\\miner.conf");
#else
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/.quantumsafefoundation";
    QString configPath = configDir + "/miner.conf";
#endif
    QFile file(configPath);
    if (!file.exists()) return;
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;
    QTextStream in(&file);
    m_customZmqEndpoints.clear();
    m_configuredThreads = 0;
    m_configuredDaemonUrl.clear();
    while (!in.atEnd()) {
      QString line = in.readLine().trimmed();
      if (line.startsWith("#") || line.isEmpty()) continue;
      if (line.startsWith("miner.zmq_endpoints=") || line.startsWith("seed_endpoints=")) {
        QString val = line.mid(QString("miner.zmq_endpoints=").length()).trimmed();
        if (line.startsWith("seed_endpoints=")) {
          val = line.mid(QString("seed_endpoints=").length()).trimmed();
        }
        for (const QString &ep : val.split(',', Qt::SkipEmptyParts)) {
          QString e = ep.trimmed();
          // Support tcp:// URIs or host:port
          if (e.startsWith("tcp://")) {
            m_customZmqEndpoints << e;
          } else {
            m_customZmqEndpoints << e; // host:port handled later
          }
        }
      } else if (line.startsWith("miner.threads=") || line.startsWith("threads=")) {
        QString val = line.startsWith("threads=")
                      ? line.mid(QString("threads=").length()).trimmed()
                      : line.mid(QString("miner.threads=").length()).trimmed();
        bool ok=false; int t = val.toInt(&ok);
        if (ok && t >= 0) m_configuredThreads = t;
      } else if (line.startsWith("local_daemon_rpc=")) {
        QString val = line.mid(QString("local_daemon_rpc=").length()).trimmed();
        if (!val.isEmpty()) m_configuredDaemonUrl = val;
      }
    }
    file.close();

    // If config only contains seed labels, append default host fallbacks so GUI can connect immediately
    if (!m_customZmqEndpoints.isEmpty()) {
      bool allSeeds = std::all_of(m_customZmqEndpoints.begin(), m_customZmqEndpoints.end(), [](const QString &s){ return s.startsWith("_seed._tcp."); });
      if (allSeeds) {
        m_customZmqEndpoints << "seeds.qsfchain.com:18072"
                              << "seed2.qsfchain.com:18072"
                              << "seeds.qsfcoin.com:18072"
                              << "seeds.qsfcoin.org:18072"
                              << "seeds.qsfnetwork.co:18072";
      }
    }
    if (m_miningLog) {
      if (!m_customZmqEndpoints.isEmpty()) {
        m_miningLog->append("[INFO] Loaded ZMQ endpoints from qsf.conf:");
        for (const auto &ep : m_customZmqEndpoints) m_miningLog->append("[INFO]   - " + ep);
      }
      if (m_configuredThreads == 0) {
        m_miningLog->append("[INFO] Threads configured: auto");
      } else if (m_configuredThreads > 0) {
        m_miningLog->append("[INFO] Threads configured: " + QString::number(m_configuredThreads));
      }
    }
    applyMinerConfigToUi();
  }

  void MainWindow::applyMinerConfigToUi() {
    if (!m_configuredDaemonUrl.isEmpty()) {
      m_daemonUrlEdit->setText(m_configuredDaemonUrl);
    }
    if (m_configuredThreads > 0) {
      m_threadsSpinBox->setValue(m_configuredThreads);
    } else {
#ifdef Q_OS_WIN
      int hw = QThread::idealThreadCount();
      int cap = qMax(1, hw / 2);
      m_threadsSpinBox->setValue(cap);
#else
      m_threadsSpinBox->setValue(QThread::idealThreadCount());
#endif
    }
    // If endpoints are specified as tcp:// URIs, connect immediately
    if (m_zmqClient && !m_customZmqEndpoints.isEmpty()) {
      for (const auto &ep : m_customZmqEndpoints) {
        if (ep.startsWith("tcp://")) {
          if (m_zmqClient->connectUri(ep)) {
            if (m_miningLog) m_miningLog->append("[INFO] ✅ ZMQ connected to " + ep);
            break;
          }
        }
      }
    }
  }

  bool MainWindow::detectAndHandleExistingDaemon() {
    // First, try to connect to existing daemon without asking questions
    if (tryConnectToExistingDaemon()) {
      m_miningLog->append("[INFO] ✅ Connected to existing daemon");
      return true;
    }
    
    // Only check for existing processes if we can't connect
    QProcess checkProcess;
    checkProcess.start("pgrep", QStringList() << "-f" << "qsf.*18071|qsf.*18072|qsf.*18070");
    checkProcess.waitForFinished(1000);
    
    if (checkProcess.exitCode() == 0) {
      QString output = checkProcess.readAllStandardOutput().trimmed();
      if (!output.isEmpty()) {
        QStringList pids = output.split('\n', Qt::SkipEmptyParts);
        m_miningLog->append("[INFO] 🔍 Found existing QSF daemon processes (PIDs: " + pids.join(", ") + ")");
        
        // Check if user has already made a choice in settings
        QSettings settings;
        QString daemonChoice = settings.value("daemon_management_choice", "").toString();
        
        if (daemonChoice.isEmpty()) {
          // Try to connect to the existing daemon first before asking
          if (tryConnectToExistingDaemon()) {
            m_miningLog->append("[INFO] ✅ Successfully connected to existing daemon, no need to restart");
            return true;
          }
          
          auto res = QMessageBox::question(this,
                                          "Existing Daemon Found",
                                          QString("Found existing QSF daemon processes (PIDs: %1), but they don't respond to RPC calls.\n\n"
                                                 "Do you want to close them and start a fresh daemon?\n"
                                                 "This will ensure full control over the daemon.\n\n"
                                                 "Your choice will be remembered for future sessions.").arg(pids.join(", ")),
                                          QMessageBox::Yes | QMessageBox::No,
                                          QMessageBox::No); // Default to No to be less intrusive
          
          // Remember user's choice
          settings.setValue("daemon_management_choice", res == QMessageBox::Yes ? "close_existing" : "use_existing");
          daemonChoice = res == QMessageBox::Yes ? "close_existing" : "use_existing";
        }
        
        if (daemonChoice == "close_existing") {
          m_miningLog->append("[INFO] 🛑 Closing existing daemon processes...");
          for (const QString& pid : pids) {
            QProcess killProcess;
            killProcess.start("kill", QStringList() << "-TERM" << pid);
            killProcess.waitForFinished(2000);
            
            // If SIGTERM didn't work, try SIGKILL
            if (killProcess.exitCode() != 0) {
              killProcess.start("kill", QStringList() << "-KILL" << pid);
              killProcess.waitForFinished(1000);
            }
          }
          
          // Wait a moment for processes to close
          QThread::msleep(2000);
          m_miningLog->append("[INFO] ✅ Existing daemon processes closed");
          return false; // Return false so we start a new daemon
        } else {
          m_miningLog->append("[INFO] ℹ️ Using existing daemon processes (user preference)");
          // Try to connect to existing daemon
          if (tryConnectToExistingDaemon()) {
            return true;
          }
        }
      }
    }
    
    return false; // No existing daemon found or connected
  }

  bool MainWindow::tryConnectToExistingDaemon() {
    QNetworkAccessManager* nam = new QNetworkAccessManager(this);
    QUrl url(m_daemonUrlEdit->text().trimmed());
    if (url.isEmpty() || !url.isValid()) url = QUrl("http://127.0.0.1:18071/json_rpc");
    if (url.path().isEmpty()) url.setPath("/json_rpc");
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    QJsonObject rpc;
    rpc["jsonrpc"] = "2.0";
    rpc["id"] = "0";
    rpc["method"] = "get_info";
    rpc["params"] = QJsonObject();
    
    QJsonDocument doc(rpc);
    QNetworkReply* reply = nam->post(req, doc.toJson());
    
    QEventLoop loop;
    connect(nam, &QNetworkAccessManager::finished, &loop, &QEventLoop::quit);
    
    QTimer::singleShot(3000, &loop, &QEventLoop::quit); // 3 second timeout
    loop.exec();
    
    bool success = (reply->error() == QNetworkReply::NoError);
    if (success) {
      m_miningLog->append("[INFO] ✅ Found existing local daemon");
    }
    
    reply->deleteLater();
    nam->deleteLater();
    return success;
  }

  bool MainWindow::autoStartLocalDaemon() {
    // Global daemon state lock to prevent multiple starts
    if (m_daemonStartInProgress) {
      m_miningLog->append("[INFO] ℹ️ Daemon start already in progress, skipping");
      return false;
    }
    
    if (m_daemonPath.isEmpty()) {
      m_miningLog->append("[ERROR] ❌ QSF daemon path not set");
      return false;
    }
    
    if (!QFile::exists(m_daemonPath)) {
      m_miningLog->append("[ERROR] ❌ QSF daemon not found at: " + m_daemonPath);
      return false;
    }
    
    // Check if daemon is already running on expected ports
#ifndef Q_OS_WIN
    QProcess checkProcess;
    checkProcess.start("pgrep", QStringList() << "-f" << "qsf.*18071|qsf.*18072|qsf.*18070");
    checkProcess.waitForFinished(1000);
    
    if (checkProcess.exitCode() == 0) {
      QString output = checkProcess.readAllStandardOutput().trimmed();
      if (!output.isEmpty()) {
        m_miningLog->append("[INFO] ℹ️ Daemon process already running on expected ports (PIDs: " + output + ")");
        m_miningLog->append("[INFO] 💡 No need to start another daemon");
        return true;
      }
    }
#else
    // On Windows, check if daemon is already running by trying to connect to RPC
    QNetworkAccessManager* nam = new QNetworkAccessManager(this);
    QNetworkRequest req(QUrl("http://127.0.0.1:18071/json_rpc"));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QJsonObject rpc;
    rpc["jsonrpc"] = "2.0";
    rpc["id"] = "0";
    rpc["method"] = "get_info";
    rpc["params"] = QJsonObject();
    QNetworkReply* reply = nam->post(req, QJsonDocument(rpc).toJson());
    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    QTimer::singleShot(2000, &loop, &QEventLoop::quit);
    loop.exec();
    bool daemonRunning = (reply->error() == QNetworkReply::NoError);
    reply->deleteLater();
    nam->deleteLater();
    if (daemonRunning) {
      m_miningLog->append("[INFO] ℹ️ Daemon already running on port 18071");
      return true;
    }
#endif
    
    m_daemonStartInProgress = true;
    m_miningLog->append("[INFO] 🚀 Starting local QSF daemon...");
    m_miningLog->append("[INFO] 📍 Daemon path: " + m_daemonPath);
    // Ensure consistent ports for process and GUI URLs
    m_localRpcPort = 18071;
    m_localZmqPort = 18072;
    m_localP2pPort = 18070;
    
    m_localDaemonProcess = new QProcess(this);
    m_localDaemonProcess->setProgram(m_daemonPath);
    
    // Don't set working directory - let daemon use default behavior
    // This ensures it behaves the same as when run manually
    
    // Set up daemon arguments - match manual run behavior (no flags = use defaults)
    QStringList arguments;
    
    // Only specify --config-file if using a non-default location
    // When using default location, let daemon auto-discover it
    // This matches the behavior when you run "./qsf" or "./qsf.exe" with no flags
    QString defaultConfigPath;
#ifdef Q_OS_WIN
    QString baseDir = QString::fromWCharArray(_wgetenv(L"PROGRAMDATA"));
    if (baseDir.isEmpty()) baseDir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    baseDir = QDir::toNativeSeparators(baseDir) + "\\quantumsafefoundation";
    defaultConfigPath = QDir::toNativeSeparators(baseDir + "\\qsf.conf");
#else
    defaultConfigPath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/.quantumsafefoundation/qsf.conf";
#endif
    QFileInfo configInfo(m_localConfigPath);
    QString absoluteConfigPath = QDir::toNativeSeparators(configInfo.absoluteFilePath());
    QString normalizedDefaultPath = QDir::toNativeSeparators(defaultConfigPath);
    
    // Compare using canonical paths to handle Windows path case sensitivity
    if (QFileInfo(absoluteConfigPath).canonicalFilePath() != QFileInfo(normalizedDefaultPath).canonicalFilePath()) {
      // Using non-default config, explicitly specify it
      arguments << "--config-file" << absoluteConfigPath;
      m_miningLog->append("[DEBUG] 🔧 Using non-default config: " + absoluteConfigPath);
    } else {
      // Using default config location - let daemon auto-discover (matches manual "./qsf" behavior)
      m_miningLog->append("[DEBUG] 🔧 Using default config (auto-discovered): " + absoluteConfigPath);
    }
    
    // Explicitly set network type to prevent any confusion
    if (m_currentNetwork == qsf::TESTNET) {
      arguments << "--testnet";
    } else if (m_currentNetwork == qsf::STAGENET) {
      arguments << "--stagenet";
    }
    // MAINNET is default, no flag needed
    
    // Only override essential settings not in config file
    arguments << "--non-interactive";  // Required when running as child process (no TTY)
    // Let config file handle ports and other settings to avoid conflicts
    // Do not detach - we want to capture output and track lifecycle
    
    // Log the exact command being run for debugging
    QString cmdLine = m_daemonPath + " " + arguments.join(" ");
    m_miningLog->append("[DEBUG] 🔧 Starting daemon with command: " + cmdLine);
    m_miningLog->append("[DEBUG] 🔧 Config file will be: " + absoluteConfigPath);
    m_miningLog->append("[DEBUG] 🔧 Config exists: " + QString(QFile::exists(absoluteConfigPath) ? "YES" : "NO"));
    
    // Add mining parameters if wallet address is available
    if (!m_walletAddress.isEmpty() && m_miningThreads > 0) {
      arguments << "--start-mining" << m_walletAddress;
      arguments << "--mining-threads" << QString::number(m_miningThreads);
      m_miningLog->append("[INFO] 🎯 Starting daemon with mining enabled");
      m_miningLog->append("[INFO] 📍 Wallet address: " + m_walletAddress);
      m_miningLog->append("[INFO] 🔧 Threads: " + QString::number(m_miningThreads));
    }
    
    m_localDaemonProcess->setArguments(arguments);
    m_localDaemonProcess->setProcessChannelMode(QProcess::MergedChannels);
    
#ifdef Q_OS_WIN
    // On Windows, use startDetached to allow the daemon to show UAC/Firewall prompts
    // This is necessary because child processes can't show elevation prompts
    m_miningLog->append("[INFO] 🔐 Windows: Starting daemon in detached mode");
    m_miningLog->append("[INFO] 💡 Windows may show a permission/firewall prompt - please allow it");
    m_miningLog->append("[INFO] 💡 If the daemon stops, check Windows Firewall settings or run qsf.exe manually first");
    // Set working directory to daemon's directory for proper config file discovery
    QFileInfo daemonInfo(m_daemonPath);
    QString workingDir = daemonInfo.absolutePath();
    bool started = QProcess::startDetached(m_daemonPath, arguments, workingDir);
    if (!started) {
      m_miningLog->append("[ERROR] ❌ Failed to start daemon on Windows");
      m_miningLog->append("[ERROR] 💡 Try starting qsf.exe manually first, then open the GUI miner");
      m_miningLog->append("[ERROR] 💡 Or run the GUI miner as Administrator");
      m_daemonStartInProgress = false;
      return false;
    }
    m_miningLog->append("[INFO] ✅ Daemon process launched (checking if it's responding...)");
    
    // Since we detached, we can't track the process directly
    // Clear the process pointer - we'll detect it via network connection
    m_localDaemonProcess->deleteLater();
    m_localDaemonProcess = nullptr;
    
    // Wait a bit then check if daemon started successfully
    QTimer::singleShot(3000, this, [this]() {
      checkLocalDaemonReady();
    });
    
    // Point GUI to local daemon URL
    m_daemonUrl = QString("http://127.0.0.1:%1").arg(m_localRpcPort);
    m_daemonUrlEdit->setText(m_daemonUrl);
    return true;
#else
    // On Linux, keep process attached to capture output
    connect(m_localDaemonProcess, &QProcess::readyRead, [this]() {
      const QByteArray out = m_localDaemonProcess->readAll();
      if (!out.isEmpty()) {
        m_miningLog->append(QString::fromUtf8(out));
      }
    });
    
    connect(m_localDaemonProcess, &QProcess::started, [this]() {
      m_miningLog->append("[INFO] 🚀 Local daemon process started");
      m_miningLog->append("[INFO] ⏳ Waiting for daemon to initialize...");
      
      // Reset retry counter when starting fresh
      m_daemonRetryCount = 0;
      
      // Check if daemon is ready after a delay
      QTimer::singleShot(5000, this, &MainWindow::checkLocalDaemonReady);
    });
    
    connect(m_localDaemonProcess, &QProcess::errorOccurred, [this](QProcess::ProcessError error) {
      m_miningLog->append("[ERROR] ❌ Failed to start local daemon: " + QString::number(error));
      m_miningLog->append("[ERROR] ❌ Error details: " + m_localDaemonProcess->errorString());
      m_daemonStartInProgress = false; // Reset lock on error
      m_daemonRetryCount = 0; // Reset retry counter
    });
    
    connect(m_localDaemonProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), [this](int exitCode, QProcess::ExitStatus exitStatus) {
      if (exitCode != 0) {
        m_miningLog->append("[ERROR] ❌ Local daemon process exited with code: " + QString::number(exitCode));
        m_miningLog->append("[ERROR] ❌ Exit status: " + QString::number(exitStatus));
      } else {
        m_miningLog->append("[INFO] ✅ Local daemon process finished normally");
      }
      m_daemonStartInProgress = false;
      m_daemonRetryCount = 0; // Reset retry counter
    });
    
    m_localDaemonProcess->start();
    
    if (!m_localDaemonProcess->waitForStarted(10000)) {
      m_miningLog->append("[ERROR] ❌ Failed to start daemon process within 10 seconds");
      m_miningLog->append("[ERROR] ❌ Process error: " + m_localDaemonProcess->errorString());
      m_daemonStartInProgress = false; // Reset lock on failure
      m_daemonRetryCount = 0; // Reset retry counter
      return false;
    }
#endif
    
    // Point GUI to local daemon URL immediately
    m_daemonUrl = QString("http://127.0.0.1:%1").arg(m_localRpcPort);
    m_daemonUrlEdit->setText(m_daemonUrl);
    return true;
  }

  void MainWindow::checkLocalDaemonReady() {
    const int maxRetries = 20;
    
    QNetworkAccessManager* nam = new QNetworkAccessManager(this);
    QUrl url(m_daemonUrlEdit->text().trimmed());
    if (url.isEmpty() || !url.isValid()) url = QUrl("http://127.0.0.1:18071/json_rpc");
    if (url.path().isEmpty()) url.setPath("/json_rpc");
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    QJsonObject rpc;
    rpc["jsonrpc"] = "2.0";
    rpc["id"] = "0";
    rpc["method"] = "get_info";
    rpc["params"] = QJsonObject();
    
    QJsonDocument doc(rpc);
    connect(nam, &QNetworkAccessManager::finished, [this, nam, maxRetries](QNetworkReply* reply) {
      if (reply->error() == QNetworkReply::NoError) {
        m_miningLog->append("[INFO] ✅ Local daemon is ready!");
        updateDaemonStatus(true);
        
        // Reset daemon start lock on success
        m_daemonStartInProgress = false;
        
        // Reset retry counter on success
        m_daemonRetryCount = 0;
      } else {
        if (m_daemonRetryCount < maxRetries) {
          m_daemonRetryCount++;
          m_miningLog->append("[WARNING] ⚠️ Local daemon not ready yet, retrying... (" + QString::number(m_daemonRetryCount) + "/" + QString::number(maxRetries) + ")");
          QTimer::singleShot(2000, this, &MainWindow::checkLocalDaemonReady);
        } else {
          m_miningLog->append("[ERROR] ❌ Daemon did not become ready in time. Please check logs above.");
          m_daemonStartInProgress = false; // Reset lock on failure
          m_daemonRetryCount = 0; // Reset retry counter
        }
      }
      reply->deleteLater();
      nam->deleteLater();
    });
    
    nam->post(req, doc.toJson());
  }

  void MainWindow::connectToRemoteDaemon() {
    m_miningLog->append("[INFO] 🔄 Connecting to remote daemon: " + m_daemonUrl);
    
    QNetworkAccessManager* nam = new QNetworkAccessManager(this);
    QNetworkRequest req(QUrl(m_daemonUrl + "/json_rpc"));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    QJsonObject rpc;
    rpc["jsonrpc"] = "2.0";
    rpc["id"] = "0";
    rpc["method"] = "get_info";
    rpc["params"] = QJsonObject();
    
    QJsonDocument doc(rpc);
    connect(nam, &QNetworkAccessManager::finished, [this, nam](QNetworkReply* reply) {
      if (reply->error() == QNetworkReply::NoError) {
        m_miningLog->append("[INFO] ✅ Connected to remote daemon");
        updateDaemonStatus(true);
      } else {
        m_miningLog->append("[ERROR] ❌ Failed to connect to remote daemon: " + reply->errorString());
        updateDaemonStatus(false);
      }
      reply->deleteLater();
      nam->deleteLater();
    });
    
    nam->post(req, doc.toJson());
  }

  bool MainWindow::tryMiningOnDaemon(const QString& daemonUrl) {
    QNetworkAccessManager* nam = new QNetworkAccessManager(this);
    QNetworkRequest req(QUrl(daemonUrl + "/json_rpc"));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    QJsonObject rpc;
    rpc["jsonrpc"] = "2.0";
    rpc["id"] = "0";
    rpc["method"] = "start_mining";
    
    QJsonObject params;
    params["miner_address"] = m_walletAddress;
    params["threads_count"] = m_miningThreads;
    params["do_background_mining"] = false;
    params["ignore_battery"] = true;
    
    rpc["params"] = params;
    
    QJsonDocument doc(rpc);
    QNetworkReply* reply = nam->post(req, doc.toJson());
    
    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    
    QTimer::singleShot(5000, &loop, &QEventLoop::quit); // 5 second timeout
    loop.exec();
    
    bool success = (reply->error() == QNetworkReply::NoError);
    reply->deleteLater();
    nam->deleteLater();
    
    return success;
  }

  // New stand-alone mining methods
  bool MainWindow::startStandaloneMining() {
    if (m_walletAddress.isEmpty()) {
      m_miningLog->append("[ERROR] ❌ No wallet address specified for mining");
      QMessageBox::critical(this, "Mining Error", "Please enter a wallet address for mining");
      return false;
    }
    
    m_miningLog->append("[INFO] 🚀 Starting stand-alone mining...");
    m_miningLog->append("[INFO] 📍 Wallet address: " + m_walletAddress);
    m_miningLog->append("[INFO] 🔧 Threads: " + QString::number(m_miningThreads));
    
    // Check if we already have a local daemon running
    if (m_localDaemonProcess && m_localDaemonProcess->state() == QProcess::Running) {
      m_miningLog->append("[INFO] ✅ Local daemon is already running, starting mining via console command");
      const QString localUrl = "http://127.0.0.1:18071";
      // Prefer console command since local daemon may not expose start_mining over JSON-RPC
      QString cmd = QString("start_mining %1 %2\n").arg(m_walletAddress).arg(m_miningThreads);
      m_localDaemonProcess->write(cmd.toUtf8());
      // Enable hashrate logging on the daemon console if we own the process
      QTimer::singleShot(1000, [this]() {
        if (m_localDaemonProcess && m_localDaemonProcess->state() == QProcess::Running) {
          m_localDaemonProcess->write("show_hr\n");
        }
      });
      // Ensure ZMQ is connected to local daemon for stats
      if (m_zmqClient && !m_zmqClient->isConnected()) {
        m_zmqClient->connect("127.0.0.1", 18072);
      }
      // Start mining status worker thread if available
      if (m_miningWorker && m_miningThread) {
        m_miningWorker->setDaemonUrl(localUrl);
        m_miningWorker->setWalletAddress(m_walletAddress);
        m_miningWorker->setThreads(static_cast<uint32_t>(m_miningThreads));
        if (m_miningThread->isRunning()) {
          m_miningThread->quit();
          m_miningThread->wait(1000);
        }
        m_miningThread->start();
      }
      m_daemonSupportsMiningRpc = false;
      m_miningActive = true;
      updateMiningStatus(true);
      updateMiningControls();
      return true;
    }
    
    // First, check if daemon supports mining RPC
    if (m_daemonRunning && checkDaemonMiningSupport(m_daemonUrl)) {
      m_daemonSupportsMiningRpc = true;
      m_miningLog->append("[INFO] ✅ Daemon supports mining RPC, using daemon mining");
      startMiningWithDaemon(m_daemonUrl);
      return true;
    } else if (m_daemonRunning) {
      // Fallback: use console command on running local daemon
      m_miningLog->append("[INFO] ⚠️ start_mining JSON-RPC not available, using console command");
      if (m_localDaemonProcess && m_localDaemonProcess->state() == QProcess::Running) {
        QString cmd = QString("start_mining %1 %2\n").arg(m_walletAddress).arg(m_miningThreads);
        m_localDaemonProcess->write(cmd.toUtf8());
        QTimer::singleShot(1000, [this]() {
          if (m_localDaemonProcess && m_localDaemonProcess->state() == QProcess::Running) {
            m_localDaemonProcess->write("show_hr\\n");
          }
        });
        if (m_zmqClient && !m_zmqClient->isConnected()) {
          m_zmqClient->connect("127.0.0.1", 18072);
        }
        m_miningActive = true;
        updateMiningStatus(true);
        updateMiningControls();
        return true;
      }
    }
    
    // If daemon doesn't support mining RPC or isn't running, start local daemon with mining
    m_daemonSupportsMiningRpc = false;
    m_miningLog->append("[INFO] 🔄 Daemon doesn't support mining RPC, starting local daemon with mining");
    bool ok = startLocalDaemonWithMining();
    if (ok) {
      // Start the mining worker to monitor hash rate against local daemon
      if (m_miningWorker && m_miningThread) {
        const QString daemonUrl = "http://127.0.0.1:18071";
        m_miningWorker->setDaemonUrl(daemonUrl);
        m_miningWorker->setWalletAddress(m_walletAddress);
        m_miningWorker->setThreads(static_cast<uint32_t>(m_miningThreads));
        if (m_miningThread->isRunning()) {
          // If already running, stop and restart to rebind config cleanly
          m_miningThread->quit();
          m_miningThread->wait(1000);
        }
        m_miningThread->start();
      }
    }
    return ok;
  }

  void MainWindow::stopStandaloneMining() {
    m_miningLog->append("[INFO] 🛑 Stopping stand-alone mining...");
    
    if (m_daemonSupportsMiningRpc && m_daemonRunning) {
      // Try to stop mining via RPC first
      onStopMining();
    } else if (m_localDaemonProcess && m_localDaemonProcess->state() == QProcess::Running) {
      // Restart daemon without mining
      restartDaemonWithoutMining();
    }
    
    updateMiningStatus(false);
    m_miningActive = false;
  }

  bool MainWindow::findFreePorts(int& rpcPort, int& zmqPort, int& p2pPort) {
    // Start from default ports and find free ones
    int baseRpcPort = 38171;
    int baseZmqPort = 38172;
    int baseP2pPort = 38170;
    
    for (int offset = 0; offset < 100; offset++) {
      rpcPort = baseRpcPort + offset;
      zmqPort = baseZmqPort + offset;
      p2pPort = baseP2pPort + offset;
      
      if (isPortAvailable(rpcPort) && isPortAvailable(zmqPort) && isPortAvailable(p2pPort)) {
        m_miningLog->append(QString("[INFO] ✅ Found free ports: RPC=%1, ZMQ=%2, P2P=%3")
                           .arg(rpcPort).arg(zmqPort).arg(p2pPort));
        return true;
      }
    }
    
    m_miningLog->append("[ERROR] ❌ Could not find free ports for local daemon");
    return false;
  }

  bool MainWindow::isPortAvailable(int port) {
    QTcpServer server;
    return server.listen(QHostAddress::Any, port);
  }

  bool MainWindow::startLocalDaemonWithMining() {
    if (m_daemonPath.isEmpty()) {
      m_miningLog->append("[ERROR] ❌ QSF daemon not found");
      return false;
    }
    
    // Check if daemon is already running - prevent multiple starts
    if (m_localDaemonProcess && m_localDaemonProcess->state() == QProcess::Running) {
      m_miningLog->append("[INFO] ℹ️ Local daemon is already running, skipping start");
      return true;
    }
    
    // Check if there's already a daemon process running on the expected ports
    QProcess checkProcess;
    checkProcess.start("pgrep", QStringList() << "-f" << "qsf.*18071|qsf.*18072|qsf.*18070");
    checkProcess.waitForFinished(1000);
    
    if (checkProcess.exitCode() == 0) {
      QString output = checkProcess.readAllStandardOutput().trimmed();
      if (!output.isEmpty()) {
        m_miningLog->append("[INFO] ℹ️ Daemon process already running on expected ports (PIDs: " + output + ")");
        m_miningLog->append("[INFO] 💡 Mining should already be active");
        return true;
      }
    }
    
    // Ensure local config exists
    generateLocalConfig();
    
    m_miningLog->append("[INFO] 🚀 Starting local daemon with mining enabled...");
    // Ensure consistent ports
    m_localRpcPort = 18071;
    m_localZmqPort = 18072;
    m_localP2pPort = 18070;
    
    // Only recreate process if it doesn't exist or has finished
    if (!m_localDaemonProcess || m_localDaemonProcess->state() == QProcess::NotRunning) {
      if (m_localDaemonProcess) {
        m_localDaemonProcess->deleteLater();
      }
      m_localDaemonProcess = new QProcess(this);
      m_localDaemonProcess->setProgram(m_daemonPath);
      
      QStringList arguments;
      arguments << "--config-file" << m_localConfigPath;
      arguments << "--rpc-bind-port" << QString::number(m_localRpcPort);
      arguments << "--zmq-rpc-bind-port" << QString::number(m_localZmqPort);
      arguments << "--p2p-bind-port" << QString::number(m_localP2pPort);
      
      // Add mining parameters
      arguments << "--start-mining" << m_walletAddress;
      arguments << "--mining-threads" << QString::number(m_miningThreads);
      
      m_localDaemonProcess->setArguments(arguments);
      m_localDaemonProcess->setProcessChannelMode(QProcess::MergedChannels);
      
      connect(m_localDaemonProcess, &QProcess::readyRead, this, &MainWindow::handleDaemonOutput);
      connect(m_localDaemonProcess, &QProcess::errorOccurred, this, &MainWindow::handleDaemonError);
      connect(m_localDaemonProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
              this, &MainWindow::handleDaemonFinished);
      
      m_localDaemonProcess->start();
      // After start, ask daemon to output hashrate periodically
      QTimer::singleShot(3000, [this]() {
        if (m_localDaemonProcess && m_localDaemonProcess->state() == QProcess::Running) {
          m_localDaemonProcess->write("show_hr\n");
        }
      });
    }
    
    // Update daemon URL to point to local daemon
    m_daemonUrl = QString("http://127.0.0.1:%1").arg(m_localRpcPort);
    m_daemonUrlEdit->setText(m_daemonUrl);
    
    // Wait for daemon to be ready
    QTimer::singleShot(5000, this, &MainWindow::checkLocalDaemonReady);
    
    return true;
  }

  bool MainWindow::startLocalDaemonWithoutMining() {
    if (m_daemonPath.isEmpty()) {
      m_miningLog->append("[ERROR] ❌ QSF daemon not found");
      return false;
    }
    
    // Check if daemon is already running
    if (m_localDaemonProcess && m_localDaemonProcess->state() == QProcess::Running) {
      m_miningLog->append("[INFO] ℹ️ Local daemon is already running");
      return true;
    }
    
    // Ensure local config exists
    generateLocalConfig();
    
    m_miningLog->append("[INFO] 🚀 Starting local daemon without mining...");
    // Ensure consistent ports
    m_localRpcPort = 18071;
    m_localZmqPort = 18072;
    m_localP2pPort = 18070;
    
    // Only recreate process if it doesn't exist or has finished
    if (!m_localDaemonProcess || m_localDaemonProcess->state() == QProcess::NotRunning) {
      if (m_localDaemonProcess) {
        m_localDaemonProcess->deleteLater();
      }
      m_localDaemonProcess = new QProcess(this);
      m_localDaemonProcess->setProgram(m_daemonPath);
      
      QStringList arguments;
      arguments << "--config-file" << m_localConfigPath;
      arguments << "--rpc-bind-port" << QString::number(m_localRpcPort);
      arguments << "--zmq-rpc-bind-port" << QString::number(m_localZmqPort);
      arguments << "--p2p-bind-port" << QString::number(m_localP2pPort);
      
      m_localDaemonProcess->setArguments(arguments);
      m_localDaemonProcess->setProcessChannelMode(QProcess::MergedChannels);
      
      connect(m_localDaemonProcess, &QProcess::readyRead, this, &MainWindow::handleDaemonOutput);
      connect(m_localDaemonProcess, &QProcess::errorOccurred, this, &MainWindow::handleDaemonError);
      connect(m_localDaemonProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
              this, &MainWindow::handleDaemonFinished);
      
      m_localDaemonProcess->start();
    }
    
    // Update daemon URL to point to local daemon
    m_daemonUrl = QString("http://127.0.0.1:%1").arg(m_localRpcPort);
    m_daemonUrlEdit->setText(m_daemonUrl);
    
    // Wait for daemon to be ready
    QTimer::singleShot(5000, this, &MainWindow::checkLocalDaemonReady);
    
    return true;
  }

  void MainWindow::restartDaemonWithMining() {
    m_miningLog->append("[INFO] 🔄 Restarting daemon with mining enabled...");
    startLocalDaemonWithMining();
  }

  void MainWindow::restartDaemonWithoutMining() {
    m_miningLog->append("[INFO] 🔄 Restarting daemon without mining...");
    startLocalDaemonWithoutMining();
  }

  QString MainWindow::generateLocalConfig() {
    QString configContent = R"(
# QSF Local Daemon Config for GUI Miner
# Auto-generated - do not edit manually

# RPC Configuration (local only)
rpc-bind-ip=127.0.0.1
rpc-bind-port=)";
    
    configContent += QString::number(m_localRpcPort) + R"(
zmq-rpc-bind-ip=127.0.0.1
zmq-rpc-bind-port=)";
    
    configContent += QString::number(m_localZmqPort) + R"(
zmq-pub=tcp://127.0.0.1:)";
    
    configContent += QString::number(m_localZmqPort + 1) + R"(

# P2P Configuration (local only)
p2p-bind-ip=127.0.0.1
p2p-bind-port=)";
    
    configContent += QString::number(m_localP2pPort) + R"(
hide-my-port=0
in-peers=0
out-peers=8

# Performance Settings
prune-blockchain=1
db-sync-mode=fast:async:250000000
block-sync-size=2048
max-concurrency=1
log-level=1

# Network Settings
igd=disabled

# Seed Nodes (direct IP addresses to avoid DNS issues)
add-priority-node=45.77.187.237:18070
add-priority-node=209.222.30.191:18070
add-priority-node=66.135.5.130:18070
add-priority-node=45.76.127.197:18070
add-priority-node=45.63.123.244:18070

# Mining Configuration (if wallet address is available)
)";

    // Add mining configuration if wallet address is available
    if (!m_miningWalletAddress.isEmpty()) {
        configContent += QString("start-mining=%1\n").arg(m_miningWalletAddress);
        configContent += "mining-threads=auto\n";
    }
    
    // Ensure directory exists
    QDir configDir = QFileInfo(m_localConfigPath).dir();
    if (!configDir.exists()) {
      configDir.mkpath(".");
    }
    
    // Write config file
    QFile configFile(m_localConfigPath);
    if (configFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
      QTextStream out(&configFile);
      out << configContent;
      configFile.close();
      m_miningLog->append("[INFO] ✅ Generated local daemon config: " + m_localConfigPath);
    } else {
      m_miningLog->append("[ERROR] ❌ Failed to write local daemon config: " + m_localConfigPath);
    }
    
    return m_localConfigPath;
  }

  void MainWindow::handleDaemonOutput() {
    if (m_localDaemonProcess) {
      const QByteArray output = m_localDaemonProcess->readAll();
      if (!output.isEmpty()) {
        QString outputStr = QString::fromUtf8(output);
        QStringList lines = outputStr.split('\n', Qt::SkipEmptyParts);
        for (const QString& line : lines) {
          if (!line.trimmed().isEmpty()) {
            m_miningLog->append(line.trimmed());
            
            // Parse hashrate from daemon output (format: "hashrate: 127.6667")
            if (line.contains("hashrate:")) {
              QRegExp hashrateRegex("hashrate:\\s*(\\d+(?:\\.\\d+)?)");
              if (hashrateRegex.indexIn(line) != -1) {
                double hashrate = hashrateRegex.cap(1).toDouble();
                
                // Update all 3 hashrate displays
                m_hashrateLabel->setText(QString::number(hashrate, 'f', 2) + " H/s");
                m_hashRateLabel->setText(QString::number(hashrate, 'f', 2) + " H/s");
                m_currentHashRate = hashrate; // Update the current hashrate variable
                
                qDebug() << "Hashrate updated from daemon output:" << hashrate;
              }
            }
            
            // Auto-refresh wallet when new blocks are found
            if (line.contains("Found block") && m_hasWallet && !m_walletAddress.isEmpty()) {
              qDebug() << "New block found, refreshing wallet balance...";
              // Delay refresh slightly to let the block propagate
              QTimer::singleShot(2000, this, &MainWindow::refreshWalletBalance);
            }
          }
        }
      }
    }
  }

  void MainWindow::handleDaemonError(int error) {
    QString errorMsg;
    switch (error) {
      case QProcess::FailedToStart:
        errorMsg = "Failed to start daemon process";
        break;
      case QProcess::Crashed:
        errorMsg = "Daemon process crashed";
        break;
      case QProcess::Timedout:
        errorMsg = "Daemon process timed out";
        break;
      case QProcess::WriteError:
        errorMsg = "Failed to write to daemon process";
        break;
      case QProcess::ReadError:
        errorMsg = "Failed to read from daemon process";
        break;
      default:
        errorMsg = "Unknown daemon process error";
        break;
    }
    
    m_miningLog->append("[ERROR] ❌ " + errorMsg);
    updateDaemonStatus(false);
  }

  void MainWindow::handleDaemonFinished(int exitCode, int exitStatus) {
    if (exitStatus == QProcess::CrashExit) {
      m_miningLog->append("[ERROR] ❌ Local daemon crashed (code=" + QString::number(exitCode) + ")");
    } else {
      m_miningLog->append("[INFO] ℹ️ Local daemon exited (code=" + QString::number(exitCode) + ")");
    }
    
    updateDaemonStatus(false);
    
    // If we were mining, stop mining
    if (m_miningActive) {
      m_miningActive = false;
      updateMiningStatus(false);
    }
    
    // Clean up daemon process safely
    if (m_localDaemonProcess) {
      m_localDaemonProcess->disconnect();
      m_localDaemonProcess->deleteLater();
      m_localDaemonProcess = nullptr;
    }
  }

  bool MainWindow::checkDaemonMiningSupport(const QString& daemonUrl) {
    QNetworkAccessManager nam;
    QNetworkRequest req(QUrl(daemonUrl + "/json_rpc"));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    QJsonObject rpc;
    rpc["jsonrpc"] = "2.0";
    rpc["id"] = "0";
    rpc["method"] = "start_mining";
    rpc["params"] = QJsonObject();
    
    QJsonDocument doc(rpc);
    
    QEventLoop loop;
    QNetworkReply* reply = nam.post(req, doc.toJson());
    
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();
    
    if (reply->error() == QNetworkReply::NoError) {
      QJsonDocument response = QJsonDocument::fromJson(reply->readAll());
      QJsonObject obj = response.object();
      
      // If we get an error with "Method not found", the daemon doesn't support mining RPC
      if (obj.contains("error")) {
        QString errorMsg = obj["error"].toObject()["message"].toString();
        if (errorMsg.contains("Method not found", Qt::CaseInsensitive)) {
          reply->deleteLater();
          return false;
        }
      }
      
      reply->deleteLater();
      return true;
    }
    
    reply->deleteLater();
    return false;
  }

  void MainWindow::updateMiningControls() {
    bool canMine = !m_walletAddress.isEmpty() && m_daemonRunning;
    
    m_startMiningBtn->setEnabled(canMine && !m_miningActive);
    m_stopMiningBtn->setEnabled(m_miningActive);
    
    if (m_miningActive) {
      m_startMiningBtn->setText("⏸️ Pause Mining");
      m_stopMiningBtn->setText("⏹️ Stop Mining");
    } else {
      m_startMiningBtn->setText("🚀 Start Mining");
      m_stopMiningBtn->setText("⏹️ Stop Mining");
    }
  }

  double MainWindow::calculateCurrentBlockReward(uint64_t height, uint64_t alreadyGeneratedCoins) {
    // QSF block reward calculation based on cryptonote_basic_impl.cpp
    // This is a simplified version of the actual get_block_reward function
    
    const int target = 60; // DIFFICULTY_TARGET_V2 = 60 seconds
    const int target_minutes = target / 60;
    const int emission_speed_factor = 20 - (target_minutes - 1); // EMISSION_SPEED_FACTOR_PER_MINUTE = 20
    
    uint64_t money_supply = UINT64_MAX; // MONEY_SUPPLY
    uint64_t base_reward = (money_supply - alreadyGeneratedCoins) >> emission_speed_factor;
    
    // Minimum subsidy (FINAL_SUBSIDY_PER_MINUTE = 5 * 10^12)
    uint64_t final_subsidy = 5000000000000ULL * target_minutes;
    if (base_reward < final_subsidy) {
      base_reward = final_subsidy;
    }
    
    // Convert to QSF (divide by 10^12)
    return static_cast<double>(base_reward) / 1000000000000.0;
  }

  void MainWindow::updateMiningStatistics() {
    if (!m_zmqClient || !m_zmqClient->isConnected()) {
      return;
    }
    
    // Get current blockchain info
    QJsonObject info = m_zmqClient->getInfo();
    if (info.isEmpty()) {
      return;
    }
    
    QJsonObject infoObj = info["info"].toObject();
    if (infoObj.isEmpty()) {
      return;
    }
    
    // Update block height and calculate reward
    if (infoObj.contains("height")) {
      uint64_t currentHeight = infoObj["height"].toInt();
      
      // Calculate block reward if height changed
      if (currentHeight != m_lastBlockHeight) {
        uint64_t alreadyGeneratedCoins = 0; // This would need to be fetched from daemon
        m_currentBlockReward = calculateCurrentBlockReward(currentHeight, alreadyGeneratedCoins);
        m_lastBlockHeight = currentHeight;
        
        // Block reward display removed
      }
    }
    
    // Get mining status for real share tracking
    if (m_isMining) {
      QJsonObject miningStatus = m_zmqClient->getMiningStatus();
      if (!miningStatus.isEmpty()) {
        // Track mining start time for uptime calculation
        if (m_daemonMiningStartTime == 0 && miningStatus["active"].toBool()) {
          m_daemonMiningStartTime = QDateTime::currentSecsSinceEpoch();
        }
        
        // Update uptime based on daemon mining start time
        if (m_daemonMiningStartTime > 0 && m_uptimeLabel) {
          uint64_t uptime = QDateTime::currentSecsSinceEpoch() - m_daemonMiningStartTime;
          QTime time(0, 0);
          time = time.addSecs(uptime);
          m_uptimeLabel->setText(time.toString("hh:mm:ss"));
        }
        
        // For solo mining, we can't get real share counts from the daemon
        // But we can track blocks found (when height increases and we're mining)
        if (infoObj.contains("height")) {
          static uint64_t lastMiningHeight = 0;
          uint64_t currentHeight = infoObj["height"].toInt();
          
          if (lastMiningHeight > 0 && currentHeight > lastMiningHeight) {
            // Block was found while we were mining - increment accepted shares
            m_realAcceptedShares++;
            if (m_acceptedSharesLabel) {
              m_acceptedSharesLabel->setText(QString::number(m_realAcceptedShares));
            }
          }
          lastMiningHeight = currentHeight;
        }
      }
    }
  }

  void MainWindow::resetMiningStatistics() {
    m_realAcceptedShares = 0;
    m_realRejectedShares = 0;
    m_daemonMiningStartTime = 0;
    m_lastBlockHeight = 0;
    m_currentBlockReward = 0.0;
    
    if (m_acceptedSharesLabel) {
      m_acceptedSharesLabel->setText("0");
    }
    if (m_rejectedSharesLabel) {
      m_rejectedSharesLabel->setText("0");
    }
    if (m_uptimeLabel) {
      m_uptimeLabel->setText("00:00:00");
    }
    // Block reward display removed
  }
}
