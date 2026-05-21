/**
 * @file src/dlc/dlc_coordinator_client.h
 * @brief HTTP client for DLC Coordinator API with wallet + partner auth
 */

#ifndef GREEN_DLC_COORDINATOR_CLIENT_H
#define GREEN_DLC_COORDINATOR_CLIENT_H

#include <QString>
#include <QJsonObject>
#include <QJsonArray>
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

    // Convenience parsers for instrument_id format
    bool is_expired(const QString& current_time) const;
    QString parse_underlying() const;    // "BTC"
    QString parse_strike() const;        // "74100"
    QString parse_expiry() const;        // "18MAR26"
    QString parse_right() const;         // "C" or "P"
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
 *
 * This is the signing context the wallet must use to produce local signatures.
 * The coordinator has built the offer and accept messages; the wallet must
 * sign the CETs, refund, and funding inputs using the provided sighashes.
 */
struct AcceptContext {
    QString order_id;
    QString instrument_id;
    QString offer_object_hex;            // Original offer (for validation placeholder)
    QString context_fingerprint;         // Bind accept-match submission to this context
    int cet_count;
    int cet_signing_job_count;

    struct CETPSigningJob {
        QString message_hash_hex;        // Hash to sign
        QString adaptor_point_hex;       // Oracle adaptor point for this outcome
    };
    std::vector<CETPSigningJob> cet_signing_jobs;

    QString refund_sighash_hex;          // Single refund signature
    std::vector<QString> funding_input_sighashes_hex;  // One per funding input
    std::vector<QString> funding_input_outpoints;      // "txid:vout" format
    std::vector<QString> funding_input_addresses;      // For UI display/verification
};

/**
 * Response from POST /orders/{order_id}/accept-match
 * Confirms the coordinator received and validated signatures
 */
struct AcceptMatchResponse {
    QString order_id;
    QString offer_object_hex;
    QString accept_object_hex;           // Now canonical
};

/**
 * Sign context response from GET /dlcs/{dlc_id}/sign-context
 * (Same structure as AcceptContext but for maker signing)
 */
struct SignContext {
    QString dlc_id;
    QString order_id;
    QString instrument_id;
    QString offer_object_hex;
    QString accept_object_hex;
    int cet_count;
    int cet_signing_job_count;

    struct CETPSigningJob {
        QString message_hash_hex;
        QString adaptor_point_hex;
    };
    std::vector<CETPSigningJob> cet_signing_jobs;

    QString refund_sighash_hex;
    std::vector<QString> funding_input_sighashes_hex;
    std::vector<QString> funding_input_outpoints;
    std::vector<QString> funding_input_addresses;
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
    bool funding_broadcasted;
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
    int funding_output_index;
    QString last_error_reason;           // Machine-readable
    QString last_error_message;          // Human-readable
    QString oracle_outcome_value;        // Oracle-attested outcome
    QString oracle_attested_at;
    int winning_cet_index;
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
 * Thread-safe for async HTTP operations via existing wallet HttpManager.
 * Never exposes or requires private keys; only public keys and signatures.
 */
class DLCCoordinatorClient {
public:
    explicit DLCCoordinatorClient(const DLCCoordinatorConfig& config);

    // ===== System Readiness =====
    /**
     * GET /auth/system-readiness
     * Verify coordinator is reachable and network matches
     */
    struct SystemReadiness {
        bool reachable;
        QString coordinator_network;     // "regtest", "testnet3", "mainnet"
    };
    SystemReadiness check_system_readiness();

    // ===== Wallet Registration =====
    /**
     * GET /auth/nonce
     * Request a signing nonce for wallet registration
     */
    QString get_nonce();

    /**
     * POST /auth/wallet
     * Register wallet with coordinator
     * xpub_signature: proof of xpub ownership (signed with account key)
     * utxos: proof of UTXO ownership
     */
    struct RegisterWalletRequest {
        QString xpub;
        QString xpub_signature;
        QString nonce;
        QString label;                   // Display name
        
        struct UTXOProof {
            QString txid;
            int vout;
            QString signature;           // Proof of ownership
            QString public_key;
        };
        std::vector<UTXOProof> utxos;
    };
    std::optional<WalletRegistration> register_wallet(const RegisterWalletRequest& req);

    /**
     * GET /auth/wallet/{wallet_id}
     * Check wallet token validity and expiry
     */
    std::optional<WalletRegistration> get_wallet_status(const QString& wallet_id);

    /**
     * POST /auth/wallet/{wallet_id}/refresh-balance
     * Refresh coordinator-side balance tracking
     */
    bool refresh_wallet_balance(const QString& wallet_id, 
                                const std::vector<QString>& utxo_outpoints);

    // ===== Partner Configuration =====
    /**
     * GET /partners/{partner_id}/config
     * Verify partner token is valid
     */
    struct PartnerConfig {
        QString partner_id;
        bool token_valid;
        QString token_expires_at;
    };
    std::optional<PartnerConfig> get_partner_config(const QString& partner_id);

    // ===== Instruments =====
    /**
     * GET /instruments/non-expired
     * Fetch all live option instruments
     */
    std::vector<OptionInstrument> get_non_expired_instruments();

    /**
     * GET /instruments/{instrument_id}
     * Fetch specific instrument by id
     */
    std::optional<OptionInstrument> get_instrument(const QString& instrument_id);

    // ===== Orderbook =====
    /**
     * GET /orderbook/{instrument_id}
     * Fetch current bid/ask spread
     */
    std::optional<OrderBook> get_orderbook(const QString& instrument_id);

