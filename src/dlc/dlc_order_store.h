/**
 * @file src/dlc/dlc_order_store.h
 * @brief Local persistence for orders and DLCs
 */

#pragma once

#include <QString>
#include <QJsonObject>
#include <vector>
#include <optional>
#include <map>

/**
 * Local order record
 * Persisted to wallet's secure storage/account metadata
 */
struct LocalOrderRecord {
    // Coordinator response fields
    QString order_id;
    QString dlc_id;
    QString instrument_id;
    QString side;
    long long quantity = 0;
    long long price = 0;
    QString status;
    QString created_at;
    QString offer_object_hex;
    QString accept_object_hex;

    // Local wallet binding
    QString wallet_id;
    QString partner_id;
    QString funding_pubkey_hex;
    QString funding_pubkey_derivation_path;  // e.g., "m/44'/0'/0'/2/0"
    
    // Idempotency & retries
    QString create_idempotency_key;
    QString accept_idempotency_key;
    std::optional<QString> accept_context_fingerprint;

    // Signing state
    bool accept_signatures_pending = false;
    bool accept_signatures_submitted = false;

    // Settlement tracking
    std::optional<QString> funding_txid;
    int funding_confirms = 0;
    std::optional<QString> settlement_txid;
    QString settlement_type;              // "cet" or "refund"

    // Timestamps
    long long last_sync_ms = 0;
    std::optional<QString> last_error_reason;
    std::optional<QString> last_error_message;

    // Serialization
    QJsonObject to_json() const;
    static LocalOrderRecord from_json(const QJsonObject& json);
};

/**
 * Local DLC record
 * Tracks DLC throughout its lifecycle
 */
struct LocalDLCRecord {
    QString dlc_id;
    QString order_id;
    QString instrument_id;
    QString wallet_id;

    // Protocol state
    QString status;                      // "offer_created", "accepted", "signed", etc.
    QString offer_object_hex;
    QString accept_object_hex;
    QString sign_object_hex;

    // Signing state (for maker)
    bool sign_required = false;
    bool sign_signatures_pending = false;
    bool sign_signatures_submitted = false;
    QString sign_idempotency_key;

    // Funding
    QString funding_txid;
    int funding_confirms = 0;
    long long funding_amount_sats = 0;

    // Settlement
    std::optional<QString> oracle_outcome;
    std::optional<QString> oracle_attested_at;
    std::optional<QString> settlement_txid;
    std::optional<QString> refund_txid;
    QString settlement_type;
    long long payout_sats = 0;

    // Timestamps
    long long last_sync_ms = 0;
    QString created_at;
    QString updated_at;

    QJsonObject to_json() const;
    static LocalDLCRecord from_json(const QJsonObject& json);
};

/**
 * Store for local order/DLC records
 * Uses wallet's account metadata system
 */
class DLCOrderStore {
public:
    explicit DLCOrderStore(const QString& wallet_id);

    // Orders
    void save_order(const LocalOrderRecord& order);
    std::optional<LocalOrderRecord> get_order(const QString& order_id) const;
    std::vector<LocalOrderRecord> get_orders_for_wallet() const;
    std::vector<LocalOrderRecord> get_open_orders_for_wallet() const;
    void update_order_status(const QString& order_id, const QString& status);

    // DLCs
    void save_dlc(const LocalDLCRecord& dlc);
    std::optional<LocalDLCRecord> get_dlc(const QString& dlc_id) const;
    std::vector<LocalDLCRecord> get_dlcs_for_wallet() const;
    void update_dlc_status(const QString& dlc_id, const QString& status);

    // Idempotency key tracking
    void save_idempotency_key(const QString& key, const QJsonObject& response);
    std::optional<QJsonObject> get_idempotency_response(const QString& key) const;

private:
    QString wallet_id_;
    std::map<QString, LocalOrderRecord> orders_;
    std::map<QString, LocalDLCRecord> dlcs_;
    std::map<QString, QJsonObject> idempotency_cache_;

    void load_from_storage();
    void persist_to_storage();
};
