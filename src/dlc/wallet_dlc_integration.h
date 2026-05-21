#pragma once

namespace dlc {

class WalletDLCIntegration {
public:
    // Called once on wallet creation/unlock
    static void register_with_coordinator(
        const std::string& wallet_id,
        const std::vector<std::string>& public_keys,
        const std::string& coordinator_url);

    // Get DLC-specific xpub for key derivation
    static std::string get_dlc_account_xpub(int account_index);

    // Hook for incoming DLC signing requests
    static void on_signing_request_received(
        const std::string& dlc_id,
        const nlohmann::json& signing_context);
};

}  // namespace dlc
