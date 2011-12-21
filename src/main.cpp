#include <QtDeclarative>
#include <QtGui/QApplication>
#include "qmlapplicationviewer.h"

#include "forumproviderlist.h"
#include "forumsession.h"
#include "gconfitemqmlproxy.h"
#include "thread.h"

Q_DECL_EXPORT int main(int argc, char *argv[])
{
    QScopedPointer<QApplication> app(createApplication(argc, argv));
    QScopedPointer<QmlApplicationViewer> viewer(QmlApplicationViewer::create());

    QCoreApplication::setApplicationName(QFileInfo(QCoreApplication::applicationFilePath()).fileName());

    ForumSession forumSession;
    // FIXME - parse forum provider files
    if (QCoreApplication::arguments().contains("--fmc")) {
        forumSession.setProvider("fmc");
        forumSession.setUrl("http://forum.meego.com");
    } else if (QCoreApplication::arguments().contains("--tmo")) {
        forumSession.setProvider("tmo");
        forumSession.setUrl("http://talk.maemo.org");
    }

    // Export the forum session
    viewer->rootContext()->setContextProperty("forumSession", &forumSession);

    // Export the GConfItem and Thread classes
    qmlRegisterType<GConfItemQmlProxy>("Forum", 1, 0, "GConfItem");
    qmlRegisterType<ForumProviderList>("Forum", 1, 0, "ForumProviderList");
    qmlRegisterUncreatableType<Thread>("Forum", 1, 0, "Thread", "Thread object creation is handled by the thread list model");

    viewer->setOrientation(QmlApplicationViewer::ScreenOrientationAuto);

    viewer->setMainQmlFile("qml/" + QCoreApplication::applicationName() + "/main.qml");
    viewer->showExpanded();

    return app->exec();
}
