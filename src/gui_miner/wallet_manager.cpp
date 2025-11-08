#include "wallet_manager.h"
#include <QDebug>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QCoreApplication>
#include "wallet/api/wallet2_api.h"

namespace qsf {

// Internal holder for libwallet pointers and network
struct GuiWalletManager::LibwalletState {
    qsf::WalletManager* manager = nullptr;
    qsf::Wallet* wallet = nullptr;
    qsf::NetworkType net = qsf::MAINNET;
};

class GuiWalletManager::LibwalletListener : public qsf::WalletListener {
public:
    explicit LibwalletListener(GuiWalletManager* owner) : m_owner(owner) {}
    void moneySpent(const std::string &, uint64_t) override {}
    void moneyReceived(const std::string &, uint64_t) override {}
    void unconfirmedMoneyReceived(const std::string &, uint64_t) override {}
    void newBlock(uint64_t) override {}
    void updated() override {}
    void refreshed() override {
        if (!m_owner) return;
        m_owner->m_isRefreshing = false;
        m_owner->updateCachedFieldsFromWallet(true);
    }
private:
    GuiWalletManager* m_owner;
};

GuiWalletManager::GuiWalletManager(QObject* parent)
    : QObject(parent)
    , m_hasWallet(false)
    , m_refreshTimer(nullptr)
    , m_autoRefresh(false)
    , m_refreshInterval(5000)
    , m_rescanQueued(false)
    , m_isRescanning(false)
    , m_isRefreshing(false)
    , m_autoRefreshWasEnabled(false)
    , m_rescanCompletedOnce(false)
    , m_blocksToUnlock(0)
    , m_timeToUnlock(0)
    , m_lib(new LibwalletState())
{
    m_refreshTimer = new QTimer(this);
    m_refreshTimer->setSingleShot(false);
    connect(m_refreshTimer, &QTimer::timeout, this, &GuiWalletManager::onRefreshTimer);
    m_listener.reset(new LibwalletListener(this));
}

GuiWalletManager::~GuiWalletManager()
{
    closeWallet();
}

bool GuiWalletManager::createWallet(const QString& password, const QString& walletPath)
{
    if (m_hasWallet) closeWallet();
    m_password = password;
    m_walletPath = walletPath;
    ensureWalletManager();
    if (!m_lib->manager) {
        emit error("Wallet manager not available");
        return false;
    }

    qsf::Wallet* w = m_lib->manager->createWallet(walletPath.toStdString(), password.toStdString(), "English", m_lib->net);
    if (!w) {
        emit error("Failed to create wallet");
        return false;
    }
    m_lib->wallet = w;
    m_lib->wallet->setListener(m_listener.get());
    // init daemon address
    setDaemonAddress(m_daemonAddress);
    // For new wallets, mark rescan not done yet
    m_rescanCompletedOnce = false;
    {
        QSettings s("QSFCoin", "QuantumSafeWallet");
        s.setValue(QString("wallet_rescan_done_%1").arg(m_walletPath), false);
    }
    updateCachedFieldsFromWallet(true);
    if (!m_rescanCompletedOnce) {
        rescanBlockchainFromZero();
    }
    return true;
}

bool GuiWalletManager::openWallet(const QString& walletPath, const QString& password)
{
    if (m_hasWallet) closeWallet();
    if (!QFile::exists(walletPath)) {
        emit error("Wallet file does not exist: " + walletPath);
        return false;
    }
    m_walletPath = walletPath;
    m_password = password;
    {
        QSettings s("QSFCoin", "QuantumSafeWallet");
        const QString key = QString("wallet_rescan_done_%1").arg(m_walletPath);
        m_rescanCompletedOnce = s.value(key, false).toBool();
    }

    ensureWalletManager();
    if (!m_lib->manager) {
        emit error("Wallet manager not available");
        return false;
    }
    qsf::Wallet* w = m_lib->manager->openWallet(walletPath.toStdString(), password.toStdString(), m_lib->net, 1, nullptr);
    if (!w) {
        emit error("Failed to open wallet");
        return false;
    }
    m_lib->wallet = w;
    m_lib->wallet->setListener(m_listener.get());
    setDaemonAddress(m_daemonAddress);
    // Start background refresh
    m_lib->wallet->setAutoRefreshInterval(m_refreshInterval);
    m_lib->wallet->startRefresh();
    updateCachedFieldsFromWallet(true);
    if (!m_rescanCompletedOnce) {
        rescanBlockchainFromZero();
    }
    return true;
}

bool GuiWalletManager::recoverWallet(const QString& password, const QString& walletPath, const QString& mnemonic, uint64_t restoreHeight)
{
    if (m_hasWallet) closeWallet();
    m_password = password;
    m_walletPath = walletPath;
    
    // Validate mnemonic seed
    if (mnemonic.trimmed().isEmpty()) {
        emit error("Mnemonic seed is empty");
        return false;
    }
    
    ensureWalletManager();
    if (!m_lib->manager) {
        emit error("Wallet manager not available");
        return false;
    }

    // Recover wallet using the mnemonic seed
    qsf::Wallet* w = m_lib->manager->recoveryWallet(
        walletPath.toStdString(),
        password.toStdString(),
        mnemonic.trimmed().toStdString(),
        m_lib->net,
        restoreHeight
    );
    
    if (!w) {
        emit error("Failed to recover wallet from mnemonic");
        return false;
    }
    
    m_lib->wallet = w;
    m_lib->wallet->setListener(m_listener.get());
    setDaemonAddress(m_daemonAddress);
    
    // Mark rescan as needed for recovered wallet
    m_rescanCompletedOnce = false;
    {
        QSettings s("QSFCoin", "QuantumSafeWallet");
        s.setValue(QString("wallet_rescan_done_%1").arg(m_walletPath), false);
    }
    
    updateCachedFieldsFromWallet(true);
    
    // Start rescan from the restore height
    if (!m_rescanCompletedOnce) {
        rescanBlockchainFromZero();
    }
    
    return true;
}

void GuiWalletManager::closeWallet()
{
    if (m_lib && m_lib->wallet && m_lib->manager) {
        m_lib->wallet->pauseRefresh();
        m_lib->manager->closeWallet(m_lib->wallet, true);
        m_lib->wallet = nullptr;
    }
    m_hasWallet = false;
    m_walletAddress.clear();
    m_balance.clear();
    
    emit walletClosed();
}

void GuiWalletManager::refreshBalance()
{
    if (!m_lib || !m_lib->wallet) return;
    m_isRefreshing = true;
    m_lib->wallet->refreshAsync();
}

bool GuiWalletManager::sweepUnmixableOutputs(QString* errorOut)
{
    if (errorOut) errorOut->clear();
    if (!m_lib || !m_lib->wallet) {
        if (errorOut) *errorOut = "No wallet loaded";
        return false;
    }
    
    try {
        // Create sweep transaction for unmixable outputs
        qsf::PendingTransaction* ptx = m_lib->wallet->createSweepUnmixableTransaction();
        
        if (!ptx) {
            if (errorOut) *errorOut = "Failed to create sweep transaction";
            return false;
        }
        
        const int status = ptx->status();
        if (status != qsf::PendingTransaction::Status_Ok) {
            if (errorOut) *errorOut = QString::fromStdString(ptx->errorString());
            m_lib->wallet->disposeTransaction(ptx);
            return false;
        }
        
        // Check if there are any transactions to commit
        if (ptx->txCount() == 0) {
            // No unmixable outputs found - this is not an error
            m_lib->wallet->disposeTransaction(ptx);
            if (errorOut) *errorOut = "No unmixable outputs found";
            return true; // Not an error - just no unmixable outputs
        }
        
        // Commit the sweep transaction
        const bool committed = ptx->commit();
        if (!committed) {
            if (errorOut) *errorOut = QString::fromStdString(ptx->errorString());
            m_lib->wallet->disposeTransaction(ptx);
            return false;
        }
        
        m_lib->wallet->disposeTransaction(ptx);
        
        // Trigger a balance refresh after sweeping
        refreshBalance();
        return true;
    } catch (...) {
        if (errorOut) *errorOut = "Exception occurred while sweeping unmixable outputs";
        return false;
    }
}

void GuiWalletManager::setAutoRefresh(bool enabled, int intervalMs)
{
    m_autoRefresh = enabled;
    m_refreshInterval = intervalMs;
    if (!m_lib || !m_lib->wallet) return;
    
    // Windows-specific: Use longer intervals for better stability
    #ifdef Q_OS_WIN
        int adjustedInterval = qMax(intervalMs, 10000); // Minimum 10 seconds on Windows
    #else
        int adjustedInterval = intervalMs;
    #endif
    
    m_lib->wallet->setAutoRefreshInterval(adjustedInterval);
    if (enabled) m_lib->wallet->startRefresh();
    else m_lib->wallet->pauseRefresh();
}

void GuiWalletManager::setDaemonAddress(const QString& daemonAddress)
{
    QString oldAddress = m_daemonAddress;
    m_daemonAddress = daemonAddress;
    if (m_lib && m_lib->wallet) {
        // sanitize to host:port
        QString a = m_daemonAddress.trimmed();
        if (a.startsWith("http://", Qt::CaseInsensitive)) a = a.mid(7);
        if (a.startsWith("https://", Qt::CaseInsensitive)) a = a.mid(8);
        int slash = a.indexOf('/');
        if (slash != -1) a = a.left(slash);
        const std::string addr = a.isEmpty() ? std::string("127.0.0.1:18071") : a.toStdString();
        m_lib->wallet->init(addr);
    }
}

void GuiWalletManager::rescanBlockchainFromZero()
{
    if (!m_hasWallet || !m_lib || !m_lib->wallet) return;
    if (m_rescanCompletedOnce) { refreshBalance(); return; }
    m_rescanQueued = false;
    m_autoRefreshWasEnabled = m_autoRefresh;
    m_isRescanning = true;
    m_lib->wallet->setRefreshFromBlockHeight(0);
    m_lib->wallet->rescanBlockchainAsync();
}

void GuiWalletManager::forceRefreshOnDaemonAvailable()
{
    if (!m_hasWallet || !m_lib || !m_lib->wallet) return;
    if (m_rescanQueued) rescanBlockchainFromZero();
    else refreshBalance();
}

void GuiWalletManager::onDaemonStatusChanged(bool daemonRunning)
{
    if (!daemonRunning) return;
    if (m_hasWallet) {
        if (!m_rescanCompletedOnce) rescanBlockchainFromZero();
        else forceRefreshOnDaemonAvailable();
    }
}

/* removed: process output/error handlers not used in libwallet mode */

void GuiWalletManager::onRefreshTimer()
{
    refreshBalance();
}

// removed: CLI-specific parseBalance and sendCommand

void GuiWalletManager::setWalletPath(const QString& walletPath)
{
    m_walletPath = walletPath;
    if (!walletPath.isEmpty()) { m_hasWallet = true; }
}

// removed: CLI-specific writePasswordFile helper

void GuiWalletManager::ensureWalletManager()
{
    if (!m_lib) m_lib.reset(new LibwalletState());
    if (!m_lib->manager) {
        m_lib->manager = qsf::WalletManagerFactory::getWalletManager();
    }
}

void GuiWalletManager::updateCachedFieldsFromWallet(bool emitSignals)
{
    if (!m_lib || !m_lib->wallet) return;
    const QString addr = QString::fromStdString(m_lib->wallet->address());
    const uint64_t balAtomic = m_lib->wallet->balance();
    const QString bal = displayAmount(balAtomic);
    
    // Update unlocked/locked balance
    uint64_t unlockedBalAtomic = m_lib->wallet->unlockedBalance();
    uint64_t lockedBalAtomic = (balAtomic > unlockedBalAtomic) ? (balAtomic - unlockedBalAtomic) : 0;
    m_unlockedBalance = displayAmount(unlockedBalAtomic);
    m_lockedBalance = displayAmount(lockedBalAtomic);
    
    // Get unlock time information (using internal wallet methods if available)
    // Note: The wallet API doesn't directly expose blocks_to_unlock/time_to_unlock,
    // so we'll calculate approximate values from unlock times in transactions
    m_blocksToUnlock = 0;
    m_timeToUnlock = 0;
    
    bool openedJustNow = false;
    if (!m_hasWallet) {
        m_hasWallet = true;
        openedJustNow = true;
    }
    if (m_walletAddress != addr) {
        m_walletAddress = addr;
        if (openedJustNow && emitSignals) emit walletOpened(m_walletAddress);
    }
    if (m_balance != bal) {
        m_balance = bal;
        if (emitSignals) emit balanceUpdated(m_balance);
    }
    if (m_isRescanning) {
        // assume rescan done once we got a refresh callback which updates balance
        m_isRescanning = false;
        m_isRefreshing = false;
        if (m_autoRefreshWasEnabled) m_refreshTimer->start(m_refreshInterval);
        m_rescanCompletedOnce = true;
        QSettings s("QSFCoin", "QuantumSafeWallet");
        s.setValue(QString("wallet_rescan_done_%1").arg(m_walletPath), true);
    }
}

QString GuiWalletManager::displayAmount(uint64_t atomic) const
{
    // Use libwallet formatter
    return QString::fromStdString(qsf::Wallet::displayAmount(atomic));
}

} // namespace qsf

