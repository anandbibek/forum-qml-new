#include <QDebug>

#ifndef QT_SIMULATOR
#include <manager.h> // Accounts::Manager
#endif

#include "forumlist.h"
#include "forumsession.h"
#include "newpost.h"
#include "searchresultthreadlist.h"
#include "thread.h"
#include "threadlist.h"

ForumSession::ForumSession(QObject *parent) :
    QObject(parent),
#ifndef QT_SIMULATOR
    m_account(0),
    m_accountSetup(0),
    m_identity(0),
#endif
    m_activeTopics(0),
    // This helps to prevent a guest login until credentials are proven to be missing
    m_missingCredentials(false)
{
    m_forums = new ForumList(this, this);

    // Disable JavaScript and loading of external objects
    m_webPage.settings()->setAttribute(QWebSettings::AutoLoadImages, false);
    m_webPage.settings()->setAttribute(QWebSettings::JavascriptEnabled, false);
    m_webPage.settings()->setAttribute(QWebSettings::LocalContentCanAccessRemoteUrls, false);
    m_webPage.settings()->setAttribute(QWebSettings::LocalContentCanAccessFileUrls, false);

    connect(&m_networkAccess, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(onReceived(QNetworkReply*)));

    // Find an active account and query credentials from signond once the provider is set
    connect(this, SIGNAL(providerChanged()),
            this, SLOT(querySignOnCredentials()));

    // If credentials cannot be found, login as guest
    connect(this, SIGNAL(missingCredentialsChanged()), this, SLOT(maybeLogin()));

    // Try logging in after the login URL is set
    connect(this, SIGNAL(loginUrlChanged()), this, SLOT(maybeLogin()));
}

void ForumSession::maybeLogin()
{
    if (m_busy || m_url.isEmpty() || m_loginUrl.isEmpty() || !m_sessionId.isEmpty())
        return;

    if (m_missingCredentials) {
        // No credentials available, forum list empty, login as guest
        if (m_forums->count() == 0)
            m_forums->load();
    } else {
        // Credentials available or still being looked for
        if (!m_userName.isEmpty() && !m_password.isEmpty()) {
            // If credentials are ready, login
            login();
        }
    }
}

#ifndef QT_SIMULATOR
void ForumSession::onAccountSetupFinished(void)
{
    delete m_accountSetup;
    m_accountSetup = 0;

    querySignOnCredentials();
}
#endif

