#include "dlc_coordinator_client.h"
#include "../httprequestactivity.h"
#include "../httpmanager.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QDateTime>

DLCCoordinatorClient::DLCCoordinatorClient(const DLCCoordinatorConfig& config, QObject* parent)
    : QObject(parent), m_config(config)
{
}

// ===== System Checks =====

void DLCCoordinatorClient::check_system_readiness()
{
    auto activity = new HttpRequestActivity(this);
    activity->setMethod("GET");
    activity->addUrl(m_config.base_url + "/auth/system-readiness");
    activity->setTimeout(m_config.timeout_ms);
    if (!m_config.partner_token.isEmpty()) {
        activity->addHeader("X-Partner-Token", m_config.partner_token);
    }
    
    connect(activity, &HttpRequestActivity::finished, this, [=, this] {
        activity->deleteLater();
        const auto response = activity->response();
        if (!activity->hasError()) {
            bool reachable = response.value("reachable").toBool(false);
            QString network = response.value("coordinator_network").toString();
            if (network == m_config.network || m_config.network.isEmpty()) {
                emit systemReadinessReady(reachable, network);
            } else {
                emit systemReadinessFailed(QString("Network mismatch: got %1, expected %2").arg(network, m_config.network));
            }
        } else {
            emit systemReadinessFailed("Failed to reach coordinator");
        }
    });
    connect(activity, &HttpRequestActivity::failed, this, [=, this] {
        activity->deleteLater();
        emit systemReadinessFailed("System readiness check failed");
    });
    
    HttpManager::instance()->exec(activity);
}

// ===== Wallet Registration =====

void DLCCoordinatorClient::register_wallet(const QString& xpub, const QString& xpub_signature,
                                          const QString& nonce, const QString& label)
{
    auto activity = new HttpRequestActivity(this);
    activity->setMethod("POST");
    activity->addUrl(m_config.base_url + "/auth/wallet");
    activity->setTimeout(m_config.timeout_ms);
    if (!m_config.partner_token.isEmpty()) {
        activity->addHeader("X-Partner-Token", m_config.partner_token);
    }
    
    QJsonObject body;
    body["xpub"] = xpub;
    body["xpub_signature"] = xpub_signature;
    body["nonce"] = nonce;
    body["label"] = label;
    body["utxos"] = QJsonArray();  // TODO: populate from wallet UTXO set
    
    activity->setData(body);
    
    connect(activity, &HttpRequestActivity::finished, this, [=, this] {
        activity->deleteLater();
        const auto response = activity->response();
        if (!activity->hasError()) {
            WalletRegistration reg;
            reg.wallet_id = response.value("wallet_id").toString();
            reg.wallet_token = response.value("wallet_token").toString();
            reg.expires_at = response.value("expires_at").toString();
            emit walletRegistered(reg);
        } else {
            emit walletRegistrationFailed(response.value("error").toString("Unknown error"));
        }
    });
    connect(activity, &HttpRequestActivity::failed, this, [=, this] {
        activity->deleteLater();
        emit walletRegistrationFailed("Wallet registration request failed");
    });
    
    HttpManager::instance()->exec(activity);
}

void DLCCoordinatorClient::get_wallet_status(const QString& wallet_id)
{
    auto activity = new HttpRequestActivity(this);
    activity->setMethod("GET");
    activity->addUrl(m_config.base_url + "/auth/wallet/" + wallet_id);
    activity->setTimeout(m_config.timeout_ms);
    if (!m_config.partner_token.isEmpty()) {
        activity->addHeader("X-Partner-Token", m_config.partner_token);
    }
    
    connect(activity, &HttpRequestActivity::finished, this, [=, this] {
        activity->deleteLater();
        const auto response = activity->response();
        if (!activity->hasError()) {
            WalletRegistration status;
            status.wallet_id = response.value("wallet_id").toString();
            status.wallet_token = response.value("wallet_token").toString();
            status.expires_at = response.value("expires_at").toString();
            emit walletStatusReady(status);
        } else {
            emit walletStatusFailed(response.value("error").toString("Unknown error"));
        }
    });
    connect(activity, &HttpRequestActivity::failed, this, [=, this] {
        activity->deleteLater();
        emit walletStatusFailed("Get wallet status request failed");
    });
    
    HttpManager::instance()->exec(activity);
}

