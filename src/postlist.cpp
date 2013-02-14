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
    roles[ImgRole] = "img";
    roles[AvatarRole] = "avatar";
    roles[StatusRole] = "status";
    roles[StatRole] = "stat";
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
    else if (role == ImgRole)
        return post->img();
    else if (role == StatusRole)
        return post->status();
    else if (role == AvatarRole)
        return post->avatar();
    else if (role == StatRole)
        return post->stat();
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
        if (post->taken()) {
            // post->setParent(m_session);
        } else {
            post->deleteLater();
        }
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

    QString nextThreadTag;
    QString pagesTag;
    if (m_session->provider() == "xda") {
        nextThreadTag = "div#pagination > ul > li.last > a";
        pagesTag = "div#toppage div#hpagination span.verthorz small";
    } else {
        // fmc, tmo
        nextThreadTag = "div.smallfont[align=center] > strong + a + a";
        pagesTag = "div.pagenav td.vbmenu_control";
    }

    // Extract threadId from the previous / next thread links
    int threadId;
    QWebElement a = document.findFirst(nextThreadTag);
    if (a.isNull()) {
        // The above doesn't work for forum.meego.com subscriptions, so check the First page link
        a = document.findFirst("div.pagenav > table > tbody > tr > td > a");
    }
    if (!a.isNull()) {
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
    } else {
        qDebug() << document.toInnerXml();
    }
    if (m_threadId == -1) {
        qDebug() << "Could not determine threadId!";
    }

    int page = 1;
    int numPages = 1;
    QString section;
    const QWebElement td = document.findFirst(pagesTag);
    if (!td.isNull()) {
        QRegExp pageExpression("Page (\\d+) of (\\d+)");
        if (pageExpression.exactMatch(td.toPlainText())) {
            page = pageExpression.cap(1).toInt();
            numPages = pageExpression.cap(2).toInt();
        }
    }
    if (numPages > 1)
        section.sprintf("Page %d of %d", page, numPages);

    // If this is the last page, mark the thread as read
    if (thread && page == numPages)
        thread->setUnread(false);

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
    /*
    <div id="breadcrumbs">
    <div id="breadcontainer">
    <ul style="width:55%;">
    <li><a href="index.php" accesskey="1"><img src="http://media.xda-developers.com/images/icons/breadcrumbs-home.png" alt="Home"/></a><span><img src="http://media.xda-developers.com/images/icons/breadcrumbs-arrow.png" alt="Arrow"/></span></li>


        <li><a href="forumdisplay.php?f=240">General discussion</a><span><img src="http://media.xda-developers.com/images/icons/breadcrumbs-arrow.png" alt="Arrow"/></span></li>


        <li><a href="forumdisplay.php?f=263">About xda-developers.com</a><span><img src="http://media.xda-developers.com/images/icons/breadcrumbs-arrow.png" alt="Arrow"/></span></li>


    <li>
     [FAQ] User signature?

    </li>
    </ul>
    </div>
                */
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

        QStringList temp = (Post::cleanupBody(comment)).split("##splitMarker##", QString::SkipEmptyParts);

        QString html = "<div style>a {color:#cc09ba}</style>" + temp.at(0) + "</div>";
        QString img = "";
        if(temp.length()>1)
            img = temp.at(1);

        // SIG = div.nextSibling();

        // Thanks
        QStringList thanks;
        QWebElement div = table.findFirst("div.thanks_postbit");
        if (!div.isNull()) {
            foreach (QWebElement a, div.findAll("td > div > a")) {
                thanks.append(a.toPlainText());
            }
        }

        // Remove comments
        QRegExp commentExpression("<\\!--.*-->");
        commentExpression.setMinimal(true);
        html.replace(commentExpression, "");

        // qDebug() << "BODY:" << html.simplified();

        Post* post = new Post(url, poster, dateTime, html, img, "", "", 0);
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

    // talk.maemo.org, forum.xda-developer.com
    QString commentTags;
    QString posterTag;
    QString postDateTag;
    QString bodyTag;
    if (m_session->provider() == "tmo") {
        commentTags = "div.mfcomment";
        posterTag = "div.username > img + span > a";
        postDateTag = "div.postdate";
        bodyTag = "div.postdetails";
    } else /* if (m_session->provider() == "xda") */ {
        commentTags = "div.post";
        posterTag = "div.post-author > div > img + a";
        postDateTag = "div.post-date";
        bodyTag = "div.post-message";
    }

    foreach (QWebElement comment, document.findAll(commentTags)) {
        QString url = "showpost.php?p=" + comment.attribute("id").replace("edit", "").replace("post", "");

        //AVATAR
        QString avatar = comment.findFirst("div.useravatar > a > img.photo").attribute("src");
        if(avatar.isNull())
            avatar = "";
        else
            avatar = "http://talk.maemo.org/" + avatar;

        //Online-offline
        int status = (comment.findFirst("div.username > img.inlineimg").attribute("alt")).indexOf("online")==-1 ? 0 : 1;
        //qDebug() << "AVATAR:" << avatar << status;

        QString poster = comment.findFirst(posterTag).toPlainText();
        QString dateTime = DateTimeHelper::parseDateTime(comment.findFirst(postDateTag).toPlainText());

        QString userStats = comment.findFirst("div.userstats").toPlainText();

        // Find the post body
        QWebElement body = comment.findFirst(bodyTag);

        // Remove attachments fieldset ?? NO!!
        //FIXME - fmc attachment isn't working
        QWebElement fieldset = body.findFirst("div > fieldset.fieldset").parent();//.takeFromDocument();

        // Parse attachments
        if (!fieldset.isNull()) {
            foreach (QWebElement b, fieldset.findAll("td")) {
                foreach (QWebElement a, b.findAll("a")) {
                    QString href = a.attribute("href");
                    if (href.startsWith("attachment.php?")) {
                        if (m_session->provider() == "tmo")
                            href = "http://talk.maemo.org/" + href;
                        a.setAttribute("href",href);
                    }
                }
                foreach (QWebElement a, b.findAll("img")) {
                    a.setAttribute("src","image://theme/icon-s-email-attachment");
                }
            }

            foreach (QWebElement a, fieldset.findAll("img")) {
                QString href = a.attribute("src");
                if (href.startsWith("attachment.php?")) {
                    if (m_session->provider() == "tmo")
                        href = "http://talk.maemo.org/" + href;
                    a.setAttribute("src",href);
                }
            }
        }

        QStringList temp = (Post::cleanupBody(body)).split("##splitMarker##", QString::SkipEmptyParts);

        QString html = "<style>a {color:#ea650a}</style>" + temp.at(0);
        QString img = "";
        if(temp.length()>1)
            img = temp.at(1);

        // Thanks
        QStringList thanks;
        QWebElement div = comment.parent().findFirst("div.thanks_postbit");
        if (!div.isNull()) {
            foreach (QWebElement a, div.findAll("td > div > a")) {
                thanks.append(a.toPlainText());
            }
        }

        // Remove comments
        QRegExp commentExpression("<\\!--.*-->");
        commentExpression.setMinimal(true);
        html.replace(commentExpression, "");

        // qDebug() << "BODY:" << html.simplified();

        QString section;
        section.sprintf("Page %d of %d", page, numPages);

        Post* post = new Post(url, poster, dateTime, html, img, userStats, avatar, status);
        post->setThanks(thanks.join(", "));
        post->setSection(section);
        post->setSubject(threadTitle); // talk.maemo.org doesn't show individual post subjects

        static const QRegExp postIdExpression("[a-z]+(\\d+)");
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
