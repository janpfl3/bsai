#ifndef BLOCKSTREAM_DLCPAGEMODEL_H
#define BLOCKSTREAM_DLCPAGEMODEL_H

#include <QQmlEngine>
#include <QSortFilterProxyModel>
#include <QString>
#include <QJsonObject>

/**
 * @file src/dlc/dlcpagemodel.h
 * @brief Model for DLC page list views (orders, positions, activity)
 */
class DLCPageModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(QString pageType READ pageType WRITE setPageType NOTIFY pageTypeChanged)
    Q_PROPERTY(QString filterStatus READ filterStatus WRITE setFilterStatus NOTIFY filterStatusChanged)
    Q_PROPERTY(QString filterInstrument READ filterInstrument WRITE setFilterInstrument NOTIFY filterInstrumentChanged)
    Q_PROPERTY(QString sortBy READ sortBy WRITE setSortBy NOTIFY sortByChanged)
    Q_PROPERTY(int itemCount READ itemCount NOTIFY itemCountChanged)
    QML_ELEMENT

public:
    enum PageType {
        OrderBook,      // Available instruments and orderbook
        OpenOrders,     // User's open/pending orders
        Positions,      // Active DLC positions
        Activity,       // Order/DLC history
        Instruments     // Filtered instrument list
    };
    Q_ENUM(PageType)

    explicit DLCPageModel(QObject* parent = nullptr);

    // Properties
    QString pageType() const { return m_page_type; }
    void setPageType(const QString& type);

    QString filterStatus() const { return m_filter_status; }
    void setFilterStatus(const QString& status);

    QString filterInstrument() const { return m_filter_instrument; }
    void setFilterInstrument(const QString& instrument);

    QString sortBy() const { return m_sort_by; }
    void setSortBy(const QString& sort_by);

    int itemCount() const;

    // Data access
    QJsonObject getItemData(int row) const;
    QString getItemId(int row) const;
    QString getItemStatus(int row) const;

signals:
    void pageTypeChanged();
    void filterStatusChanged();
    void filterInstrumentChanged();
    void sortByChanged();
    void itemCountChanged();

public slots:
    void clearFilters();
    void refresh();
    void setSourceModel(QAbstractItemModel* model);

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;
    bool lessThan(const QModelIndex& source_left, const QModelIndex& source_right) const override;

private:
    QString m_page_type;
    QString m_filter_status;
    QString m_filter_instrument;
    QString m_sort_by = "created_at";

    bool filterByStatus(const QJsonObject& item) const;
    bool filterByInstrument(const QJsonObject& item) const;
};

#endif // BLOCKSTREAM_DLCPAGEMODEL_H

