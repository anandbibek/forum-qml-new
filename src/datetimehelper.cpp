#include <QDate>
#include <QDebug>
#include <QRegExp>

#include "datetimehelper.h"

DateTimeHelper::DateTimeHelper()
{
}

QString DateTimeHelper::parseDateTime(const QString dateTime)
{
    static QRegExp dateTimeExpression("\\s*(Today|Yesterday|\\d{2}-\\d{2}-\\d{2,4}|\\d+[tsnr][htd] [A-Z][a-z]+ \\d{4}) ?, (\\d{2}:\\d{2}(.[AP]M|))");
    if (dateTimeExpression.exactMatch(dateTime)) {
        QString date = DateTimeHelper::parseDate(dateTimeExpression.cap(1));
        QString time = DateTimeHelper::parseTime(dateTimeExpression.cap(2));
        if (date == "Today")
            return time;
        else
            return date + " | " + time;
    }

    qDebug() << "Failed to parse date / time string:" << dateTime;
    return dateTime;
}

QString DateTimeHelper::parseDate(const QString date)
{
    if (date == "Today" || date == "Yesterday") {
        return date;
    } else {
        static QRegExp dateExpression("(\\d+)[tsnr][htd] ([A-Z][a-z]+) (\\d{4})");
        if (dateExpression.exactMatch(date)) {
            int day = dateExpression.cap(1).toInt();
            QString month = dateExpression.cap(2);
            int m;
            if (month == "Jan" || month == "January")
                m = 1;
            else if (month == "Feb" || month == "February")
                m = 2;
            else if (month == "Mar" || month == "March")
                m = 3;
            else if (month == "Apr" || month == "April")
                m = 4;
            else if (month == "May")
                m = 5;
            else if (month == "Jun" || month == "June")
                m = 6;
            else if (month == "Jul" || month == "July")
                m = 7;
            else if (month == "Aug" || month == "August")
                m = 8;
            else if (month == "Sep" || month == "September")
                m = 9;
            else if (month == "Oct" || month == "October")
                m = 10;
            else if (month == "Nov" || month == "November")
                m = 11;
            else if (month == "Dec" || month == "December")
                m = 12;
            else {
                qDebug() << "Failed to parse date string:" << date;
                return date;
            }
            int year = dateExpression.cap(3).toInt();
            return QDate(year, m, day).toString("dd.MM.yy");
        }
        static QRegExp euDateExpression("(\\d\\d)-(\\d\\d)-(\\d\\d)");
        if (euDateExpression.exactMatch(date)) {
            int month = euDateExpression.cap(1).toInt();
            int day = euDateExpression.cap(2).toInt();
            int year = 2000 + euDateExpression.cap(3).toInt();
            return QDate(year, month, day).toString("dd.MM.yy");
        }
        static QRegExp usDateExpression("(\\d\\d)-(\\d\\d)-(\\d{4})");
        if (usDateExpression.exactMatch(date)) {
            int month = usDateExpression.cap(1).toInt();
            int day = usDateExpression.cap(2).toInt();
            int year = usDateExpression.cap(3).toInt();
            return QDate(year, month, day).toString("dd.MM.yy");
        }
    }

    qDebug() << "2>Failed to parse date string:" << date;
    return date;
}

QString DateTimeHelper::parseTime(const QString time)
{
    static QRegExp timeExpression24h("\\d{2}:\\d{2}");
    if (timeExpression24h.exactMatch(time))
        return time;
    static QRegExp timeExpression12h("(\\d{2})(:\\d{2}) ([AP])M");
    if (timeExpression12h.exactMatch(time)) {
        QTime t = QTime::fromString(time, "hh:mm AP");
        return t.toString("hh:mm");
    }
    qDebug() << "3>Failed to parse time string:" << time;
    return time;
}
