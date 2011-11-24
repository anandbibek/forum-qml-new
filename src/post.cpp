#include <QtWebKit>

#include "post.h"

Post::Post(QObject* parent) :
    QObject(parent),
    m_postId(-1)
{
}

Post::Post(const QString url, const QString poster, const QString dateTime, const QString body, QObject *parent) :
    QObject(parent),
    m_body(body),
    m_dateTime(dateTime),
    m_poster(poster),
    m_postId(-1),
    m_url(url)
{
}

QString Post::body() const
{
    return m_body;
}

QString Post::dateTime() const {
    return m_dateTime;
}

QString Post::poster() const
{
    return m_poster;
}

int Post::postId() const
{
    return m_postId;
}

QString Post::section() const
{
    return m_section;
}

QString Post::subject() const
{
    return m_subject;
}

QString Post::thanks() const
{
    return m_thanks;
}

QString Post::url() const
{
    return m_url;
}

void Post::setBody(const QString body)
{
    if (m_body != body) {
        m_body = body;
        emit bodyChanged();
    }
}

void Post::setPostId(int postId)
{
    if (m_postId != postId) {
        m_postId = postId;
        emit postIdChanged();
    }
}

void Post::setSection(const QString section)
{
    m_section = section;
}

void Post::setSubject(const QString subject)
{
    m_subject = subject;
}

void Post::setThanks(const QString thanks)
{
    if (m_thanks != thanks) {
        m_thanks = thanks;
        emit thanksChanged();
    }
}

static QString cleanupWhitespace(QWebElement& body)
{
    if (body.findFirst("pre").isNull()) {
        return body.toInnerXml().simplified();
    } else {
        // TODO: Remove excess whitespace outside of <pre> tags
        return body.toInnerXml();
    }
}

static void replaceSmiley(QWebElement& img)
{
    // talk.maemo.org, forum.meego.com
    QRegExp smileyExpression("images/smilies/(.*).gif");
    if (smileyExpression.exactMatch(img.attribute("src"))) {
        QString type = smileyExpression.cap(1);
        QString icon;

        if (type == "smile") // :)
            icon = "happy";
        else if (type == "frown") // :(
            icon = "sad";
        else if (type == "confused") // :confused:
            icon = "worried";
        else if (type == "mad") // :mad:
            icon = "angry";
        else if (type == "tongue") // :p
            icon = "tongue";
        else if (type == "wink") // ;)
            icon = "wink";
        else if (type == "biggrin") // :D
            icon = "very-happy";
        else if (type == "redface") // :o
            icon = "worried";
        else if (type == "rolleyes") // :rolleyes:
            icon = "sarcastic";
        else if (type == "cool") // :cool:
            icon = "cool";
        else if (type == "eek") // :eek:
            icon = "surprised";
        else {
            qDebug() << type;
            icon = "";
        }
        img.setAttribute("src", "image://theme/icon-s-messaging-smiley-" + icon);
    }
}

