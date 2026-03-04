#ifndef BLOCKSTREAM_SWAPQUOTECONTROLLER_H
#define BLOCKSTREAM_SWAPQUOTECONTROLLER_H

#include "controller.h"

#include <QVariantMap>

class SwapQuoteControllerPrivate;

class SwapQuoteController : public Controller
{
    Q_OBJECT
    Q_PROPERTY(QVariantMap quote READ quote NOTIFY updated)
    Q_PROPERTY(bool lightning READ isLightning WRITE setLightning NOTIFY updated)
    Q_PROPERTY(QString receiveNetworkKey READ receiveNetworkKey NOTIFY updated)
    Q_PROPERTY(QString sendNetworkKey READ sendNetworkKey NOTIFY updated)
    QML_ELEMENT
public:
    SwapQuoteController(QObject* parent = nullptr);
    ~SwapQuoteController();
    bool isLightning() const;
    void setLightning(bool lightning);
    QVariantMap quote() const;
    QString receiveNetworkKey() const;
    QString sendNetworkKey() const;
public slots:
    void receive(const QString& amount);
    void send(const QString& amount);
    void swapNetworks();
signals:
    void updated();
private:
    SwapQuoteControllerPrivate* const d;
    void update();
};

#endif // BLOCKSTREAM_SWAPQUOTECONTROLLER_H
