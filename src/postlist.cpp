#include <QDebug>

#include "datetimehelper.h"
#include "forumsession.h"
#include "post.h"
#include "postlist.h"
#include "thread.h"

PostList::PostList(ForumSession* session, QObject *parent) :
    QAbstractListModel(parent),
    m_session(session),
    m_firstPage(0),
    m_jumpToIndex(0),
    m_lastPage(0),
    m_numPages(0),
    m_threadId(-1)
{
    QHash<int, QByteArray> roles;
    roles[UrlRole] = "url";
    roles[PosterRole] = "poster";
    roles[DateTimeRole] = "dateTime";
    roles[BodyRole] = "body";
    roles[SectionRole] = "section";
    roles[ThanksRole] = "thanks";
    setRoleNames(roles);

    QObject::connect(m_session, SIGNAL(receivedPostList(QWebElement, int)),
                     this, SLOT(onReceived(QWebElement, int)));
}

PostList::~PostList(void)
{
    clear();
}

int PostList::count() const
{
    return m_posts.count();
}

int PostList::firstPage() const
{
    return m_firstPage;
}

int PostList::jumpToIndex() const
{
    return m_jumpToIndex;
}

int PostList::lastPage() const
{
    return m_lastPage;
}

int PostList::numPages() const
{
    return m_numPages;
}

int PostList::threadId() const
{
    return m_threadId;
}

QString PostList::url() const
{
    return m_url;
}

void PostList::setThreadId(int threadId)
{
    m_threadId = threadId;
}

void PostList::setUrl(const QString url)
{
    m_firstPage = 0;
    m_url = url;

    qDebug() << "Requesting" << url << "for post list ...";
    m_session->get(url);
}

QVariant PostList::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    Post* post = m_posts.at(index.row());
    if (role == UrlRole)
        return post->url();
    else if (role == PosterRole)
        return post->poster();
    else if (role == DateTimeRole)
        return post->dateTime();
    else if (role == BodyRole)
        return post->body();
    else if (role == SectionRole)
        return post->section();
    else if (role == ThanksRole)
        return post->thanks();

    return QVariant();
}

int PostList::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);

    return m_posts.count();
}

void PostList::clear(void)
{
    beginRemoveRows(QModelIndex(), 0, m_posts.count());
    while (!m_posts.empty()) {
        Post* post = m_posts.takeFirst();
        post->deleteLater();
    }
    endRemoveRows();
}

void PostList::onPostChanged()
{
    Post* post = qobject_cast<Post*>(sender());
    if (!post)
        return;

    int row = m_posts.indexOf(post);
    if (row >= 0)
        dataChanged(index(row, 0), index(row, 0));
}

