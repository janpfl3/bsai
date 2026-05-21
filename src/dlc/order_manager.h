#pragma once

#include "dlc_coordinator_client.h"
#include "dlc_contract_state.h"
#include <memory>
#include <map>

namespace dlc {

enum class OrderRole {
    TAKER,    // accepting an order from book
    MAKER     // resting order waiting to be matched
};

struct PlaceOrderRequest {
    std::string symbol;
    std::string side;              // "buy" or "sell"
    long long price_sats;
    long long quantity;            // notional or contracts
    long long collateral_sats;
    OrderRole role;
    std::string expiry;            // relative: "1h", "1d"
};

struct OrderStatus {
    std::string order_id;
    std::string symbol;
    std::string side;
    long long price;
    long long quantity;
    std::string status;            // "open", "matched", "filled", "cancelled"
    std::vector<std::string> matched_dlc_ids;
    int64_t created_at_ms;
    int64_t expires_at_ms;
};

class OrderManager {
public:
    explicit OrderManager(std::shared_ptr<DLCCoordinatorClient> client);

    // Place a new order
    std::string place_order(const PlaceOrderRequest& req);

    // Taker: accept a matched order (initiate contract signing)
    bool accept_matched_order(const std::string& matched_order_id);

    // Taker: sign CETs for matched order
    bool sign_and_submit_taker_signatures(const std::string& dlc_id);

    // Maker: sign CETs when resting order matched
    bool sign_and_submit_maker_signatures(const std::string& dlc_id);

    // Cancel a resting order
    bool cancel_order(const std::string& order_id);

    // Query
    OrderStatus get_order_status(const std::string& order_id);
    std::vector<MatchedOrder> get_matched_orders();
    std::vector<OrderStatus> get_open_orders();

private:
    std::shared_ptr<DLCCoordinatorClient> coordinator_;
    std::map<std::string, OrderStatus> local_orders_;
};

}  // namespace dlc