QString Post::cleanupBody(QWebElement& body)
{
    // talk.maemo.org
    QWebElementCollection quotes = body.findAll("div.quote");
    foreach (QWebElement quote, quotes) {
        // FIXME - Just remove the image that links to the original post for now
        quote.findFirst("a > img").parent().removeFromDocument();
#if 0
        quote.takeFromDocument();
        QString poster = quote.findFirst("cite > strong").toPlainText();
        QString url = quote.findFirst("cite > a").attribute("href");
        QRegExp whitespaceExpression("\\s+");
        qDebug() << "Removed quote by" << poster << "@" << url << ":" << quote.findFirst("blockquote").toPlainText().replace(whitespaceExpression, " ");
#endif
    }

    // forum.meego.com
    foreach (QWebElement div, body.findAll("div")) {
        if (div.firstChild().toPlainText() == "Quote:") {
            div.firstChild().removeFromDocument();
            // FIXME - div's second child is a table - fix that
            // Just Remove the image that links to the original post for now
            div.findFirst("a > img").parent().removeFromDocument();
        }
    }

    // talk.maemo.org
    QWebElement signature = body.findFirst("div.signature").takeFromDocument();
    Q_UNUSED(signature);

    // talk.maemo.org
    QWebElement lastEdit = body.findFirst("div > hr + em").parent().takeFromDocument();
    Q_UNUSED(lastEdit);

    QWebElementCollection anchors = body.findAll("a");
    foreach (QWebElement a, anchors) {
        // Only keep the "href" attribute
        foreach (QString attribute, a.attributeNames()) {
            if (attribute != "href")
                a.removeAttribute(attribute);
        }
    }

    // Replace all <strike> tags with <s>
    QWebElementCollection strikes = body.findAll("strike");
    foreach (QWebElement strike, strikes)
        strike.replace("<s>" + strike.toInnerXml() + "</s>");

    // Clean up images
    QWebElementCollection images = body.findAll("img"); // .inlineimg
    foreach (QWebElement img, images) {
        // Only keep the "src" attribute
        foreach (QString attribute, img.attributeNames()) {
            if (attribute != "src")
                img.removeAttribute(attribute);
        }

        // Replace smileys with local icons
        replaceSmiley(img);

#if 0   // FIXME - QML Label doesn't support max-width style
        // Restrict width to fit in portrait mode
        if (img.attribute("src").startsWith("http://")) {
            img.setAttribute("style", "max-width:448px");
        }
#endif
    }

    // Clean up code tags
    QWebElementCollection codeTags = body.findAll("div > div + pre.panel");
    foreach (QWebElement pre, codeTags) {
        // Remove all attributes from <div><div /><pre class="panel" /></div>
        foreach (QString attribute, pre.attributeNames()) {
            pre.removeAttribute(attribute);
        }
        QWebElement div = pre.previousSibling();
        foreach (QString attribute, div.attributeNames()) {
            div.removeAttribute(attribute);
        }
        div = pre.parent();
        foreach (QString attribute, div.attributeNames()) {
            div.removeAttribute(attribute);
        }
        div.setAttribute("style", "margin-left:16px");
        pre.setAttribute("style", "font-size:13px;font-family:monospace");
    }

    // TODO: Parse "ul.midcom_toolbar" for reply link and thanks

    return cleanupWhitespace(body);
}