    // ===== Order Lifecycle =====
    /**
     * POST /orders
     * Create a new order (buy or sell)
     *
     * Headers:
     *   Authorization: Bearer <wallet_token>
     *   X-Partner-Token: <partner_token>
     *
     * The coordinator builds the canonical DLC Offer; wallet only provides
     * the funding public key and receives the hex-encoded offer.
     */
    struct CreateOrderRequest {
        QString instrument_id;
        QString side;                    // "buy" or "sell"
        long long quantity;              // Contract count
        QString price;                   // null (coordinator derives from terms)
        QString idempotency_key;         // Stable, app-generated
        QString funding_pubkey_hex;      // 33-byte compressed secp256k1
    };
    std::optional<Order> create_order(const QString& wallet_token,
                                       const CreateOrderRequest& req);

    /**
     * GET /orders
     * Fetch all orders for the active wallet
     */
    std::vector<Order> get_orders(const QString& wallet_token);

    /**
     * GET /orders/{order_id}
     * Fetch specific order details
     */
    std::optional<Order> get_order(const QString& wallet_token,
                                    const QString& order_id);

    /**
     * POST /orders/{order_id}/cancel
     * Cancel a resting order (only if status permits)
     */
    bool cancel_order(const QString& wallet_token,
                      const QString& order_id,
                      const QString& idempotency_key);

    // ===== Taker Accept Flow =====
    /**
     * POST /orders/{order_id}/accept-context
     * Step 1: Get signing context for taker to sign accept artifacts
     */
    std::optional<AcceptContext> get_accept_context(
        const QString& wallet_token,
        const QString& order_id,
        const QString& funding_pubkey_hex);

    /**
     * POST /orders/{order_id}/accept-match
     * Step 2: Submit local signatures for accept
     *
     * Headers:
     *   Authorization: Bearer <wallet_token>
     *   X-Partner-Token: <partner_token>
     */
    struct SubmitAcceptRequest {
        QString funding_pubkey_hex;
        QString context_fingerprint;     // From accept-context
        QJsonObject context_snapshot;    // Optional diagnostics
        QString idempotency_key;
        std::vector<QString> cet_adaptor_signatures_hex;
        QString refund_signature_hex;
        std::vector<QString> funding_signatures_hex;
    };
    std::optional<AcceptMatchResponse> submit_accept_match(
        const QString& wallet_token,
        const QString& order_id,
        const SubmitAcceptRequest& req);

    // ===== Maker Sign Flow =====
    /**
     * GET /dlcs/{dlc_id}/sign-context
     * Step 1: Get signing context for maker to sign DLC
     */
    std::optional<SignContext> get_sign_context(const QString& wallet_token,
                                                 const QString& dlc_id);

    /**
     * POST /dlcs/{dlc_id}/sign
     * Step 2: Submit maker signatures
     *
     * Headers:
     *   Authorization: Bearer <wallet_token>
     */
    struct SubmitSignRequest {
        QString idempotency_key;
        std::vector<QString> cet_adaptor_signatures_hex;
        QString refund_signature_hex;
        std::vector<QString> funding_signatures_hex;
    };
    std::optional<SignResponse> submit_sign(const QString& wallet_token,
                                             const QString& dlc_id,
                                             const SubmitSignRequest& req);

    // ===== Status & Settlement =====
    /**
     * GET /dlcs/{dlc_id}
     * Fetch DLC status and settlement details
     */
    std::optional<DLCDetail> get_dlc_detail(const QString& wallet_token,
                                             const QString& dlc_id);

    /**
     * GET /dlcs/{dlc_id}/settlement-status
     * Fetch settlement/attestation status
     */
    struct SettlementStatus {
        QString dlc_id;
        QString status;
        QString oracle_outcome;
        QString settlement_type;         // "cet" or "refund"
        QString closing_txid;
    };
    std::optional<SettlementStatus> get_settlement_status(const QString& wallet_token,
                                                           const QString& dlc_id);

    /**
     * GET /dlcs/{dlc_id}/funding-transaction
     * Fetch funding transaction details
     */
    struct FundingTransaction {
        QString funding_txid;
        QString funding_tx_hex;
        int output_index;
        long long amount_sats;
    };
    std::optional<FundingTransaction> get_funding_transaction(const QString& wallet_token,
                                                               const QString& dlc_id);

    /**
     * GET /dlcs/{dlc_id}/payout-data
     * Fetch payout/settlement breakdown
     */
    struct PayoutData {
        QString dlc_id;
        long long payout_satoshi;        // Settlement amount
        QString settlement_address;      // If available
    };
    std::optional<PayoutData> get_payout_data(const QString& wallet_token,
                                               const QString& dlc_id);

    // ===== OpenAPI Schema =====
    /**
     * GET /openapi.json
     * Fetch live coordinator OpenAPI schema for reference
     */
    QJsonObject get_openapi_schema();

private:
    DLCCoordinatorConfig config_;

    /**
     * Internal HTTP request helper
     * Automatically adds wallet token and partner token headers
     */
    struct HttpRequest {
        QString method;                  // "GET", "POST", etc.
        QString endpoint;                // "/auth/wallet", "/orders", etc.
        std::optional<QString> wallet_token;  // If omitted, no Authorization header
        QJsonObject body;                // For POST/PUT
        bool include_partner_token = true;
    };

    std::optional<QJsonObject> make_request(const HttpRequest& req);

    /**
     * Error parsing and logging
     */
    struct APIError {
        int http_status;
        QString reason;                  // Machine-readable
        QString detail;                  // Human-readable
        QString request_id;              // For support
    };
    APIError last_error_;

    void log_error(const APIError& err);
};

#endif // GREEN_DLC_COORDINATOR_CLIENT_H
