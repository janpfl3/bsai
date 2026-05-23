/**

    @file src/dlc/dlc_coordinator_client.h
    @brief HTTP client for DLC Coordinator API with wallet + partner auth */

#ifndef GREEN_DLC_COORDINATOR_CLIENT_H
#define GREEN_DLC_COORDINATOR_CLIENT_H

#include 
#include 
#include 
#include 
#include 
#include 
#include 
#include 
#include 

/**

    Configuration for DLC Coordinator endpoint and authentication */ struct DLCCoordinatorConfig { QString base_url; QString network; QString partner_id; QString partner_token; int timeout_ms = 15000; bool verify_ssl = true; };

struct WalletRegistration {
    QString wallet_id;
    QString wallet_token;
    QString expires_at;
};

struct OptionInstrument {
    int id;
    QString instrument_id;
    QString type;
    QString oracle_label;
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

// CETP signing job shared type
struct CETPSigningJob {
    QString message_hash_hex;
    QString adaptor_point_hex;
};

struct OrderBookLevel {
    long long price;
    long long quantity;
    QString instrument_id;
    QString strike;
};

struct OrderBook {
    QString instrument_id;
    std::vector bids;
    std::vector asks;
    QString timestamp_ms;
};

struct Order {
    QString order_id;
    QString dlc_id;
    QString instrument_id;
    QString side;
    long long quantity;
    long long price;
    QString status;
    QString created_at;
    QString offer_object_hex;
    QString accept_object_hex;
    bool pending_match_accept;
    QString matched_order_id;
    QString matched_dlc_id;
    QString matched_offer_object_hex;

bool is_maker = false;
bool sign_required = false;
QString dlc_status;

};

struct AcceptContext {
    QString order_id;
    QString instrument_id;
    QString offer_object_hex;
    QString context_fingerprint;
    int cet_count;
    int cet_signing_job_count;
    QVector cet_signing_jobs;
    QString refund_sighash_hex;
    QVector funding_input_sighashes_hex;
    QVector funding_input_outpoints;
    QVector funding_input_addresses;
};

struct AcceptMatchResponse {
    QString order_id;
    QString offer_object_hex;
    QString accept_object_hex;
};

struct SignContext {
    QString dlc_id;
    QString order_id;
    QString instrument_id;
    QString offer_object_hex;
    QString accept_object_hex;
    QString context_fingerprint;
    int cet_count;
    int cet_signing_job_count;
    QVector cet_signing_jobs;
    QString refund_sighash_hex;
    QVector funding_input_sighashes_hex;
    QVector funding_input_outpoints;
    QVector funding_input_addresses;
};

struct SignResponse {
    QString dlc_id;
    QString status;
    QString sign_object_hex;
    QString funding_tx_hex;
    QString funding_txid;
    bool funding_broadcasted = false;
    QString funding_broadcast_error;
};

struct DLCDetail {
    QString dlc_id;
    QString order_id;
    QString instrument_id;
    QString offer_object_hex;
    QString accept_object_hex;
    QString sign_object_hex;
    QString funding_txid;
    int funding_output_index = -1;
    QString last_error_reason;
    QString last_error_message;
    QString oracle_outcome_value;
    QString oracle_attested_at;
    int winning_cet_index = -1;
    QString settlement_type;
    QString closing_txid;
    QString refund_txid;
    QString confirmed_at;
    QJsonObject decoded_offer;
    QJsonObject decoded_accept;
    QJsonObject decoded_sign;
    QJsonObject oracle_context;
    QString status;
    QString created_at;
    QString updated_at;
};

class DLCCoordinatorClient : public QObject {
    Q_OBJECT
public:
    explicit DLCCoordinatorClient(const DLCCoordinatorConfig& config, QObject* parent = nullptr);

void check_system_readiness();

QString get_nonce();
void register_wallet(const QString& xpub, const QString& xpub_signature,
                     const QString& nonce, const QString& label);
void get_wallet_status(const QString& wallet_id);
void get_instruments();
void get_orderbook(const QString& instrument_id);

void create_order(const QString& wallet_id, const QString& instrument_id,
                  const QString& side, long long quantity, long long price);
void get_orders(const QString& wallet_id);
void get_order(const QString& order_id);
void cancel_order(const QString& order_id);

void get_accept_context(const QString& order_id, const QString& funding_pubkey);
void submit_accept(const QString& order_id, const QString& funding_pubkey,
                   const QString& context_fingerprint,
                   const QStringList& cet_adaptor_sigs,
                   const QString& refund_sig,
                   const QStringList& funding_sigs);

void get_sign_context(const QString& dlc_id);
void submit_sign(const QString& dlc_id,
                 const QStringList& cet_adaptor_sigs,
                 const QString& refund_sig,
                 const QStringList& funding_sigs);

void get_dlc(const QString& dlc_id);
void get_dlcs(const QString& wallet_id);

signals:
    void systemReadinessReady(bool reachable, const QString& network);
    void systemReadinessFailed(const QString& reason);

void walletRegistered(const WalletRegistration& reg);
void walletRegistrationFailed(const QString& reason);
void walletStatusReady(const WalletRegistration& status);
void walletStatusFailed(const QString& reason);

void instrumentsReady(const QVector<OptionInstrument>& instruments);
void instrumentsFailed(const QString& reason);
void orderbookReady(const QString& instrument_id, const QVector<OrderBookLevel>& bids, const QVector<OrderBookLevel>& asks);
void orderbookFailed(const QString& reason);

void orderCreated(const Order& order);
void orderCreateFailed(const QString& reason);
void ordersReady(const QVector<Order>& orders);
void ordersFailed(const QString& reason);
void orderReady(const Order& order);
void orderFailed(const QString& reason);

void acceptContextReady(const AcceptContext& ctx);
void acceptContextFailed(const QString& reason);
void acceptSubmitted(const Order& order);
void acceptSubmitFailed(const QString& reason);
void request_taker_accept_signatures(const QString& order_id, const AcceptContext& ctx, const QString& wallet_id);
void taker_accept_completed(const QString& order_id);

void signContextReady(const SignContext& ctx);
void signContextFailed(const QString& reason);
void signSubmitted(const SignResponse& resp);
void signSubmitFailed(const QString& reason);
void request_maker_sign_signatures(const QString& dlc_id, const SignContext& ctx, const QString& wallet_id);
void maker_sign_completed(const QString& dlc_id);

void dlcReady(const DLCDetail& dlc);
void dlcFailed(const QString& reason);
void dlcsReady(const QVector<DLCDetail>& dlcs);
void dlcsFailed(const QString& reason);

void operation_state_changed(const QString& order_id, int state, const QString& status);
void operation_failed(const QString& order_id, const QString& reason);
void operation_retrying(const QString& order_id, int attempt);

void dlc_funded(const QString& dlc_id, const QString& funding_txid);
void dlc_settled(const QString& dlc_id, const QString& closing_txid);

private:
    DLCCoordinatorConfig m_config;
};

#endif // GREEN_DLC_COORDINATOR_CLIENT_H
