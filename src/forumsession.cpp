#include <QDebug>

#ifndef QT_SIMULATOR
#include <manager.h> // Accounts::Manager
#endif

#include "forumlist.h"
#include "forumlistcache.h"
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
    m_busy(false),
    // This helps to prevent a guest login until credentials are proven to be missing
    m_missingCredentials(false)
{
    m_forums = new ForumList(this, this);
    // ForumListCache* cache = new ForumListCache(m_forums, this);

    // Disable JavaScript and loading of external objects
    m_webPage.settings()->setAttribute(QWebSettings::AutoLoadImages, false);
    m_webPage.settings()->setAttribute(QWebSettings::JavascriptEnabled, false);
    m_webPage.settings()->setAttribute(QWebSettings::PrintElementBackgrounds, false);
    m_webPage.settings()->setAttribute(QWebSettings::LocalContentCanAccessRemoteUrls, false);
    m_webPage.settings()->setAttribute(QWebSettings::LocalContentCanAccessFileUrls, false);

    connect(&m_networkAccess, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(onReceived(QNetworkReply*)));

    connect(&m_webPage, SIGNAL(loadFinished(bool)),
            this, SLOT(webElementFunc(bool)));

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
        // No credentials available, security token still empty, login as guest
        if (m_securityToken.isEmpty()) {
            m_forums->load();
        }
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
    QString sessionId;

    // Check out cookies
    foreach (QNetworkCookie cookie, m_networkAccess.cookieJar()->cookiesForUrl(QUrl(m_url))) {
        if (cookie.name() == "bbsessionhash")
            sessionId = cookie.value();
    }

    // Now handle the contents
    const QByteArray html = reply->readAll().constData();

    if (html.isEmpty())
    {
        qDebug() << "ERROR:" << reply->errorString();
        qDebug() << "Received empty page!";
        m_busy = false;
        emit busyChanged();
        return;
    }

    rep_url = reply->url();
    path = reply->url().path();
    m_webPage.mainFrame()->setContent(html);

}

/*====================== BROKE APART THE FUNCTION =========================*/

