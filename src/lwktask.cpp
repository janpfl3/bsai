#include "account.h"
#include "address.h"
#include "context.h"
#include "lwk/lwk.hpp"
#include "lwktask.h"
#include "network.h"
#include "util.h"

#include <leveldb/db.h>

#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <wally_wrapper.h>

#include <QByteArray>
#include <QDebug>
#include <QJsonObject>
#include <QJsonDocument>
#include <QString>
#include <QtConcurrentRun>

struct Logger : public lwk::Logging
{
    void log(const lwk::LogLevel& level, const std::string& message) override
    {
        switch (int(level)) {
        // case 1: qDebug() << "lwk(debug)" << message.c_str(); break;
        case 2: qInfo() << "lwk(info)" << message.c_str(); break;
        case 3: qWarning() << "lwk(warn)" << message.c_str(); break;
        case 4: qCritical() << "lwk(error)" << message.c_str(); break;
        default: return;
        }
    }
};

struct Store : public lwk::ForeignStore
{
    static leveldb::DB* db;
    static int count;
    Store()
    {
        qDebug() << Q_FUNC_INFO << count;
        if (!db) {
            leveldb::Options options;
            options.create_if_missing = true;
            auto status = leveldb::DB::Open(options, GetDataDir("lwk").toStdString(), &db);
            Q_ASSERT(status.ok());
        }
        count++;
    }
    ~Store()
    {
        qDebug() << Q_FUNC_INFO << count;
        count--;
        if (count == 0) {
            delete db;
            db = nullptr;
        }
    }
    std::optional<std::vector<uint8_t>> get(const std::string &key) override
    {
        std::string raw;
        auto status = db->Get(leveldb::ReadOptions(), key, &raw);
        if (!status.ok()) return std::nullopt;
        return std::make_optional<std::vector<uint8_t>>(
            reinterpret_cast<const uint8_t*>(raw.data()),
            reinterpret_cast<const uint8_t*>(raw.data()) + raw.size());
    }
    void put(const std::string &key, const std::vector<uint8_t> &value) override
    {
        const auto raw = leveldb::Slice(reinterpret_cast<const char*>(value.data()), value.size());
        db->Put(leveldb::WriteOptions(), key, raw);
    }
    void remove(const std::string &key) override {
        db->Delete(leveldb::WriteOptions(), key);
    }
};

leveldb::DB* Store::db{nullptr};
int Store::count{0};

QString derive_mnemonic(bool mainnet, const QString& mnemonic, const QString& passphrase, int index)
{
    unsigned char seed_bytes[BIP39_SEED_LEN_512];

    bip39_mnemonic_to_seed512(mnemonic.toUtf8().constData(), passphrase.toUtf8().constData(), seed_bytes, BIP39_SEED_LEN_512);

    const uint32_t version = mainnet ? BIP32_VER_MAIN_PRIVATE : BIP32_VER_TEST_PRIVATE;

    ext_key key;
    bip32_key_from_seed(seed_bytes, BIP39_SEED_LEN_512, version, BIP32_FLAG_SKIP_HASH, &key);

    unsigned char entropy_bytes[HMAC_SHA512_LEN];
    size_t entropy_len;

    bip85_get_bip39_entropy(&key, NULL, 12, index, entropy_bytes, HMAC_SHA512_LEN, &entropy_len);

    char* derived_mnemonic_chars;
    bip39_mnemonic_from_bytes(nullptr, entropy_bytes, entropy_len, &derived_mnemonic_chars);

    const auto derived_mnemonic = QString::fromLatin1(derived_mnemonic_chars);

    wally_free_string(derived_mnemonic_chars);

    return derived_mnemonic;
}

class InvoiceControllerPrivate
{
public:
    QString address;
    QString satoshi;
    QString description;
    int timer_id{-1};
    bool busy{false};
    ReverseSwap* swap{nullptr};
};

InvoiceController::InvoiceController(QObject *parent)
    : Controller(parent)
    , d(new InvoiceControllerPrivate)
{
}

