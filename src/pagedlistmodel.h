#ifndef PAGEDLISTMODEL_H
#define PAGEDLISTMODEL_H

#include <QAbstractListModel>

class PagedListModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int firstPage READ firstPage NOTIFY firstPageChanged)
    Q_PROPERTY(int lastPage READ lastPage NOTIFY lastPageChanged)
    Q_PROPERTY(int numPages READ numPages NOTIFY numPagesChanged)
public:
    explicit PagedListModel(QObject *parent = 0);

    int firstPage() const;
    int lastPage() const;
    int numPages() const;

signals:
    void firstPageChanged();
    void lastPageChanged();
    void numPagesChanged();

public slots:

protected:
    int m_firstPage;
    int m_lastPage;
    int m_numPages;
};

#endif // PAGEDLISTMODEL_H
