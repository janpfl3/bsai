#include "dlc_state_machine.h"
#include <QDateTime>
#include <QDebug>

DLCStateMachine::DLCStateMachine(DLCCoordinatorClient* client, QObject* parent)
    : QObject(parent), m_client(client), m_poll_timer(new QTimer(this))
{
    Q_ASSERT(m_client);
    
    // Connect coordinator signals
    connect(m_client, &DLCCoordinatorClient::acceptContextReady,
            this, &DLCStateMachine::on_accept_context_ready);
    connect(m_client, &DLCCoordinatorClient::acceptContextFailed,
            this, &DLCStateMachine::on_accept_context_failed);
    connect(m_client, &DLCCoordinatorClient::acceptSubmitted,
            this, &DLCStateMachine::on_accept_submitted);
    connect(m_client, &DLCCoordinatorClient::acceptSubmitFailed,
            this, &DLCStateMachine::on_accept_submit_failed);
    
    connect(m_client, &DLCCoordinatorClient::signContextReady,
            this, &DLCStateMachine::on_sign_context_ready);
    connect(m_client, &DLCCoordinatorClient::signContextFailed,
            this, &DLCStateMachine::on_sign_context_failed);
    connect(m_client, &DLCCoordinatorClient::signSubmitted,
            this, &DLCStateMachine::on_sign_submitted);
    connect(m_client, &DLCCoordinatorClient::signSubmitFailed,
            this, &DLCStateMachine::on_sign_submit_failed);
    
    connect(m_client, &DLCCoordinatorClient::dlcReady,
            this, &DLCStateMachine::on_dlc_ready);
    connect(m_client, &DLCCoordinatorClient::dlcFailed,
            this, &DLCStateMachine::on_dlc_failed);
    
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
    
    m_client->submit_accept(
        order_id,
        op->funding_pubkey_hex,
        op->accept_context.context_fingerprint,
        cet_adaptor_sigs,
        refund_sig,
        funding_sigs);
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
    
    m_client->submit_sign(
        dlc_id,
        cet_adaptor_sigs,
        refund_sig,
        funding_sigs);
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

void DLCStateMachine::on_accept_context_ready(AcceptContext context)
{
    auto op = get_operation(context.order_id);
    if (!op) return;
    
    op->accept_context = context;
    op->accept_context_fetched = true;
    
    set_operation_state(context.order_id, OperationState::TakerSigningAccept,
                       "Waiting for local signing");
    
    // Request wallet to sign locally
    emit request_taker_accept_signatures(
        context.order_id,
        context,
        "");  // wallet_id could be passed if available
}

void DLCStateMachine::on_accept_context_failed(QString error)
{
    qWarning() << "Accept context failed:" << error;
    // Find and update operations in TakerAwaitingAcceptContext state
    for (auto& op : m_operations) {
        if (op.state == OperationState::TakerAwaitingAcceptContext) {
            op.state = OperationState::Error;
            op.status_text = error;
            emit operation_failed(op.order_id, error);
        }
    }
}

void DLCStateMachine::on_accept_submitted(Order order)
{
    auto op = get_operation(order.order_id);
    if (!op) return;
    
    set_operation_state(order.order_id, OperationState::TakerAcceptSubmitted,
                       "Accept submitted successfully");
    emit taker_accept_completed(order.order_id);
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

void DLCStateMachine::on_sign_context_ready(SignContext context)
{
    // Find operation by dlc_id
    auto it = m_operations.begin();
    while (it != m_operations.end()) {
        if (it.value().dlc_id == context.dlc_id) {
            break;
        }
        ++it;
    }
    if (it == m_operations.end()) return;
    
    auto& op = it.value();
    op.sign_context = context;
    op.sign_context_fetched = true;
    
    set_operation_state(op.order_id, OperationState::MakerSigningSign,
                       "Waiting for local signing");
    
    // Request wallet to sign locally
    emit request_maker_sign_signatures(
        context.dlc_id,
        context,
        "");  // wallet_id
}

void DLCStateMachine::on_sign_context_failed(QString error)
{
    qWarning() << "Sign context failed:" << error;
    for (auto& op : m_operations) {
        if (op.state == OperationState::MakerAwaitingSignContext) {
            op.state = OperationState::Error;
            op.status_text = error;
            emit operation_failed(op.order_id, error);
        }
    }
}

void DLCStateMachine::on_sign_submitted(SignResponse response)
{
    // Find operation by dlc_id
    auto it = m_operations.begin();
    while (it != m_operations.end()) {
        if (it.value().dlc_id == response.dlc_id) {
            break;
        }
        ++it;
    }
    if (it == m_operations.end()) return;
    
    auto& op = it.value();
    set_operation_state(op.order_id, OperationState::MakerSignSubmitted,
                       "Sign submitted successfully");
    emit maker_sign_completed(response.dlc_id);
    
    if (response.funding_broadcasted && !response.funding_txid.isEmpty()) {
        set_operation_state(op.order_id, OperationState::DLCFunded,
                           QString("DLC funded: %1").arg(response.funding_txid));
        emit dlc_funded(response.dlc_id, response.funding_txid);
    }
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

void DLCStateMachine::on_dlc_ready(DLCDetail dlc)
{
    // Find operation and update based on DLC status
    auto it = m_operations.begin();
    while (it != m_operations.end()) {
        if (it.value().dlc_id == dlc.dlc_id) {
            break;
        }
        ++it;
    }
    if (it == m_operations.end()) return;
    
    auto& op = it.value();
    
    if (dlc.status == "signed") {
        set_operation_state(op.order_id, OperationState::DLCSignedAwaitingFunding,
                           QString("DLC signed, awaiting funding"));
    } else if (dlc.status == "funded") {
        set_operation_state(op.order_id, OperationState::DLCFunded,
                           QString("DLC funded: %1").arg(dlc.funding_txid));
        emit dlc_funded(dlc.dlc_id, dlc.funding_txid);
    } else if (dlc.status == "settled") {
        set_operation_state(op.order_id, OperationState::DLCSettled,
                           QString("DLC settled: %1").arg(dlc.closing_txid));
        emit dlc_settled(dlc.dlc_id, dlc.closing_txid);
    }
}

void DLCStateMachine::on_dlc_failed(QString error)
{
    qWarning() << "DLC query failed:" << error;
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
        case OperationState::DLCSignedAwaitingFunding:
            m_client->get_dlc(op.dlc_id);
            break;
        default:
            break;
        }
    }
}

void DLCStateMachine::advance_taker_operation(OrderOperation& op)
{
    if (op.state != OperationState::TakerAwaitingAcceptContext) return;
    
    // Request accept context
    qDebug() << "Fetching accept context for order" << op.order_id;
    m_client->get_accept_context(op.order_id, op.funding_pubkey_hex);
}

void DLCStateMachine::advance_maker_operation(OrderOperation& op)
{
    if (op.state != OperationState::MakerAwaitingSignContext) return;
    
    // Request sign context
    qDebug() << "Fetching sign context for DLC" << op.dlc_id;
    m_client->get_sign_context(op.dlc_id);
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