InvoiceController::~InvoiceController()
{
    delete d;
}

QString InvoiceController::address() const
{
    return d->address;
}

void InvoiceController::setAddress(const QString& address)
{
    if (d->address == address) return;
    d->address = address;
    emit addressChanged();
    invalidate();
}

QString InvoiceController::satoshi() const
{
    return d->satoshi;
}

void InvoiceController::setSatoshi(const QString& satoshi)
{
    if (d->satoshi == satoshi) return;
    d->satoshi = satoshi;
    emit satoshiChanged();
    if (d->swap) {
        m_context->removeSwap(d->swap);
        d->swap->deleteLater();
        d->swap = nullptr;
        emit swapChanged();
    }
}

QString InvoiceController::description() const
{
    return d->description;
}

void InvoiceController::setDescription(const QString& description)
{
    if (d->description == description) return;
    d->description = description;
    emit descriptionChanged();
}

bool InvoiceController::isBusy() const
{
    return d->busy;
}

ReverseSwap *InvoiceController::swap() const
{
    return d->swap;
}

void InvoiceController::request()
{
    if (d->busy) return;
    d->busy = true;
    emit busyChanged();
    invalidate();
}

void InvoiceController::invalidate()
{
    if (d->timer_id != -1) killTimer(d->timer_id);
    d->timer_id = startTimer(50);
}

bool InvoiceController::isValid() const
{
    if (!m_context) return false;
    if (!m_context->m_boltz_session) return false;
    if (d->address.isEmpty()) return false;
    bool ok = false;
    auto satoshi = d->satoshi.toULongLong(&ok);
    if (!ok || satoshi == 0) return false;
    return true;
}

void InvoiceController::update()
{
    try {
        if (isValid()) {
            auto address = lwk::Address::init(d->address.toStdString());
            auto satoshi = d->satoshi.toULongLong();

            using Watcher = QFutureWatcher<std::shared_ptr<lwk::InvoiceResponse>>;
            const auto watcher = new Watcher(this);
            watcher->setFuture(QtConcurrent::run([=, this]() -> std::shared_ptr<lwk::InvoiceResponse> {
                try {
                    return m_context->m_boltz_session->invoice(satoshi, d->description.toStdString(), address, nullptr);
                } catch (lwk::lwk_error::Generic error) {
                    qDebug() << Q_FUNC_INFO << "generic error";
                    return nullptr;
                } catch (...) {
                    qDebug() << Q_FUNC_INFO << "unknown error";
                    return nullptr;
                }
            }));
            connect(watcher, &Watcher::finished, this, [=, this] {
                watcher->deleteLater();
                auto invoice_response = watcher->result();
                if (invoice_response) {
                    d->swap = new ReverseSwap(invoice_response, m_context);
                    m_context->addSwap(d->swap);
                    emit swapChanged();
                    d->busy = false;
                    emit busyChanged();
                } else {
                    qDebug() << Q_FUNC_INFO << "failed to create invoice, retry";
                    invalidate();
                }
            });
            return;
        }
    } catch (lwk::lwk_error::Generic error) {
        qDebug() << Q_FUNC_INFO << error.what();
    }

    if (d->swap) {
        d->swap = nullptr;
        emit swapChanged();
    }
}

void InvoiceController::timerEvent(QTimerEvent* event)
{
    if (event->timerId() == d->timer_id) {
        killTimer(d->timer_id);
        d->timer_id = -1;
        update();
    }
}

class SubmarineControllerPrivate {
public:
    int timer_id{-1};
    QString payment;
    QString refund_address;
    bool busy{false};
    QVariant error;
    SubmarineSwap* swap{nullptr};
};

SubmarineController::SubmarineController(QObject* parent)
    : Controller(parent)
    , d(new SubmarineControllerPrivate)
{
}

SubmarineController::~SubmarineController()
{
    delete d;
}

