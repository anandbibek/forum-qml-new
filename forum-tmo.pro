# Folders containing QML code to be shipped with the application
tmo.source = qml/forum-tmo
tmo.target = qml
DEPLOYMENTFOLDERS += tmo

include(forum-qml.pri)

# View header title image and splash image
images.files = images/maemo.org.png
images.path = /opt/tmo/images
splash.files = images/forum-tmo-splash.jpg
splash.path = /usr/share/themes/blanco/meegotouch/images/splash
INSTALLS += images splash

# Account provider definition and icon
provider.files = tmo.provider
provider.path = /usr/share/accounts/providers
providericon.files = images/icon-m-service-maemo.png
providericon.path = /usr/share/themes/blanco/meegotouch/icons
INSTALLS += provider providericon

# Please do not modify the following two lines. Required for deployment.
include(qmlapplicationviewer/qmlapplicationviewer.pri)
qtcAddDeployment()