static QString innerXmlToBbCode(QWebElement element)
{
    static const QRegExp postExpression("showthread.php?p=(\\d+)#post(\\d+)");

 // qDebug() << element.tagName() << ":" << element.toOuterXml().simplified().replace("<br>", "<br>\n");

    QWebElement next;
    for (QWebElement tag = element.firstChild(); !tag.isNull(); tag = next) {
        next = tag.nextSibling();

        if (tag.tagName() == "B")
            tag.replace("[b]" + innerXmlToBbCode(tag) + "[/b]");
        else if (tag.tagName() == "I")
            tag.replace("[i]" + innerXmlToBbCode(tag) + "[/i]");
        else if (tag.tagName() == "U")
            tag.replace("[u]" + innerXmlToBbCode(tag) + "[/u]");
        else if (tag.tagName() == "FONT") {
            if (tag.hasAttribute("color"))
                tag.replace("[color=" + tag.attribute("color") + "]" + innerXmlToBbCode(tag) + "[/color]");
            else if (tag.hasAttribute("size"))
                tag.replace("[size=" + tag.attribute("size") + "]" + innerXmlToBbCode(tag) + "[/size]");
            else if (tag.hasAttribute("face"))
                tag.replace("[font=" + tag.attribute("face") + "]" + innerXmlToBbCode(tag) + "[/font]");
        } else if (tag.tagName() == "SPAN") {
            if (tag.hasClass("highlight"))
                tag.replace("[highlight]" + innerXmlToBbCode(tag) + "[/highlight]");
        } else if (tag.tagName() == "DIV") {
            if (tag.hasAttribute("align")) {
                const QString align = tag.attribute("align");
                if (align == "left" || align == "center" || align == "right")
                    tag.replace("[" + align + "]" + innerXmlToBbCode(tag) + "[/" + align + "]");
            }
            // talk.maemo.org
            else if (tag.firstChild().tagName() == "DIV" && innerXmlToBbCode(tag.firstChild()) == "Code:") {
                // TODO: Handle [php] / "PHP Code:" and [html] / "HTML Code:", remove all <span style=color> tags
                const QWebElement pre = tag.firstChild().nextSibling();
                if (pre == tag.lastChild()) {
                    tag.replace("[code]\n" + innerXmlToBbCode(pre) + "\n[/code]");
                }
            }
            // talk.maemo.org
            else if (tag.hasClass("quote")) {
                const QWebElement block = tag.lastChild();
                if (block.tagName() == "BLOCKQUOTE") {
                    if (block == tag.firstChild()) {
                        tag.replace("[quote]" + innerXmlToBbCode(block) + "[/quote]");
                    } else {
                        const QWebElement cite = tag.firstChild();
                        if (cite.tagName() == "CITE" && cite.nextSibling() == block) {
                            const QWebElement strong = cite.firstChild();
                            if (strong.tagName() == "STRONG") {
                                const QString poster = strong.toPlainText();
                                const QWebElement a = strong.nextSibling();
                                if (a.tagName() == "A" && postExpression.exactMatch(a.attribute("href"))) {
                                    const QString postId = postExpression.cap(1);
                                    tag.replace("[quote=" + poster + ";" + postId + "]" + innerXmlToBbCode(block) + "[/quote]");
                                } else
                                    tag.replace("[quote=" + poster + "]" + innerXmlToBbCode(block) + "[/quote]");
                            } else
                                qDebug() << "Failed to parse quote:" << tag.toOuterXml();
                        } else
                            qDebug() << "Failed to parse quote:" << tag.toOuterXml();
                    }
                } else
                    qDebug() << "Failed to parse quote:" << tag.toOuterXml();
            }
            // forum.meego.com
            else if (!tag.findFirst("table > tbody > tr > td.alt2 > div > strong").isNull()) {
                const QWebElement strong = tag.findFirst("table > tbody > tr > td.alt2 > div > strong");
                const QString poster = strong.toPlainText();
                const QWebElement quote = strong.parent().nextSibling();
                if (quote.tagName() == "DIV")
                    tag.replace("[quote=" + poster + "]" + innerXmlToBbCode(quote) + "[/quote]");
                else
                    qDebug() << "Failed to parse quote:" << tag.toOuterXml();
            } else if (!tag.findFirst("table > tbody > tr > td.alt2").isNull()) {
                const QWebElement quote = tag.findFirst("table > tbody > tr > td.alt2");
                tag.replace("[quote]" + innerXmlToBbCode(quote) + "[/quote]");
            } else {
                qDebug() << "Unhandled div:" << tag.toOuterXml();
            }
        } else if (tag.tagName() == "BLOCKQUOTE") {
            if (tag.firstChild() == tag.lastChild() && tag.firstChild().tagName() == "DIV") {
                tag.replace("[indent]" + innerXmlToBbCode(tag.firstChild()) + "[/indent]");
            }
        } else if (tag.tagName() == "A") {
            QString href = tag.attribute("href");
            if (href.startsWith("mailto:"))
                tag.replace("[email=" + href.replace("mailto:", "") + "]" + innerXmlToBbCode(tag) + "[/email]");
            else /* if (tag.attribute("target") == "_blank") */ {
                if (href.startsWith("showthread.php?t=")) {
                    const QString threadId = href.mid(13);
                    tag.replace("[thread=" + threadId + "]" + innerXmlToBbCode(tag) + "[/url]");
                } else if (postExpression.exactMatch(href)) {
                    const QString postId = postExpression.cap(1);
                    tag.replace("[post=" + postId + "]" + innerXmlToBbCode(tag) + "[/url]");
                } else
                    tag.replace("[url=" + href + "]" + innerXmlToBbCode(tag) + "[/url]");
            } /* else
                qDebug() << "Failed to parse link:" << tag.toOuterXml(); */
        } else if (tag.tagName() == "UL") {
            QString list = "[list]\n";
            for (QWebElement li = tag.firstChild(); !li.isNull(); li = li.nextSibling()) {
                qDebug() << "[*]" << li.tagName();
                if (li.tagName() != "LI") {
                    qDebug() << "[*] skipping" << li.tagName();
                    continue;
                }
                list += "[*]" + innerXmlToBbCode(li) + "\n";
            }
            list += "[/list]";
            tag.replace(list);
        } else if (tag.tagName() == "OL" && tag.attribute("type") == "1") {
            QString list = "[list=1]\n";
            for (QWebElement li = tag.firstChild(); !li.isNull(); li = li.nextSibling()) {
                if (li.tagName() != "LI")
                    continue;
                list += "[*]" + innerXmlToBbCode(li) + "\n";
            }
            tag.replace(list);
        }
#if 0
        // FIXME - For whatever reason, this doesn't seem to work.
        else if (tag.tagName() == "IMG" /* && tag.hasClass("inlineimg") */) {
            if (tag.attribute("src").startsWith("image://theme")) {
                // TODO: Replace with smiley BB Code
                qDebug() << "replacing image " + tag.toOuterXml() + " with :)";
                tag.replace(":)");
            } else {
             // tag.replace("[img]" + tag.attribute("src") + "[/img]");
            }
        }
#endif
        // TODO: Handle [noparse]
        // TODO: Handle [attach]
        else if (tag.tagName() == "S") {
            tag.replace("[strike]" + innerXmlToBbCode(tag) + "[/strike]");
        } else if (tag.tagName() == "BODY") {
            tag.setInnerXml(innerXmlToBbCode(tag));
        }
    }

    return element.toInnerXml();
}

