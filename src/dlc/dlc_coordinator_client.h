/**
 * @file src/dlc/dlc_coordinator_client.h
 * @brief HTTP client for DLC Coordinator API with wallet + partner auth
 */

#ifndef GREEN_DLC_COORDINATOR_CLIENT_H
#define GREEN_DLC_COORDINATOR_CLIENT_H

#include <QString>
#include <QStringList>
#include <QJsonObject>
#include <QJsonArray>
#include <QVector>
#include <QObject>
#include <memory>
#include <optional>
#include <vector>

/**
 * Configuration for DLC Coordinator endpoint and authentication
 */
struct DLCCoordinatorConfig {
    QString base_url;                    // e.g., "http://192.168.0.198:8000"
    QString network;                     // "regtest", "testnet3", or "mainnet"
    QString partner_id;                  // e.g., "partner-1"
    QString partner_token;               // X-Partner-Token (app/build config, not user secret)
    int timeout_ms = 15000;
    bool verify_ssl = true;
};

/**
 * Lightweight auth response holder
 */
struct WalletRegistration {
    QString wallet_id;
    QString wallet_token;
    QString expires_at;  // ISO 8601 timestamp
};

/**
 * Option instrument response from /instruments
 */
struct OptionInstrument {
    int id;                              // numeric id
    QString instrument_id;               // e.g., "BTC-18MAR26-74100-C"
    QString type;                        // "call", "put", etc.
    QString oracle_label;                // oracle name
    QString oracle_announcement_url;
    QString expires_at;
    QString created_at;
    QString updated_at;

    bool is_expired(const QString& current_time) const;
    QString parse_underlying() const;
    QString parse_strike() const;
    QString parse_expiry() const;
    QString parse_right() const;
};

// CETP signing job shared type (used by AcceptContext and SignContext)
struct CETPSigningJob {
    QString message_hash_hex;
    QString adaptor_point_hex;
};

/**
 * Order book entry
 */
struct OrderBookLevel {
    long long price;                     // sats
    long long quantity;                  // contract count
    QString instrument_id;
    QString strike;
};

struct OrderBook {
    QString instrument_id;
    std::vector<OrderBookLevel> bids;
    std::vector<OrderBookLevel> asks;
    QString timestamp_ms;
};

/**
 * Order response from POST /orders or GET /orders
 */
struct Order {
    QString order_id;
    QString dlc_id;
    QString instrument_id;
    QString side;                        // "buy" or "sell"
    long long quantity;
    long long price;                     // sats
    QString status;                      // "open", "pending_accept", "filled", "cancelled", etc.
    QString created_at;
    QString offer_object_hex;            // Canonical coordinator-provided DLC Offer
    QString accept_object_hex;           // Canonical accept message (null until taker signs)
    bool pending_match_accept;           // true = matched as taker, awaiting accept signing
    QString matched_order_id;
    QString matched_dlc_id;
    QString matched_offer_object_hex;

    // For maker flow
    bool is_maker = false;
    bool sign_required = false;
    QString dlc_status;
};

/**
 * Accept context response from POST /orders/{order_id}/accept-context
 */
struct AcceptContext {
    QString order_id;
    QString instrument_id;
    QString offer_object_hex;            // Original offer (for validation placeholder)
    QString context_fingerprint;         // Bind accept-match submission to this context
    int cet_count;
    int cet_signing_job_count;

    QVector<CETPSigningJob> cet_signing_jobs;

    QString refund_sighash_hex;          // Single refund signature
    QVector<QString> funding_input_sighashes_hex;  // One per funding input
    QVector<QString> funding_input_outpoints;      // "txid:vout" format
    QVector<QString> funding_input_addresses;      // For UI display/verification
};

/**
 * Response from POST /orders/{order_id}/accept-match
 */
struct AcceptMatchResponse {
    QString order_id;
    QString offer_object_hex;
    QString accept_object_hex;           // Now canonical
};

/**
 * Sign context response from GET /dlcs/{dlc_id}/sign-context
 */
struct SignContext {
    QString dlc_id;
    QString order_id;
    QString instrument_id;
    QString offer_object_hex;
    QString accept_object_hex;
    QString context_fingerprint;
    int cet_count;
    int cet_signing_job_count;

    QVector<CETPSigningJob> cet_signing_jobs;

    QString refund_sighash_hex;
    QVector<QString> funding_input_sighashes_hex;
    QVector<QString> funding_input_outpoints;
    QVector<QString> funding_input_addresses;
};

/**
 * Sign submission response from POST /dlcs/{dlc_id}/sign
 */
struct SignResponse {
    QString dlc_id;
    QString status;                      // "signed"
    QString sign_object_hex;             // Canonical sign message
    QString funding_tx_hex;              // Unsigned funding tx
    QString funding_txid;
    bool funding_broadcasted = false;
    QString funding_broadcast_error;     // null if success
};

/**
 * DLC detail response from GET /dlcs/{dlc_id}
 */
