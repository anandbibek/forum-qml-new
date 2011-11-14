# Folders containing QML code to be shipped with the application
forum.source = qml/forum
forum.target = qml
components.source = qml/components
components.target = qml
DEPLOYMENTFOLDERS += forum components

# Additional import path used to resolve QML modules in Creator's code model
QML_IMPORT_PATH =

# Use QtNetwork for network access
QT += network

# Use QtWebKit to parse forum pages
QT += webkit
CONFIG += webkit

CONFIG += link_pkgconfig
PKGCONFIG += accounts-qt AccountSetup libsignon-qt

# No use for QtMobility (yet)
# CONFIG += mobility
# MOBILITY +=

# Speed up launching on MeeGo/Harmattan when using applauncherd daemon
CONFIG += qdeclarative-boostable

# C++ source code files
HEADERS += \
    $$PWD/src/forumlist.h \
    $$PWD/src/forum.h \
    $$PWD/src/forumsession.h \
    $$PWD/src/threadlist.h \
    $$PWD/src/thread.h \
    $$PWD/src/post.h \
    $$PWD/src/postlist.h \
    $$PWD/src/pagedlistmodel.h \
    $$PWD/src/datetimehelper.h \
    $$PWD/src/newpost.h \
    $$PWD/src/searchresultthreadlist.h

SOURCES += \
    $$PWD/src/main.cpp \
    $$PWD/src/forumlist.cpp \
    $$PWD/src/forum.cpp \
    $$PWD/src/forumsession.cpp \
    $$PWD/src/threadlist.cpp \
    $$PWD/src/thread.cpp \
    $$PWD/src/post.cpp \
    $$PWD/src/postlist.cpp \
    $$PWD/src/pagedlistmodel.cpp \
    $$PWD/src/datetimehelper.cpp \
    $$PWD/src/newpost.cpp \
    $$PWD/src/searchresultthreadlist.cpp
