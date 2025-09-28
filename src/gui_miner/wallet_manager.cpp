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

void GuiWalletManager::setAutoRefresh(bool enabled, int intervalMs)
{
    m_autoRefresh = enabled;
    m_refreshInterval = intervalMs;
    if (!m_lib || !m_lib->wallet) return;
    m_lib->wallet->setAutoRefreshInterval(intervalMs);
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

} // namespace qsf