// ===== Market Data =====

void DLCCoordinatorClient::get_instruments()
{
    auto activity = new HttpRequestActivity(this);
    activity->setMethod("GET");
    activity->addUrl(m_config.base_url + "/instruments");
    activity->setTimeout(m_config.timeout_ms);
    
    connect(activity, &HttpRequestActivity::finished, this, [=, this] {
        activity->deleteLater();
        const auto response = activity->response();
        if (!activity->hasError()) {
            QVector<OptionInstrument> instruments;
            const auto items = response.value("instruments").toArray();
            for (const auto& item : items) {
                const auto obj = item.toObject();
                OptionInstrument inst;
                inst.instrument_id = obj.value("instrument_id").toString();
                inst.type = obj.value("type").toString();
                inst.oracle_label = obj.value("oracle_label").toString();
                inst.expires_at = obj.value("expires_at").toString();
                inst.created_at = obj.value("created_at").toString();
                instruments.append(inst);
            }
            emit instrumentsReady(instruments);
        } else {
            emit instrumentsFailed(response.value("error").toString("Failed to fetch instruments"));
        }
    });
    connect(activity, &HttpRequestActivity::failed, this, [=, this] {
        activity->deleteLater();
        emit instrumentsFailed("Instruments request failed");
    });
    
    HttpManager::instance()->exec(activity);
}

void DLCCoordinatorClient::get_orderbook(const QString& instrument_id)
{
    auto activity = new HttpRequestActivity(this);
    activity->setMethod("GET");
    activity->addUrl(m_config.base_url + "/orderbook/" + instrument_id);
    activity->setTimeout(m_config.timeout_ms);
    
    connect(activity, &HttpRequestActivity::finished, this, [=, this] {
        activity->deleteLater();
        const auto response = activity->response();
        if (!activity->hasError()) {
            QVector<OrderBookLevel> bids, asks;
            
            const auto bids_array = response.value("bids").toArray();
            for (const auto& item : bids_array) {
                const auto obj = item.toObject();
                OrderBookLevel level;
                level.price = obj.value("price").toVariant().toLongLong();
                level.quantity = obj.value("quantity").toVariant().toLongLong();
                bids.append(level);
            }
            
            const auto asks_array = response.value("asks").toArray();
            for (const auto& item : asks_array) {
                const auto obj = item.toObject();
                OrderBookLevel level;
                level.price = obj.value("price").toVariant().toLongLong();
                level.quantity = obj.value("quantity").toVariant().toLongLong();
                asks.append(level);
            }
            
            emit orderbookReady(instrument_id, bids, asks);
        } else {
            emit orderbookFailed(response.value("error").toString("Failed to fetch orderbook"));
        }
    });
    connect(activity, &HttpRequestActivity::failed, this, [=, this] {
        activity->deleteLater();
        emit orderbookFailed("Orderbook request failed");
    });
    
    HttpManager::instance()->exec(activity);
}

// ===== Order Lifecycle =====

void DLCCoordinatorClient::create_order(const QString& wallet_id, const QString& instrument_id,
                                       const QString& side, long long quantity, long long price)
{
    auto activity = new HttpRequestActivity(this);
    activity->setMethod("POST");
    activity->addUrl(m_config.base_url + "/orders");
    activity->setTimeout(m_config.timeout_ms);
    
    QJsonObject body;
    body["wallet_id"] = wallet_id;
    body["instrument_id"] = instrument_id;
    body["side"] = side;
    body["quantity"] = QJsonValue(static_cast<double>(quantity));
    body["price"] = QJsonValue(static_cast<double>(price));
    
    activity->setData(body);
    
    connect(activity, &HttpRequestActivity::finished, this, [=, this] {
        activity->deleteLater();
        const auto response = activity->response();
        if (!activity->hasError()) {
            Order order;
            order.order_id = response.value("order_id").toString();
            order.dlc_id = response.value("dlc_id").toString();
            order.instrument_id = response.value("instrument_id").toString();
            order.side = response.value("side").toString();
            order.quantity = response.value("quantity").toVariant().toLongLong();
            order.price = response.value("price").toVariant().toLongLong();
            order.status = response.value("status").toString();
            order.pending_match_accept = response.value("pending_match_accept").toBool();
            order.matched_order_id = response.value("matched_order_id").toString();
            order.matched_dlc_id = response.value("matched_dlc_id").toString();
            order.matched_offer_object_hex = response.value("matched_offer_object_hex").toString();
            emit orderCreated(order);
        } else {
            emit orderCreateFailed(response.value("error").toString("Failed to create order"));
        }
    });
    connect(activity, &HttpRequestActivity::failed, this, [=, this] {
        activity->deleteLater();
        emit orderCreateFailed("Create order request failed");
    });
    
    HttpManager::instance()->exec(activity);
}

