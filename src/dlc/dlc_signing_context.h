#pragma once

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

namespace dlc {

struct SigningInput {
    std::string tx_hex;           // unsigned transaction
    std::vector<int> input_indices; // which inputs to sign
    std::string pubkey;            // signing pubkey
};

struct SignedCET {
    std::string cet_hex;
    std::vector<std::string> signatures;  // one per input
};

class DLCSigningContext {
public:
    // Constructor receives a reference to the wallet's HD key provider
    // The key provider should implement derivation but NOT expose private keys
    explicit DLCSigningContext(const std::string& account_xprv_path);
    
    // Sign funding transaction (wallet-initiated)
    std::string sign_funding_tx(const std::string& funding_tx_hex);

    // Sign CET (commitment transaction) with given outcome
    SignedCET sign_cet(const std::string& cet_hex,
                       const std::string& outcome_path);

    // Sign refund transaction
    std::string sign_refund_tx(const std::string& refund_tx_hex);

    // Verify coordinator-provided signatures (before accepting)
    bool verify_coordinator_signature(const std::string& message,
                                      const std::string& signature,
                                      const std::string& coordinator_pubkey);

private:
    // Private key access - only here, never passed externally
    std::string derive_and_sign(const std::string& path,
                                const std::string& message_hash);
};

}  // namespace dlc