void ForumSession::onReceived(QNetworkReply *reply)
{
    // Check if there are any HTTP errors
    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "HTTP ERROR" << reply->error() << ":" << reply->errorString();

        m_busy = false;
        emit busyChanged();

        return;
    }

    // If this is a redirect, maybe follow it
    QVariant redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
    if (!redirect.isNull()) {
        QUrl url = redirect.toUrl();
        if (url == QUrl("https://meego.com/forum")) {
            // Hard-coded to skip one layer of indirection
            url = QUrl("http://forum.meego.com");
            qDebug() << "Redirect - requesting " << url.toString() << " to login ...";
            QNetworkRequest request(url);
            m_networkAccess.get(request);
        } else if (url.toString().startsWith(m_url)) {
            qDebug() << "Redirect - requesting " << url.toString() << " ...";
            QNetworkRequest request(url);
            m_networkAccess.get(request);
        } else {
            qDebug() << "Unknown redirect: " << url;

            m_busy = false;
            emit busyChanged();
        }
        return;
    }

    QString userName = m_userName;
    QString sessionId;

    // Check out cookies
    foreach (QNetworkCookie cookie, m_networkAccess.cookieJar()->cookiesForUrl(QUrl(m_url))) {
        if (cookie.name() == "bbsessionhash")
            sessionId = cookie.value();
    }

    // Now handle the contents
    const QString html = QString::fromLatin1(reply->readAll().constData());

    if (html.isEmpty())
    {
        qDebug() << "ERROR:" << reply->errorString();
        qDebug() << "Received empty page!";
        m_busy = false;
        emit busyChanged();
        return;
    }
    m_webPage.mainFrame()->setHtml(html);
    const QWebElement document = m_webPage.mainFrame()->documentElement();

    if (reply->url() == (m_loginUrl.startsWith("http") ? QUrl(m_loginUrl)
                                                       : QUrl(m_url + "/" + m_loginUrl))) {
        // This is a reply to the login POST request

        // For talk.maemo.org: The first <form name="postvarform"> element
        const QWebElement form = document.findFirst("form[name=\"postvarform\"]");
        if (form.attribute("name") == "postvarform") {
            QRegExp urlExpression(m_url + "/index.php\\?s=([a-f\\d]+)");
            if (urlExpression.exactMatch(form.attribute("action"))) {
                sessionId = urlExpression.cap(1);
            } else {
                qDebug() << "Form action URL does not match:" << form.attribute("action");
            }

            QWebElement strong = form.findFirst("strong");
            QRegExp messageExpression("Thank you for logging in, (.+)\\.");
            if (messageExpression.exactMatch(strong.toPlainText())) {
                userName = messageExpression.cap(1);
            } else {
                qDebug() << "Login message does not match:" << strong.toPlainText();
            }
        } else {
            QWebElement div = document.findFirst("table.tborder div.panel > div > div");
            if (!div.isNull()) {
                QString message = div.toPlainText();
                qDebug() << "Error message:" << message;

                emit loginFailed(message);
            } else {
                qDebug() << "================================================================================";
                qDebug() << html;
                qDebug() << "================================================================================";
                qDebug() << "Unknown error. Dumped page";

                emit loginFailed("Unknown error");
            }
        }

        const QWebElement meta = document.findFirst("meta[http-equiv=\"Refresh\"]");
        if (!meta.isNull()) {
            QRegExp refreshExpression("\\d+; URL=(.+)");
            if (refreshExpression.exactMatch(meta.attribute("content"))
                    && refreshExpression.cap(1).startsWith(m_url)) {
                QUrl url(refreshExpression.cap(1));
                qDebug() << "Redirect - requesting " << url.toString() << "to login ...";
                QNetworkRequest request(url);
                m_networkAccess.get(request);

                return;
            } else {
                qDebug() << "Unknown meta refresh element:" << meta.toOuterXml();
            }
        }
    }

    // Parse elements found on every page

    // For forum.meego.com
    const QWebElement a = document.findFirst("div#account-menu div div div ul.drupal_menu li a");
    if (!a.isNull()) {
     // QString userPageUrl = a.attribute("href");

        QRegExp userNameExpression("My account \\((.+)\\)");
        if (userNameExpression.exactMatch(a.toPlainText())) {
            userName = userNameExpression.cap(1);
            if (userName != m_userName)
                qDebug() << "User name differs:" << userName << "!=" << m_userName;
        } else if (a.toPlainText() == "Login") {
            if (!m_userName.isEmpty())
                qDebug() << "Found login link --> logged out";
            userName = "";
        } else {
            qDebug() << "User account information does not match:" << a.toPlainText();
        }
    }

    // For talk.maemo.org
    const QWebElement a2 = document.findFirst("div#muser-login div.welcomemember a");
    if (!a2.isNull()) {
     // QString userPageUrl = a2.attribute("href");

        userName = a2.toPlainText();
        if (userName != m_userName)
            qDebug() << "User name differs:" << userName << "!=" << m_userName;
    }
    QWebElement form = document.findFirst("form[action=\"login.php?do=login\"]");
    if (!form.isNull()) {
        if (!m_userName.isEmpty())
            qDebug() << "Found login form --> logged out";
        userName = "";
        m_securityToken = "guest";
    }

    // talk.maemo.org - Search form for security token
    form = document.findFirst("form[action=\"search.php?do=process\"]");
    if (!form.isNull()) {
        m_securityToken = form.findFirst("input[name=\"securitytoken\"]").attribute("value");
    }

    // talk.maemo.org - parse the active topic list
    const QWebElementCollection topics = document.findAll("ul.mrecent > li > h3 > a");
    if (topics.count() > 0) {
        if (!m_activeTopics) {
            m_activeTopics = new ThreadList(this, this);
            emit activeTopicsChanged();
        }
        m_activeTopics->parseActiveTopics(topics);
    }

    // Depending on URL, jump to parsing the contained forum list, thread list, or post list
    QString url = reply->url().toString();

    qDebug() << "REPLY URL:" << url;

    if (url == m_url || url.startsWith(m_url + "/index.php")) {
        // Parse the forum list
        emit receivedForumList(document);
    } else if (url.startsWith(m_url + "/forumdisplay.php")) {
        // Parse the thread list
        emit receivedThreadList(document);
    } else if (url.startsWith(m_url + "/showthread.php")) {
        int postId = -1;
        QRegExp postIdExpression("p=(\\d+)#post(\\d+)");
        if (postIdExpression.indexIn(url) != -1) {
            postId = postIdExpression.cap(1).toInt();
            if (postId != postIdExpression.cap(2).toInt())
                qDebug() << "Failed to parse postId in URL:" << url << "- assuming postId =" << postId;
        }

        // Parse the post list
        emit receivedPostList(document, postId);
    } else if (url.startsWith(m_url + "/newthread.php") || url.startsWith(m_url + "/newreply.php")) {
        QWebElement div = document.findFirst("table.tborder div.panel > div > blockquote > p");
        if (!div.isNull()) {
            qDebug() << "Error message:" << div.toPlainText();
            // "Your submission could not be processed because a security token was missing."
        } else {
            emit receivedNewPost(document);
        }
    } else if (url.startsWith(m_url + "/search.php?searchid=")) {
        int searchId = url.mid(m_url.length() + 21, -1).toInt();
        // Parse search result
        emit receivedSearchResultThreadList(document, searchId);
    } else if (url == m_url + "/search.php?do=process") {
        // Empty search result
        emit receivedSearchResultThreadList(document, 0);
    } else if (url.startsWith(m_url + "/subscription.php")) {
        // Parse the thread list
        emit receivedThreadList(document);
    }

    // Update login status
    if (!userName.isEmpty() && !sessionId.isEmpty() && m_sessionId != sessionId) {
        m_sessionId = sessionId;
        emit sessionIdChanged();
        emit loginSucceeded();

        qDebug() << "Logged into" << m_url << "as" << userName << "- session id" << m_sessionId;
    }
    if ((userName.isEmpty() || sessionId.isEmpty()) && m_sessionId != "") {
        qDebug() << userName << sessionId;
        m_sessionId = "";
        emit sessionIdChanged();

        qDebug() << "Logged out";
    }

    m_busy = false;
    emit busyChanged();
}