QString SubmarineController::payment() const
{
    return d->payment;
}

void SubmarineController::setPayment(const QString& payment)
{
    if (d->payment == payment) return;
    d->payment = payment;
    invalidate();
}

QString SubmarineController::refundAddress() const
{
    return d->refund_address;
}

void SubmarineController::setRefundAddress(const QString& refund_address)
{
    if (d->refund_address == refund_address) return;
    d->refund_address = refund_address;
    invalidate();
}

bool SubmarineController::isBusy() const
{
    return d->busy;
}

QVariant SubmarineController::error() const
{
    return d->error;
}

void SubmarineController::setError(const QVariant& error)
{
    if (d->error == error) return;
    d->error = error;
    emit errorChanged();
}

SubmarineSwap *SubmarineController::swap() const
{
    return d->swap;
}

void SubmarineController::setSwap(SubmarineSwap* swap)
{
    if (d->swap == swap) return;
    d->swap = swap;
    emit swapChanged();
}

void SubmarineController::invalidate()
{
    if (d->timer_id != -1) killTimer(d->timer_id);
    d->timer_id = startTimer(50);
    d->busy = true;
    emit busyChanged();
}

void SubmarineController::timerEvent(QTimerEvent* event)
{
    if (event->timerId() == d->timer_id) {
        killTimer(d->timer_id);
        d->timer_id = -1;
        update();
    }
}

void SubmarineController::update()
{
    if (!m_context->m_boltz_session) return;

    for (const auto swap : m_context->m_swaps) {
        auto submarine_swap = qobject_cast<SubmarineSwap*>(swap);
        if (submarine_swap && submarine_swap->invoice() == d->payment) {
            qDebug() << Q_FUNC_INFO << "found swap for invoice" << submarine_swap->status();
            d->busy = false;
            emit busyChanged();
            setSwap(submarine_swap);
            setError({});
            return;
        }
    }

    typedef std::variant<std::shared_ptr<lwk::PreparePayResponse>, std::pair<std::string, uint64_t>, std::string> Result;
    using Watcher = QFutureWatcher<Result>;
    const auto watcher = new Watcher(this);
    watcher->setFuture(QtConcurrent::run([=, this]() -> Result {
        try {
            auto lightning_payment = lwk::LightningPayment::init(d->payment.toStdString());
            auto address = lwk::Address::init(d->refund_address.toStdString());
            return m_context->m_boltz_session->prepare_pay(lightning_payment, address, nullptr);
        } catch (lwk::lwk_error::Generic error) {
            qDebug() << Q_FUNC_INFO << "generic error"
                << QString::fromStdString(error.msg);
            return error.msg;
        } catch (lwk::lwk_error::MagicRoutingHint error) {
            qDebug() << Q_FUNC_INFO << "magic routing hint error"
                << QString::fromStdString(error.uri)
                << QString::fromStdString(error.address)
                << error.amount;
            return std::make_pair(error.address, error.amount);
        } catch (...) {
            qDebug() << Q_FUNC_INFO << "unexpected error";
            return std::string();
        }
    }));
    connect(watcher, &Watcher::finished, this, [=, this] {
        watcher->deleteLater();
        const auto result = watcher->result();
        d->busy = false;
        emit busyChanged();

        if (result.index() == 0) {
            auto prepare_pay_response = std::get<0>(result);
            auto swap = new SubmarineSwap(d->payment, prepare_pay_response, context());
            context()->addSwap(swap);
            setSwap(swap);
            setError({});
            return;
        }

        if (result.index() == 1) {
            auto swap = new SubmarineSwap(QString::fromStdString(std::get<1>(result).first), std::get<1>(result).second, context());
            context()->addSwap(swap);
            setSwap(swap);
            setError({});
            return;
        }

        qDebug() << Q_FUNC_INFO << "failed to create swap";
        setSwap(nullptr);
        const auto error = QString::fromStdString(std::get<2>(result));
        if (error.contains("a swap with this invoice exists already")) {
            setError(QString("id_there_is_already_a_swap_in"));
        } else if (error.contains("could not find route to pay invoice")) {
            setError(QString("Could not find route to pay invoice"));
        } else {
            setError(QString(error));
        }
    });
}

