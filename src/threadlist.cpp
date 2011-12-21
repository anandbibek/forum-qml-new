#include <QDebug>

#include "datetimehelper.h"
#include "forumsession.h"
#include "postlist.h"
#include "thread.h"
#include "threadlist.h"

ThreadList::ThreadList(ForumSession* session, QObject *parent) :
    PagedListModel(parent),
    m_session(session),
    m_stickySection(true)
{
    QHash<int, QByteArray> roles;
    roles[ForumRole] = "forum";
    roles[PosterRole] = "poster";
    roles[RepliesRole] = "replies";
    roles[RatingValueRole] = "ratingValue";
    roles[SectionRole] = "section";
    roles[DateTimeRole] = "dateTime";
    roles[TitleRole] = "title";
    roles[UnreadRole] = "unread";
    roles[VotesRole] = "votes";
    roles[UrlRole] = "url";
    roles[AttachmentsRole] = "attachments";
    setRoleNames(roles);
}

QString ThreadList::url() const
{
    return m_url;
}

int ThreadList::count() const
{
    return children().count();
}

void ThreadList::setUrl(QString url)
{
    if (m_url != url) {
        m_firstPage = 0;
        m_url = url;

        QObject::connect(m_session, SIGNAL(receivedThreadList(QWebElement)),
                         this, SLOT(onReceived(QWebElement)));

        qDebug() << "Requesting" << url << "for thread list ...";
        m_session->get(url);
    }
}

QVariant ThreadList::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    Thread* thread = qobject_cast<Thread*>(children().at(index.row()));
    if (role == UrlRole)
        return thread->url();
    else if (role == TitleRole)
        return thread->title();
    else if (role == RepliesRole)
        return thread->replies();
    else if (role == ForumRole)
        return thread->forum();
    else if (role == PosterRole)
        return thread->poster();
    else if (role == RatingValueRole)
        return thread->ratingValue();
    else if (role == SectionRole)
        return thread->section();
    else if (role == DateTimeRole)
        return thread->dateTime();
    else if (role == UnreadRole)
        return thread->unread();
    else if (role == VotesRole)
        return thread->votes();
    else if (role == AttachmentsRole)
        return thread->attachments();

    return QVariant();
}

int ThreadList::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)

    return children().count();
}