void ForumSession::webElementFunc(bool status)
{
    if(!status) {
        qDebug() << "ERROR: ";
        qDebug() << "Loading QWebPage failed";
        m_busy = false;
        emit busyChanged();
        return;
    }

    QString userName = m_userName;

    const QWebElement document = m_webPage.mainFrame()->documentElement();

    if (rep_url == (m_loginUrl.startsWith("http") ? QUrl(m_loginUrl)
                    : QUrl(m_url + "/" + m_loginUrl))) {
        // This is a reply to the login POST request

        // For talk.maemo.org: The first <form name="postvarform"> element
        const QWebElement form = document.findFirst("form[name=\"postvarform\"]");
        if (form.attribute("name") == "postvarform") {
            QRegExp urlExpression(m_url + "/index.php\\?s=([a-f\\d]+)");
            if (urlExpression.exactMatch(form.attribute("action"))) {
                c_sessionId = urlExpression.cap(1);
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
            // maemo.org - Talk error message
            QWebElement div = document.findFirst("table.tborder div.panel > div > div");
            if (!div.isNull()) {
                QString message = div.toPlainText();
                qDebug() << "Error message:" << message;

                emit loginFailed(message);
            } else {
                // MeeGo Forum error message
                div = document.findFirst("div.messages");
                if (!div.isNull()) {
                    QString message = div.toPlainText();
                    qDebug() << "Error message:" << message;

                    emit loginFailed(message);
                } else {
                    qDebug() << "================================================================================";
                    qDebug() << document.toPlainText();
                    qDebug() << "================================================================================";
                    qDebug() << "197, forumsession.cpp :: Unknown error. Dumped page";

                    emit loginFailed("Unknown error");
                }
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

    // Scan embedded JavaScript for security token
    foreach (const QWebElement script, document.findAll("script")) {
        static QRegExp securityTokenExpression("var SECURITYTOKEN = \"([-\\da-fgust]*)\"");
        if (securityTokenExpression.indexIn(script.toInnerXml()) != -1) {
            QString securityToken = securityTokenExpression.cap(1);
            if (m_securityToken != securityToken)
                m_securityToken = securityToken;
            break;
        }
    }

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
        if (path != "/subscription.php")
            userName = "";
        if (m_securityToken != "guest") {
            qDebug() << "SETTING SECURITY TOKEN TO \"guest\"";
            m_securityToken = "guest";
        }
    }

#if 0
    // talk.maemo.org - Search form for security token
    form = document.findFirst("form[action=\"search.php?do=process\"]");
    if (!form.isNull()) {
        QString securityToken = form.findFirst("input[name=\"securitytoken\"]").attribute("value");
        if (m_securityToken != securityToken) {
            m_securityToken = securityToken;
            qDebug() << "SECURITY TOKEN CHANGED:" << securityToken;
        }
    }
#endif

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
    // const QUrl& url = reply->url();
    // QString path = reply->url().path();

    qDebug() << "REPLY URL:" << rep_url.toString();

    if (path.isEmpty() || path == "/index.php" || rep_url.toString() == m_url) {
        // Parse the forum list
        emit receivedForumList(document);
    } else if (path == "/forumdisplay.php") {
        if (rep_url.queryItemValue("do") == "markread") {
            QWebElement error = document.findFirst("td.panelsurround > div.panel > div > div");
            if (!error.isNull()) {
                qDebug() << "Error:" << error.toPlainText().simplified();
            }
            // else emit markedForumRead
        } else {
            // Parse the thread list
            emit receivedThreadList(document);
        }
    } else if (path == "/showthread.php" || path =="/announcement.php") {
        int postId = -1;
        if (rep_url.hasQueryItem("p"))
            postId = rep_url.queryItemValue("p").toInt();
        // Parse the post list
        emit receivedPostList(document, postId);
    } else if (path == "/newthread.php" || path == "/newreply.php" || path == "/editpost.php") {
        QWebElement div = document.findFirst("table.tborder div.panel > div > blockquote > p");
        if (!div.isNull()) {
            qDebug() << "Error message:" << div.toPlainText();
            // "Your submission could not be processed because a security token was missing."
        } else {
            emit receivedNewPost(document);
        }
    } else if (path == "/search.php") {
        if (rep_url.hasQueryItem("searchid")) {
            int searchId = rep_url.queryItemValue("searchid").toInt();
            // Parse search result
            emit receivedSearchResultThreadList(document, searchId);
        } else if (rep_url.queryItemValue("do") == "process") {
            // Empty search result
            emit receivedSearchResultThreadList(document, 0);
        } else {
            qDebug() << "Unknown search result";
        }
        disconnect(this, SIGNAL(receivedSearchResultThreadList(QWebElement, int)));

    } else if (path == "/subscription.php") {
        const QString mode = rep_url.queryItemValue("do");
        if (mode == "addsubscription") {
            // Right now we just acknowledge the subscription without email notification
            QString threadId = document.findFirst("input[name=threadid]").attribute("value");

            if (!threadId.isEmpty()) {
                QUrl url(m_url + "/subscription.php");
                url.addQueryItem("do", "doaddsubscription");
                url.addQueryItem("threadid", threadId);

                QUrl data;
                data.addQueryItem("s", "");
                data.addQueryItem("securitytoken", m_securityToken);
                data.addQueryItem("do", "doaddsubscription");
                data.addQueryItem("threadid", threadId);
                data.addQueryItem("url", "index.php");
                // QString emailUpdate = document.findFirst("select[name=emailupdate] > option[selected=selected]").attribute("value");
                data.addQueryItem("emailupdate", m_notif); // Implemented custom email notification!! Yaaay!!
                QString folderId = document.findFirst("select[name=folderid] > option[selected=selected]").attribute("value");
                data.addQueryItem("folderid", folderId); // Subscriptions
                data.addQueryItem("submit", "Add Subscription");

                post(url, data.encodedQuery());
            }
        } else if (mode == "doaddsubscription") {
            int threadId = rep_url.queryItemValue("threadid").toInt();
            emit subscriptionChanged(threadId, true);
            disconnect(this, SIGNAL(subscriptionChanged(int, bool)));
        } else if (mode == "removesubscription") {
            int threadId = rep_url.queryItemValue("t").toInt();
            emit subscriptionChanged(threadId, false);
            disconnect(this, SIGNAL(subscriptionChanged(int, bool)));
        } else {
            // Parse the thread list
            emit receivedThreadList(document);
        }
    } else if (path == "/post_thanks.php") {
        QString postId = rep_url.queryItemValue("p");
        QStringList thanks;
        foreach (const QWebElement a, document.findAll("table tr td div a"))
            thanks.append(a.toPlainText());

        emit receivedThanks(postId, thanks);
        disconnect(this, SIGNAL(receivedThanks(QString, QStringList)), 0, 0);
    }

    // Update login status
    if (!userName.isEmpty() && !c_sessionId.isEmpty() && m_sessionId != c_sessionId) {
        m_sessionId = c_sessionId;
        emit sessionIdChanged();
        emit loginSucceeded();

        qDebug() << "Logged into" << m_url << "as" << userName << "- session id" << m_sessionId;
    }
    if ((userName.isEmpty() || c_sessionId.isEmpty()) && m_sessionId != "") {
        qDebug() << userName << c_sessionId;
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
    // Find an account for provider, enabled or otherwise
    m_account = 0;
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

QString ForumSession::notif(void) const
{
    return m_notif;
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

void ForumSession::reset(void)
{
    // Clear login credentials, security token and session id
    setUserName("");
    setPassword("");
    setMissingCredentials(false);
    m_securityToken = "";
    m_sessionId = "";

    // Delete active topics list
    delete m_activeTopics;
    m_activeTopics = 0;
    emit activeTopicsChanged();

    // Clear forum list
    if (m_forums) {
        delete m_forums;
        m_forums = 0;
        emit forumsChanged();
    }
    m_forums = new ForumList(this, this);
    emit forumsChanged();
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

        if (provider == "fmc") {
            setUrl("http://forum.meego.com");
        } else if (provider == "tmo") {
            setUrl("http://talk.maemo.org");
        } else if (provider == "nokia") {
            setUrl("http://www.developer.nokia.com/Community/Discussion/");
        } else if (provider == "xda") {
            setUrl("http://forum.xda-developers.com");
        }

        // Reset the forum session
        reset();

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

void ForumSession::setNotif(QString notif)
{
    m_notif = notif;
}

QObject* ForumSession::createNewPost(void)
{
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

QString ForumSession::forumUrl(int forumId) const
{
    return QString(m_url + "/forumdisplay.php?f=%1").arg(forumId);
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

        //TODO: Verify space in username bug fix
        m_userName = m_userName.replace(" ","+");

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

void ForumSession::subscribe(QObject* thread)
{
    Thread* t = qobject_cast<Thread*>(thread);
    if (!t)
        return;

    connect(this, SIGNAL(subscriptionChanged(int, bool)),
            t, SLOT(onSubscriptionChanged(int, bool)));

    QUrl url("subscription.php");
    url.addQueryItem("do", "addsubscription");
    url.addQueryItem("t", QString("%1").arg(t->threadId()));

    get(url);
}

QObject* ForumSession::subscribedThreads(void)
{
    ThreadList* m_subscriptions = new ThreadList(this, this);

    QObject::connect(this, SIGNAL(receivedThreadList(QWebElement)),
                     m_subscriptions, SLOT(onReceived(QWebElement)));

    get(QUrl(m_url + "/subscription.php"));

    return m_subscriptions;
}

void ForumSession::thank(QObject* post)
{
    Post* p = qobject_cast<Post*>(post);
    if (!p || m_securityToken.isEmpty())
        return;

    connect(this, SIGNAL(receivedThanks(QString, QStringList)),
            p, SLOT(onThanksReceived(QString, QStringList)));

    QUrl url("post_thanks.php");
    url.addQueryItem("do", "post_thanks_add");
    url.addQueryItem("using_ajax", "1");
    url.addQueryItem("p", QString("%1").arg(p->postId()));
    url.addQueryItem("securitytoken", m_securityToken);

    get(url);
}

void ForumSession::unsubscribe(QObject* thread)
{
    Thread* t = qobject_cast<Thread*>(thread);
    if (!t)
        return;

    connect(this, SIGNAL(subscriptionChanged(int, bool)),
            t, SLOT(onSubscriptionChanged(int, bool)));

    QUrl url("subscription.php");
    url.addQueryItem("do", "removesubscription");
    url.addQueryItem("t", QString("%1").arg(t->threadId()));

    get(url);
}



void ForumSession::unThank(QObject* post)
{
    Post* p = qobject_cast<Post*>(post);
    if (!p || m_securityToken.isEmpty())
        return;

    connect(this, SIGNAL(receivedThanks(QString, QStringList)),
            p, SLOT(onThanksReceived(QString, QStringList)));

    QUrl url("post_thanks.php");
    url.addQueryItem("do", "post_thanks_remove_user");
    url.addQueryItem("using_ajax", "1");
    url.addQueryItem("p", QString("%1").arg(p->postId()));
    url.addQueryItem("securitytoken", m_securityToken);

    get(url);
}

void ForumSession::markRead(QObject* forum)
{
    Forum* f = qobject_cast<Forum*>(forum);
    if (!f || m_securityToken.isEmpty())
        return;

    QUrl url("forumdisplay.php");
    url.addQueryItem("do", "markread");
    url.addQueryItem("f", QString("%1").arg(f->forumId()));
    url.addQueryItem("markreadhash", m_securityToken);

    get(url);
}