#ifndef QT_SIMULATOR
void ForumSession::onSignOnError(const SignOn::Error& error)
{
    qDebug() << "SignOn Error:" << error.message();
}

void ForumSession::onSignOnIdentityInfo(const SignOn::IdentityInfo& info)
{
    qDebug() << "SignOn: Received identity information" << info.id();

    if (!info.isStoringSecret()) {
        qDebug() << "SignOn: No secret stored";
    }

    SignOn::AuthSession* session;
    SignOn::SessionData data;
    data.setUserName(info.userName());

    // Create a new SignOn::AuthSession to request the password
    session = m_identity->createSession(QString("password"));

    connect(session, SIGNAL(response(const SignOn::SessionData&)),
            this, SLOT(onSignOnResponse(const SignOn::SessionData&)));
    connect(session, SIGNAL(error(const SignOn::Error &)),
            this, SLOT(onSignOnError(const SignOn::Error &)));
    session->process(data, QLatin1String("password"));
}

void ForumSession::onSignOnResponse(const SignOn::SessionData& data)
{
    QString secret = data.getProperty("Secret").toString();
    QString userName = data.getProperty("UserName").toString();

    if (secret.isEmpty() || userName.isEmpty()) {
        qDebug() << "SignOn: Empty credentials!";
        setMissingCredentials(true);
    } else {
        qDebug() << "SignOn: Received credentials, logging in";
        setUserName(userName);
        setPassword(secret);
        setMissingCredentials(false);
        login();
    }
}
#endif // !defined(QT_SIMULATOR)

void ForumSession::querySignOnCredentials(void)
{
    qDebug() << "ForumSession::querySignOnCredentials()";
    if (m_provider.isEmpty()) {
        qDebug() << "Error: provider is empty";
        return;
    }

#ifndef QT_SIMULATOR
    if (m_account) {
        delete m_account;
        m_account = 0;
    }

    // Find an account for provider, enabled or otherwise
    Accounts::Manager m_manager;
    Accounts::AccountIdList accounts = m_manager.accountList();
    foreach (Accounts::AccountId id, accounts) {
        Accounts::Account* account = m_manager.account(id);
        if (account->providerName() == m_provider) {
            m_account = account;
            break;
        }
    }

    int credentialsId = 0;
    if (m_account) {
        if (m_account->enabled()) {
            // If there is an enabled tmo account, get corresponding SignOn credentials
            qDebug() << "Accounts: Found account" << m_account->displayName();
            credentialsId = m_account->valueAsInt("CredentialsId");

            qDebug() << "    " << m_account->allKeys();
            qDebug() << "    username =" << m_account->valueAsString("username");
            qDebug() << "    CredentialsId =" << credentialsId;
            qDebug() << "    name =" << m_account->valueAsString("name");
            Accounts::ServiceList services = m_account->services();
            foreach (Accounts::Service* service, services) {
                qDebug() << "    service:" << service->name();
            }
        } else {
            qDebug() << "Accounts: Found disabled account" << m_account->displayName();
        }
    }

    if (credentialsId > 0) {
        // Query SignOn credentials corresponding to m_account
        qDebug() << "Querying identity #" << credentialsId;

        m_identity = SignOn::Identity::existingIdentity(credentialsId);
        connect(m_identity, SIGNAL(info(const SignOn::IdentityInfo&)),
                this, SLOT(onSignOnIdentityInfo(const SignOn::IdentityInfo&)));
        m_identity->queryInfo();

        setMissingCredentials(false);

        return;
    }
#endif // !defined(QT_SIMULATOR)

    setMissingCredentials(true);
}

