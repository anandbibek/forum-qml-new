#include "searchresultthreadlist.h"

SearchResultThreadList::SearchResultThreadList(ForumSession* session, QObject* parent) :
    ThreadList(session, parent),
    m_searchId(-1)
{
}

void SearchResultThreadList::onReceived(QWebElement document, int searchId)
{
    qDebug() << "Received search result" << searchId;

    if (searchId > 0) {
        m_searchId = searchId;
        m_url = QString("search.php?searchid=%1").arg(searchId);

        m_stickySection = false;
        ThreadList::onReceived(document);
    } else {
        // talk.maemo.org
        QWebElement li = document.findFirst("table.tborder > tbody > tr > td.alt1 > ol > li");
        if (!li.isNull() &&
            li.parent().parent().parent().previousSibling().firstChild().toPlainText() ==
            "The following errors occurred with your search:") {
            if (li.toPlainText().startsWith("Sorry - no matches.")) {
                // Not an error
            } else {
                qDebug() << "Error:" << li.toPlainText();
            }
        } else {
            qDebug() << document.toOuterXml();
            qDebug() << "==========================================";
            qDebug() << "Unknown error. Dumped page.";
        }
    }
}
