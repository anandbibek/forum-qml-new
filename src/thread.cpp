#include "forumsession.h"
#include "postlist.h"
#include "thread.h"
#include "threadlist.h"

Thread::Thread(const QString url, const QString title, int replies, const QString poster, const QString section, const QString dateTime, QObject *parent) :
    QObject(parent),
    m_attachments(0),
    m_dateTime(dateTime),
    m_model(0),
    m_openMode(FirstUnreadPost),
    m_poster(poster),
    m_ratingValue(0.0),
    m_replies(replies),
    m_section(section),
    m_subscribed(false),
    m_threadId(-1),
    m_title(title),
    m_unread(true),
    m_url(url),
    m_votes(-1)
{
}

Thread::Thread(const Thread& other, QObject *parent) :
    QObject(parent),
    m_attachments(other.m_attachments),
    m_dateTime(other.m_dateTime),
    m_forum(other.m_forum),
    m_lastPostUrl(other.m_lastPostUrl),
    m_model(other.m_model),
    m_openMode(FirstUnreadPost),
    m_poster(other.m_poster),
    m_ratingValue(other.m_ratingValue),
    m_replies(other.m_replies),
    m_section(other.m_section),
    m_subscribed(false),
    m_threadId(other.m_threadId),
    m_title(other.m_title),
    m_unread(other.m_unread),
    m_url(other.m_url),
    m_votes(other.m_votes)
{
}

QObject* Thread::model()
{
    // Create the post list model when it is first requested
    if (!m_model) {
        ThreadList* threadList = qobject_cast<ThreadList*>(parent());
        if (threadList) {
            m_model = new PostList(threadList->forumSession(), this);
            if (m_openMode == FirstPost)
                m_model->setUrl(m_url);
            else if (m_openMode == FirstUnreadPost)
                m_model->setUrl(m_url + "&goto=newpost");
            else if (m_openMode == LastPost)
                m_model->setUrl(m_lastPostUrl);
        }
    }

    return m_model;
}

int Thread::attachments() const
{
    return m_attachments;
}

QString Thread::dateTime() const
{
    return m_dateTime;
}

QString Thread::forum() const
{
    return m_forum;
}

QString Thread::lastPostUrl() const
{
    return m_lastPostUrl;
}

Thread::OpenMode Thread::openMode() const
{
    return m_openMode;
}

QString Thread::poster() const
{
    return m_poster;
}

float Thread::ratingValue() const
{
    return m_ratingValue;
}

int Thread::replies() const
{
    return m_replies;
}

QString Thread::section() const
{
    return m_section;
}

bool Thread::subscribed() const
{
    return m_subscribed;
}

QString Thread::title() const
{
    return m_title;
}

int Thread::threadId() const
{
    return m_threadId;
}

bool Thread::unread() const
{
    return m_unread;
}

QString Thread::url() const
{
    return m_url;
}

int Thread::votes() const
{
    return m_votes;
}

void Thread::setAttachments(int attachments)
{
    if (m_attachments != attachments) {
        m_attachments = attachments;
        emit attachmentsChanged();
    }
}

void Thread::setForum(const QString forum)
{
    m_forum = forum;
}

void Thread::setLastPostUrl(const QString lastPostUrl)
{
    m_lastPostUrl = lastPostUrl;
}

void Thread::setOpenMode(OpenMode openMode)
{
    if (m_openMode != openMode)
    {
        m_openMode = openMode;
        if (m_model) {
            if (m_openMode == FirstPost)
                m_model->setUrl(m_url);
            else if (m_openMode == FirstUnreadPost)
                m_model->setUrl(m_url + "&goto=newpost");
            else if (m_openMode == LastPost)
                m_model->setUrl(m_lastPostUrl);
        }
    }
}

void Thread::setRatingValue(float ratingValue)
{
    if (m_ratingValue != ratingValue) {
        m_ratingValue = ratingValue;
        emit ratingValueChanged();
    }
}

void Thread::setSubscribed(bool subscribed)
{
    if (m_subscribed != subscribed) {
        m_subscribed = subscribed;
        emit subscribedChanged();
    }
}

void Thread::setThreadId(int threadId)
{
    m_threadId = threadId;
}

void Thread::setUnread(bool unread)
{
    if (m_unread != unread) {
        m_unread = unread;
        emit unreadChanged();
    }
}

void Thread::setUrl(QString url)
{
    m_url = url;
}

void Thread::setVotes(int votes)
{
    if (m_votes != votes) {
        m_votes = votes;
        emit votesChanged();
    }
}

void Thread::release()
{
    if (m_taken)
        m_taken = false;

    // If this thread is not parented to a thread list, delete it
    ThreadList* threadList = qobject_cast<ThreadList*>(parent());
    if (!threadList)
        deleteLater();
}

void Thread::take()
{
    m_taken = true;
}

bool Thread::taken()
{
    return m_taken;
}

void Thread::onSubscriptionChanged(int threadId, bool subscribed)
{
    if (m_threadId == threadId)
        setSubscribed(subscribed);
}
