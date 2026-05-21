/**
 * @file src/dlc/dlc_account_service.h
 * @brief Wallet registration and token management
 */

#pragma once

#include <QString>
#include <optional>
#include "dlc_coordinator_client.h"

/**
 * Service to manage wallet registration with coordinator
 * Handles nonce flow, UTXO proof, token refresh
 */
class DLCAccountService : public QObject {
    Q_OBJECT

public:
    explicit DLCAccountService(Session* session, QObject* parent = nullptr);

    /**
     * Register active wallet with coordinator
     * - Gets nonce from coordinator
     * - Signs nonce with account xprv (never sent)
     * - Collects UTXO proofs
     * - Posts registration
     */
    struct RegistrationRequest {
        QString wallet_label;
        QString xpub;
        std::vector<QString> utxo_outpoints;  // Proofs of funds
    };
    void register_wallet(const RegistrationRequest& req);

    /**
     * Check if wallet token is still valid
     * Refresh if expiring soon
     */
    bool check_and_refresh_token(const QString& wallet_id,
                                 const QString& wallet_token);

    /**
     * Verify coordinator network matches wallet network
     */
    bool verify_network_compatibility();

    /**
     * Verify partner token is valid
     */
    bool verify_partner_token();

signals:
    void registration_success(const WalletRegistration& reg);
    void registration_failed(const QString& error);
    void token_refreshed();
    void readiness_check_failed(const QString& error);

private:
    Session* session_;
    std::unique_ptr<DLCCoordinatorClient> coordinator_client_;

    QString sign_nonce_with_account(const QString& nonce);
    std::vector<QString> get_utxo_proofs(const std::vector<QString>& outpoints);
};
