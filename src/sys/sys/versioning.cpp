#include <QDebug>
#include <QFileInfo>
#include "sys/sys/versioning.h"
#include "sys/qt/unique_qvector.h"

///////////////////////////////////////////////////////////
//
// VersionFileList
//
///////////////////////////////////////////////////////////

QStringList VersionFileList::getPathedNames()
{
    QStringList qsl;
    for (const VersionedFile & file : *this)
    {
        qsl << file.getPathedName();
    }
    return qsl;
}

VersionList VersionFileList::getVersionList()
{
    VersionList vl;
    for (VersionedFile & file : *this)
    {
        vl.add(file.getVersionedName());
    }
    return vl;
}

void VersionFileList::sort()
{
    std::sort(begin(),end(),CompareVersionName);
}

QStringList VersionFileList::getNames()
{
    QStringList qsl;
    for (VersionedFile & file : *this)
    {
        qsl << file.getVersionedName().get();
    }
    return qsl;
}

bool VersionFileList::CompareVersionName(VersionedFile & a, VersionedFile & b)
{
    return     a.getVersionedName().getUnversioned().toLower()  < b.getVersionedName().getUnversioned().toLower()
           || (a.getVersionedName().getUnversioned().toLower() == b.getVersionedName().getUnversioned().toLower() && a.getVersionedName().getVersion() < b.getVersionedName().getVersion());
}

VersionFileList VersionFileList::filter(QString filter, bool lowerCase)
{
    if (lowerCase)
        filter = filter.toLower();

    VersionFileList filtered;
    for (VersionedFile & file : *this)
    {
        QString name = file.getVersionedName().get();
        if (lowerCase)
        {
            name = name.toLower();
        }
        if (name.contains(filter))
        {
            filtered.push_back(file);
        }
    }
    return filtered;
}

///////////////////////////////////////////////////////////
//
// VersionedFile
//
///////////////////////////////////////////////////////////

void VersionedFile::setFromFullPathname(QString namedPath)
{
    full = namedPath;
    QString pname = stripPath(namedPath);
    path = namedPath.remove(pname);
    name.set(pname);    // this excludes .xml, .bmp, .dat
}

void VersionedFile::updateFromVersionedName(VersionedName & vname)
{
    QString old = stripPath(full);
    old.remove(".xml");
    old.remove(".dat");
    name = vname;
    full.replace(old,vname.get());
}

QString VersionedFile::stripPath(QString path)
{
    QFileInfo info(path);
    return info.fileName();
}

///////////////////////////////////////////////////////////
//
// Version List
//
///////////////////////////////////////////////////////////

bool VersionList::add(const VersionedName &name)
{
    if (!name.isEmpty())
    {
        list.push_back(name);
        return true;
    }
    else
    {
        return false;
    }
}

bool VersionList::prepend(const VersionedName &name)
{
    if (!name.isEmpty())
    {
        list.push_front(name);
        return true;
    }
    else
    {
        return false;
    }
}

VersionList VersionList::filter(QString filter, bool lowerCase)
{
    if (lowerCase)
        filter = filter.toLower();

    VersionList filtered;
    for (const VersionedName & version : std::as_const(list))
    {
        QString name = version.get();
        if (lowerCase)
        {
            name = name.toLower();
        }
        if (name.contains(filter))
        {
            filtered.add(version);
        }
    }
    return filtered;
}

bool VersionList::contains(VersionedName & vn)
{
    for (auto & vname : std::as_const(list))
    {
        if (vname == vn)
        {
            return true;
        }
    }
    return false;
}

QStringList VersionList::getNames()
{
    QStringList qsl;
    for (const VersionedName & vn : std::as_const(list))
    {
        qsl << vn.get();
    }
    return qsl;
}

void VersionList::setNames(QStringList & slist)
{
    for (const QString & name : std::as_const(slist))
    {
        VersionedName vn(name);
        add(vn);
    }

    sort();
}

void VersionList::removeDuplicates()
{
    UniqueQVector<VersionedName> vns = list;

    list.clear();

    for (auto & vn : vns)
    {
        add(vn);
    }
}

void VersionList::sort()
{
    std::sort(list.begin(),list.end(),CompareVersionName);
}

bool VersionList::CompareVersionName(const VersionedName & a, const VersionedName & b)
{
    return     a.getUnversioned().toLower()  < b.getUnversioned().toLower()
           || (a.getUnversioned().toLower() == b.getUnversioned().toLower() && a.getVersion() < b.getVersion()) ;
}

///////////////////////////////////////////////////////////
//
// Version Name
//
///////////////////////////////////////////////////////////

VersionedName::VersionedName(const VersionedName & other)
{
    name = other.name;
}

void VersionedName::set(QString name)
{
    if (name.isEmpty())
    {
        clear();
        return;
    }
    name.remove(".xml");
    name.remove(".dat");
    name.remove(".bmp");
    this->name = name;
}

void VersionedName::setVersion(uint ver)
{
    if (ver == 0)
    {
        set(getUnversioned());
    }
    else
    {
        set(getUnversioned() + ".v" + QString::number(ver));
    }
}

QString VersionedName::getUnversioned() const
{
    QStringList parts = name.split(".v");
    if (parts.empty())
    {
        return QString();
    }
    else
    {
        return parts[0];
    }
}

uint VersionedName::getVersion() const
{
    QStringList parts = name.split(".v");
    if (parts.count() ==2 )
    {
        QString ver = parts[1];
        return ver.toUInt();
    }
    else
    {
        return 0;
    }
}