LwkCreateSessionTask::LwkCreateSessionTask(Context* context)
    : ContextTask(context)
{
}

void LwkCreateSessionTask::update()
{
    if (m_status != Status::Ready) {
        return;
    }

    setStatus(Status::Active);

    if (m_context->m_boltz_session) {
        setStatus(Status::Finished);
        return;
    }

    if (!m_context->isMainnet()) {
        setStatus(Status::Failed);
        return;
    }

    const auto m = m_context->credentials().value("mnemonic").toString();
    const auto p = m_context->credentials().value("bip39_passphrase").toString();
    const auto r = derive_mnemonic(m_context->isMainnet(), m, p, 26589);

    using Watcher = QFutureWatcher<std::shared_ptr<lwk::BoltzSession>>;
    const auto watcher = new Watcher(this);
    watcher->setFuture(QtConcurrent::run([=, this]() -> std::shared_ptr<lwk::BoltzSession> {
        try {
            auto mnemonic = lwk::Mnemonic::init(r.toStdString());
            auto network = m_context->isMainnet() ? lwk::Network::mainnet() : lwk::Network::testnet();
            auto session = lwk::BoltzSession::from_builder({
                .network = network,
                .client = lwk::AnyClient::from_electrum(network->default_electrum_client()),
                .mnemonic = mnemonic,
                .logging = std::make_shared<Logger>(),
                .polling = true,
                .referral_id = "blockstream",
                .random_preimages = true,
                .store = lwk::ForeignStoreLink::init(std::make_shared<Store>())
            });

            try {
                session->refresh_swap_info();
            } catch (lwk::lwk_error::Generic error) {
                qDebug() << Q_FUNC_INFO << "refresh_swap_info error";
            }

            return session;
        } catch(lwk::lwk_error::Generic error) {
            qDebug() << Q_FUNC_INFO << "generic error";
            return nullptr;
        } catch (...) {
            qDebug() << Q_FUNC_INFO << "unexpected error";
            return nullptr;
        }
    }));
    connect(watcher, &Watcher::finished, this, [=, this] {
        watcher->deleteLater();
        auto session = watcher->result();
        if (!session) {
            setStatus(Status::Failed);
            return;
        }

        for (const auto swap_id : session->pending_swap_ids()) {
            auto data = session->get_swap_data(swap_id);
            if (!data) continue;

            try {
                const auto swap_data = QJsonDocument::fromJson(QByteArray::fromStdString(*data)).object();
                const auto type = swap_data.value("swap_type").toString();
                Swap* swap = nullptr;
                if (type == "submarine") {
                    auto invoice = swap_data.value("bolt11_invoice").toString();
                    swap = new SubmarineSwap(invoice, session->restore_prepare_pay(*data), m_context);
                } else if (type == "chain") {
                    swap = new ChainSwap(session->restore_lockup(*data), m_context);
                } else if (type == "reverse") {
                    swap = new ReverseSwap(session->restore_invoice(*data), m_context);
                } else {
                    qWarning() << Q_FUNC_INFO << "unexpected swap type" << swap_id.c_str() << qPrintable(type);
                }
                if (swap) {
                    m_context->addSwap(swap);
                }
            } catch (lwk::lwk_error::Generic error) {
                qDebug() << Q_FUNC_INFO << "error: " << error.msg.c_str();
                qDebug() << Q_FUNC_INFO << "swap: " << data->c_str();
            }
        }

        m_context->m_boltz_session = session;

        setStatus(Status::Finished);
    });
}

class ChainSwapControllerPrivate
{
public:
    int timer_id{-1};
    QJsonObject swaps_info;
    Address* refund_address{nullptr};
    Address* claim_address{nullptr};
    QString amount;
    bool busy{false};
    ChainSwap* swap{nullptr};
    bool commited{false};
};

