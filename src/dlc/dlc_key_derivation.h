#pragma once

#include <string>
#include <vector>

namespace dlc {

// BIP32 derivation for DLC keys (e.g., m/44'/0'/0'/2/0 for options)
class DLCKeyDerivation {
public:
    explicit DLCKeyDerivation(const std::string& account_xpub);

    // Get public key for funding/CETs at index
    std::string get_dlc_pubkey(int index);

    // Get refund key at index
    std::string get_refund_pubkey(int index);

    // Derive oracle pubkey (for oracle signature verification)
    std::string get_oracle_pubkey_for_instrument(const std::string& instrument_id);

private:
    std::string account_xpub_;
    
    // Helper to derive at specific path
    std::string derive_pubkey(const std::string& path);
};

}  // namespace dlc
