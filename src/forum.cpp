#include "forum.h"
#include "forumlist.h"
#include "forumsession.h"
#include "threadlist.h"

Forum::Forum(const QString url, int forumId, const QString title, const QString subtitle, QObject *parent) :
    QObject(parent),
    m_forumId(forumId),
    m_model(0),
    m_numPosts(0),
    m_numThreads(0),
    m_section(""),
    m_subtitle(subtitle),
    m_title(title),
    m_unread(true),
    m_url(url),
    m_viewing(-1)
{
}

int Forum::forumId() const
{
    return m_forumId;
}

QObject* Forum::model()
{
    // Create the thread list model when it is first requested
    if (!m_model) {
        ForumList* forumList = qobject_cast<ForumList*>(parent());
        m_model = new ThreadList(forumList ? forumList->forumSession() : 0, this);
        m_model->setUrl(m_url);
    }

    return m_model;
}

int Forum::numPosts() const
{
    return m_numPosts;
}

int Forum::numThreads() const
{
    return m_numThreads;
}

QString Forum::section() const
{
    return m_section;
}

QString Forum::subtitle() const
{
    return m_subtitle;
}

QString Forum::title() const
{
    return m_title;
}

bool Forum::unread() const
{
    return m_unread;
}

QString Forum::url() const
{
    return m_url;
}

int Forum::viewing() const
{
    return m_viewing;
}

void Forum::setNumPosts(int numPosts)
{
    m_numPosts = numPosts;
}

void Forum::setNumThreads(int numThreads)
{
    m_numThreads = numThreads;
}

void Forum::setSection(const QString section)
{
    m_section = section;
}

void Forum::setUnread(bool unread)
{
    if (m_unread != unread) {
        m_unread = unread;
        emit unreadChanged();
    }
}

void Forum::setViewing(int viewing)
{
    m_viewing = viewing;
}

void Forum::markRead(void)
{
    // MeeGo Forum: YAHOO.util.Connect.asyncRequest("POST","ajax.php?do=markread&f="+this.forumid

    ForumList* forumList = qobject_cast<ForumList*>(parent());
    if (forumList)
        forumList->forumSession()->markRead(this);

    setUnread(false);
    if (m_model)
        m_model->markAllRead();
}
