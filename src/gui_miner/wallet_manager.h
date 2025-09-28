#ifndef WALLET_MANAGER_H
#define WALLET_MANAGER_H

#include <QObject>
#include <QString>
#include <QTimer>
#include <QSettings>
#include <memory>

namespace qsf {

class MainWindow;

class WalletManager : public QObject
{
    Q_OBJECT

public:
    explicit WalletManager(QObject* parent = nullptr);
    ~WalletManager();

    // Wallet operations
    bool createWallet(const QString& password, const QString& walletPath);
    bool openWallet(const QString& walletPath, const QString& password);
    void closeWallet();
    
    // Wallet info
    QString getAddress() const { return m_walletAddress; }
    bool hasWallet() const { return m_hasWallet; }
    QString getBalance() const { return m_balance; }
    
    // Balance updates
    void refreshBalance();
    void setAutoRefresh(bool enabled, int intervalMs = 5000);
    void setDaemonAddress(const QString& daemonAddress);
    void rescanBlockchainFromZero();
    void forceRefreshOnDaemonAvailable();
    void onDaemonStatusChanged(bool daemonRunning);
    void setPassword(const QString& password) { m_password = password; }
    
    // Wallet path management
    void setWalletPath(const QString& walletPath);

    // Transfers
    // Creates and commits a transaction. Returns true on success and fills txidOut
    bool sendTransaction(const QString& toAddress, const QString& amountStr, QString* txidOut, QString* errorOut);

    // Receive helpers
    QString getPrimaryAddress() const { return m_walletAddress; }
    QString makeIntegratedAddress(const QString& paymentId) const;
    QString makePaymentUri(const QString& address, const QString& paymentId, const QString& amountStr, const QString& description, QString* errorOut) const;

signals:
    void walletOpened(const QString& address);
    void walletClosed();
    void balanceUpdated(const QString& balance);
    void error(const QString& message);

private slots:
    void onRefreshTimer();

private:
    void ensureWalletManager();
    void updateCachedFieldsFromWallet(bool emitSignals);
    QString displayAmount(uint64_t atomic) const;
    
    QString m_walletPath;
    QString m_password;
    QString m_walletAddress;
    QString m_balance;
    bool m_hasWallet;
    QString m_daemonAddress;
    bool m_rescanQueued;
    bool m_isRescanning;
    bool m_isRefreshing;
    bool m_autoRefreshWasEnabled;
    bool m_rescanCompletedOnce;
    QTimer* m_refreshTimer;
    bool m_autoRefresh;
    int m_refreshInterval;
    
    // libwallet API objects
    class LibwalletListener;
    std::unique_ptr<LibwalletListener> m_listener;
    struct LibwalletState;
    std::unique_ptr<LibwalletState> m_lib;
};

} // namespace qsf

#endif // WALLET_MANAGER_H
