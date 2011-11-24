#ifndef FORUMLIST_H
#define FORUMLIST_H

#include <QAbstractListModel>
#include <QtNetwork>
#include <QtWebKit>

#include "forum.h"

class ForumSession;

class ForumList : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(QString url READ url WRITE setUrl NOTIFY urlChanged)
public:
    enum ForumRoles {
        UrlRole = Qt::UserRole + 1,
        TitleRole,
        ViewingRole,
        SubtitleRole,
        SectionRole,
        NumThreadsRole
    };
    explicit ForumList(ForumSession* forumSession, QObject *parent = 0);

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    int rowCount(const QModelIndex& parent = QModelIndex()) const;

    void setUrl(QString url);

    void addForums(QList<Forum*>& list);
    int count() const;
    QString url() const;
    Q_INVOKABLE QObject* get(int index) const;
    Q_INVOKABLE void load(void);
    ForumSession* forumSession() const;

signals:
    void countChanged();
    void urlChanged();

public slots:
    void onReceived(const QWebElement& document);

private:
    void clear(void);

    QNetworkAccessManager m_networkAccess;
    ForumSession* m_session;

    QString m_url;

    friend class ForumListCache;
};

#endif // FORUMLIST_H
