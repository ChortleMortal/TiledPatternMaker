#include "model/styles/casing_set.h"
#include "model/styles/casing.h"

CasingSet::CasingSet()
{
    map =  std::make_shared<Map>("CasingSet");
}

void CasingSet::buildMap()
{
    map->wipeout();
    for (auto & casing : *this)
    {
        casing->addToMap(map);
    }
    //qDebug() << map->info();
}

void CasingSet::validate()
{
    qInfo() << "validating casings - start";
    for (auto & casing : *this)
    {
        if (!casing->validate())
        {
            qWarning() << "LOG2 casing" << casing->edgeIndex << "INVALID";
        }
    }
    qInfo() << "validating casings - end";
}

void CasingSet::dump(QString str)
{
    qDebug() << "start" << str;
    for (auto & casing : *this)
    {
        casing->dump();
    }
    qDebug() << "end";
}
