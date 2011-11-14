#include "forumsession.h"
#include "newpost.h"

NewPost::NewPost(ForumSession* session, QObject* parent) :
    Post(parent),
    m_session(session),
    m_loggedInUser(0),
    m_postStartTime(0),
    m_forumId(-1),
    m_threadId(-1)
{
    QObject::connect(this, SIGNAL(postIdChanged(void)),
                     this, SLOT(onPostIdChanged(void)));
}

int NewPost::forumId() const
{
    return m_forumId;
}

QString NewPost::preview() const
{
    return m_preview;
}

QString NewPost::securityToken() const
{
    return m_securityToken;
}

int NewPost::threadId() const
{
    return m_threadId;
}

void NewPost::setForumId(int forumId)
{
    qDebug() << "::setForumId" << forumId;
    if (m_forumId != forumId) {
        m_forumId = forumId;
        emit forumIdChanged();

        // Generate a securityToken
        QUrl url(QString("newthread.php?do=newthread&f=%1").arg(forumId));
        QObject::connect(m_session, SIGNAL(receivedNewPost(QWebElement)),
                         this, SLOT(onReceived(QWebElement)));
        m_session->get(url);
    }
}

void NewPost::setSecurityToken(const QString securityToken)
{
    m_securityToken = securityToken;
}

void NewPost::setThreadId(int threadId)
{
    qDebug() << "NewPost::setThreadId(" << threadId << ");";
    if (m_threadId != threadId) {
        m_threadId = threadId;
        emit threadIdChanged();

        // Generate a securityToken
        QUrl url(QString("newreply.php?do=newreply&t=%1").arg(threadId));
        QObject::connect(m_session, SIGNAL(receivedNewPost(QWebElement)),
                         this, SLOT(onReceived(QWebElement)));
        m_session->get(url);
    }
}

void NewPost::onPostIdChanged(void)
{
    // Generate a securityToken
    QUrl url(QString("newreply.php?do=newreply&p=%1").arg(postId()));
    QObject::connect(m_session, SIGNAL(receivedNewPost(QWebElement)),
                     this, SLOT(onReceived(QWebElement)));
    m_session->get(url);
}

void NewPost::onReceived(QWebElement document)
{
    qDebug() << "Received new post";

    QObject::disconnect(m_session, SIGNAL(receivedNewPost(QWebElement)),
                        this, SLOT(onReceived(QWebElement)));

    if (m_securityToken.isEmpty()) {
        QWebElement form = document.findFirst("form[name=vbform]");
        qDebug() << form.attribute("action");
        //
        QRegExp actionExpression("newreply.php\\?do=postreply&t=(\\d+)");
        if (actionExpression.exactMatch(form.attribute("action"))) {
            qDebug() << "setting thread id to %d";
            m_threadId = actionExpression.cap(1).toInt();
        }

        m_securityToken = document.findFirst("input[name=securitytoken]").attribute("value");
        if (m_securityToken.isEmpty())
            qDebug() << "Failed to parse security token";

        // posthash is required for server side handling of attachments
        m_postHash = document.findFirst("input[name=posthash]").attribute("value");
        if (m_postHash.isEmpty())
            qDebug() << "Failed to parse post hash";

        m_postStartTime = document.findFirst("input[name=poststarttime]").attribute("value").toInt();
        m_loggedInUser = document.findFirst("input[name=loggedinuser]").attribute("value").toInt();
    } else {
        QWebElement preview = document.findFirst("script + table.tborder tr > td.tcat");
     // if (preview.isNull())
     //     preview = document.findFirst("table.tborder > tbody > tr > td.tcat");
        if (!preview.isNull()) {
            if (preview.toPlainText() == "Preview") {
                QWebElement body = preview.parent().nextSibling().firstChild();
                // Remove icon and title
                body.firstChild().removeFromDocument(); // "<div.smallfont><img src=\"ICON\"><strong>TITLE</strong></div>"
                body.firstChild().removeFromDocument(); // <hr>

                m_preview = Post::cleanupBody(body);

                // Remove comments
                QRegExp commentExpression("<\\!--.*-->");
                commentExpression.setMinimal(true);
                m_preview.replace(commentExpression, "");

                emit previewChanged();
            } else if (preview.toPlainText().startsWith("The following errors occurred with your submission:")) {
                emit errorMessage(preview.parent().nextSibling().firstChild().toPlainText());
            } else {
                qDebug() << "===============================================";
                qDebug() << document.toOuterXml();
                qDebug() << "===============================================";
                qDebug() << "Unknown error. Dumped preview page.";
                emit errorMessage("Unknown error");
            }
        } else {
            qDebug() << "===============================================";
            qDebug() << document.toOuterXml();
            qDebug() << "===============================================";
            qDebug() << "Unknown error. Dumped page.";
            emit errorMessage("Unknown error");
        }
    }
}