ChainSwapController::ChainSwapController(QObject* parent)
    : Controller(parent)
    , d(new ChainSwapControllerPrivate)
{
    invalidate();
}

ChainSwapController::~ChainSwapController()
{
    if (!d->commited && d->swap) {
        context()->removeSwap(d->swap);
        d->swap->deleteLater();
    }
    delete d;
}

QString ChainSwapController::amount() const
{
    return d->amount;
}

void ChainSwapController::setAmount(const QString& amount)
{
    if (d->amount == amount) return;
    d->amount = amount;
    emit amountChanged();
    invalidate();
}

ChainSwap *ChainSwapController::swap() const
{
    return d->swap;
}

void ChainSwapController::commit()
{
    d->commited = true;
}

Address* ChainSwapController::refundAddress() const
{
    return d->refund_address;
}

void ChainSwapController::setRefundAddress(Address* refund_address)
{
    if (d->refund_address == refund_address) return;
    d->refund_address = refund_address;
    emit refundAddressChanged();
    invalidate();
}

Address* ChainSwapController::claimAddress() const
{
    return d->claim_address;
}

void ChainSwapController::setClaimAddress(Address* claim_address)
{
    if (d->claim_address == claim_address) return;
    d->claim_address = claim_address;
    emit claimAddressChanged();
    invalidate();
}

void ChainSwapController::invalidate()
{
    if (d->timer_id != -1) killTimer(d->timer_id);
    d->timer_id = startTimer(50);
    d->busy = true;
    emit busyChanged();
}

void ChainSwapController::timerEvent(QTimerEvent* event)
{
    if (event->timerId() == d->timer_id) {
        killTimer(d->timer_id);
        d->timer_id = -1;
        update();
    }
}

bool ChainSwapController::isValid() const
{
    if (!context()) return false;

    if (!d->claim_address) return false;
    if (!d->refund_address) return false;
    bool ok = false;
    uint64_t amount = d->amount.toULongLong(&ok);
    if (!ok || amount == 0) return false;
    return true;
}

bool ChainSwapController::isBusy() const
{
    return d->busy;
}

void ChainSwapController::update()
{
    if (!m_context->m_boltz_session) return;

    if (!isValid()) {
        if (d->busy) {
            d->busy = false;
            emit busyChanged();
        }
        if (d->swap) {
            d->swap = nullptr;
            emit swapChanged();
        }
        return;
    }

    using Watcher = QFutureWatcher<std::pair<bool, std::shared_ptr<lwk::LockupResponse>>>;
    const auto watcher = new Watcher(this);
    const auto session = m_context->m_boltz_session;
    uint64_t amount = d->amount.toULongLong();
    watcher->setFuture(QtConcurrent::run([=, this]() -> std::pair<bool, std::shared_ptr<lwk::LockupResponse>> {
        try {
            auto claim_network = d->claim_address->account()->network();
            auto refund_network = d->refund_address->account()->network();

            std::shared_ptr<lwk::LockupResponse> lockup_response;
            if (claim_network->isLiquid() && !refund_network->isLiquid()) {
                auto refund_address = lwk::BitcoinAddress::init(d->refund_address->address().toStdString());
                auto claim_address = lwk::Address::init(d->claim_address->address().toStdString());

                lockup_response = session->btc_to_lbtc(amount, refund_address, claim_address, nullptr);
            }
            if (!claim_network->isLiquid() && refund_network->isLiquid()) {
                auto refund_address = lwk::Address::init(d->refund_address->address().toStdString());
                auto claim_address = lwk::BitcoinAddress::init(d->claim_address->address().toStdString());

                lockup_response = session->lbtc_to_btc(amount, refund_address, claim_address, nullptr);
            }
            return std::make_pair(true, lockup_response);
        } catch (...) {
            return std::make_pair(false, nullptr);
        }
    }));
    connect(watcher, &Watcher::finished, this, [=, this] {
        watcher->deleteLater();
        auto result = watcher->result();
        if (!result.first) {
            d->timer_id = startTimer(200);
            return;
        }
        if (d->busy) {
            d->busy = false;
            emit busyChanged();
        }
        if (result.second) {
            d->swap = new ChainSwap(result.second, m_context);
            m_context->addSwap(d->swap);
            emit swapChanged();
        }
    });
}

