#ifndef NEWPOST_H
#define NEWPOST_H

#include <QtCore>
#include <QtWebKit>

#include "post.h"

class ForumSession;

class NewPost : public Post
{
    Q_OBJECT
    Q_PROPERTY(int forumId READ forumId WRITE setForumId NOTIFY forumIdChanged)
    Q_PROPERTY(bool editPost READ editPost WRITE setEditPost )
    Q_PROPERTY(QString securityToken READ securityToken WRITE setSecurityToken)
    Q_PROPERTY(QString preview READ preview NOTIFY previewChanged)
    Q_PROPERTY(int threadId READ threadId WRITE setThreadId NOTIFY threadIdChanged)
public:
    explicit NewPost(ForumSession* session, QObject* parent = 0);

    int forumId() const;
    QString securityToken() const;
    QString preview() const;
    int threadId() const;
    bool editPost() const;

    void setForumId(int forumId);
    void setSecurityToken(const QString securityToken);
    void setThreadId(int threadId);

    Q_INVOKABLE void requestPreview();
    Q_INVOKABLE void submit();
    Q_INVOKABLE void edit();
    Q_INVOKABLE void setEditPost(bool editPost);

signals:
    void forumIdChanged(void);
    void previewChanged(void);
    void threadIdChanged(void);

    void errorMessage(QString message);

private slots:
    void onPostIdChanged(void);
    void onReceived(QWebElement document);

private:
    ForumSession* m_session;
    int m_loggedInUser;
    QString m_postHash;
    unsigned int m_postStartTime;

    QString m_emailUpdate;
    int m_forumId;
    bool m_editPost;
    QString m_preview;
    QString m_securityToken;
    int m_threadId;
};

#endif // NEWPOST_H
