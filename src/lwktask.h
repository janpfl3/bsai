#ifndef GREEN_LWK
#define GREEN_LWK

#include "controller.h"
#include "swap.h"
#include "task.h"

class ReverseSwap;

class LwkCreateSessionTask : public ContextTask
{
public:
    LwkCreateSessionTask(Context* context);
    void update() override;
};

class InvoiceControllerPrivate;
class InvoiceController : public Controller
{
    Q_OBJECT
    Q_PROPERTY(QString address READ address WRITE setAddress NOTIFY addressChanged)
    Q_PROPERTY(QString satoshi READ satoshi WRITE setSatoshi NOTIFY satoshiChanged)
    Q_PROPERTY(QString description READ description WRITE setDescription NOTIFY descriptionChanged)
    Q_PROPERTY(bool busy READ isBusy NOTIFY busyChanged)
    Q_PROPERTY(ReverseSwap* swap READ swap NOTIFY swapChanged)
    QML_ELEMENT
public:
    InvoiceController(QObject* parent = nullptr);
    ~InvoiceController();
    QString address() const;
    void setAddress(const QString& address);
    QString satoshi() const;
    void setSatoshi(const QString& satoshi);
    QString description() const;
    void setDescription(const QString& description);
    bool isBusy() const;
    ReverseSwap* swap() const;
public slots:
    void request();
signals:
    void addressChanged();
    void satoshiChanged();
    void descriptionChanged();
    void busyChanged();
    void swapChanged();
protected:
    void timerEvent(QTimerEvent* event);
private:
    InvoiceControllerPrivate* const d;
    void invalidate();
    void update();
    bool isValid() const;
};

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

class ChainSwapControllerPrivate;
class ChainSwapController : public Controller
{
    Q_OBJECT
    Q_PROPERTY(QString amount READ amount WRITE setAmount NOTIFY amountChanged)
    Q_PROPERTY(Address* refundAddress READ refundAddress WRITE setRefundAddress NOTIFY refundAddressChanged)
    Q_PROPERTY(Address* claimAddress READ claimAddress WRITE setClaimAddress NOTIFY claimAddressChanged)
    Q_PROPERTY(bool busy READ isBusy NOTIFY busyChanged)
    Q_PROPERTY(ChainSwap* swap READ swap NOTIFY swapChanged)
    QML_ELEMENT
public:
    ChainSwapController(QObject* parent = nullptr);
    ~ChainSwapController();
    QString amount() const;
    void setAmount(const QString& amount);
    Address* refundAddress() const;
    void setRefundAddress(Address* refund_address);
    Address* claimAddress() const;
    void setClaimAddress(Address* claim_address);
    bool isValid() const;
    bool isBusy() const;
    ChainSwap* swap() const;
public slots:
    void commit();
signals:
    void amountChanged();
    void refundAddressChanged();
    void claimAddressChanged();
    void busyChanged();
    void swapChanged();
protected:
    void timerEvent(QTimerEvent* event);
private:
    ChainSwapControllerPrivate* const d;
    void invalidate();
    void update();
};

#endif // GREEN_LWK
