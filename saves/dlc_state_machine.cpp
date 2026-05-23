#include "dlc_state_machine.h"
#include <QDateTime>
#include <QDebug>

DLCStateMachine::DLCStateMachine(DLCCoordinatorClient* client, const QString& wallet_token, QObject* parent)
    : QObject(parent)
    , m_client(client)
    , m_poll_timer(new QTimer(this))
    , m_default_wallet_token(wallet_token)
{
    Q_ASSERT(m_client);

    // Polling timer
    connect(m_poll_timer, &QTimer::timeout, this, &DLCStateMachine::on_poll_timer);
}

DLCStateMachine::~DLCStateMachine()
{
    stop_polling();
}

void DLCStateMachine::set_polling_interval_ms(int interval)
{
    m_polling_interval_ms = interval;
    if (m_poll_timer->isActive()) {
        m_poll_timer->setInterval(interval);
    }
}

void DLCStateMachine::set_max_retries(int max_retries)
{
    m_max_retries = max_retries;
}

void DLCStateMachine::track_order(const Order& order)
{
    OrderOperation op;
    op.order_id = order.order_id;
    op.dlc_id = order.dlc_id;
    op.instrument_id = order.instrument_id;
    op.side = order.side;
    op.match_role = order.pending_match_accept ? "taker" : (order.is_maker ? "maker" : "");
    op.state = OperationState::Idle;
    op.status_text = "Tracking order";

    // If caller supplied a wallet token in order.offer/metadata, it should be set on op.wallet_token.
    // Otherwise the default provided to the state machine constructor will be used.
    op.wallet_token = m_default_wallet_token;

    m_operations[order.order_id] = op;

    // If this is a taker pending accept, start immediately
    if (order.pending_match_accept && op.match_role == "taker") {
        set_operation_state(order.order_id, OperationState::TakerAwaitingAcceptContext, "Fetching accept context");
        advance_taker_operation(m_operations[order.order_id]);
    }
}

void DLCStateMachine::untrack_order(const QString& order_id)
{
    m_operations.remove(order_id);
    m_retry_counts.remove(order_id);
}

DLCStateMachine::OrderOperation* DLCStateMachine::get_operation(const QString& order_id)
{
    auto it = m_operations.find(order_id);
    if (it != m_operations.end()) {
        return &it.value();
    }
    return nullptr;
}

void DLCStateMachine::submit_taker_accept_signatures(
    const QString& order_id,
    const QStringList& cet_adaptor_sigs,
    const QString& refund_sig,
    const QStringList& funding_sigs)
{
    auto op = get_operation(order_id);
    if (!op) return;

    if (op->accept_context.order_id.isEmpty()) {
        qWarning() << "No accept context for order" << order_id;
        return;
    }

    set_operation_state(order_id, OperationState::TakerSubmittingAccept, "Submitting accept signatures");

    DLCCoordinatorClient::SubmitAcceptRequest req;
    req.funding_pubkey_hex = op->funding_pubkey_hex;
    req.context_fingerprint = op->accept_context.context_fingerprint;
    req.idempotency_key = QString("accept:%1:%2").arg(order_id, op->accept_context.context_fingerprint);
    req.cet_adaptor_signatures_hex = std::vector<QString>(cet_adaptor_sigs.begin(), cet_adaptor_sigs.end());
    req.refund_signature_hex = refund_sig;
    req.funding_signatures_hex = std::vector<QString>(funding_sigs.begin(), funding_sigs.end());

    const QString wallet_token = op->wallet_token.isEmpty() ? m_default_wallet_token : op->wallet_token;

    auto resp = m_client->submit_accept_match(wallet_token, order_id, req);
    if (resp.has_value()) {
        // Accept matched/accepted; update op and emit success
        set_operation_state(order_id, OperationState::TakerAcceptSubmitted, "Accept submitted successfully");
        emit taker_accept_completed(order_id);
    } else {
        // failure handling: retry logic
        qWarning() << "submit_accept_match failed for order" << order_id;
        // treat as an accept submit failure
        on_accept_submit_failed(QString("Failed to submit accept for %1").arg(order_id));
    }
}