QObject* ForumSession::activeTopics() const
{
    return m_activeTopics;
}

bool ForumSession::busy(void) const
{
    return m_busy;
}

bool ForumSession::canSignOn(void) const
{
#ifdef QT_SIMULATOR
    return false;
#else
    return true;
#endif
}

QObject* ForumSession::forums(void) const
{
    return m_forums;
}

QString ForumSession::loginUrl(void) const
{
    return m_loginUrl;
}

bool ForumSession::missingCredentials(void) const
{
    return m_missingCredentials;
}

QString ForumSession::password(void) const
{
    return QString("");
}

QString ForumSession::provider(void) const
{
    return m_provider;
}

QString ForumSession::sessionId(void) const
{
    return m_sessionId;
}

QString ForumSession::url(void) const
{
    return m_url;
}

QString ForumSession::userName(void) const
{
    return m_userName;
}

void ForumSession::setLoginUrl(const QString loginUrl)
{
    if (m_loginUrl != loginUrl) {
        m_loginUrl = loginUrl;
        emit loginUrlChanged();
    }
}

void ForumSession::setMissingCredentials(bool missingCredentials)
{
    if (m_missingCredentials != missingCredentials) {
        m_missingCredentials = missingCredentials;
        emit missingCredentialsChanged();
    }
}

void ForumSession::setPassword(const QString password)
{
    if (m_password != password) {
        m_password = password;
    }
}

void ForumSession::setProvider(const QString provider)
{
    if (m_provider != provider) {
        m_provider = provider;
        emit providerChanged();
    }
}

void ForumSession::setUrl(const QString url)
{
    if (m_url != url) {
        m_url = url;

        if (m_url == "http://forum.meego.com")
            m_loginUrl = "https://meego.com/user/login?destination=forum";
        else
            m_loginUrl = "login.php?do=login";

        emit urlChanged();
        emit loginUrlChanged();
    }
}

void ForumSession::setUserName(const QString userName)
{
    if (m_userName != userName) {
        m_userName = userName;
        emit userNameChanged();
    }
}

QObject* ForumSession::createNewPost(void)
{
    qDebug() << "Create new post";
    return new NewPost(this, this);
}

QString ForumSession::externalUrl(const QString url) const
{
    QString u = url;
    if (m_sessionId.isEmpty()) {
        QRegExp sessionIdExpression("s=[a-f\\d]+\\&");
        u = m_url + "/" + u.replace(sessionIdExpression, "");
    } else {
        u = m_url + "/" + u;
    }
    return u;
}

void ForumSession::get(const QUrl& url)
{
    QUrl u;
    if (url.toString().startsWith("http")) {
        if (!url.toString().startsWith(m_url))
            return;
        u = url;
    } else {
        u = QUrl(m_url + "/" + url.toString());
    }
    qDebug() << "URL:" << u.toString();

    m_busy = true;
    emit busyChanged();

    QNetworkRequest request(u);
    m_networkAccess.get(request);
}

