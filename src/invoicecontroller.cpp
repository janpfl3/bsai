#include "invoicecontroller.h"

#include "context.h"
#include "lwk/lwk.hpp"
#include "swap.h"

#include <QDebug>
#include <QtConcurrentRun>

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
