#ifndef THREAD_H
#define THREAD_H

#include <QObject>

class PostList;

class Thread : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int attachments READ attachments WRITE setAttachments NOTIFY attachmentsChanged)
    Q_PROPERTY(QString dateTime READ dateTime)
    Q_PROPERTY(QString forum READ forum NOTIFY forumChanged)
    Q_PROPERTY(QString lastPostUrl READ lastPostUrl WRITE setLastPostUrl)
    Q_PROPERTY(QObject* model READ model NOTIFY modelChanged)
    Q_PROPERTY(OpenMode openMode READ openMode WRITE setOpenMode)
    Q_PROPERTY(QString poster READ poster)
    Q_PROPERTY(float ratingValue READ ratingValue WRITE setRatingValue NOTIFY ratingValueChanged)
    Q_PROPERTY(int replies READ replies)
    Q_PROPERTY(QString section READ section)
    Q_PROPERTY(bool subscribed READ subscribed WRITE setSubscribed NOTIFY subscribedChanged)
    Q_PROPERTY(QString title READ title NOTIFY titleChanged)
    Q_PROPERTY(int threadId READ threadId WRITE setThreadId)
    Q_PROPERTY(bool unread READ unread WRITE setUnread NOTIFY unreadChanged)
    Q_PROPERTY(int votes READ votes WRITE setVotes NOTIFY votesChanged)
    Q_PROPERTY(QString url READ url WRITE setUrl)
    Q_ENUMS(OpenMode)
public:
    enum OpenMode {
        FirstPost,
        FirstUnreadPost,
        LastPost
    };
    explicit Thread(const QString url, const QString title, int replies, const QString poster, const QString section, QString dateTime, QObject *parent = 0);
    explicit Thread(const Thread& other, QObject *parent = 0);

    int attachments() const;
    QString dateTime() const;
    QString forum() const;
    QString lastPostUrl() const;
    QObject* model();
    OpenMode openMode() const;
    QString poster() const;
    float ratingValue() const;
    int replies() const;
    QString section() const;
    bool subscribed() const;
    QString title() const;
    int threadId() const;
    bool unread() const;
    QString url() const;
    int votes() const;

    void setAttachments(int attachments);
    void setForum(const QString forum);
    void setLastPostUrl(const QString lastPostUrl);
    void setOpenMode(OpenMode openMode);
    void setRatingValue(float ratingValue);
    void setSubscribed(bool subscribed);
    void setThreadId(int threadId);
    void setUnread(bool unread);
    void setUrl(const QString url);
    void setVotes(int votes);

    Q_INVOKABLE QObject* detachWithModel(void);

signals:
    void attachmentsChanged(void);
    void forumChanged(void);
    void modelChanged(void);
    void ratingValueChanged(void);
    void subscribedChanged(void);
    void titleChanged(void);
    void unreadChanged(void);
    void votesChanged(void);

public slots:

private:
    int m_attachments;
    QString m_dateTime;
    QString m_forum;
    QString m_lastPostUrl;
    PostList* m_model;
    OpenMode m_openMode;
    QString m_poster;
    float m_ratingValue;
    int m_replies;
    QString m_section;
    bool m_subscribed;
    int m_threadId;
    QString m_title;
    bool m_unread;
    QString m_url;
    int m_votes;
};

#endif // THREAD_H
