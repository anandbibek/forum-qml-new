#include "pagedlistmodel.h"

PagedListModel::PagedListModel(QObject *parent) :
    QAbstractListModel(parent),
    m_firstPage(0),
    m_lastPage(0),
    m_numPages(0)
{
}

int PagedListModel::firstPage() const
{
    return m_firstPage;
}

int PagedListModel::lastPage() const
{
    return m_lastPage;
}

int PagedListModel::numPages() const
{
    return m_numPages;
}
