#include "model/settings/worklist.h"
#include "sys/tiledpatternmaker.h"

Worklist::Worklist()
{}

void Worklist::set(QString name, VersionList & list)
{
    clear();

    listname = name;
    vlist    = list;

    emit theApp->sig_workListChanged();
}

VersionList &  Worklist::get()
{
    vlist.sort();
    return vlist;
}


void Worklist::add(VersionedName version)
{
    if (vlist.add(version))
    {
        emit theApp->sig_workListChanged();
    }
}

void Worklist::remove(VersionedName version)
{
    if (vlist.remove(version))
    {
        emit theApp->sig_workListChanged();
    }
}

void Worklist::clear()
{
    listname.clear();
    vlist.clear();
    emit theApp->sig_workListChanged();
}

void Worklist::load(QSettings & s)
{
    listname = s.value("workListName3","").toString();

    vlist.clear();
    QStringList qsl = s.value("worklist","").toStringList();
    qsl.removeDuplicates();

    for (auto & str : std::as_const(qsl))
    {
        if (!str.isEmpty())
        {
            VersionedName vn(str);
            vlist.add(vn);
        }
    }

    vlist.sort();
}

void Worklist::save(QSettings & s)
{
    s.setValue("workListName3",listname);

    QStringList qsl;
    for (const VersionedName & vn : vlist)
    {
        if (!vn.isEmpty())
        {
            qsl.push_back(vn.get());
        }
    }

    s.setValue("worklist",qsl);
}