void DLCCoordinatorClient::get_orders(const QString& wallet_id)
{
    auto activity = new HttpRequestActivity(this);
    activity->setMethod("GET");
    activity->addUrl(m_config.base_url + "/orders?wallet_id=" + wallet_id);
    activity->setTimeout(m_config.timeout_ms);
    
    connect(activity, &HttpRequestActivity::finished, this, [=, this] {
        activity->deleteLater();
        const auto response = activity->response();
        if (!activity->hasError()) {
            QVector<Order> orders;
            const auto items = response.value("orders").toArray();
            for (const auto& item : items) {
                const auto obj = item.toObject();
                Order order;
                order.order_id = obj.value("order_id").toString();
                order.dlc_id = obj.value("dlc_id").toString();
                order.instrument_id = obj.value("instrument_id").toString();
                order.side = obj.value("side").toString();
                order.quantity = obj.value("quantity").toVariant().toLongLong();
                order.price = obj.value("price").toVariant().toLongLong();
                order.status = obj.value("status").toString();
                order.dlc_status = obj.value("dlc_status").toString();
                order.pending_match_accept = obj.value("pending_match_accept").toBool();
                order.matched_order_id = obj.value("matched_order_id").toString();
                order.matched_dlc_id = obj.value("matched_dlc_id").toString();
                order.sign_required = obj.value("sign_required").toBool();
                orders.append(order);
            }
            emit ordersReady(orders);
        } else {
            emit ordersFailed(response.value("error").toString("Failed to fetch orders"));
        }
    });
    connect(activity, &HttpRequestActivity::failed, this, [=, this] {
        activity->deleteLater();
        emit ordersFailed("Get orders request failed");
    });
    
    HttpManager::instance()->exec(activity);
}

void DLCCoordinatorClient::get_order(const QString& order_id)
{
    auto activity = new HttpRequestActivity(this);
    activity->setMethod("GET");
    activity->addUrl(m_config.base_url + "/orders/" + order_id);
    activity->setTimeout(m_config.timeout_ms);
    
    connect(activity, &HttpRequestActivity::finished, this, [=, this] {
        activity->deleteLater();
        const auto response = activity->response();
        if (!activity->hasError()) {
            Order order;
            order.order_id = response.value("order_id").toString();
            order.dlc_id = response.value("dlc_id").toString();
            order.instrument_id = response.value("instrument_id").toString();
            order.side = response.value("side").toString();
            order.quantity = response.value("quantity").toVariant().toLongLong();
            order.price = response.value("price").toVariant().toLongLong();
            order.status = response.value("status").toString();
            order.dlc_status = response.value("dlc_status").toString();
            order.pending_match_accept = response.value("pending_match_accept").toBool();
            order.matched_order_id = response.value("matched_order_id").toString();
            order.matched_dlc_id = response.value("matched_dlc_id").toString();
            order.sign_required = response.value("sign_required").toBool();
            emit orderReady(order);
        } else {
            emit orderFailed(response.value("error").toString("Failed to fetch order"));
        }
    });
    connect(activity, &HttpRequestActivity::failed, this, [=, this] {
        activity->deleteLater();
        emit orderFailed("Get order request failed");
    });
    
    HttpManager::instance()->exec(activity);
}

