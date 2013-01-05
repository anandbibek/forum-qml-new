#ifndef FORUMTHREAD_H
#define FORUMTHREAD_H

#include <QAbstractListModel>
#include <QtWebKit>

class ForumSession;
class Post;

class PostList : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(int firstPage READ firstPage NOTIFY firstPageChanged)
    Q_PROPERTY(int jumpToIndex READ jumpToIndex)
    Q_PROPERTY(int lastPage READ lastPage NOTIFY lastPageChanged)
    Q_PROPERTY(int numPages READ numPages NOTIFY numPagesChanged)
    Q_PROPERTY(int threadId READ threadId WRITE setThreadId)
    Q_PROPERTY(QString url READ url WRITE setUrl)
public:
    enum ForumRoles {
        UrlRole = Qt::UserRole + 1,
        PosterRole,
        DateTimeRole,
        BodyRole,
        ImgRole,
        StatRole,
        SectionRole,
        ThanksRole
    };
    explicit PostList(ForumSession* session, QObject *parent = 0);
    ~PostList(void);

    int count() const;
    int firstPage() const;
    int jumpToIndex() const;
    int lastPage() const;
    int numPages() const;
    int threadId() const;
    QString url() const;

    void setThreadId(int url);
    void setUrl(const QString url);

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    int rowCount(const QModelIndex& parent = QModelIndex()) const;

    Q_INVOKABLE QObject* get(int index) const;
    Q_INVOKABLE void load(int page);

signals:
    void countChanged();
    void firstPageChanged();
    void lastPageChanged();
    void numPagesChanged();

public slots:
    void onPostChanged();
    void onReceived(QWebElement document, int postId);

private:
    void clear(void);

    QList<Post*> m_posts;
    ForumSession* m_session;

    int m_firstPage;
    int m_jumpToIndex;
    int m_lastPage;
    int m_numPages;
    int m_threadId;
    QString m_url;
};

#endif // FORUMTHREAD_H
