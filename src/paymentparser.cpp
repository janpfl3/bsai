#include "paymentparser.h"
#include "lwk/lwk.hpp"

#include <memory>

namespace {

void fill(QVariantMap& payment, std::shared_ptr<lwk::BitcoinAddress> address)
{
    const bool is_mainnet = address->is_mainnet();
    payment.insert("network", is_mainnet ? "bitcoin" : "testnet");
    payment.insert("address", QString::fromStdString(address->to_string()));
}

void fill(QVariantMap& payment, std::shared_ptr<lwk::Address> address)
{
    const bool is_mainnet = address->network()->is_mainnet();
    payment.insert("network", is_mainnet ? "liquid" : "testnet-liquid");
    payment.insert("address", QString::fromStdString(address->to_string()));
}

void fill(QVariantMap& payment, std::shared_ptr<lwk::Bip21> bip21)
{
    fill(payment, bip21->address());
    if (bip21->amount().has_value()) {
        payment.insert("amount", QString::number(*bip21->amount()));
    }
    if (bip21->label().has_value()) {
        payment.insert("label", QString::fromStdString(*bip21->label()));
    }
    if (bip21->message().has_value()) {
        payment.insert("message", QString::fromStdString(*bip21->message()));
    }
}

void fill(QVariantMap& payment, lwk::LiquidBip21 liquid_bip21)
{
    fill(payment, liquid_bip21.address);
    payment.insert("asset_id", QString::fromStdString(liquid_bip21.asset));
    if (liquid_bip21.satoshi.has_value()) {
        payment.insert("amount", QString::number(*liquid_bip21.satoshi));
    }
}

void fill(QVariantMap& payment, std::shared_ptr<lwk::Bolt11Invoice> invoice) {
    if (invoice->amount_milli_satoshis().has_value()) {
        payment.insert("amount_milli_satoshis", QString::number(*invoice->amount_milli_satoshis()));
    }
    payment.insert("expiry_time", QString::number(invoice->expiry_time()));
    payment.insert("invoice", QString::fromStdString(invoice->to_string()));
    payment.insert("invoice_description", QString::fromStdString(invoice->invoice_description()));
    payment.insert("min_final_cltv_expiry_delta", QString::number(invoice->min_final_cltv_expiry_delta()));
    payment.insert("network", QString::fromStdString(invoice->network()).toLower());
    if (invoice->payee_pub_key().has_value()) {
        payment.insert("payee_pub_key", QString::fromStdString(*invoice->payee_pub_key()));
    }
    payment.insert("payment_hash", QString::fromStdString(invoice->payment_hash()));
    payment.insert("payment_secret", QString::fromStdString(invoice->payment_secret()));
    payment.insert("timestamp", QDateTime::fromSecsSinceEpoch(invoice->timestamp()));
}
}

class PaymentParserPrivate
{
public:
    QString input;
    QVariantMap payment;
};

PaymentParser::PaymentParser(QObject* parent)
    : QObject(parent)
    , d(new PaymentParserPrivate)
{
}

PaymentParser::~PaymentParser()
{
    delete d;
}

QString PaymentParser::input() const
{
    return d->input;
}

void PaymentParser::setInput(const QString& input)
{
    if (d->input == input) return;
    d->input = input;
    emit inputChanged();

    QVariantMap result;
    try {
        auto payment = lwk::Payment::init(input.toStdString());
        if (payment->kind() == lwk::PaymentKind::kBitcoinAddress) {
            result.insert("valid", true);
            result.insert("type", "address");
            fill(result, payment->bitcoin_address());
        } else if (payment->kind() == lwk::PaymentKind::kLiquidAddress) {
            result.insert("valid", true);
            result.insert("type", "address");
            fill(result, payment->liquid_address());
        } else if (payment->kind() == lwk::PaymentKind::kLightningInvoice) {
            result.insert("valid", true);
            result.insert("type", "lightning_invoice");
            fill(result, payment->lightning_invoice());
        } else if (payment->kind() == lwk::PaymentKind::kBip21) {
            result.insert("valid", true);
            result.insert("type", "bip21");
            fill(result, payment->bip21());
        } else if (payment->kind() == lwk::PaymentKind::kLiquidBip21) {
            result.insert("valid", true);
            result.insert("type", "bip21");
            fill(result, *payment->liquid_bip21());
        } else if (payment->kind() == lwk::PaymentKind::kLightningOffer) {
            result.insert("valid", true);
            result.insert("type", "bolt12");
            if (payment->lightning_offer()) {
                result.insert("lightning_offer", QString::fromStdString(*payment->lightning_offer()));
            }
        } else if (payment->kind() == lwk::PaymentKind::kLnUrl) {
            result.insert("valid", true);
            result.insert("type", "lnurl");
            if (payment->lnurl()) {
                result.insert("lnurl", QString::fromStdString(*payment->lnurl()));
            }
        } else if (payment->kind() == lwk::PaymentKind::kBip353) {
            result.insert("valid", true);
            result.insert("type", "bip353");
        } else {
            qDebug() << Q_FUNC_INFO << "unhandled kind" << int(payment->kind());
            result.insert("valid", false);
        }
    } catch (lwk::lwk_error::Generic) {
        qDebug() << Q_FUNC_INFO << "error";
        result.insert("valid", false);
    }
    result.insert("string", input);
    d->payment = result;
    emit updated();
}

QVariantMap PaymentParser::payment() const
{
    return d->payment;
}