void DLCCoordinatorClient::cancel_order(const QString& order_id)
{
    auto activity = new HttpRequestActivity(this);
    activity->setMethod("DELETE");
    activity->addUrl(m_config.base_url + "/orders/" + order_id);
    activity->setTimeout(m_config.timeout_ms);
    
    connect(activity, &HttpRequestActivity::finished, this, [=, this] {
        activity->deleteLater();
        // Order cancelled - caller should poll for updated state
    });
    connect(activity, &HttpRequestActivity::failed, this, [=, this] {
        activity->deleteLater();
        emit orderFailed("Cancel order request failed");
    });
    
    HttpManager::instance()->exec(activity);
}

// ===== Accept Flow =====

void DLCCoordinatorClient::get_accept_context(const QString& order_id, const QString& funding_pubkey)
{
    auto activity = new HttpRequestActivity(this);
    activity->setMethod("POST");
    activity->addUrl(m_config.base_url + "/orders/" + order_id + "/accept-context");
    activity->setTimeout(m_config.timeout_ms);
    
    QJsonObject body;
    body["funding_pubkey_hex"] = funding_pubkey;
    activity->setData(body);
    
    connect(activity, &HttpRequestActivity::finished, this, [=, this] {
        activity->deleteLater();
        const auto response = activity->response();
        if (!activity->hasError()) {
            AcceptContext context;
            context.order_id = response.value("order_id").toString();
            context.instrument_id = response.value("instrument_id").toString();
            context.offer_object_hex = response.value("offer_object_hex").toString();
            context.context_fingerprint = response.value("context_fingerprint").toString();
            context.cet_count = response.value("cet_count").toInt();
            context.cet_signing_job_count = response.value("cet_signing_job_count").toInt();
            
            const auto jobs_array = response.value("cet_signing_jobs").toArray();
            for (const auto& job : jobs_array) {
                const auto obj = job.toObject();
                CETPSigningJob signing_job;
                signing_job.message_hash_hex = obj.value("message_hash_hex").toString();
                signing_job.adaptor_point_hex = obj.value("adaptor_point_hex").toString();
                context.cet_signing_jobs.append(signing_job);
            }
            
            context.refund_sighash_hex = response.value("refund_sighash_hex").toString();
            
            const auto funding_sighashes = response.value("funding_input_sighashes_hex").toArray();
            for (const auto& sig : funding_sighashes) {
                context.funding_input_sighashes_hex.append(sig.toString());
            }
            
            const auto outpoints = response.value("funding_input_outpoints").toArray();
            for (const auto& outpoint : outpoints) {
                context.funding_input_outpoints.append(outpoint.toString());
            }
            
            const auto addresses = response.value("funding_input_addresses").toArray();
            for (const auto& addr : addresses) {
                context.funding_input_addresses.append(addr.toString());
            }
            
            emit acceptContextReady(context);
        } else {
            emit acceptContextFailed(response.value("error").toString("Failed to get accept context"));
        }
    });
    connect(activity, &HttpRequestActivity::failed, this, [=, this] {
        activity->deleteLater();
        emit acceptContextFailed("Get accept context request failed");
    });
    
    HttpManager::instance()->exec(activity);
}

void DLCCoordinatorClient::submit_accept(const QString& order_id, const QString& funding_pubkey,
                                        const QString& context_fingerprint,
                                        const QStringList& cet_adaptor_sigs,
                                        const QString& refund_sig,
                                        const QStringList& funding_sigs)
{
    auto activity = new HttpRequestActivity(this);
    activity->setMethod("POST");
    activity->addUrl(m_config.base_url + "/orders/" + order_id + "/accept-match");
    activity->setTimeout(m_config.timeout_ms);
    
    QJsonObject body;
    body["funding_pubkey_hex"] = funding_pubkey;
    body["context_fingerprint"] = context_fingerprint;
    body["idempotency_key"] = "accept:" + order_id + ":" + context_fingerprint;
    
    QJsonArray cet_array;
    for (const auto& sig : cet_adaptor_sigs) {
        cet_array.append(sig);
    }
    body["cet_adaptor_signatures_hex"] = cet_array;
    body["refund_signature_hex"] = refund_sig;
    
    QJsonArray funding_array;
    for (const auto& sig : funding_sigs) {
        funding_array.append(sig);
    }
    body["funding_signatures_hex"] = funding_array;
    
    activity->setData(body);
    
    connect(activity, &HttpRequestActivity::finished, this, [=, this] {
        activity->deleteLater();
        const auto response = activity->response();
        if (!activity->hasError()) {
            Order order;
            order.order_id = response.value("order_id").toString();
            order.status = response.value("status").toString();
            emit acceptSubmitted(order);
        } else {
            emit acceptSubmitFailed(response.value("error").toString("Failed to submit accept"));
        }
    });
    connect(activity, &HttpRequestActivity::failed, this, [=, this] {
        activity->deleteLater();
        emit acceptSubmitFailed("Submit accept request failed");
    });
    
    HttpManager::instance()->exec(activity);
}