void ForumSession::login(void)
{
    m_busy = true;
    emit busyChanged();

    // If this is called from QML code after manually setting credentials
    if (!m_userName.isEmpty() && !m_password.isEmpty())
        setMissingCredentials(false);

    // FIXME - Remove sessionId cookies
    m_networkAccess.setCookieJar(new QNetworkCookieJar());

    // Prepare login POST request
    QUrl url = m_loginUrl.startsWith("http") ? QUrl(m_loginUrl)
                                             : QUrl(m_url + "/" + m_loginUrl);
    QNetworkRequest request(url);
    // Workaround for http://www.qtcentre.org/threads/21470-SSL-Problem
    QSslConfiguration config;
    config.setPeerVerifyMode(QSslSocket::VerifyNone);
    config.setProtocol(QSsl::TlsV1);
    request.setSslConfiguration(config);

    // Prepare login POSTDATA
    QUrl data;
    if (m_url == "http://talk.maemo.org" || m_url == "http://forum.xda-developers.com") {
        QString passwordMd5 = QString(QCryptographicHash::hash(m_password.toUtf8(), QCryptographicHash::Md5).toHex());

        data.addQueryItem("vb_login_username", m_userName);
        data.addQueryItem("vb_login_password", "");
        data.addQueryItem("s", "");
        data.addQueryItem("cookieuser", "1");
        data.addQueryItem("securitytoken", "guest");
        data.addQueryItem("do", "login");
        data.addQueryItem("vb_login_md5password", passwordMd5);
        data.addQueryItem("vb_login_md5password_utf", passwordMd5);
    } else if (m_url == "http://forum.meego.com") {
        data.addQueryItem("name", m_userName);
        data.addQueryItem("pass", m_password);
        data.addQueryItem("form_id", "user_login");
    } else {
        qDebug() << "Login not implemented for" << m_url;

        m_busy = false;
        emit busyChanged();

        return;
    }

    qDebug() << "Requesting" << url.toString() << "to login ...";
    m_networkAccess.post(request, data.encodedQuery());
}

void ForumSession::post(const QUrl& url, const QByteArray& data)
{
    QUrl u;
    if (url.toString().startsWith("http")) {
        if (!url.toString().startsWith(m_url))
            return;
        u = url;
    } else {
        u = QUrl(m_url + "/" + url.toString());
    }
    qDebug() << "POST URL:" << u.toString();
    qDebug() << "POST DATA:" << data;

    m_busy = true;
    emit busyChanged();

    QNetworkRequest request(u);
    m_networkAccess.post(request, data);
}

void ForumSession::refresh(void)
{
    get(m_url);
}

QObject* ForumSession::search(const QString& action)
{
    qDebug() << "ForumSession::search(" << action << ")";

    SearchResultThreadList* m_search = new SearchResultThreadList(this, this);

    if (action == "getdaily" || action == "getnew") {
        QObject::connect(this, SIGNAL(receivedSearchResultThreadList(QWebElement, int)),
                         m_search, SLOT(onReceived(QWebElement, int)));

        QUrl url("/search.php");
        url.addQueryItem("do", action);

        get(url);
    } else {
        QObject::connect(this, SIGNAL(receivedSearchResultThreadList(QWebElement, int)),
                         m_search, SLOT(onReceived(QWebElement, int)));

        QUrl url(m_url  + "/search.php");
        url.addQueryItem("do", "process");

        QUrl data;
        data.addQueryItem("query", action);
        data.addQueryItem("submit", "GO");
        data.addQueryItem("do", "process");
        data.addQueryItem("quicksearch", "1");
        data.addQueryItem("childforums", "1");
        data.addQueryItem("exactname", "1");
        data.addQueryItem("s", "");
        data.addQueryItem("securitytoken", m_securityToken);

        post(url, data.encodedQuery());
    }

    return m_search;
}

void ForumSession::signOn(void)
{
    qDebug() << "ForumSession::signOn";

#ifndef QT_SIMULATOR
    if (!m_accountSetup) {
        m_accountSetup = new AccountSetup::ProviderPluginProxy(this);
        connect(m_accountSetup, SIGNAL(finished()),
                this, SLOT(onAccountSetupFinished()));
    }
    if (m_account) {
        qDebug() << "m_accountSetup->editAccount(" << m_account << ", \"\" )";
        m_accountSetup->editAccount(m_account, "" /* serviceType */);
    } else {
        Accounts::Manager m_manager;
        Accounts::Provider* provider = m_manager.provider(m_provider);
        if (provider) {
            qDebug() << "m_accountSetup->createAccount(" << provider << ", \"\" )";
            m_accountSetup->createAccount(provider, "" /* serviceType */);
        } else {
            qDebug() << "Failed to find account provider";
        }
    }
#endif // !defined(QT_SIMULATOR)
}

QObject* ForumSession::subscribedThreads(void)
{
    ThreadList* m_subscriptions = new ThreadList(this, this);

    QObject::connect(this, SIGNAL(receivedThreadList(QWebElement)),
                     m_subscriptions, SLOT(onReceived(QWebElement)));

    get(QUrl(m_url + "/subscription.php"));

    return m_subscriptions;
}
