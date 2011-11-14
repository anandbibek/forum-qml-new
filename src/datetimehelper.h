#ifndef DATETIMEHELPER_H
#define DATETIMEHELPER_H

#include <QString>

class DateTimeHelper
{
public:
    DateTimeHelper();

    static QString parseDateTime(const QString dateTime);
    static QString parseDate(QString date);
    static QString parseTime(QString time);
};

#endif // DATETIMEHELPER_H
