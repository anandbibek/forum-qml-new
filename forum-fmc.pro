# Folders containing QML code to be shipped with the application
fmc.source = qml/forum-fmc
fmc.target = qml
DEPLOYMENTFOLDERS += fmc

include(forum-qml.pri)

# View header title image and splash image
images.files = images/meego.png
images.path = /opt/forum-fmc/images
splash.files = images/forum-fmc-splash.jpg
splash.path = /usr/share/themes/blanco/meegotouch/images/splash
INSTALLS += images splash

# Account provider definition and icon
provider.files = fmc.provider
provider.path = /usr/share/accounts/providers
providericon.files = images/icon-m-service-meego.png
providericon.path = /usr/share/themes/blanco/meegotouch/icons
INSTALLS += provider providericon

# Please do not modify the following two lines. Required for deployment.
include(qmlapplicationviewer/qmlapplicationviewer.pri)
qtcAddDeployment()

