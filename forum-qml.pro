# Folders containing QML code to be shipped with the application
components.source = qml/components
components.target = qml
forum.source = qml/forum
forum.target = qml
fmc.source = qml/forum-fmc
fmc.target = qml
tmo.source = qml/forum-tmo
tmo.target = qml
nokia.source = qml/forum-nokia
nokia.target = qml
xda.source = qml/forum-xda
xda.target = qml
DEPLOYMENTFOLDERS = components forum fmc tmo nokia xda

# Use QtNetwork for network access
QT += network

# Use QtWebKit to parse forum pages
QT += webkit
CONFIG += webkit

CONFIG += link_pkgconfig
PKGCONFIG += accounts-qt AccountSetup gq-gconf libsignon-qt

# No use for QtMobility (yet)
# CONFIG += mobility
# MOBILITY +=

# Speed up launching on MeeGo/Harmattan when using applauncherd daemon
CONFIG += qdeclarative-boostable

# View header title image and splash image
images.files = images/meego.png
images.path = /opt/forum-fmc/images
splash.files = images/forum-fmc-splash.jpg
splash.path = /usr/share/themes/blanco/meegotouch/images/splash
INSTALLS += images splash

# Account provider definition and icon
provider.files = fmc.provider tmo.provider
provider.path = /usr/share/accounts/providers
providericon.files = images/icon-m-service-meego.png images/icon-m-service-maemo.png
providericon.path = /usr/share/themes/blanco/meegotouch/icons
INSTALLS += provider providericon

# Application settings
settings.files = forumsettings.xml
settings.path = /usr/share/duicontrolpanel/uidescriptions
INSTALLS += settings

# Please do not modify the following two lines. Required for deployment.
include(qmlapplicationviewer/qmlapplicationviewer.pri)
qtcAddDeployment()

unix {
    !isEmpty(MEEGO_VERSION_MAJOR) {
        desktopfile.files += forum-tmo_harmattan.desktop
        icon.files += forum-tmo80.png
    }
}

# C++ source code files
HEADERS += \
    src/forum.h \
    src/forumlist.h \
    src/forumlistcache.h \
    src/forumproviderlist.h \
    src/forumsession.h \
    src/gconfitemqmlproxy.h \
    src/threadlist.h \
    src/thread.h \
    src/post.h \
    src/postlist.h \
    src/pagedlistmodel.h \
    src/datetimehelper.h \
    src/newpost.h \
    src/searchresultthreadlist.h

SOURCES += \
    src/main.cpp \
    src/forum.cpp \
    src/forumlist.cpp \
    src/forumlistcache.cpp \
    src/forumproviderlist.cpp \
    src/forumsession.cpp \
    src/gconfitemqmlproxy.cpp \
    src/threadlist.cpp \
    src/thread.cpp \
    src/post.cpp \
    src/postlist.cpp \
    src/pagedlistmodel.cpp \
    src/datetimehelper.cpp \
    src/newpost.cpp \
    src/searchresultthreadlist.cpp
