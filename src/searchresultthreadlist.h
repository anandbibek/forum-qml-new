#ifndef SEARCHRESULTTHREADLIST_H
#define SEARCHRESULTTHREADLIST_H

#include <QtWebKit>

#include "threadlist.h"

class SearchResultThreadList : public ThreadList
{
    Q_OBJECT
public:
    explicit SearchResultThreadList(ForumSession* forumSession, QObject* parent = 0);

public slots:
    void onReceived(QWebElement document, int searchId);

private:
    int m_searchId;
};

#endif // SEARCHRESULTTHREADLIST_H
