#ifndef GREEN_DLC_STATE_MACHINE_H
#define GREEN_DLC_STATE_MACHINE_H

#include "dlc_coordinator_client.h"
#include <QString>
#include <QObject>
#include <QMap>
#include <QTimer>
#include <memory>

/**
 * DLC Negotiation State Machine
 *
 * Implements the wallet-side automation for DLC order matching, signing, and funding.
 * Follows the spec:
 *   Taker: pending_accept -> submit accept -> wait for accept_submitted
 *   Maker: pending_fill -> request sign context -> submit sign -> wait for sign_submitted
 *   Both: monitor funding and settlement
 */
class DLCStateMachine : public QObject {
    Q_OBJECT
public:
    enum class OperationState {
        Idle,
        // Taker states
        TakerAwaitingAcceptContext,
        TakerSigningAccept,
        TakerSubmittingAccept,
        TakerAcceptSubmitted,
        // Maker states
        MakerAwaitingSignContext,
        MakerSigningSign,
        MakerSubmittingSign,
        MakerSignSubmitted,
        // Terminal
        DLCSignedAwaitingFunding,
        DLCFunded,
        DLCSettled,
        Error
    };

    struct OrderOperation {
        QString order_id;
        QString dlc_id;
        QString instrument_id;
        QString side;              // "buy" or "sell"
        QString match_role;        // "taker" or "maker"
        OperationState state = OperationState::Idle;
        QString status_text;
        
        // Context caching
        AcceptContext accept_context;
        SignContext sign_context;
        bool accept_context_fetched = false;
        bool sign_context_fetched = false;
        
        // Local key storage
        QString funding_pubkey_hex;
        QString funding_privkey_hex;  // Store locally, never send
    };

    explicit DLCStateMachine(DLCCoordinatorClient* client, QObject* parent = nullptr);
    ~DLCStateMachine();

    // ===== Configuration =====
    void set_polling_interval_ms(int interval);
    void set_max_retries(int max_retries);

    // ===== Order Tracking =====
    void track_order(const Order& order);
    void untrack_order(const QString& order_id);
    OrderOperation* get_operation(const QString& order_id);
    
    // ===== Manual Signing Callbacks =====
    /**
     * When wallet signs CET adaptor signatures locally, call this
     * Format: hex-encoded DER signatures
     */
    void submit_taker_accept_signatures(
        const QString& order_id,
        const QStringList& cet_adaptor_sigs,
        const QString& refund_sig,
        const QStringList& funding_sigs);

    void submit_maker_sign_signatures(
        const QString& dlc_id,
        const QStringList& cet_adaptor_sigs,
        const QString& refund_sig,
        const QStringList& funding_sigs);

    // ===== Polling Loop =====
    void start_polling();
    void stop_polling();
    void poll_now();

signals:
    // Operation state changes
    void operation_state_changed(QString order_id, OperationState new_state, QString status);
    
    // Requests for wallet to sign locally
    void request_taker_accept_signatures(
        QString order_id,
        AcceptContext context,  // Wallet should sign this
        QString wallet_id);
        
    void request_maker_sign_signatures(
        QString dlc_id,
        SignContext context,    // Wallet should sign this
        QString wallet_id);

    // Operation completions
    void taker_accept_completed(QString order_id);
    void maker_sign_completed(QString dlc_id);
    void dlc_funded(QString dlc_id, QString funding_txid);
    void dlc_settled(QString dlc_id, QString settlement_txid);
    
    // Errors
    void operation_failed(QString order_id, QString error_reason);
    void operation_retrying(QString order_id, int attempt);

private slots:
    // HTTP response handlers from DLCCoordinatorClient
    void on_accept_context_ready(AcceptContext context);
    void on_accept_context_failed(QString error);
    void on_accept_submitted(Order order);
    void on_accept_submit_failed(QString error);
    
    void on_sign_context_ready(SignContext context);
    void on_sign_context_failed(QString error);
    void on_sign_submitted(SignResponse response);
    void on_sign_submit_failed(QString error);
    
    void on_dlc_ready(DLCDetail dlc);
    void on_dlc_failed(QString error);
    
    // Polling timer
    void on_poll_timer();

private:
    DLCCoordinatorClient* m_client;
    QMap<QString, OrderOperation> m_operations;  // order_id -> operation
    QTimer* m_poll_timer;
    int m_polling_interval_ms = 3000;            // 3 second default
    int m_max_retries = 5;
    QMap<QString, int> m_retry_counts;           // order_id -> current retry
    
    // Internal state machine logic
    void advance_taker_operation(OrderOperation& op);
    void advance_maker_operation(OrderOperation& op);
    void handle_cet_signing_jobs(const QString& order_id, std::vector<CETPSigningJob>& jobs);
    
    void set_operation_state(const QString& order_id, OperationState state, const QString& status);
    void log_operation_event(const QString& order_id, const QString& event);
};

#endif // GREEN_DLC_STATE_MACHINE_H
