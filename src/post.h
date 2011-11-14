#ifndef POST_H
#define POST_H

#include <QObject>
#include <QtWebKit>

class Post : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString body READ body WRITE setBody NOTIFY bodyChanged)
    Q_PROPERTY(QString dateTime READ dateTime)
    Q_PROPERTY(QString poster READ poster)
    Q_PROPERTY(int postId READ postId WRITE setPostId NOTIFY postIdChanged)
    Q_PROPERTY(QString section READ section WRITE setSection)
    Q_PROPERTY(QString subject READ subject WRITE setSubject)
    Q_PROPERTY(QString url READ url)
public:
    explicit Post(QObject* parent = 0);
    explicit Post(const QString url, const QString poster, const QString dateTime, const QString body, QObject* parent = 0);

    QString body() const;
    QString dateTime() const;
    QString poster() const;
    int postId() const;
    QString section() const;
    QString subject() const;
    QString url() const;

    void setBody(const QString body);
    void setPostId(int postId);
    void setSection(const QString section);
    void setSubject(const QString subject);

    static QString cleanupBody(QWebElement& body);
    Q_INVOKABLE QString toBbCode() const;

signals:
    void bodyChanged(void);
    void postIdChanged(void);

public slots:

private:
    QString m_body;
    QString m_dateTime;
    QString m_poster;
    int m_postId;
    QString m_section;
    QString m_subject;
    QString m_url;
};

#endif // POST_H
