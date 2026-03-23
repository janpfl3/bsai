#ifndef BLOCKSTREAM_SUBMARINECONTROLLER_H
#define BLOCKSTREAM_SUBMARINECONTROLLER_H

#include "controller.h"
#include "swap.h"

#include <QVariant>

class SubmarineControllerPrivate;

class SubmarineController : public Controller
{
    Q_OBJECT
    Q_PROPERTY(QString payment READ payment WRITE setPayment NOTIFY paymentChanged)
    Q_PROPERTY(QString refundAddress READ refundAddress WRITE setRefundAddress NOTIFY refundAddressChanged)
    Q_PROPERTY(bool busy READ isBusy NOTIFY busyChanged)
    Q_PROPERTY(QVariant error READ error NOTIFY errorChanged)
    Q_PROPERTY(SubmarineSwap* swap READ swap NOTIFY swapChanged)
    QML_ELEMENT
public:
    SubmarineController(QObject* parent = nullptr);
    ~SubmarineController();
    QString payment() const;
    void setPayment(const QString& payment);
    QString refundAddress() const;
    void setRefundAddress(const QString& refund_address);
    bool isBusy() const;
    QVariant error() const;
    SubmarineSwap* swap() const;
public slots:
    void setLockupTransaction(ChainTransaction* transaction);
signals:
    void paymentChanged();
    void refundAddressChanged();
    void busyChanged();
    void swapChanged();
    void errorChanged();
protected:
    void timerEvent(QTimerEvent* event);
private:
    SubmarineControllerPrivate* const d;
    void invalidate();
    void update();
    void setError(const QVariant& error);
    void setSwap(SubmarineSwap* swap);
};

#endif // BLOCKSTREAM_SUBMARINECONTROLLER_H
