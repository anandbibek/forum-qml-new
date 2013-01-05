#ifndef POST_H
#define POST_H

#include <QObject>
#include <QtWebKit>

class Post : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString body READ body WRITE setBody NOTIFY bodyChanged)
    Q_PROPERTY(QString img READ img)
    Q_PROPERTY(QString stat READ stat)
    Q_PROPERTY(QString dateTime READ dateTime)
    Q_PROPERTY(QString poster READ poster)
    Q_PROPERTY(int postId READ postId WRITE setPostId NOTIFY postIdChanged)
    Q_PROPERTY(QString section READ section WRITE setSection)
    Q_PROPERTY(QString subject READ subject WRITE setSubject)
    Q_PROPERTY(QString thanks READ thanks WRITE setThanks NOTIFY thanksChanged)
    Q_PROPERTY(QString url READ url)
public:
    explicit Post(QObject* parent = 0);
    explicit Post(const QString url, const QString poster, const QString dateTime, const QString body, const QString img, const QString stat, QObject* parent = 0);

    QString body() const;
    QString img() const;
    QString stat() const;
    QString dateTime() const;
    QString poster() const;
    int postId() const;
    QString section() const;
    QString subject() const;
    QString thanks() const;
    QString url() const;

    void setBody(const QString body);
    void setPostId(int postId);
    void setSection(const QString section);
    void setSubject(const QString subject);
    void setThanks(const QString thanks);

    static QString cleanupBody(QWebElement& body);
    Q_INVOKABLE void release();
    Q_INVOKABLE void take();
    bool taken();
    Q_INVOKABLE bool thankedBy(const QString userName) const;
    Q_INVOKABLE QString toBbCode() const;

signals:
    void bodyChanged(void);
    void postIdChanged(void);
    void thanksChanged(void);

public slots:
    void onThanksReceived(QString postId, QStringList thanks);

private:
    QString m_body;
    QString m_img;
    QString m_stat;
    QString m_dateTime;
    QString m_poster;
    int m_postId;
    QString m_section;
    QString m_subject;
    QString m_thanks;
    QString m_url;

    bool m_taken;
};

#endif // POST_H
