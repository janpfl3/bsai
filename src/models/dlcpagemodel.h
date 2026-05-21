class DlcPageModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(int currentTab READ currentTab WRITE setCurrentTab NOTIFY currentTabChanged)
    Q_PROPERTY(bool loading READ loading NOTIFY loadingChanged)
    Q_PROPERTY(bool actionInProgress READ actionInProgress NOTIFY actionInProgressChanged)
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY errorMessageChanged)
    Q_PROPERTY(QString infoMessage READ infoMessage NOTIFY infoMessageChanged)
    // ... more properties for each data model (orders, instruments, balances, etc.)
    
public:
    Q_INVOKABLE void load();
    Q_INVOKABLE void setCurrentTab(int index);
    Q_INVOKABLE void refreshCurrentTab();
    Q_INVOKABLE void activateWalletForDlc(const QString& walletId);
    Q_INVOKABLE void createOrder(const QString& instrumentId, const QString& side, 
                                 double quantity, int premiumSats);
    Q_INVOKABLE void cancelOpenOrder(const QString& orderId);
    Q_INVOKABLE void clearTransientMessages();
    Q_INVOKABLE void simulateOptionPayout(/* params */);
    
signals:
    void currentTabChanged();
    void loadingChanged();
    void actionInProgressChanged();
    void errorMessageChanged();
    void infoMessageChanged();
    // ...
    
private slots:
    void onOrdersUpdated();
    void onNegotiationPassTick();
    void onApiError(const QString& errorMsg);
    
private:
    void refreshOverviewTab();
    void refreshOrderbookTab();
    void refreshOrdersTab();
    void runNegotiationPass();
    
    int m_currentTab = 0;
    bool m_loading = false;
    bool m_actionInProgress = false;
    // ... member variables for state
    QTimer m_negotiationTimer; // 15s
    std::shared_ptr<DlcRepository> m_repository;
};