void ThreadList::onReceived(QWebElement document)
{
    qDebug() << "Received thread list";

    QObject::disconnect(m_session, SIGNAL(receivedThreadList(QWebElement)),
                        this, SLOT(onReceived(QWebElement)));

    QString pagesTag;
    if (m_session->provider() == "xda")
        pagesTag = "div#hpagination span small";
    else
        // fmc, tmo
        pagesTag = "div.pagenav table tr td.vbmenu_control";

    // Parse page information
    int page = 1;
    int numPages = 1;
    QWebElement td = document.findFirst(pagesTag);
    if (!td.isNull()) {
        QRegExp pageExpression("Page (\\d+) of (\\d+)");
        if (pageExpression.exactMatch(td.toPlainText())) {
            page = pageExpression.cap(1).toInt();
            numPages = pageExpression.cap(2).toInt();
        }
    }
    qDebug() << "Thread list page" << page << "of" << numPages << "(" << m_firstPage << "-" << m_lastPage << ")";
    if (m_firstPage > 0 && page == m_firstPage) {
        // This is a reload of the first page, force clearing the list
        m_lastPage = m_firstPage;
        emit lastPageChanged();
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
        qDebug() << "Resetting page";
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

    // forum.meego.com - Extract forum name from breadcrumb bar
    QString forum;
    const QWebElement span = document.findFirst(".drupal_breadcrumb .drupal_separator");
    if (!span.isNull()) {
        forum = span.parent().toPlainText();
        if (forum.startsWith("Forum")) {
            // User Control Panel -> Subscriptions doesn't contain valid forum information
            if (!forum.startsWith("ForumUser Control Panel"))
                forum = forum.mid(5);
            else
                forum = "";
        }
    }

    QString threadsTag;
    QString posterPattern;
    if (m_session->provider() == "xda") {
        // forum.xda-developers.com
        threadsTag = "tbody > tr.threadbit-tr > td:first-child > img";
        posterPattern = "Originally Posted By: (.*)";
    } else if (m_session->provider() == "fmc") {
        // forum.meego.com
        threadsTag = "table.tborder tr > td.alt1 > img";
        posterPattern = "(.*) @ (\\d{2}-\\d{2}-\\d{2,4})";
    }
    QRegExp posterExpression(posterPattern);

    // forum.meego.com, forum.xda-developers.com
    QWebElementCollection threads = document.findAll(threadsTag);
    foreach (QWebElement img, threads) {
        // Read / unread / hot / closed
        QWebElement td = img.parent();
        if (!td.attribute("id").startsWith("td_threadstatusicon")) {
            qDebug() << "False positive for thread?" << td.parent().toOuterXml();
        }
        bool unread = false;
        QString src = img.attribute("src");
        if (src.endsWith(".gif") || src.endsWith(".png"))
            src = src.mid(0, src.length() - 4);
        QString status = src;
        if (status.endsWith("/thread")) {
            // Read thread
        } else if (status.endsWith("/thread_new")) {
            // Unread thread
            unread = true;
        } else if (status.endsWith("/thread_hot")) {
            // Hot thread
        } else if (status.endsWith("/thread_hot_lock")) {
            // Hot, closed thread
        } else if (status.endsWith("/thread_hot_new")) {
            // Hot, unread thread
            unread = true;
        } else if (status.endsWith("/thread_dot")) {
            // Own posts in this thread
        } else if (status.endsWith("/thread_dot_hot")) {
            // Hot, own posts in this thread
        } else if (status.endsWith("/thread_dot_hot_new")) {
            // Hot, own posts in this unread thread
            unread = true;
        } else {
            qDebug() << "THREAD STATUS:" << status;
        }

        // Thread icon
        td = td.nextSibling();

        // Title or sticky icon column
        bool sticky = false;
        td = td.nextSibling();
        if (!td.attribute("id").startsWith("td_threadtitle")) {
            // forum.xda-developers.com has an additional column for the sticky icon
            if (td.firstChild().tagName() == "IMG" && td.firstChild().attribute("alt") == "Sticky Thread")
                sticky = true;
            td = td.nextSibling();
        }

        // Title, Tags, Attachments
        QString title;
        QString url;
        int threadId = -1;
        QString tags;
        int attachments = 0;
        bool subscribed = false;
        // Find the first direct child <div>
        QWebElement div = td.firstChild();
        while (!div.isNull() && div.tagName() != "DIV")
            div = div.nextSibling();
        if (!div.isNull()) {
            foreach (QWebElement a, div.findAll("a")) {
                if (a.attribute("id").startsWith("thread_title_")) {
                    title = a.toPlainText();
                    if (title.isEmpty()) {
                        qDebug() << "Empty title:" << a.parent().toOuterXml();
                    }
                    url = a.attribute("href");
                    QRegExp threadIdExpression("thread_title_(\\d+)");
                    if (threadIdExpression.exactMatch(a.attribute("id"))) {
                        threadId = threadIdExpression.cap(1).toInt();
                    } else {
                        qDebug() << "Failed to parse thread ID:" << a.toOuterXml();
                    }
                }
            }

            // Sticky, tags and attachment icons are located in a floating <span>
            foreach (QWebElement span, div.findAll("span")) {
                if (span.attribute("style") == "float:right") {
                    foreach (QWebElement img, span.findAll("img.inlineimg")) {
                        if (img.attribute("src") == "silk/misc/sticky.gif") {
                            sticky = true;
                        } else if (img.attribute("src") == "silk/misc/tag.png") {
                            tags = img.attribute("alt");
                            Q_UNUSED(tags);
                        } else if (img.attribute("src") == "silk/misc/paperclip.gif") {
                            QRegExp attachmentsExpression("(\\d+) Attachment\\(s\\)");
                            if (attachmentsExpression.exactMatch(img.attribute("alt")))
                                attachments = attachmentsExpression.cap(1).toInt();
                        } else if (img.attribute("src") == "silk/misc/subscribed.gif") {
                            subscribed = true;
                        }
                    }
                }
            }
        }

        // Add stickies only on the first page
        if (sticky && page > 1)
            continue;

        // Thread starter, Ratings
        QString threadStarter;
        int votes = -1;
        float ratingValue = 0.0;
        div = div.nextSibling();
        if (div.tagName() == "DIV") {
            foreach (QWebElement span, div.findAll("span")) {
#if 0
                QRegExp userIdExpression("member.php\\?(s=\\d+\\&|)u=(\\d+)");
                if (span.attribute("style") == "cursor:pointer" && userIdExpression.indexIn(span.attribute("onclick")) != -1) {
                    QString userId = userIdExpression.cap(2);
                    Q_UNUSED(userId)
                }
#endif
                if (span.attribute("style") == "float:right") {
                    QRegExp ratingExpression("Thread Rating: ([,\\d]+) votes, ([\\.\\d]+) average.");
                    if (ratingExpression.exactMatch(span.findFirst("img.inlineimg").attribute("alt"))) {
                        votes = ratingExpression.cap(1).replace(",", "").toInt();
                        ratingValue = ratingExpression.cap(2).toFloat();
                    } else {
                        qDebug() << div.findFirst("span[style=float:right]").toOuterXml();
                    }
                }
            }

            if (posterExpression.exactMatch(div.toPlainText())) {
                threadStarter = posterExpression.cap(1);
#if 0
                QString date = posterExpression.cap(2);
                Q_UNUSED(date);
#endif
            } else {
                qDebug() << "Failed to parse poster:" << div.toOuterXml().simplified();
            }
        }

        // Last Post or Rating column
        td = td.nextSibling();
        if (!td.hasAttribute("title")) {
            // forum.xda-developers.com has an additional column for the rating
            const QWebElement span = td.firstChild();
            if (span.attribute("style") == "float:right") {
                QRegExp ratingExpression("Thread Rating: ([,\\d]+) votes, ([\\.\\d]+) average.");
                if (ratingExpression.exactMatch(span.findFirst("img.inlineimg").attribute("alt"))) {
                    votes = ratingExpression.cap(1).replace(",", "").toInt();
                    ratingValue = ratingExpression.cap(2).toFloat();
                } else {
                    qDebug() << div.findFirst("span[style=float:right]").toOuterXml();
                }
            }
            td = td.nextSibling();
        }

        // Replies, Views, Last Post
        int replies = 0;
        int views;
        QString date;
        QString time;
        QString lastPostUrl;
        QRegExp repliesExpression("Replies: ([,\\d]+), Views: ([,\\d]+)");
        if (repliesExpression.exactMatch(td.attribute("title"))) {
            replies = repliesExpression.cap(1).replace(",", "").toInt();
            views = repliesExpression.cap(2).replace(",", "").toInt();
        } else {
            qDebug() << "Failed to parse replies:" << td.toOuterXml().simplified();
        }
        div = td.firstChild();
        if (div.tagName() == "DIV") {
            QRegExp dateTimeExpression("(Today|Yesterday|\\d{2}-\\d{2}-\\d{2,4}|\\d+[tsnr][htd] [A-Z][a-z]+ \\d{4}) (\\d{2}:\\d{2}( [AP]M|))\\s*by (.*)\\s*");
            if (dateTimeExpression.exactMatch(div.toPlainText())) {
                date = DateTimeHelper::parseDate(dateTimeExpression.cap(1));
                time = DateTimeHelper::parseTime(dateTimeExpression.cap(2));
                // Skip last poster
            } else {
                qDebug() << "Failed to parse date:" << div.toPlainText();
            }
            lastPostUrl = div.findFirst("a.nound img.inlineimg").parent().attribute("href");
        }

        Q_UNUSED(views);

        td = td.nextSibling(); // Replies column
        td = td.nextSibling(); // Views column

        td = td.nextSibling(); // Forum column
        if (!td.isNull()) {
            const QWebElement a = td.firstChild();
            if (a.tagName() == "A")
                forum = a.toPlainText();
        }

        QString section = (sticky && m_stickySection) ? "Sticky" : date;
        QString dateTime;
        if (date == "Today")
            dateTime = time;
        else if (date == "Yesterday")
            dateTime = QDate::currentDate().addDays(-1).toString("dd.MM.yyyy");
        else
            dateTime = date;

        beginInsertRows(QModelIndex(), children().count(), children().count());
        Thread* thread = new Thread(url, title, replies, threadStarter, section, dateTime);
        thread->setForum(forum);
        thread->setLastPostUrl(lastPostUrl);
        thread->setUnread(unread);
        thread->setAttachments(attachments);
        thread->setSubscribed(subscribed);
        thread->setThreadId(threadId);
        if (votes > 0) {
            thread->setVotes(votes);
            thread->setRatingValue(ratingValue);
        }
        connect(thread, SIGNAL(unreadChanged()),
                this, SLOT(onThreadChanged()));
        thread->setParent(this);
        endInsertRows();
    }

    // talk.maemo.org
    threads = document.findAll("td div.DiscussionTopic");
    foreach (QWebElement thread, threads) {
        QString plainText = thread.toPlainText();
        bool sticky = plainText.startsWith("Sticky:");

        // Add stickies only on the first page
        if (sticky && page > 1)
            continue;

        bool poll = sticky ? false : plainText.startsWith("Poll:");
        QWebElement a = thread.findFirst("span.threadTitle a");
        if (a.isNull()) {
            qDebug() << "Failed to parse thread:";
            qDebug() << thread.toOuterXml();
        }
        QString url = a.attribute("href");
        bool unread = a.attribute("style") == "font-weight:bolder";
        QString title = a.toPlainText();

        // ...
        // "span.threadPages a"

        int votes = -1;
        float ratingValue = -0.0;
        foreach (QWebElement img, thread.findAll("img.inlineimg")) {
            QRegExp ratingExpression("Thread Rating: ([,\\d]+) votes, ([\\.\\d]+) average\\.\\s*");
            if (ratingExpression.exactMatch(img.attribute("alt"))) {
                votes = ratingExpression.cap(1).replace(",", "").toInt();
                ratingValue = ratingExpression.cap(2).toFloat();
            }
        }

        a = thread.parent().findFirst("div.threadbitinfo a");
        if (a.isNull()) {
            qDebug() << "Failed to parse thread:";
            qDebug() << thread.parent().toOuterXml();
        }
        QString threadStarter = a.toPlainText();
        a = a.nextSibling();
        QString firstUnreadUrl = a.attribute("href");
        if (firstUnreadUrl.replace("goto=newpost&", "") != url) {
            qDebug() << "Unknown go-to-first-unread-post URL:" << firstUnreadUrl;
        }
        int replies = a.toPlainText().replace(",", "").toInt();

        // Forum this post was posted into
        a = a.nextSibling();
        if (a.tagName() == "A")
            forum = a.toPlainText();

        a = a.nextSibling(); // Skip last poster

        QString lastPostUrl;
        a = a.nextSibling();
        if (a.tagName() == "A")
            lastPostUrl = a.attribute("href");

        plainText = a.parent().toPlainText();
        QRegExp viewsExpression("Views: ([,\\d]+)");
        int views = 0;
        if (viewsExpression.indexIn(plainText) != -1) {
            views = viewsExpression.cap(1).replace(",", "").toInt();
        }
        QRegExp dateTimeExpression("Last Post: (Today|Yesterday|\\d{2}-\\d{2}-\\d{4}|\\d*[tsnr][htd] [A-Z][a-z]+ \\d{4}) (\\d{2}\\:\\d{2}(.[AP]M|))");
        QString date;
        QString time;
        if (dateTimeExpression.indexIn(plainText) != -1) {
            date = DateTimeHelper::parseDate(dateTimeExpression.cap(1));
            time = DateTimeHelper::parseTime(dateTimeExpression.cap(2));
        }

        Q_UNUSED(poll);
        Q_UNUSED(views);

        QString section = (sticky && m_stickySection) ? "Sticky" : date;
        QString dateTime;
        if (date == "Today")
            dateTime = time;
        else if (date == "Yesterday")
            dateTime = QDate::currentDate().addDays(-1).toString("dd.MM.yyyy");
        else
            dateTime = date;

        beginInsertRows(QModelIndex(), children().count(), children().count());
        Thread* thread = new Thread(url, title, replies, threadStarter, section, dateTime);
        thread->setForum(forum);
        thread->setLastPostUrl(lastPostUrl);
        thread->setUnread(unread);
        if (votes > 0) {
            thread->setVotes(votes);
            thread->setRatingValue(ratingValue);
        }
        connect(thread, SIGNAL(unreadChanged()),
                this, SLOT(onThreadChanged()));
        thread->setParent(this);
        endInsertRows();
    }
    emit countChanged();
/*
    if (children().count() == 0) {
        // talk.maemo.org
        QWebElement div = document.findFirst("table.tborder tr td div.panel div");
        if (!div.isNull()) {
            qDebug() << "Error message:" << div.toPlainText();
        } else {
            qDebug() << "Unknown error. Dumping page:";
            qDebug() << document.toOuterXml();
        }
    }
*/
}

void ThreadList::onThreadChanged()
{
    Thread* thread = qobject_cast<Thread*>(sender());
    if (!thread)
        return;

    int row = children().indexOf(thread);
    if (row >= 0)
        dataChanged(index(row, 0), index(row, 0));
}

void ThreadList::parseActiveTopics(const QWebElementCollection& topics)
{
    clear();

    beginInsertRows(QModelIndex(), children().count(), children().count() + topics.count() - 1);
    foreach (QWebElement topic, topics) {
        // Thread URL and title
        QString url = topic.attribute("href");
        QString title;
        int replies = 0;
        QRegExp titleExpression("(.+) \\(([,\\d]+)\\)");
        if (titleExpression.exactMatch(topic.toPlainText())) {
            title = titleExpression.cap(1);
            replies = titleExpression.cap(2).replace(",", "").toInt();
        }

        // Last poster and time of last post
        QString lastPoster;
        QString time;
        QRegExp lastPostExpression("by ([^(]+) - (\\d+ .+ ago)$");
        lastPostExpression.setMinimal(true);
        if (lastPostExpression.indexIn(topic.parent().parent().toPlainText())) {
            lastPoster = lastPostExpression.cap(1);
            time = lastPostExpression.cap(2);
        }
        QString section = time;
        QRegExp timeExpression("(\\d+) (mins?) ago");
        if (timeExpression.exactMatch(time)) {
            if (timeExpression.cap(2).startsWith("min")) {
                int minutes = timeExpression.cap(1).toInt();
                if (minutes < 10)
                    section = "Last 10 minutes";
                else if (minutes < 30)
                    section = "Last 30 minutes";
                else if (minutes < 60)
                    section = "Last hour";
            }
        }
        QString dateTime = time;

        Thread* thread = new Thread(url, title, replies, lastPoster, section, dateTime);
        thread->setUnread(false);

        // Forum this post was posted into
        QWebElement a = topic.parent().nextSibling();
        if (a.tagName() == "A") {
            QString forumUrl = a.attribute("href");
            Q_UNUSED(forumUrl);
            thread->setForum(a.toPlainText());
        }

        thread->setParent(this);
    }
    endInsertRows();
    emit countChanged();
}

void ThreadList::clear(void) {
    // If there is nothing to remove, endRemoveRows() will crash in QDeclarativeListView::itemsRemoved(int, int)
    if (children().count() == 0)
        return;
    beginRemoveRows(QModelIndex(), 0, children().count());
    while (!children().empty()) {
        Thread* thread = qobject_cast<Thread*>(children().at(0));
        if (!thread)
            continue;
        if (thread->taken()) {
            // Parent to forum session, so the thread is kept around
            // even when the QML context gets destroyed temporarily
            thread->setParent(m_session);
        } else {
            thread->setParent(0);
            thread->deleteLater();
        }
    }
    endRemoveRows();
    emit countChanged();
}

QObject* ThreadList::get(int index) const
{
    if (index < 0 || index > children().count())
        return 0;

    return children().at(index);
}

void ThreadList::load(int page)
{
    QString url = m_url + QString("&page=%1").arg(page);
    qDebug() << "Requesting" << url << "for thread list ...";

    QObject::connect(m_session, SIGNAL(receivedThreadList(QWebElement)),
                     this, SLOT(onReceived(QWebElement)));

    m_session->get(QUrl(url));
}

ForumSession* ThreadList::forumSession() const
{
    return m_session;
}
