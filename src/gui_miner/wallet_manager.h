#ifndef WALLET_MANAGER_H
#define WALLET_MANAGER_H

#include <QObject>
#include <QString>
#include <QTimer>
#include <QSettings>
#include <QList>
#include <vector>
#include <memory>

namespace qsf {

class MainWindow;

class GuiWalletManager : public QObject
{
    Q_OBJECT

public:
    explicit GuiWalletManager(QObject* parent = nullptr);
    ~GuiWalletManager();

    // Wallet operations
    bool createWallet(const QString& password, const QString& walletPath);
    bool openWallet(const QString& walletPath, const QString& password);
    bool recoverWallet(const QString& password, const QString& walletPath, const QString& mnemonic, uint64_t restoreHeight = 0);
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
    
    // Sweep unmixable outputs (dust) to consolidate balance
    bool sweepUnmixableOutputs(QString* errorOut = nullptr);
    
    // Transaction history
    struct TransactionInfo {
        QString txid;
        QString direction; // "in" or "out"
        QString amount;
        QString fee;
        uint64_t blockHeight;
        uint64_t confirmations;
        uint64_t unlockTime;
        qint64 timestamp;
        QString paymentId;
        QString description;
        bool isPending;
        bool isFailed;
        bool isCoinbase;
        std::vector<std::pair<QString, QString>> transfers; // address, amount pairs
    };
    QList<TransactionInfo> getTransactionHistory(QString* errorOut = nullptr);
    
    // Rescan spent outputs
    bool rescanSpent(QString* errorOut = nullptr);
    
    // Sweep all balance
    bool sweepAll(const QString& toAddress, QString* txidOut = nullptr, QString* errorOut = nullptr);
    bool sweepAllToSelf(QString* txidOut = nullptr, QString* errorOut = nullptr);
    
    // Subaddress management
    QString createSubaddress(uint32_t accountIndex, const QString& label, QString* errorOut = nullptr);
    QList<std::pair<QString, QString>> getSubaddresses(uint32_t accountIndex, QString* errorOut = nullptr); // address, label pairs
    
    // Balance information
    QString getUnlockedBalance() const;
    QString getLockedBalance() const;
    uint64_t getBlocksToUnlock() const;
    uint64_t getTimeToUnlock() const; // seconds
    
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
    QString m_unlockedBalance;
    QString m_lockedBalance;
    uint64_t m_blocksToUnlock;
    uint64_t m_timeToUnlock;
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