// ===== Sign Flow =====

void DLCCoordinatorClient::get_sign_context(const QString& dlc_id)
{
    auto activity = new HttpRequestActivity(this);
    activity->setMethod("GET");
    activity->addUrl(m_config.base_url + "/dlcs/" + dlc_id + "/sign-context");
    activity->setTimeout(m_config.timeout_ms);
    
    connect(activity, &HttpRequestActivity::finished, this, [=, this] {
        activity->deleteLater();
        const auto response = activity->response();
        if (!activity->hasError()) {
            SignContext context;
            context.dlc_id = response.value("dlc_id").toString();
            context.order_id = response.value("order_id").toString();
            context.instrument_id = response.value("instrument_id").toString();
            context.offer_object_hex = response.value("offer_object_hex").toString();
            context.accept_object_hex = response.value("accept_object_hex").toString();
            context.context_fingerprint = response.value("context_fingerprint").toString();
            context.cet_count = response.value("cet_count").toInt();
            context.cet_signing_job_count = response.value("cet_signing_job_count").toInt();
            
            const auto jobs_array = response.value("cet_signing_jobs").toArray();
            for (const auto& job : jobs_array) {
                const auto obj = job.toObject();
                CETPSigningJob signing_job;
                signing_job.message_hash_hex = obj.value("message_hash_hex").toString();
                signing_job.adaptor_point_hex = obj.value("adaptor_point_hex").toString();
                context.cet_signing_jobs.append(signing_job);
            }
            
            context.refund_sighash_hex = response.value("refund_sighash_hex").toString();
            
            const auto funding_sighashes = response.value("funding_input_sighashes_hex").toArray();
            for (const auto& sig : funding_sighashes) {
                context.funding_input_sighashes_hex.append(sig.toString());
            }
            
            const auto outpoints = response.value("funding_input_outpoints").toArray();
            for (const auto& outpoint : outpoints) {
                context.funding_input_outpoints.append(outpoint.toString());
            }
            
            const auto addresses = response.value("funding_input_addresses").toArray();
            for (const auto& addr : addresses) {
                context.funding_input_addresses.append(addr.toString());
            }
            
            emit signContextReady(context);
        } else {
            emit signContextFailed(response.value("error").toString("Failed to get sign context"));
        }
    });
    connect(activity, &HttpRequestActivity::failed, this, [=, this] {
        activity->deleteLater();
        emit signContextFailed("Get sign context request failed");
    });
    
    HttpManager::instance()->exec(activity);
}