struct DLCDetail {
    QString dlc_id;
    QString order_id;
    QString instrument_id;
    QString offer_object_hex;
    QString accept_object_hex;
    QString sign_object_hex;
    QString funding_txid;
    int funding_output_index = -1;
    QString last_error_reason;           // Machine-readable
    QString last_error_message;          // Human-readable
    QString oracle_outcome_value;        // Oracle-attested outcome
    QString oracle_attested_at;
    int winning_cet_index = -1;
    QString settlement_type;             // "cet" or "refund"
    QString closing_txid;                // CET broadcast
    QString refund_txid;                 // If settlement was refund
    QString confirmed_at;
    QJsonObject decoded_offer;
    QJsonObject decoded_accept;
    QJsonObject decoded_sign;
    QJsonObject oracle_context;
    QString status;                      // Protocol state
    QString created_at;
    QString updated_at;
};

/**
 * Main client for DLC Coordinator API
 *
 * Async, emits signals on completion.
 */
class DLCCoordinatorClient : public QObject {
    Q_OBJECT
public:
    explicit DLCCoordinatorClient(const DLCCoordinatorConfig& config, QObject* parent = nullptr);

    // System readiness (async)
    void check_system_readiness();

    // Wallet registration (async)
    QString get_nonce();
    void register_wallet(const QString& xpub, const QString& xpub_signature,
                         const QString& nonce, const QString& label);
    void get_wallet_status(const QString& wallet_id);
    void get_instruments();
    void get_orderbook(const QString& instrument_id);

    // Order lifecycle (async)
    void create_order(const QString& wallet_id, const QString& instrument_id,
                      const QString& side, long long quantity, long long price);
    void get_orders(const QString& wallet_id);
    void get_order(const QString& order_id);
    void cancel_order(const QString& order_id);

    // Accept flow (async)
    void get_accept_context(const QString& order_id, const QString& funding_pubkey);
    void submit_accept(const QString& order_id, const QString& funding_pubkey,
                       const QString& context_fingerprint,
                       const QStringList& cet_adaptor_sigs,
                       const QString& refund_sig,
                       const QStringList& funding_sigs);

    // Sign flow (async)
    void get_sign_context(const QString& dlc_id);
    void submit_sign(const QString& dlc_id,
                     const QStringList& cet_adaptor_sigs,
                     const QString& refund_sig,
                     const QStringList& funding_sigs);

    // Status queries (async)
    void get_dlc(const QString& dlc_id);
    void get_dlcs(const QString& wallet_id);

signals:
    // system readiness
    void systemReadinessReady(bool reachable, const QString& network);
    void systemReadinessFailed(const QString& reason);

    // wallet registration
    void walletRegistered(const WalletRegistration& reg);
    void walletRegistrationFailed(const QString& reason);
    void walletStatusReady(const WalletRegistration& status);
    void walletStatusFailed(const QString& reason);

    // market data
    void instrumentsReady(const QVector<OptionInstrument>& instruments);
    void instrumentsFailed(const QString& reason);
    void orderbookReady(const QString& instrument_id, const QVector<OrderBookLevel>& bids, const QVector<OrderBookLevel>& asks);
    void orderbookFailed(const QString& reason);

    // orders
    void orderCreated(const Order& order);
    void orderCreateFailed(const QString& reason);
    void ordersReady(const QVector<Order>& orders);
    void ordersFailed(const QString& reason);
    void orderReady(const Order& order);
    void orderFailed(const QString& reason);

    // accept flow
    void acceptContextReady(const AcceptContext& ctx);
    void acceptContextFailed(const QString& reason);
    void acceptSubmitted(const Order& order);
    void acceptSubmitFailed(const QString& reason);
    void request_taker_accept_signatures(const QString& order_id, const AcceptContext& ctx, const QString& wallet_id);
    void taker_accept_completed(const QString& order_id);

    // sign flow
    void signContextReady(const SignContext& ctx);
    void signContextFailed(const QString& reason);
    void signSubmitted(const SignResponse& resp);
    void signSubmitFailed(const QString& reason);
    void request_maker_sign_signatures(const QString& dlc_id, const SignContext& ctx, const QString& wallet_id);
    void maker_sign_completed(const QString& dlc_id);

    // state & status
    void dlcReady(const DLCDetail& dlc);
    void dlcFailed(const QString& reason);
    void dlcsReady(const QVector<DLCDetail>& dlcs);
    void dlcsFailed(const QString& reason);

    // operation lifecycle (used by state machine)
    void operation_state_changed(const QString& order_id, int state, const QString& status);
    void operation_failed(const QString& order_id, const QString& reason);
    void operation_retrying(const QString& order_id, int attempt);

    void dlc_funded(const QString& dlc_id, const QString& funding_txid);
    void dlc_settled(const QString& dlc_id, const QString& closing_txid);

private:
    DLCCoordinatorConfig m_config;
};

#endif // GREEN_DLC_COORDINATOR_CLIENT_H

