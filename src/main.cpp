#include <QtDeclarative>
#include <QtGui/QApplication>
#include "qmlapplicationviewer.h"

#include "forumsession.h"
#include "thread.h"

#include "post.h"

Q_DECL_EXPORT int main(int argc, char *argv[])
{
    QScopedPointer<QApplication> app(createApplication(argc, argv));
    QScopedPointer<QmlApplicationViewer> viewer(QmlApplicationViewer::create());

    QCoreApplication::setApplicationName(QFileInfo(QCoreApplication::applicationFilePath()).fileName());

    // Export the ForumSession and Thread classes
    qmlRegisterType<ForumSession>("Forum", 1, 0, "ForumSession");
    qmlRegisterUncreatableType<Thread>("Forum", 1, 0, "Thread", "Thread object creation is handled by the thread list model");

    viewer->setOrientation(QmlApplicationViewer::ScreenOrientationAuto);

    viewer->setMainQmlFile("qml/" + QCoreApplication::applicationName() + "/main.qml");
    viewer->showExpanded();

    return app->exec();
}
