#pragma once

#include <string>
#include <nlohmann/json.hpp>

namespace dlc {

enum class DLCState {
    // Offer phase
    OFFER_CREATED,
    OFFER_SENT,
    OFFER_ACCEPTED_LOCALLY,  // taker accepts
    
    // Signing phase
    AWAITING_COUNTERPARTY_SIGNATURE,
    SIGNATURES_RECEIVED,
    CONTRACT_SIGNED,
    
    // Funding phase
    FUNDING_TX_CREATED,
    FUNDING_TX_BROADCAST,
    FUNDING_CONFIRMED,
    
    // Active
    ACTIVE,
    
    // Settlement
    ORACLE_ATTESTATION_RECEIVED,
    CET_SIGNED_LOCALLY,
    SETTLEMENT_BROADCAST,
    SETTLED,
    
    // Error states
    FAILED,
    CANCELLED
};

struct DLCContractStatus {
    std::string dlc_id;
    std::string order_id;
    DLCState state;
    
    // Participants
    std::string our_role;           // "maker" or "taker"
    std::string counterparty_id;
    
    // Contract details
    std::string instrument_id;
    long long collateral_sats;
    long long payout_recipient;
    
    // Transaction hashes
    std::string funding_tx_id;
    std::string settlement_tx_id;
    
    // Oracle/Settlement info
    std::string oracle_event_id;
    std::string oracle_attestation;
    std::string settlement_outcome;
    int block_height_at_settlement;
    
    // Timestamps
    int64_t created_at_ms;
    int64_t expires_at_ms;
    
    // Error tracking
    std::string last_error;
};

class DLCContractStateMachine {
public:
    DLCContractStateMachine();

    // Transition methods
    void on_offer_created(const std::string& dlc_id, 
                         const nlohmann::json& offer);
    void on_offer_accepted(const std::string& dlc_id);
    void on_signatures_received(const std::string& dlc_id,
                                const nlohmann::json& counterparty_sigs);
    void on_funding_tx_broadcast(const std::string& dlc_id,
                                 const std::string& tx_id);
    void on_funding_confirmed(const std::string& dlc_id);
    void on_settlement_initiated(const std::string& dlc_id,
                                 const std::string& outcome);
    void mark_failed(const std::string& dlc_id, const std::string& reason);
    void mark_cancelled(const std::string& dlc_id);

    // Query
    DLCContractStatus get_status(const std::string& dlc_id) const;
    bool is_valid_transition(DLCState current, DLCState next) const;

private:
    std::map<std::string, DLCContractStatus> contracts_;
};

}  // namespace dlc
