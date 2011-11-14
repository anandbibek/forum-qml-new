#include <QDebug>

#include "forumlist.h"
#include "forumsession.h"

ForumList::ForumList(ForumSession* session, QObject *parent) :
    QAbstractListModel(parent),
    m_session(session)
{
    QHash<int, QByteArray> roles;
    roles[UrlRole] = "url";
    roles[TitleRole] = "title";
    roles[ViewingRole] = "viewing";
    roles[SubtitleRole] = "subtitle";
    roles[SectionRole] = "section";
    roles[NumThreadsRole] = "numThreads";
    setRoleNames(roles);

    QObject::connect(m_session, SIGNAL(receivedForumList(QWebElement)),
                     this, SLOT(onReceived(QWebElement)));
}

QVariant ForumList::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    Forum* forum = qobject_cast<Forum*>(children().at(index.row()));
    if (role == UrlRole)
        return forum->url();
    else if (role == TitleRole)
        return forum->title();
    else if (role == ViewingRole)
        return forum->viewing();
    else if (role == SubtitleRole)
        return forum->subtitle();
    else if (role == SectionRole)
        return forum->section();
    else if (role == NumThreadsRole)
        return forum->numThreads();

    return QVariant();
}

int ForumList::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)

    return children().count();
}

void ForumList::clear(void)
{
    beginRemoveRows(QModelIndex(), 0, children().count());
    while (!children().empty()) {
        QObject* forum = children().at(0);
        forum->setParent(0);
        forum->deleteLater();
    }
    endRemoveRows();
}

static Forum* parseForum(QWebElement& a)
    {
    QString url = a.attribute("href");
    int forumId;
    static QRegExp urlExpression("forumdisplay.php\\?(s=[a-f\\d]+&|)f=(\\d+)");
    if (urlExpression.exactMatch(url)) {
        forumId = urlExpression.cap(2).toInt();
    } else {
        qDebug() << "Failed to parse forum URL:" << a.parent().toInnerXml().simplified();
        forumId = -1;
    }
    QString title = a.toPlainText();
    QString subtitle;
    QWebElement tag = a.parent().nextSibling();
    if (tag.tagName() == "P" || tag.tagName() == "DIV") {
        subtitle = tag.toPlainText();
    } else {
        qDebug() << "Failed to parse forum subtitle:" << a.parent().toInnerXml().simplified();
    }
    // Create the forum object, further information is optional
    Forum *forum = new Forum(url, forumId, title, subtitle);

    // Current number of viewers
    QWebElement span = a.nextSibling();
    QRegExp viewingExpression("\\((\\d+) Viewing\\)");
    if (viewingExpression.exactMatch(span.toPlainText()))
        forum->setViewing(viewingExpression.cap(1).toInt());

    return forum;
}

void ForumList::onReceived(const QWebElement& document)
{
    qDebug() << "Received forum list";

    clear();

    QString categoryTags;
    QString categoryTitleTag;
    QString skipCategory;
    QString forumListTag;
    if (m_session->url() == "http://talk.maemo.org") {
        categoryTags = "div.forum_category";
        categoryTitleTag = "h1 > a";
        skipCategory = "Old";
        forumListTag = "div.main_forum";
    }
    if (m_session->url() == "http://forum.xda-developers.com") {
        categoryTags = "div.forumbox";
        categoryTitleTag = "h2 > a";
        skipCategory = "What's Going On?";
        forumListTag = "ol.forumlist";
    }

    QList<Forum*> list;

    // talk.maemo.org, forum.xda-developer.com
    if (!categoryTags.isEmpty()) {
        QWebElementCollection categories = document.findAll(categoryTags);
        foreach (QWebElement category, categories) {
            QWebElement a = category.findFirst(categoryTitleTag);
            if (a.isNull()) {
                qDebug() << "Failed to parse forum category:";
                qDebug() << category.toOuterXml();
                continue;
            }
            QString section = a.toPlainText();

            // Skip the archived sections of talk.maemo.org
            if (section == skipCategory)
                break;

            QWebElement div = category.findFirst(forumListTag);
            QWebElementCollection forums = div.findAll("h3 > a");
            foreach (QWebElement a, forums) {
                // URL, title, subtitle, and current number of viewers
                Forum* forum = parseForum(a);
#if 0
                // Last post information
                QWebElement div = a.parent().nextSibling().nextSibling();
                if (div.tagName() == "DIV" && div.classes().contains("last_reply")) {
                    QWebElement a = div.firstChild();
                    if (a.tagName() == "A") {
                        QString postTitle = a.toPlainText();
                        QString firstUnreadUrl = a.attribute("href"); // first unread
                    }
                    a = a.nextSibling();
                    if (a.tagName() == "A")
                        QString poster = a.toPlainText();
                    a = a.nextSibling();
                    if (a.tagName() == "A")
                        QString lastPosturl = a.attribute("href"); // last post
                    QRegExp dateTimeExpression("@ ([0-9]+[snrt][tdh] [A-Z][a-z][a-z] \\d{4}) (\\d{2}:\\d{2})");
                    if (dateTimeExpression.indexIn(div.toPlainText()) != -1) {
                        QString date = dateTimeExpression.cap(1);
                        QString time = dateTimeExpression.cap(2);
                    }
                }
#endif
                // Forum statistics
                div = div.nextSibling();
                if (div.tagName() == "DIV" && div.classes().contains("forum_stats")) {
                    QRegExp statsExpression("Threads: ([,\\d]+) \\| Posts: ([,\\d]+)");
                    if (statsExpression.exactMatch(div.toPlainText())) {
                        forum->setNumThreads(statsExpression.cap(1).replace(",", "").toInt());
                        forum->setNumPosts(statsExpression.cap(2).replace(",", "").toInt());
                    }
                }

                forum->setSection(section);

                list.append(forum);
            }
        }
    }
    //<div id="info-footer-time"><small>All times are GMT +1. The time now is <span class="time">06:37 PM</span>.</small></div>

    // forum.meego.com
    if (categoryTags.isEmpty()) {
        QWebElementCollection forums = document.findAll("table.tborder > tbody > tr > td > table > tbody > tr > td > div > a");
        foreach (QWebElement a, forums) {
            // URL, title, subtitle, and current number of viewers
            Forum* forum = parseForum(a);
#if 0
            QWebElement img = a.parent().parent().parent().firstChild().firstChild();
             if (img.tagName() == "IMG")
                if (img.attribute("src") == "silk/statusicon/forum_old.gif")
                    forum->setUnread(false);
#endif
            list.append(forum);
        }
    }

    // Add entries to the forum list model
    if (list.count() > 0) {
        beginInsertRows(QModelIndex(), 0, list.count() - 1);
        while (!list.empty())
            list.takeFirst()->setParent(this);
        endInsertRows();
        emit countChanged();
    }
}

void ForumList::load(void)
{
    m_session->get(QUrl(m_session->url()));
}

int ForumList::count() const
{
    return children().count();
}

QString ForumList::url() const
{
    return m_url;
}

void ForumList::setUrl(QString uri)
{
    m_url = uri;
}

QObject* ForumList::get(int index) const
{
    if (index < 0 || index > children().count())
        return 0;

    return children().at(index);
}

ForumSession* ForumList::forumSession() const
{
    return m_session;
}
