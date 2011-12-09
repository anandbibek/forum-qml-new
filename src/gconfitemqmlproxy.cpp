#include <QDebug>
#include "gconfitemqmlproxy.h"

GConfItemQmlProxy::GConfItemQmlProxy(QObject *parent) :
   QObject(parent)
{
#ifndef QT_SIMULATOR
   _gconfItem = NULL;
#endif
   _key = "";
}

const QString & GConfItemQmlProxy::key()
{
   return _key;
}

void GConfItemQmlProxy::setKey(const QString &newKey)
{
#ifndef QT_SIMULATOR
   if (_gconfItem) {
       delete _gconfItem;
   }
   _gconfItem = new GConfItem(newKey);
#endif
   _key = newKey;

#ifndef QT_SIMULATOR
   connect(_gconfItem, SIGNAL(valueChanged()), this, SLOT(updateValue()));
#endif
}

const QVariant & GConfItemQmlProxy::value()
{
#ifndef QT_SIMULATOR
   _value = _gconfItem->value(_defaultValue);
#endif
   return _value;
}

void GConfItemQmlProxy::setValue(const QVariant &newValue)
{
#ifndef QT_SIMULATOR
   if (_gconfItem) {
       _gconfItem->set(newValue);
       _value = newValue;
   }
#endif
}

const QVariant & GConfItemQmlProxy::defaultValue()
{
   return _defaultValue;
}

void GConfItemQmlProxy::setDefaultValue(const QVariant &newValue)
{
   _defaultValue = newValue;
#ifdef QT_SIMULATOR
   _value = _defaultValue;
#endif
}

void GConfItemQmlProxy::updateValue()
{
#ifndef QT_SIMULATOR
   _value = _gconfItem->value(_defaultValue);
   emit valueChanged();
#endif
}
