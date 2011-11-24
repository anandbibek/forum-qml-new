#include <QDesktopServices>
#include <QSettings>

#include "forum.h"
#include "forumlist.h"
#include "forumlistcache.h"
#include "forumsession.h"

ForumListCache::ForumListCache(ForumList* forumList, QObject* parent) :
    QObject(parent),
    m_forumList(forumList)
{
    // Load cached forum list
    QSettings settings(QDesktopServices::storageLocation(QDesktopServices::CacheLocation) + "/" +
                       QCoreApplication::applicationName() + "/forums.conf", QSettings::IniFormat);
    int count = settings.beginReadArray("Forums");
    QList<Forum*> list;
    for (int i = 0; i < count; ++i) {
        settings.setArrayIndex(i);
        int forumId = settings.value("forumId").toInt();
        QString subtitle = settings.value("subtitle").toString();
        QString title = settings.value("title").toString();
        QString url = "";
        Forum* forum = new Forum(url, forumId, title, subtitle);
        if (settings.value("section").isValid())
            forum->setSection(settings.value("section").toString());
        list.append(forum);
    }
    settings.endArray();

    // Add entries to the forum list model
    m_forumList->addForums(list);

    connect(m_forumList, SIGNAL(countChanged()),
            this, SLOT(onForumListChanged()));
}

void ForumListCache::onForumListChanged()
{
    QSettings settings(QDesktopServices::storageLocation(QDesktopServices::CacheLocation) + "/" +
                       QCoreApplication::applicationName() + "/forums.conf", QSettings::IniFormat);

    // TODO: compare cached list with current list, only write if changes were made

    settings.beginWriteArray("Forums", m_forumList->count());
    int i = 0;
    foreach(QObject* child, m_forumList->children()) {
        Forum* forum = qobject_cast<Forum*>(child);
        if (forum) {
            settings.setArrayIndex(i++);
            settings.setValue("forumId", QVariant(forum->forumId()));
            settings.setValue("title", QVariant(forum->title()));
            settings.setValue("subtitle", QVariant(forum->subtitle()));
            if (!forum->section().isEmpty())
                settings.setValue("section", QVariant(forum->section()));
        }
    }
    settings.endArray();
}
