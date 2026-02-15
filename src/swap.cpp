#include "swap.h"

#include <memory>

#include <QDebug>
#include <QFutureWatcher>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>
#include <QtConcurrentRun>

namespace {

template <typename T> QString swapId(const std::shared_ptr<T>& response)
{
    return QString::fromStdString(response->swap_id());
}

Swap::Status statusFromState(lwk::PaymentState state)
{
    switch (state) {
    case lwk::PaymentState::kContinue: return Swap::Status::Pending;
    case lwk::PaymentState::kSuccess: return Swap::Status::Completed;
    case lwk::PaymentState::kFailed: return Swap::Status::Failed;
    default: Q_UNREACHABLE();
    }
}

} // namespace

Swap::Swap(const QString& id, Context* context)
    : ContextTransaction(id, context)
{
}

void Swap::sync()
{
    using Watcher = QFutureWatcher<Swap::Status>;
    const auto watcher = new Watcher(this);
    watcher->setFuture(QtConcurrent::run([=, this] {
        try {
            auto state = advance();
            return statusFromState(state);
        } catch(lwk::lwk_error::NoBoltzUpdate error) {
            return Status::Pending;
        } catch(...) {
            return Status::Failed;
        }
    }));
    connect(watcher, &Watcher::finished, this, [=, this] {
        watcher->deleteLater();
        setStatus(watcher->result());
        if (m_status == Status::Pending) {
            QTimer::singleShot(1000, this, &Swap::sync);
        }
    });
}

void Swap::setStatus(Status status)
{
    if (m_status == status) return;
    m_status = status;
    emit statusChanged();
}

ReverseSwap::ReverseSwap(std::shared_ptr<lwk::InvoiceResponse> invoice_response, Context *context)
    : Swap(swapId(invoice_response), context)
    , m_invoice_response(invoice_response)
{
}

QVariantMap ReverseSwap::data() const
{
    try {
        return {
            { "fee", QVariant::fromValue<uint64_t>(m_invoice_response->fee() ? *m_invoice_response->fee() : 0) },
            { "invoice", QString::fromStdString(m_invoice_response->bolt11_invoice()->to_string()) },
        };
    } catch (...) {
        return {};
    }
}

lwk::PaymentState ReverseSwap::advance()
{
    return m_invoice_response->advance();
}

SubmarineSwap::SubmarineSwap(std::shared_ptr<lwk::PreparePayResponse> prepare_pay_response, Context* context)
    : Swap(swapId(prepare_pay_response), context)
    , m_prepare_pay_response(prepare_pay_response)
{
}

SubmarineSwap::SubmarineSwap(const QString& address, uint64_t amount, Context* context)
    : Swap(QUuid::createUuid().toString(QUuid::WithoutBraces), context)
    , m_address(address)
    , m_amount(amount)
{
}

QVariantMap SubmarineSwap::data() const
{
    if (!m_prepare_pay_response) {
        return {
            { "address", m_address },
            { "amount", QString::number(m_amount) }
        };
    }
    try {
        return {
            { "address", QString::fromStdString(m_prepare_pay_response->uri_address()->to_string()) },
            { "amount", QString::number(m_prepare_pay_response->uri_amount()) },
            { "boltz_fee", QString::number(m_prepare_pay_response->boltz_fee().value_or(0)) },
            { "fee", QString::number(m_prepare_pay_response->fee().value_or(0)) }
        };
    } catch (...) {
        return {};
    }
}

lwk::PaymentState SubmarineSwap::advance()
{
    if (!m_prepare_pay_response) return lwk::PaymentState::kSuccess;
    return m_prepare_pay_response->advance();
}

ChainSwap::ChainSwap(std::shared_ptr<lwk::LockupResponse> lockup_response, Context* context)
    : Swap(swapId(lockup_response), context)
    , m_lockup_response(lockup_response)
{
}

QVariantMap ChainSwap::data() const
{
    try {
        QVariantMap result;
        if (m_lockup_response->boltz_fee()) {
            result.insert("boltzFee", QString::number(*m_lockup_response->boltz_fee()));
        }
        if (m_lockup_response->fee()) {
            result.insert("fee", QString::number(*m_lockup_response->fee()));
        }
        result.insert("chainFrom", QString::fromStdString(m_lockup_response->chain_from()));
        result.insert("chainTo", QString::fromStdString(m_lockup_response->chain_to()));
        result.insert("expectedAmount", QString::number(m_lockup_response->expected_amount()));
        result.insert("lockupAddress", QString::fromStdString(m_lockup_response->lockup_address()));
        return result;
    } catch (...) {
        return {};
    }
}

lwk::PaymentState ChainSwap::advance()
{
    return m_lockup_response->advance();
}
