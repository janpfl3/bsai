#ifndef BLOCKSTREAM_SWAP_H
#define BLOCKSTREAM_SWAP_H

#include <QObject>
#include <QQmlEngine>

#include <memory>

#include "context.h"
#include "green.h"
#include "lwk/lwk.hpp"

class Swap : public ContextTransaction
{
    Q_OBJECT
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    Q_PROPERTY(QVariantMap data READ data CONSTANT)
    QML_ELEMENT
    QML_UNCREATABLE("")
public:
    enum class Status {
        Pending,
        Completed,
        Failed,
    };
    Q_ENUM(Status)
    Swap(const QString& id, Context* context);
    Status status() const { return m_status; }
    virtual QVariantMap data() const = 0;
    void sync();
signals:
    void statusChanged();
protected:
    void setStatus(Status status);
    virtual lwk::PaymentState advance() = 0;
private:
    Status m_status{Status::Pending};
};

class ReverseSwap : public Swap
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("")
public:
    ReverseSwap(std::shared_ptr<lwk::InvoiceResponse> invoice_response, Context* context);
    QVariantMap data() const override;
    lwk::PaymentState advance() override;
private:
    std::shared_ptr<lwk::InvoiceResponse> m_invoice_response;
};

class SubmarineSwap : public Swap
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("")
public:
    SubmarineSwap(std::shared_ptr<lwk::PreparePayResponse> prepare_pay_response, Context* context);
    SubmarineSwap(const QString& address, uint64_t amount, Context* context);
    QVariantMap data() const override;
protected:
    lwk::PaymentState advance() override;
private:
    std::shared_ptr<lwk::PreparePayResponse> m_prepare_pay_response;
    QString m_address;
    uint64_t m_amount;
};

class ChainSwap : public Swap
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("")
public:
    ChainSwap(std::shared_ptr<lwk::LockupResponse> lockup_response, Context* context);
    QVariantMap data() const override;
protected:
    lwk::PaymentState advance() override;
private:
    std::shared_ptr<lwk::LockupResponse> m_lockup_response;
};

#endif // BLOCKSTREAM_SWAP_H
