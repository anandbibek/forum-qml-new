#ifndef FORUMPROVIDERLIST_H
#define FORUMPROVIDERLIST_H

#include <QAbstractListModel>

class ForumProviderList : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    enum ForumProviderRoles {
        NameRole = Qt::UserRole + 1,
        IconNameRole,
        ProviderRole
    };
    explicit ForumProviderList(QObject *parent = 0);

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    int rowCount(const QModelIndex& parent = QModelIndex()) const;

    int count() const;
    Q_INVOKABLE QVariant get(int index) const;

signals:
    void countChanged();
    
public slots:
    
};

#endif // FORUMPROVIDERLIST_H
