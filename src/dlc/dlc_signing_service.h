/**
 * @file src/dlc/dlc_signing_service.h
 * @brief Local signing for DLC CETs, refund, and funding inputs
 */

#pragma once

#include <QString>
#include <vector>
#include "session.h"

/**
 * Service to produce DLC signatures locally without exposing private keys
 */
class DLCSigningService {
public:
    /**
     * Constructor receives reference to wallet's key derivation system
     * (e.g., GDK session for local signing)
     */
    explicit DLCSigningService(Session* session);

    /**
     * Sign a CET adaptor signature
     *
     * @param message_hash_hex   Hash to sign (from accept-context)
     * @param adaptor_point_hex  Oracle adaptor point (from accept-context)
     * @param funding_pubkey_derivation_path  BIP32 path for key derivation
     * @return Adaptor signature hex, or empty on error
     */
    QString sign_cet_adaptor(
        const QString& message_hash_hex,
        const QString& adaptor_point_hex,
        const QString& funding_pubkey_derivation_path);

    /**
     * Sign refund transaction
     *
     * @param refund_sighash_hex  Refund hash (from accept-context)
     * @param funding_pubkey_derivation_path
     * @return Refund signature hex, or empty on error
     */
    QString sign_refund(
        const QString& refund_sighash_hex,
        const QString& funding_pubkey_derivation_path);

    /**
     * Sign funding input
     *
     * @param funding_input_sighash_hex  Sighash for this input
     * @param funding_pubkey_derivation_path
     * @return Funding signature hex, or empty on error
     */
    QString sign_funding_input(
        const QString& funding_input_sighash_hex,
        const QString& funding_pubkey_derivation_path);

    /**
     * Batch sign CET adaptor jobs
     * Preserves coordinator order exactly
     */
    struct SigningResult {
        bool success = false;
        std::vector<QString> cet_adaptor_signatures;
        QString refund_signature;
        std::vector<QString> funding_signatures;
        QString error_message;
    };
    SigningResult sign_accept_context(
        const std::vector<QString>& message_hashes,
        const std::vector<QString>& adaptor_points,
        const QString& refund_sighash,
        const std::vector<QString>& funding_sighashes,
        const QString& funding_pubkey_derivation_path);

private:
    Session* session_;

    // Helper to derive public key for verification
    QString derive_pubkey_at_path(const QString& derivation_path);

    // Helper to call GDK signing
    QString call_gdk_sign(const QString& hash_hex,
                         const QString& derivation_path);
};