QVariantMap parseQuote(const lwk::Quote& quote)
{
    return {
        { "send_amount", QString::number(quote.send_amount) },
        { "receive_amount", QString::number(quote.receive_amount) },
        { "network_fee", QString::number(quote.network_fee) },
        { "boltz_fee", QString::number(quote.boltz_fee) },
        { "min", QString::number(quote.min) },
        { "max", QString::number(quote.max) }
    };
}

class SwapQuoteControllerPrivate
{
public:
    QVariantMap quote;
    lwk::SwapAsset send_asset{lwk::SwapAsset::kOnchain};
    lwk::SwapAsset receive_asset{lwk::SwapAsset::kLiquid};
    bool send_amount{true};
    QString amount;
};

SwapQuoteController::SwapQuoteController(QObject* parent)
    : Controller(parent)
    , d(new SwapQuoteControllerPrivate)
{
    connect(this, &Controller::contextChanged, this, &SwapQuoteController::update);
}

SwapQuoteController::~SwapQuoteController()
{
    delete d;
}

bool SwapQuoteController::isLightning() const
{
    return d->send_asset == lwk::SwapAsset::kLightning || d->receive_asset == lwk::SwapAsset::kLightning;
}

void SwapQuoteController::setLightning(bool lightning)
{
    if (d->send_asset == lwk::SwapAsset::kOnchain) {
        d->send_asset = lwk::SwapAsset::kLightning;
    } else if (d->receive_asset == lwk::SwapAsset::kOnchain) {
        d->receive_asset = lwk::SwapAsset::kLightning;
    }
    update();
}

QVariantMap SwapQuoteController::quote() const
{
    return d->quote;
}

QString SwapQuoteController::receiveNetworkKey() const
{
    return d->receive_asset == lwk::SwapAsset::kLiquid ? "liquid" : "bitcoin";
}

QString SwapQuoteController::sendNetworkKey() const
{
    return d->send_asset == lwk::SwapAsset::kLiquid ? "liquid" : "bitcoin";
}

void SwapQuoteController::receive(const QString& amount)
{
    if (!context()) return;
    if (d->amount == amount) return;
    d->send_amount = false;
    d->amount = amount;
    update();
}

void SwapQuoteController::send(const QString& amount)
{
    if (!context()) return;
    if (d->amount == amount) return;
    d->send_amount = true;
    d->amount = amount;
    update();
}

void SwapQuoteController::swapNetworks()
{
    if (!context()) return;
    qSwap(d->send_asset, d->receive_asset);
    update();
}

void SwapQuoteController::update()
{
    if (!m_context) return;
    if (!m_context->m_boltz_session) return;

    bool ok = false;
    uint64_t satoshi = d->amount.toULongLong(&ok);
    if (!ok) {
        d->quote.clear();
        emit updated();
        return;
    }

    using Watcher = QFutureWatcher<QVariantMap>;
    const auto watcher = new Watcher(this);
    const auto session = m_context->m_boltz_session;
    watcher->setFuture(QtConcurrent::run([=, this]() -> QVariantMap {
        try {
            auto builder = d->send_amount ? session->quote(satoshi) : session->quote_receive(satoshi);

            builder->send(d->send_asset);
            builder->receive(d->receive_asset);

            return parseQuote(builder->build());
        } catch (...) {
            return {};
        }
    }));
    connect(watcher, &Watcher::finished, this, [=, this] {
        watcher->deleteLater();
        d->quote = watcher->result();
        emit updated();
    });
}