namespace qsf {

bool GuiWalletManager::sendTransaction(const QString& toAddress, const QString& amountStr, QString* txidOut, QString* errorOut)
{
    if (txidOut) txidOut->clear();
    if (errorOut) errorOut->clear();
    if (!m_lib || !m_lib->wallet) { if (errorOut) *errorOut = "No wallet loaded"; return false; }

    // Basic validation
    const std::string addr = toAddress.trimmed().toStdString();
    if (!qsf::Wallet::addressValid(addr, m_lib->net)) {
        if (errorOut) *errorOut = "Invalid recipient address";
        return false;
    }

    // Parse amount using libwallet helper to atomic units
    uint64_t amountAtomic = 0;
    try {
        amountAtomic = qsf::Wallet::amountFromString(amountStr.trimmed().toStdString());
    } catch (...) {
        if (errorOut) *errorOut = "Invalid amount";
        return false;
    }
    if (amountAtomic == 0) { if (errorOut) *errorOut = "Amount must be greater than 0"; return false; }

    // Create transaction
    qsf::optional<uint64_t> amountOpt(amountAtomic);
    qsf::PendingTransaction* ptx = m_lib->wallet->createTransaction(
        addr, "", amountOpt, /*mixin*/ 0,
        qsf::PendingTransaction::Priority_Low, /*subaddr_account*/ 0, {});

    if (!ptx) { if (errorOut) *errorOut = "Failed to create transaction"; return false; }

    const int status = ptx->status();
    if (status != qsf::PendingTransaction::Status_Ok) {
        if (errorOut) *errorOut = QString::fromStdString(ptx->errorString());
        m_lib->wallet->disposeTransaction(ptx);
        return false;
    }

    // Commit transaction
    const bool committed = ptx->commit();
    if (!committed) {
        if (errorOut) *errorOut = QString::fromStdString(ptx->errorString());
        m_lib->wallet->disposeTransaction(ptx);
        return false;
    }

    // Extract txids
    const std::vector<std::string> ids = ptx->txid();
    if (!ids.empty() && txidOut) {
        *txidOut = QString::fromStdString(ids.front());
    }

    m_lib->wallet->disposeTransaction(ptx);

    // Trigger a balance refresh shortly after
    refreshBalance();
    return true;
}

} // namespace qsf