void DLCStateMachine::submit_maker_sign_signatures(
    const QString& dlc_id,
    const QStringList& cet_adaptor_sigs,
    const QString& refund_sig,
    const QStringList& funding_sigs)
{
    // Find operation by dlc_id
    auto it = m_operations.begin();
    while (it != m_operations.end()) {
        if (it.value().dlc_id == dlc_id) {
            break;
        }
        ++it;
    }
    if (it == m_operations.end()) {
        qWarning() << "No operation for DLC" << dlc_id;
        return;
    }

    auto& op = it.value();
    set_operation_state(op.order_id, OperationState::MakerSubmittingSign, "Submitting sign signatures");

    DLCCoordinatorClient::SubmitSignRequest req;
    req.idempotency_key = QString("sign:%1").arg(dlc_id);
    req.cet_adaptor_signatures_hex = std::vector<QString>(cet_adaptor_sigs.begin(), cet_adaptor_sigs.end());
    req.refund_signature_hex = refund_sig;
    req.funding_signatures_hex = std::vector<QString>(funding_sigs.begin(), funding_sigs.end());

    const QString wallet_token = op.wallet_token.isEmpty() ? m_default_wallet_token : op.wallet_token;

    auto opt_resp = m_client->submit_sign(wallet_token, dlc_id, req);
    if (opt_resp.has_value()) {
        SignResponse resp = *opt_resp;
        // Update operation on success
        set_operation_state(op.order_id, OperationState::MakerSignSubmitted, "Sign submitted successfully");
        emit maker_sign_completed(dlc_id);
        if (resp.funding_broadcasted && !resp.funding_txid.isEmpty()) {
            set_operation_state(op.order_id, OperationState::DLCFunded, QString("DLC funded: %1").arg(resp.funding_txid));
            emit dlc_funded(dlc_id, resp.funding_txid);
        }
    } else {
        qWarning() << "submit_sign failed for DLC" << dlc_id;
        on_sign_submit_failed(QString("Failed to submit sign for %1").arg(dlc_id));
    }
}

void DLCStateMachine::start_polling()
{
    m_poll_timer->setInterval(m_polling_interval_ms);
    m_poll_timer->start();
    qDebug() << "DLC polling started at" << m_polling_interval_ms << "ms interval";
}

void DLCStateMachine::stop_polling()
{
    m_poll_timer->stop();
    qDebug() << "DLC polling stopped";
}

void DLCStateMachine::poll_now()
{
    on_poll_timer();
}

void DLCStateMachine::on_accept_context_ready(AcceptContext /*context*/)
{
    // Not used in synchronous mode (left for compatibility)
}

void DLCStateMachine::on_accept_context_failed(QString /*error*/)
{
    // Not used in synchronous mode
}

void DLCStateMachine::on_accept_submitted(Order /*order*/)
{
    // Not used in synchronous mode
}

void DLCStateMachine::on_accept_submit_failed(QString error)
{
    qWarning() << "Accept submit failed:" << error;
    // Find operation in TakerSubmittingAccept
    for (auto& op : m_operations) {
        if (op.state == OperationState::TakerSubmittingAccept) {
            int& retry_count = m_retry_counts[op.order_id];
            if (retry_count < m_max_retries) {
                retry_count++;
                set_operation_state(op.order_id, OperationState::TakerAwaitingAcceptContext,
                                   QString("Retrying accept context (attempt %1)").arg(retry_count));
                emit operation_retrying(op.order_id, retry_count);
            } else {
                op.state = OperationState::Error;
                op.status_text = error;
                emit operation_failed(op.order_id, error);
            }
        }
    }
}

void DLCStateMachine::on_sign_context_ready(SignContext /*context*/)
{
    // Not used in synchronous mode
}

void DLCStateMachine::on_sign_context_failed(QString /*error*/)
{
    // Not used in synchronous mode
}

void DLCStateMachine::on_sign_submitted(SignResponse /*response*/)
{
    // Not used in synchronous mode
}

void DLCStateMachine::on_sign_submit_failed(QString error)
{
    qWarning() << "Sign submit failed:" << error;
    for (auto& op : m_operations) {
        if (op.state == OperationState::MakerSubmittingSign) {
            int& retry_count = m_retry_counts[op.order_id];
            if (retry_count < m_max_retries) {
                retry_count++;
                set_operation_state(op.order_id, OperationState::MakerAwaitingSignContext,
                                   QString("Retrying sign context (attempt %1)").arg(retry_count));
                emit operation_retrying(op.order_id, retry_count);
            } else {
                op.state = OperationState::Error;
                op.status_text = error;
                emit operation_failed(op.order_id, error);
            }
        }
    }
}

