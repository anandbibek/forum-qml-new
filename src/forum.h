#ifndef FORUM_H
#define FORUM_H

#include <QObject>

class ForumSession;
class ThreadList;

class Forum : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int forumId READ forumId NOTIFY forumIdChanged)
    Q_PROPERTY(QObject* model READ model NOTIFY modelChanged)
    Q_PROPERTY(int numPosts READ numPosts WRITE setNumPosts)
    Q_PROPERTY(int numThreads READ numThreads WRITE setNumThreads)
    Q_PROPERTY(QString section READ section WRITE setSection)
    Q_PROPERTY(QString subtitle READ subtitle)
    Q_PROPERTY(QString title READ title NOTIFY titleChanged)
    Q_PROPERTY(bool unread READ unread WRITE setUnread NOTIFY unreadChanged)
    Q_PROPERTY(QString url READ url)
    Q_PROPERTY(int viewing READ viewing WRITE setViewing)
public:
    explicit Forum(const QString url, int forumId, const QString title, const QString subtitle, QObject *parent = 0);

    int forumId() const;
    QObject* model();
    int numPosts() const;
    int numThreads() const;
    QString section() const;
    QString subtitle() const;
    QString title() const;
    bool unread() const;
    QString url() const;
    int viewing() const;

    void setNumPosts(int numPosts);
    void setNumThreads(int numThreads);
    void setSection(const QString section);
    void setUnread(bool unread);
    void setViewing(int viewing);

    Q_INVOKABLE void markRead(void);

signals:
    void forumIdChanged(void);
    void modelChanged(void);
    void titleChanged(void);
    void unreadChanged(void);

private:
    int m_forumId;
    ThreadList* m_model;
    int m_numPosts;
    int m_numThreads;
    QString m_section;
    QString m_subtitle;
    QString m_title;
    bool m_unread;
    QString m_url;
    int m_viewing;
};

#endif // FORUM_H
