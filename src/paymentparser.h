#ifndef BLOCKSTREAM_PAYMENT_PARSER_H
#define BLOCKSTREAM_PAYMENT_PARSER_H

#include "green.h"

#include <QObject>
#include <QQmlEngine>

class PaymentParserPrivate;
class PaymentParser : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString input READ input WRITE setInput NOTIFY inputChanged)
    Q_PROPERTY(QVariantMap payment READ payment NOTIFY updated)
    QML_ELEMENT
public:
    PaymentParser(QObject* parent = nullptr);
    ~PaymentParser();
    QString input() const;
    void setInput(const QString& input);
    QVariantMap payment() const;
signals:
    void inputChanged();
    void updated();
private:
    PaymentParserPrivate* const d;
};

#endif // BLOCKSTREAM_PAYMENT_PARSER_H
