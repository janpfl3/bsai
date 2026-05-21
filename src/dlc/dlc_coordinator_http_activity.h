/**
 * @file src/dlc/dlc_coordinator_http_activity.h
 * @brief SessionActivity for DLC Coordinator requests
 */

#pragma once

#include "session.h"
#include <QString>
#include <QJsonObject>

class DLCCoordinatorActivity : public SessionActivity {
    Q_OBJECT

public:
    enum Type {
        GetNonce,
        RegisterWallet,
        GetInstruments,
        CreateOrder,
        GetOrders,
        GetAcceptContext,
        SubmitAcceptMatch,
        GetSignContext,
        SubmitSign,
        GetDLCDetail,
    };

    explicit DLCCoordinatorActivity(Type type, QObject* parent = nullptr)
        : SessionActivity(parent), m_type(type)
    {
    }

    Type type() const { return m_type; }

    // Request setup methods
    void set_endpoint(const QString& endpoint) { m_endpoint = endpoint; }
    void set_method(const QString& method) { m_method = method; }
    void set_body(const QJsonObject& body) { m_body = body; }
    void set_wallet_token(const QString& token) { m_wallet_token = token; }
    void set_idempotency_key(const QString& key) { m_idempotency_key = key; }

    // Response access
    QJsonObject response() const { return m_response; }

protected:
    void exec() override {
        // TODO: Implement HTTP request handling
        qDebug() << "DLCCoordinatorActivity::exec() stub";
        finish();
    }

private:
    Type m_type;
    QString m_endpoint;
    QString m_method;
    QJsonObject m_body;
    QString m_wallet_token;
    QString m_idempotency_key;
    QJsonObject m_response;
};