void DLCStateMachine::on_dlc_ready(DLCDetail /*dlc*/)
{
    // Not used in synchronous mode
}

void DLCStateMachine::on_dlc_failed(QString /*error*/)
{
    // Not used in synchronous mode
}

void DLCStateMachine::on_poll_timer()
{
    for (auto& op : m_operations) {
        switch (op.state) {
        case OperationState::TakerAwaitingAcceptContext:
            advance_taker_operation(op);
            break;
        case OperationState::MakerAwaitingSignContext:
            advance_maker_operation(op);
            break;
        case OperationState::DLCSignedAwaitingFunding: {
            const QString wallet_token = op.wallet_token.isEmpty() ? m_default_wallet_token : op.wallet_token;
            auto opt = m_client->get_dlc_detail(wallet_token, op.dlc_id);
            if (opt.has_value()) {
                on_dlc_ready(*opt);
            } else {
                on_dlc_failed(QString("Failed to fetch DLC %1").arg(op.dlc_id));
            }
            break;
        }
        default:
            break;
        }
    }
}

void DLCStateMachine::advance_taker_operation(OrderOperation& op)
{
    if (op.state != OperationState::TakerAwaitingAcceptContext) return;

    qDebug() << "Fetching accept context for order" << op.order_id;
    const QString wallet_token = op.wallet_token.isEmpty() ? m_default_wallet_token : op.wallet_token;
    auto opt_ctx = m_client->get_accept_context(wallet_token, op.order_id, op.funding_pubkey_hex);
    if (opt_ctx.has_value()) {
        op.accept_context = *opt_ctx;
        op.accept_context_fetched = true;
        set_operation_state(op.order_id, OperationState::TakerSigningAccept, "Waiting for local signing");
        // Request wallet to sign locally
        emit request_taker_accept_signatures(op.order_id, op.accept_context, op.wallet_token);
    } else {
        // failed to get accept context; apply retry logic
        qWarning() << "Failed to get accept context for order" << op.order_id;
        int& retry_count = m_retry_counts[op.order_id];
        if (retry_count < m_max_retries) {
            retry_count++;
            set_operation_state(op.order_id, OperationState::TakerAwaitingAcceptContext,
                               QString("Retrying accept context (attempt %1)").arg(retry_count));
            emit operation_retrying(op.order_id, retry_count);
        } else {
            op.state = OperationState::Error;
            op.status_text = "Failed to fetch accept context";
            emit operation_failed(op.order_id, op.status_text);
        }
    }
}

void DLCStateMachine::advance_maker_operation(OrderOperation& op)
{
    if (op.state != OperationState::MakerAwaitingSignContext) return;

    qDebug() << "Fetching sign context for DLC" << op.dlc_id;
    const QString wallet_token = op.wallet_token.isEmpty() ? m_default_wallet_token : op.wallet_token;
    auto opt_ctx = m_client->get_sign_context(wallet_token, op.dlc_id);
    if (opt_ctx.has_value()) {
        op.sign_context = *opt_ctx;
        op.sign_context_fetched = true;
        set_operation_state(op.order_id, OperationState::MakerSigningSign, "Waiting for local signing");
        emit request_maker_sign_signatures(op.dlc_id, op.sign_context, op.wallet_token);
    } else {
        qWarning() << "Failed to get sign context for DLC" << op.dlc_id;
        int& retry_count = m_retry_counts[op.order_id];
        if (retry_count < m_max_retries) {
            retry_count++;
            set_operation_state(op.order_id, OperationState::MakerAwaitingSignContext,
                               QString("Retrying sign context (attempt %1)").arg(retry_count));
            emit operation_retrying(op.order_id, retry_count);
        } else {
            op.state = OperationState::Error;
            op.status_text = "Failed to fetch sign context";
            emit operation_failed(op.order_id, op.status_text);
        }
    }
}

void DLCStateMachine::set_operation_state(const QString& order_id, OperationState state, const QString& status)
{
    auto op = get_operation(order_id);
    if (!op) return;

    op->state = state;
    op->status_text = status;

    log_operation_event(order_id, status);
    emit operation_state_changed(order_id, state, status);
}

void DLCStateMachine::log_operation_event(const QString& order_id, const QString& event)
{
    qDebug() << QDateTime::currentDateTime().toString(Qt::ISODate)
             << "[" << order_id << "]" << event;
}

