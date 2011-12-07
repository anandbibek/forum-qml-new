#ifndef FORUMTHREADLIST_H
#define FORUMTHREADLIST_H

#include <QAbstractListModel>
#include <QtWebKit>

#include "pagedlistmodel.h"

class ForumSession;
class Thread;

class ThreadList : public PagedListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(QString url READ url WRITE setUrl)
public:
    enum ForumRoles {
        UrlRole = Qt::UserRole + 1,
        TitleRole,
        RatingValueRole,
        RepliesRole,
        ForumRole,
        PosterRole,
        SectionRole,
        DateTimeRole,
        UnreadRole,
        VotesRole,
        AttachmentsRole
    };
    explicit ThreadList(ForumSession* forumSession, QObject *parent = 0);

    int count() const;
    QString url() const;

    void setUrl(QString uri);

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    int rowCount(const QModelIndex& parent = QModelIndex()) const;
    void parseActiveTopics(const QWebElementCollection& topics);
    ForumSession* forumSession() const;

    Q_INVOKABLE void clear(void);
    Q_INVOKABLE QObject* get(int index) const;
    Q_INVOKABLE void load(int page);

signals:
    void countChanged();

public slots:
    void onReceived(QWebElement document);
    void onThreadChanged();

private:
    QObject* lastActiveThread;
    ForumSession *m_session;

protected:
    bool m_stickySection;

    QString m_url;
};

#endif // FORUMTHREADLIST_H
