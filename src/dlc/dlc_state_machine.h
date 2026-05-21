/**
 * @file src/dlc/dlc_state_machine.h
 * @brief State transitions for orders and DLCs
 */

#pragma once

#include <QString>
#include <QObject>
#include <map>

/**
 * Order state machine
 * Tracks valid transitions and enforces business logic
 */
class OrderStateMachine : public QObject {
    Q_OBJECT

public:
    enum State {
        // Creation
        CREATED_LOCALLY,          // User submitted, awaiting coordinator response
        OPEN,                     // Resting in orderbook
        PENDING_ACCEPT,           // Matched as taker, awaiting taker acceptance
        
        // Terminal
        FILLED,                   // Matched and signed
        CANCELLED,                // User or coordinator cancelled
        EXPIRED,                  // Instrument expired
        REJECTED,                 // Coordinator rejected
    };

    explicit OrderStateMachine(const QString& order_id);

    State current_state() const { return m_state; }
    QString state_name() const;

    // Transitions
    bool transition_to(State next_state);
    bool can_transition_to(State next_state) const;

    // Query
    bool is_open_and_cancellable() const;
    bool requires_taker_action() const;
    bool is_terminal() const;

private:
    QString order_id_;
    State m_state = CREATED_LOCALLY;

    bool is_valid_transition(State current, State next) const;
};

/**
 * DLC state machine
 * Tracks protocol lifecycle and settlement
 */
class DLCStateMachine : public QObject {
    Q_OBJECT

public:
    enum State {
        // Offer & Accept
        OFFER_CREATED,
        ACCEPTED,

        // Signing
        SIGN_REQUIRED,            // Maker must sign
        SIGNED,

        // Funding & Active
        FUNDING_TX_CREATED,
        FUNDING_BROADCASTED,
        FUNDING_CONFIRMED,
        MATURED,

        // Settlement
        ORACLE_ATTESTED,          // Oracle has published attestation
        CET_BROADCASTED,          // Winning CET broadcast
        CET_CONFIRMED,
        REFUND_BROADCASTED,       // Fallback refund broadcast
        REFUND_CONFIRMED,

        // Terminal
        CLOSED,                   // All confirmations done
        FAILED,
    };

    explicit DLCStateMachine(const QString& dlc_id);

    State current_state() const { return m_state; }
    QString state_name() const;

    bool transition_to(State next_state);
    bool is_terminal() const;

    // Query for UI
    bool is_active() const;                   // Can still receive updates
    bool awaiting_oracle() const;
    bool settlement_pending() const;

private:
    QString dlc_id_;
    State m_state = OFFER_CREATED;

    bool is_valid_transition(State current, State next) const;
};