static QString smileyToBbCode(QString src)
{
    if (src == "happy")
        return ":)";
    else if (src == "sad")
        return ":(";
    // There is no local equivalent for the :confused: emoticon
 // else if (src == "XXX")
 //     return ":confused";
    else if (src == "angry")
        return ":mad:";
    else if (src == "tongue")
        return ":p";
    else if (src == "wink")
        return ";)";
    else if (src == "very-happy")
        return ":D";
    else if (src == "worried")
        return ":o";
    else if (src == "sarcastic")
        return ":rolleyes:";
    else if (src == "cool")
        return ":cool:";
    else if (src == "surprised")
        return ":eek:";
    else
        return "";
}

bool Post::thankedBy(const QString userName) const
{
    return m_thanks == userName ||
           m_thanks.startsWith(userName + ",") ||
           m_thanks.contains(" " + userName + ",") ||
           m_thanks.endsWith(" " + userName);
}

QString Post::toBbCode() const
{
    qDebug() << "M_BODY" << m_body.simplified();

    // Do we really have to create a whole QWebPage instance just because
    // QWebElement::setOuterXml doesn't work for empty elements?
    QWebPage page;
    page.mainFrame()->setHtml(m_body);
    const QWebElement body = page.mainFrame()->documentElement().findFirst("body");

    // Handle <br> and <img> tags manually, QtWebKit won't let us above
    QString result = innerXmlToBbCode(body).replace(QRegExp("<br> ?"), "\n");
    QRegExp imgExpression("<img src=\"([^\"]*)\">");
    for (int pos = imgExpression.indexIn(result); pos != -1; pos = imgExpression.indexIn(result, pos)) {
        QString src = imgExpression.cap(1);
        if (src.startsWith("image://theme/icon-s-messaging-smiley-")) {
            result.replace(pos, imgExpression.matchedLength(), smileyToBbCode(src.mid(38)));
        } else {
            result.replace(pos, imgExpression.matchedLength(), "[img]" + src + "[/img]");
        }
    }

    return result;
}

void Post::onThanksReceived(QString postId, QStringList thanks)
{
    setThanks(thanks.join(", "));
}