void NewPost::requestPreview()
{
    QUrl url;
    if (m_threadId > 0)
        url = QUrl(QString("newreply.php?do=newreply&t=%1").arg(m_threadId));
    else if (m_forumId > 0)
        url = QUrl(QString("newthread.php?do=newthread&f=%1").arg(m_forumId));
    else
        return;

    QByteArray data("subject=");
    data.append(QUrl::toPercentEncoding(subject()));
    data.append("&message=");
    data.append(QUrl::toPercentEncoding(body())); // TODO: bbCode()
    data.append("&wysiwyg=0");
    data.append("&iconid=0");
    data.append("&s=&securitytoken=");
    data.append(m_securityToken);
    if (m_threadId > 0)
        data.append(QString("&t=%1&do=postreply").arg(m_threadId));
    else if (m_forumId > 0)
        data.append(QString("&f=%1&do=postthread").arg(m_forumId));
    data.append("&posthash=");
    data.append(m_postHash);
    data.append(QString("&poststarttime=%1").arg(m_postStartTime));
    data.append(QString("&loggedinuser=%1").arg(m_loggedInUser));
    data.append("&preview=Preview+Post&parseurl=1&emailupdate=9999&polloptions=4");

    QObject::connect(m_session, SIGNAL(receivedNewPost(QWebElement)),
                     this, SLOT(onReceived(QWebElement)));
    m_session->post(url, data);
}

void NewPost::submit()
{
    QUrl url;
    if (m_threadId > 0)
        url = QUrl(QString("newreply.php?do=newreply&t=%1").arg(m_threadId));
    else if (m_forumId > 0)
        url = QUrl(QString("newthread.php?do=newthread&f=%1").arg(m_forumId));
    else
        return;

    QByteArray data("subject=");
    data.append(QUrl::toPercentEncoding(subject()));
    data.append("&message=");
    data.append(QUrl::toPercentEncoding(body())); // TODO: bbCode()
    data.append("&wysiwyg=0");
    data.append("&iconid=0");
    data.append("&s=&securitytoken=");
    data.append(m_securityToken);
    if (m_threadId > 0)
        data.append(QString("&t=%1&do=postreply").arg(m_threadId));
    else if (m_forumId > 0)
        data.append(QString("&f=%1&do=postthread").arg(m_forumId));
    data.append("&posthash=");
    data.append(m_postHash);
    data.append(QString("&poststarttime=%1").arg(m_postStartTime));
    data.append(QString("&loggedinuser=%1").arg(m_loggedInUser));
    data.append("&sbutton=");
    if (m_threadId > 0)
        data.append("Submit+Reply");
    else
        data.append("Submit+New+Thread");
    data.append("&parseurl=1&emailupdate=9999&polloptions=4");

    QObject::connect(m_session, SIGNAL(receivedNewPost(QWebElement)),
                     this, SLOT(onReceived(QWebElement)));
    m_session->post(url, data);
}