void PostList::onReceived(QWebElement document, int postId)
{
    Thread* thread = qobject_cast<Thread*>(QObject::parent());

    qDebug() << "Received a post list";

    // Extract threadId from the previous / next thread links
    int threadId;
    const QWebElement a = document.findFirst("div.smallfont[align=center] > strong + a + a");
    if (!a.isNull()) {
        if (a.previousSibling().toPlainText() == "Previous Thread" && a.toPlainText() == "Next Thread") {
            QRegExp threadIdExpression("showthread.php\\?t=(\\d+).*");
            if (threadIdExpression.exactMatch(a.attribute("href"))) {
                threadId = threadIdExpression.cap(1).toInt();
                if (m_threadId != threadId) {
                    if (m_threadId != -1) {
                        qDebug() << "Thread ID different:" << m_threadId << "!=" << threadId << "- ignoring.";
                        return;
                    } else {
                        qDebug() << "Setting thread ID to" << threadId;
                    }
                    m_threadId = threadId;
                    if (thread) {
                        if (thread->threadId() == -1)
                            thread->setThreadId(threadId);
                        else if (thread->threadId() != threadId)
                            qDebug() << "Parent thread ID != post list thread ID!";
                    }
                }
            }
        }
    }
    if (m_threadId == -1) {
        qDebug() << "Could not determine threadId!";
    }

    int page = 1;
    int numPages = 1;
    QString section;
    // forum.meego.com, talk.maemo.org
    const QWebElement td = document.findFirst("div.pagenav td.vbmenu_control");
    if (!td.isNull()) {
        QRegExp pageExpression("Page (\\d+) of (\\d+)");
        if (pageExpression.exactMatch(td.toPlainText())) {
            page = pageExpression.cap(1).toInt();
            numPages = pageExpression.cap(2).toInt();
        }
    }
    if (numPages > 1)
        section.sprintf("Page %d of %d", page, numPages);

    qDebug() << section << "(" << m_firstPage << "-" << m_lastPage << ")";
    if (m_lastPage > 0 && page == m_lastPage) {
        // This is a reload of the last page
        m_firstPage = m_lastPage;
        emit firstPageChanged();
    }
    if (m_firstPage == 0 || page == m_firstPage - 1) {
        m_firstPage = page;
        emit firstPageChanged();
    }
    if (m_lastPage == 0 || page == m_lastPage + 1) {
        m_lastPage = page;
        emit lastPageChanged();
    }
    if (page < m_firstPage || page > m_lastPage) {
        m_firstPage = m_lastPage = page;
        emit firstPageChanged();
        emit lastPageChanged();
    }
    if (m_numPages != numPages) {
        m_numPages = numPages;
        emit numPagesChanged();
    }

    if (m_firstPage == m_lastPage)
        clear();

    // Extract thread title from the breadcrumb bar
    QString threadTitle;
    QWebElement crumbs = document.findFirst("div#breadcrumb");
    QRegExp threadTitleExpression("&gt;\\s*([^<>]*)\\s*$");
    if (threadTitleExpression.indexIn(crumbs.toInnerXml()) != -1) {
        crumbs.setInnerXml(threadTitleExpression.cap(1));
        threadTitle = crumbs.toPlainText();
    }
    qDebug() << "THREAD TITLE:" << threadTitle;

    // talk.maemo.org - extract subscription status from the thread tools menu
    if (thread) {
        const QWebElement img = document.findFirst("table.tborder > tbody > tr > td.alt1 > div > div > img[alt=Subscription]");
        if (!img.isNull()) {
            const QWebElement a = img.nextSibling();
            if (a.tagName() == "A") {
                const QString href = a.attribute("href");
                if (href.startsWith("subscription.php?do=removesubscription"))
                    thread->setSubscribed(true);
                if (href.startsWith("subscription.php?do=addsubscription"))
                    thread->setSubscribed(false);
            }
        }
    }

    Post* jumpToPost = 0;
    QList<Post*> list;

    // forum.meego.com
    foreach (QWebElement comment, document.findAll("div.vb_message")) {
        const QWebElement table = comment.parent().parent().parent();
        QString url;

        QWebElement a = table.findFirst("a.bigusername");
     // a.attribute("href") == "member.php?u=(\\d+)"
        QString poster = a.toPlainText();
        QString dateTime = DateTimeHelper::parseDateTime(table.findFirst("td.thead > div.normal").nextSibling().toPlainText());

     // a.nextSibling().attribute("alt").endsWith("is offline")

        QString html = Post::cleanupBody(comment);

        // SIG = div.nextSibling();

        // Thanks
        QStringList thanks;
        QWebElement div = table.findFirst("div.thanks_postbit");
        if (!div.isNull()) {
            qDebug() << "Thanks:";
            foreach (QWebElement a, div.findAll("td > div > a")) {
                thanks.append(a.toPlainText());
            }
        }

        // Remove comments
        QRegExp commentExpression("<\\!--.*-->");
        commentExpression.setMinimal(true);
        html.replace(commentExpression, "");

        qDebug() << "BODY:" << html.simplified();

        Post* post = new Post(url, poster, dateTime, html);
        post->setThanks(thanks.join(", "));
        post->setSection(section);

        static const QRegExp postIdExpression("post_message_(\\d+)");
        if (postIdExpression.exactMatch(comment.attribute("id"))) {
            post->setPostId(postIdExpression.cap(1).toInt());

            // Remember the post object the URL told us to jump to
            if (postId == post->postId())
                jumpToPost = post;
        } else {
            qDebug() << "Failed to parse postId:" << comment.toOuterXml().simplified();
        }

        list.append(post);
    }

    // talk.maemo.org
    foreach (QWebElement comment, document.findAll("div.mfcomment")) {
        QString url = "showpost.php?p=" + comment.attribute("id").replace("post", "");
     // qDebug() << "AVATAR:" << comment.findFirst("div.useravatar > a > img.photo").attribute("alt");

        QString poster = comment.findFirst("div.username > img + span > a").toPlainText();
        QString dateTime = DateTimeHelper::parseDateTime(comment.findFirst("div.postdate").toPlainText());

        QString userStats = comment.findFirst("div.userstats").toPlainText();
        Q_UNUSED(userStats);

        // Find the post body
        QWebElement body = comment.findFirst("div.postdetails");

        // Remove attachments fieldset
        QWebElement fieldset = body.findFirst("div > fieldset.fieldset").parent().takeFromDocument();

        QString html = Post::cleanupBody(body);

        // Thanks
        QStringList thanks;
        QWebElement div = comment.parent().findFirst("div.thanks_postbit");
        if (!div.isNull()) {
            qDebug() << "Thanks:";
            foreach (QWebElement a, div.findAll("td > div > a")) {
                thanks.append(a.toPlainText());
            }
        }

        // Remove comments
        QRegExp commentExpression("<\\!--.*-->");
        commentExpression.setMinimal(true);
        html.replace(commentExpression, "");

        qDebug() << "BODY:" << html.simplified();

        QString section;
        section.sprintf("Page %d of %d", page, numPages);

        Post* post = new Post(url, poster, dateTime, html);
        post->setThanks(thanks.join(", "));
        post->setSection(section);
        post->setSubject(threadTitle); // talk.maemo.org doesn't show individual post subjects

        static const QRegExp postIdExpression("post(\\d+)");
        if (postIdExpression.exactMatch(comment.attribute("id"))) {
            post->setPostId(postIdExpression.cap(1).toInt());

            // Remember the post object the URL told us to jump to
            if (postId == post->postId())
                jumpToPost = post;
        } else {
            qDebug() << "Failed to parse postId:" << comment.toOuterXml().simplified();
        }

        // Parse attachments
        if (!fieldset.isNull()) {
            // qDebug() << "REMOVED FIELDSET:" << fieldset.toOuterXml();
            foreach (QWebElement a, fieldset.findAll("td a")) {
                QString href = a.attribute("href");
                if (href.startsWith("attachment.php?")) {
                    // TODO: "(name:.*) (([\\.\\d]+) KB, (\\d+) views)"
                    qDebug() << "ATTACHMENT:" << a.parent().toPlainText() << href;
                }
            }
        }

        list.append(post);
    }

    // Add entries to the list model in the correct place and order
    if (list.count() > 0) {
        if (page == m_firstPage) {
            beginInsertRows(QModelIndex(), 0, list.count() - 1);
            while (!list.empty()) {
                Post* post = list.takeLast();
                connect(post, SIGNAL(thanksChanged()),
                        this, SLOT(onPostChanged()));
                m_posts.prepend(post);
            }
        } else {
            beginInsertRows(QModelIndex(), m_posts.count(), m_posts.count() + list.count() - 1);
            m_posts.append(list);
        }

        // Export index of the post the URL told us to jump to
        m_jumpToIndex = jumpToPost ? m_posts.indexOf(jumpToPost) : -1;

        endInsertRows();
    }
    emit countChanged();
    qDebug() << m_posts.count();
}

QObject* PostList::get(int index) const
{
    if (index < 0 || index > m_posts.count())
        return 0;

    return m_posts[index];
}

void PostList::load(int page)
{
    m_url.sprintf("/showthread.php?t=%d&page=%d", m_threadId, page);
    m_url = m_session->url() + m_url;

    qDebug() << "Requesting thread" << m_url << "for post list ...";

    m_session->get(QUrl(m_url));
}
