#ifndef FORUMSESSION_H
#define FORUMSESSION_H

#include <QObject>
#include <QtNetwork>
#include <QtWebKit>

#ifndef QT_SIMULATOR
#include <account.h> // Accounts::Account
#include <AccountSetup/ProviderPluginProxy>
#include <SignOn/AuthService>
#include <SignOn/Identity>
#include <SignOn/SessionData>
#endif

class ForumList;
class ThreadList;

class ForumSession : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QObject* activeTopics READ activeTopics NOTIFY activeTopicsChanged)
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
    Q_PROPERTY(bool canSignOn READ canSignOn)
    Q_PROPERTY(QObject* forums READ forums NOTIFY forumsChanged)
    Q_PROPERTY(QString loginUrl READ loginUrl WRITE setLoginUrl NOTIFY loginUrlChanged)
    Q_PROPERTY(bool missingCredentials READ missingCredentials NOTIFY missingCredentialsChanged)
    Q_PROPERTY(QString password READ password WRITE setPassword)
    Q_PROPERTY(QString provider READ provider WRITE setProvider NOTIFY providerChanged)
    Q_PROPERTY(QString sessionId READ sessionId NOTIFY sessionIdChanged)
    Q_PROPERTY(QString url READ url WRITE setUrl NOTIFY urlChanged)
    Q_PROPERTY(QString notif READ notif WRITE setNotif)
    Q_PROPERTY(QString userName READ userName WRITE setUserName NOTIFY userNameChanged)
public:
    enum LoginStatus {
        Succeeded,
        Failed
    };
    explicit ForumSession(QObject *parent = 0);

    QObject* activeTopics() const;
    bool busy(void) const;
    bool canSignOn(void) const;
    QObject* forums(void) const;
    QString loginUrl(void) const;
    bool missingCredentials(void) const;
    QString password(void) const;
    QString provider(void) const;
    QString sessionId(void) const;
    QString url(void) const;
    QString notif(void) const;
    QString userName(void) const;

    void setLoginUrl(const QString loginUrl);
    void setPassword(const QString password);
    void setProvider(const QString provider);
    void setUrl(const QString url);
    void setUserName(const QString userName);

    Q_INVOKABLE QObject* createNewPost(void);
    Q_INVOKABLE QString externalUrl(const QString url) const;
    QString forumUrl(int forumId) const;
    void get(const QUrl& url);
    Q_INVOKABLE void login(void);
    void post(const QUrl& url, const QByteArray& data);
    Q_INVOKABLE void refresh(void);
    Q_INVOKABLE QObject* search(const QString& action);
    Q_INVOKABLE void signOn(void);
    Q_INVOKABLE void subscribe(QObject* thread);
    Q_INVOKABLE QObject* subscribedThreads(void);
    Q_INVOKABLE void thank(QObject* post);
    Q_INVOKABLE void unsubscribe(QObject* thread);
    Q_INVOKABLE void unThank(QObject* post);
    Q_INVOKABLE void setNotif(const QString notif);
    Q_INVOKABLE void markRead(QObject* forum);

private:
    void reset(void);
    void setMissingCredentials(bool missingCredentials);

signals:
    void activeTopicsChanged(void);
    void busyChanged(void);
    void forumsChanged(void);
    void loginUrlChanged(void);
    void missingCredentialsChanged(void);
    void sessionIdChanged(void);
    void providerChanged(void);
    void urlChanged(void);
    void userNameChanged(void);

    void error(const QString& message);
    void loginFailed(const QString& message);
    void loginSucceeded(void);
    void receivedForumList(QWebElement document);
    void receivedNewPost(QWebElement document);
    void receivedPostList(QWebElement document, int postId);
    void receivedSearchResultThreadList(QWebElement document, int searchId);
    void receivedThanks(const QString& postId, const QStringList& thanks);
    void receivedThreadList(QWebElement document);
    void subscriptionChanged(int threadId, bool subscribed);

public slots:
    void maybeLogin(void);
#ifndef QT_SIMULATOR
    void onAccountSetupFinished(void);
#endif
    void onReceived(QNetworkReply *reply);
    void webElementFunc(bool status);
#ifndef QT_SIMULATOR
    void onSignOnError(const SignOn::Error& error);
    void onSignOnIdentityInfo(const SignOn::IdentityInfo& info);
    void onSignOnResponse(const SignOn::SessionData& data);
#endif
    void querySignOnCredentials(void);

private:
#ifndef QT_SIMULATOR
    Accounts::Account* m_account;
    AccountSetup::ProviderPluginProxy* m_accountSetup;
    SignOn::AuthService m_authService;
    SignOn::Identity* m_identity;
#endif
    QNetworkAccessManager m_networkAccess;
    QString m_securityToken;
    QWebPage m_webPage;

    ThreadList* m_activeTopics;
    bool m_busy;
    ForumList* m_forums;
    bool m_missingCredentials;
    QString m_loginUrl;
    QString m_password;
    QString m_provider;
    QString m_sessionId;
    QString m_url;
    QString m_notif;  //email notification setting
    QString m_userName;

    QUrl rep_url;
    QString path;
    QString c_sessionId;
};

#endif // FORUMSESSION_H