void DLCCoordinatorClient::submit_sign(const QString& dlc_id,
                                      const QStringList& cet_adaptor_sigs,
                                      const QString& refund_sig,
                                      const QStringList& funding_sigs)
{
    auto activity = new HttpRequestActivity(this);
    activity->setMethod("POST");
    activity->addUrl(m_config.base_url + "/dlcs/" + dlc_id + "/sign");
    activity->setTimeout(m_config.timeout_ms);
    
    QJsonObject body;
    body["idempotency_key"] = "sign:" + dlc_id;
    
    QJsonArray cet_array;
    for (const auto& sig : cet_adaptor_sigs) {
        cet_array.append(sig);
    }
    body["cet_adaptor_signatures_hex"] = cet_array;
    body["refund_signature_hex"] = refund_sig;
    
    QJsonArray funding_array;
    for (const auto& sig : funding_sigs) {
        funding_array.append(sig);
    }
    body["funding_signatures_hex"] = funding_array;
    
    activity->setData(body);
    
    connect(activity, &HttpRequestActivity::finished, this, [=, this] {
        activity->deleteLater();
        const auto response = activity->response();
        if (!activity->hasError()) {
            SignResponse sign_resp;
            sign_resp.dlc_id = response.value("dlc_id").toString();
            sign_resp.status = response.value("status").toString();
            sign_resp.sign_object_hex = response.value("sign_object_hex").toString();
            sign_resp.funding_tx_hex = response.value("funding_tx_hex").toString();
            sign_resp.funding_txid = response.value("funding_txid").toString();
            sign_resp.funding_broadcasted = response.value("funding_broadcasted").toBool();
            sign_resp.funding_broadcast_error = response.value("funding_broadcast_error").toString();
            emit signSubmitted(sign_resp);
        } else {
            emit signSubmitFailed(response.value("error").toString("Failed to submit sign"));
        }
    });
    connect(activity, &HttpRequestActivity::failed, this, [=, this] {
        activity->deleteLater();
        emit signSubmitFailed("Submit sign request failed");
    });
    
    HttpManager::instance()->exec(activity);
}

// ===== Status Queries =====

void DLCCoordinatorClient::get_dlc(const QString& dlc_id)
{
    auto activity = new HttpRequestActivity(this);
    activity->setMethod("GET");
    activity->addUrl(m_config.base_url + "/dlcs/" + dlc_id);
    activity->setTimeout(m_config.timeout_ms);
    
    connect(activity, &HttpRequestActivity::finished, this, [=, this] {
        activity->deleteLater();
        const auto response = activity->response();
        if (!activity->hasError()) {
            DLCDetail dlc;
            dlc.dlc_id = response.value("dlc_id").toString();
            dlc.order_id = response.value("order_id").toString();
            dlc.instrument_id = response.value("instrument_id").toString();
            dlc.status = response.value("status").toString();
            dlc.funding_txid = response.value("funding_txid").toString();
            dlc.closing_txid = response.value("closing_txid").toString();
            dlc.settlement_type = response.value("settlement_type").toString();
            dlc.confirmed_at = response.value("confirmed_at").toString();
            emit dlcReady(dlc);
        } else {
            emit dlcFailed(response.value("error").toString("Failed to fetch DLC"));
        }
    });
    connect(activity, &HttpRequestActivity::failed, this, [=, this] {
        activity->deleteLater();
        emit dlcFailed("Get DLC request failed");
    });
    
    HttpManager::instance()->exec(activity);
}

void DLCCoordinatorClient::get_dlcs(const QString& wallet_id)
{
    auto activity = new HttpRequestActivity(this);
    activity->setMethod("GET");
    activity->addUrl(m_config.base_url + "/dlcs?wallet_id=" + wallet_id);
    activity->setTimeout(m_config.timeout_ms);
    
    connect(activity, &HttpRequestActivity::finished, this, [=, this] {
        activity->deleteLater();
        const auto response = activity->response();
        if (!activity->hasError()) {
            QVector<DLCDetail> dlcs;
            const auto items = response.value("dlcs").toArray();
            for (const auto& item : items) {
                const auto obj = item.toObject();
                DLCDetail dlc;
                dlc.dlc_id = obj.value("dlc_id").toString();
                dlc.order_id = obj.value("order_id").toString();
                dlc.instrument_id = obj.value("instrument_id").toString();
                dlc.status = obj.value("status").toString();
                dlc.funding_txid = obj.value("funding_txid").toString();
                dlc.settlement_type = obj.value("settlement_type").toString();
                dlcs.append(dlc);
            }
            emit dlcsReady(dlcs);
        } else {
            emit dlcsFailed(response.value("error").toString("Failed to fetch DLCs"));
        }
    });
    connect(activity, &HttpRequestActivity::failed, this, [=, this] {
        activity->deleteLater();
        emit dlcsFailed("Get DLCs request failed");
    });
    
    HttpManager::instance()->exec(activity);
}
