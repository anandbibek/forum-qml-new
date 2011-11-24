#ifndef FORUMLISTCACHE_H
#define FORUMLISTCACHE_H

#include <QObject>

class ForumList;

class ForumListCache : public QObject
{
    Q_OBJECT
public:
    explicit ForumListCache(ForumList* forumList, QObject* parent = 0);

signals:

public slots:
    void onForumListChanged(void);

private:
    ForumList* m_forumList;
};

#endif // FORUMLISTCACHE_H
