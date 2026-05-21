#ifndef BLOCKSTREAM_DLCSTATEMODEL_H
#define BLOCKSTREAM_DLCSTATEMODEL_H

#include <QObject>
#include <QString>
#include <QJsonObject>
#include <QQmlEngine>
#include <map>
#include <optional>

/**
 * @file src/dlc/dlcstatemodel.h
 * @brief State model for DLC lifecycle tracking and UI state representation
 */
class DLCStateModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString dlcId READ dlcId NOTIFY dlcIdChanged)
    Q_PROPERTY(QString orderId READ orderId NOTIFY orderIdChanged)
    Q_PROPERTY(QString currentState READ currentState NOTIFY currentStateChanged)
    Q_PROPERTY(QString displayState READ displayState NOTIFY displayStateChanged)
    Q_PROPERTY(bool isTerminal READ isTerminal NOTIFY isTerminalChanged)
    Q_PROPERTY(bool isActive READ isActive NOTIFY isActiveChanged)
    Q_PROPERTY(QString role READ role NOTIFY roleChanged)
    Q_PROPERTY(long long collateralSats READ collateralSats NOTIFY collateralSatsChanged)
    Q_PROPERTY(long long payoutSats READ payoutSats NOTIFY payoutSatsChanged)
    Q_PROPERTY(QString fundingTxId READ fundingTxId NOTIFY fundingTxIdChanged)
    Q_PROPERTY(int fundingConfirmations READ fundingConfirmations NOTIFY fundingConfirmationsChanged)
    Q_PROPERTY(QString settlementTxId READ settlementTxId NOTIFY settlementTxIdChanged)
    Q_PROPERTY(QString settlementOutcome READ settlementOutcome NOTIFY settlementOutcomeChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)
    QML_ELEMENT

public:
    enum DLCState {
        // Offer & Accept phase
        OFFER_CREATED,
        ACCEPTED,

        // Signing phase
        SIGN_REQUIRED,
        SIGNED,

        // Funding phase
        FUNDING_TX_CREATED,
        FUNDING_BROADCASTED,
        FUNDING_CONFIRMED,
        MATURED,

        // Settlement phase
        ORACLE_ATTESTED,
        CET_BROADCASTED,
        CET_CONFIRMED,
        REFUND_BROADCASTED,
        REFUND_CONFIRMED,

        // Terminal states
        CLOSED,
        FAILED,
        CANCELLED
    };
    Q_ENUM(DLCState)

    explicit DLCStateModel(QObject* parent = nullptr);

    // Initialization
    void initializeFromDLC(const QString& dlc_id, const QJsonObject& dlc_data);
    void initializeFromOrder(const QString& order_id, const QJsonObject& order_data);

    // Properties
    QString dlcId() const { return m_dlc_id; }
    QString orderId() const { return m_order_id; }
    QString currentState() const { return m_current_state; }
    QString displayState() const;
    bool isTerminal() const;
    bool isActive() const;
    QString role() const { return m_role; }
    long long collateralSats() const { return m_collateral_sats; }
    long long payoutSats() const { return m_payout_sats; }
    QString fundingTxId() const { return m_funding_tx_id; }
    int fundingConfirmations() const { return m_funding_confirmations; }
    QString settlementTxId() const { return m_settlement_tx_id; }
    QString settlementOutcome() const { return m_settlement_outcome; }
    QString lastError() const { return m_last_error; }

    // State transitions
    Q_INVOKABLE bool canTransitionTo(const QString& next_state) const;
    Q_INVOKABLE void transitionTo(const QString& next_state);

    // Status queries
    Q_INVOKABLE bool awaitingSignature() const;
    Q_INVOKABLE bool awaitingFunding() const;
    Q_INVOKABLE bool awaitingOracle() const;
    Q_INVOKABLE bool settlementPending() const;

    // Data access
    Q_INVOKABLE QJsonObject toJson() const;
    Q_INVOKABLE void updateFromJson(const QJsonObject& data);

    // UI helpers
    Q_INVOKABLE QString getProgressPercent() const;
    Q_INVOKABLE QString getStatusColor() const;
    Q_INVOKABLE QString getStatusIcon() const;

signals:
    void dlcIdChanged();
    void orderIdChanged();
    void currentStateChanged();
    void displayStateChanged();
    void isTerminalChanged();
    void isActiveChanged();
    void roleChanged();
    void collateralSatsChanged();
    void payoutSatsChanged();
    void fundingTxIdChanged();
    void fundingConfirmationsChanged();
    void settlementTxIdChanged();
    void settlementOutcomeChanged();
    void lastErrorChanged();
    void stateTransitioned(const QString& from_state, const QString& to_state);
    void errorOccurred(const QString& error_message);

public slots:
    void onSignatureReceived();
    void onFundingBroadcasted();
    void onFundingConfirmed();
    void onOracleAttested(const QString& outcome);
    void onSettlementBroadcasted();
    void onError(const QString& error_message);

private:
    QString m_dlc_id;
    QString m_order_id;
    QString m_current_state = "OFFER_CREATED";
    QString m_role;  // "maker" or "taker"
    long long m_collateral_sats = 0;
    long long m_payout_sats = 0;
    QString m_funding_tx_id;
    int m_funding_confirmations = 0;
    QString m_settlement_tx_id;
    QString m_settlement_outcome;
    QString m_last_error;

    // State machine validation
    std::map<QString, std::vector<QString>> m_valid_transitions;
    void initializeStateTransitions();

    // Helper methods
    bool isValidTransition(const QString& from_state, const QString& to_state) const;
    QString stateToDisplayString(const QString& state) const;
};

#endif // BLOCKSTREAM_DLCSTATEMODEL_H

