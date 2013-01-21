#include "forumproviderlist.h"

#ifndef QT_SIMULATOR
#include <account.h> // Accounts::Manager
#include <manager.h>
#include <provider.h>
#endif

class Provider {
public:
    Provider(QString name, QString displayName, QString iconName)
        : m_name(name), m_displayName(displayName), m_iconName(iconName) { }
    QString name() const { return m_name; }
    QString displayName() const { return m_displayName; }
    QString iconName() const { return m_iconName; }
private:
    QString m_name;
    QString m_displayName;
    QString m_iconName;
};

QList<Provider*> providers;

#include <QDebug>
ForumProviderList::ForumProviderList(QObject *parent) :
    QAbstractListModel(parent)
{
    QHash<int, QByteArray> roles;
    roles[NameRole] = "name";
    roles[IconNameRole] = "iconName";
    setRoleNames(roles);

    providers.clear();
#ifdef QT_SIMULATOR
    providers.append(new Provider("tmo", "maemo.org - Talk", "icon-m-service-maemo"));
    providers.append(new Provider("fmc", "MeeGo Forum", "icon-m-service-meego"));
    providers.append(new Provider("nokia", "Nokia Developer Discussions", "icon-m-service-nokia-developer"));
    providers.append(new Provider("xda", "xda-developers Forum", "icon-m-service-xda-developer"));
#else
    Accounts::Manager m_manager;
    foreach (Accounts::Provider* provider, m_manager.providerList()) {
        // FIXME - how to detect forum providers? - service types?
        if (provider->name() == "tmo" || provider->name() == "fmc" || provider->name() == "nokia" || provider->name() == "xda")
            providers.append(new Provider(provider->name(), provider->displayName(), provider->iconName()));
    }
    //Providers not fully supported
    //providers.append(new Provider("nokia", "Nokia Developer Discussions", "icon-m-service-nokia-developer"));
    //providers.append(new Provider("xda", "xda-developers Forum", "icon-m-service-xda-developer"));
#endif
}

QVariant ForumProviderList::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    Provider* provider = providers.at(index.row());
    if (role == NameRole)
        return provider->displayName();
    else if (role == IconNameRole)
        return provider->iconName();
    else if (role == ProviderRole)
        return provider->name();
    return QVariant();
}

int ForumProviderList::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)

    return providers.count();
}

int ForumProviderList::count() const
{
    return providers.count();
}

QVariant ForumProviderList::get(int index) const
{
    Provider* provider = providers.at(index);

    QVariantMap result;
    result["name"] = provider->displayName();
    result["iconName"] = provider->iconName();
    result["provider"] = provider->name();
    return result;
}