namespace qsf {

QString GuiWalletManager::makeIntegratedAddress(const QString& paymentId) const
{
    if (!m_lib || !m_lib->wallet) return QString();
    const std::string pid = paymentId.trimmed().toStdString();
    const std::string integrated = m_lib->wallet->integratedAddress(pid);
    return QString::fromStdString(integrated);
}

QString GuiWalletManager::makePaymentUri(const QString& address, const QString& paymentId, const QString& amountStr, const QString& description, QString* errorOut) const
{
    if (errorOut) errorOut->clear();
    if (!m_lib || !m_lib->wallet) { if (errorOut) *errorOut = "No wallet loaded"; return QString(); }
    uint64_t amountAtomic = 0;
    if (!amountStr.trimmed().isEmpty()) {
        try { amountAtomic = qsf::Wallet::amountFromString(amountStr.trimmed().toStdString()); }
        catch (...) { if (errorOut) *errorOut = "Invalid amount"; return QString(); }
    }
    std::string err;
    const std::string uri = m_lib->wallet->make_uri(
        address.trimmed().toStdString(),
        paymentId.trimmed().toStdString(),
        amountAtomic,
        description.trimmed().toStdString(),
        std::string(),
        err);
    if (!err.empty()) { if (errorOut) *errorOut = QString::fromStdString(err); return QString(); }
    return QString::fromStdString(uri);
}

QList<GuiWalletManager::TransactionInfo> GuiWalletManager::getTransactionHistory(QString* errorOut)
{
    QList<TransactionInfo> result;
    if (errorOut) errorOut->clear();
    if (!m_lib || !m_lib->wallet) {
        if (errorOut) *errorOut = "No wallet loaded";
        return result;
    }
    
    try {
        qsf::TransactionHistory* history = m_lib->wallet->history();
        if (!history) {
            if (errorOut) *errorOut = "Failed to get transaction history";
            return result;
        }
        
        history->refresh();
        int count = history->count();
        for (int i = 0; i < count; ++i) {
            qsf::TransactionInfo* ti = history->transaction(i);
            if (!ti) continue;
            
            TransactionInfo info;
            info.txid = QString::fromStdString(ti->hash());
            info.direction = (ti->direction() == qsf::TransactionInfo::Direction_In) ? "in" : "out";
            info.amount = displayAmount(ti->amount());
            info.fee = displayAmount(ti->fee());
            info.blockHeight = ti->blockHeight();
            info.confirmations = ti->confirmations();
            info.unlockTime = ti->unlockTime();
            info.timestamp = ti->timestamp();
            info.paymentId = QString::fromStdString(ti->paymentId());
            info.description = QString::fromStdString(ti->description());
            info.isPending = ti->isPending();
            info.isFailed = ti->isFailed();
            info.isCoinbase = ti->isCoinbase();
            
            // Get transfers
            const auto& transfers = ti->transfers();
            for (const auto& transfer : transfers) {
                info.transfers.push_back({QString::fromStdString(transfer.address), displayAmount(transfer.amount)});
            }
            
            result.append(info);
        }
    } catch (...) {
        if (errorOut) *errorOut = "Exception occurred while getting transaction history";
    }
    
    return result;
}

bool GuiWalletManager::rescanSpent(QString* errorOut)
{
    if (errorOut) errorOut->clear();
    if (!m_lib || !m_lib->wallet) {
        if (errorOut) *errorOut = "No wallet loaded";
        return false;
    }
    
    try {
        bool result = m_lib->wallet->rescanSpent();
        if (!result) {
            // Check for error status
            if (errorOut) *errorOut = "Rescan spent failed - check daemon connection";
        }
        return result;
    } catch (...) {
        if (errorOut) *errorOut = "Exception occurred while rescanning spent outputs";
        return false;
    }
}

bool GuiWalletManager::sweepAll(const QString& toAddress, QString* txidOut, QString* errorOut)
{
    if (txidOut) txidOut->clear();
    if (errorOut) errorOut->clear();
    if (!m_lib || !m_lib->wallet) {
        if (errorOut) *errorOut = "No wallet loaded";
        return false;
    }
    
    // Validate address
    const std::string addr = toAddress.trimmed().toStdString();
    if (!qsf::Wallet::addressValid(addr, m_lib->net)) {
        if (errorOut) *errorOut = "Invalid recipient address";
        return false;
    }
    
    try {
        // Create sweep transaction (amount = 0 means sweep all)
        qsf::optional<uint64_t> amountOpt; // Empty optional = sweep all
        qsf::PendingTransaction* ptx = m_lib->wallet->createTransaction(
            addr, "", amountOpt, /*mixin*/ 0,
            qsf::PendingTransaction::Priority_Low, /*subaddr_account*/ 0, {});
        
        if (!ptx) {
            if (errorOut) *errorOut = "Failed to create sweep transaction";
            return false;
        }
        
        const int status = ptx->status();
        if (status != qsf::PendingTransaction::Status_Ok) {
            if (errorOut) *errorOut = QString::fromStdString(ptx->errorString());
            m_lib->wallet->disposeTransaction(ptx);
            return false;
        }
        
        // Commit transaction
        const bool committed = ptx->commit();
        if (!committed) {
            if (errorOut) *errorOut = QString::fromStdString(ptx->errorString());
            m_lib->wallet->disposeTransaction(ptx);
            return false;
        }
        
        // Extract txids
        const std::vector<std::string> ids = ptx->txid();
        if (!ids.empty() && txidOut) {
            *txidOut = QString::fromStdString(ids.front());
        }
        
        m_lib->wallet->disposeTransaction(ptx);
        
        // Trigger a balance refresh
        refreshBalance();
        return true;
    } catch (...) {
        if (errorOut) *errorOut = "Exception occurred while sweeping all balance";
        return false;
    }
}

bool GuiWalletManager::sweepAllToSelf(QString* txidOut, QString* errorOut)
{
    // Sweep all to primary address
    return sweepAll(m_walletAddress, txidOut, errorOut);
}

QString GuiWalletManager::createSubaddress(uint32_t accountIndex, const QString& label, QString* errorOut)
{
    if (errorOut) errorOut->clear();
    if (!m_lib || !m_lib->wallet) {
        if (errorOut) *errorOut = "No wallet loaded";
        return QString();
    }
    
    try {
        m_lib->wallet->addSubaddress(accountIndex, label.toStdString());
        
        // Get the newly created subaddress
        size_t numSubaddresses = m_lib->wallet->numSubaddresses(accountIndex);
        if (numSubaddresses > 0) {
            // Get the last subaddress (the one we just created)
            QString address = QString::fromStdString(m_lib->wallet->address(accountIndex, static_cast<uint32_t>(numSubaddresses - 1)));
            return address;
        }
        
        if (errorOut) *errorOut = "Failed to get created subaddress";
        return QString();
    } catch (...) {
        if (errorOut) *errorOut = "Exception occurred while creating subaddress";
        return QString();
    }
}

QList<std::pair<QString, QString>> GuiWalletManager::getSubaddresses(uint32_t accountIndex, QString* errorOut)
{
    QList<std::pair<QString, QString>> result;
    if (errorOut) errorOut->clear();
    if (!m_lib || !m_lib->wallet) {
        if (errorOut) *errorOut = "No wallet loaded";
        return result;
    }
    
    try {
        // Get subaddresses directly from wallet API
        size_t numSubs = m_lib->wallet->numSubaddresses(accountIndex);
        for (size_t i = 0; i < numSubs; ++i) {
            QString address = QString::fromStdString(m_lib->wallet->address(accountIndex, static_cast<uint32_t>(i)));
            QString label = QString::fromStdString(m_lib->wallet->getSubaddressLabel(accountIndex, static_cast<uint32_t>(i)));
            result.append({address, label});
        }
    } catch (...) {
        if (errorOut) *errorOut = "Exception occurred while getting subaddresses";
    }
    
    return result;
}

QString GuiWalletManager::getUnlockedBalance() const
{
    return m_unlockedBalance;
}

QString GuiWalletManager::getLockedBalance() const
{
    return m_lockedBalance;
}

uint64_t GuiWalletManager::getBlocksToUnlock() const
{
    return m_blocksToUnlock;
}

uint64_t GuiWalletManager::getTimeToUnlock() const
{
    return m_timeToUnlock;
}

} // namespace qsf
