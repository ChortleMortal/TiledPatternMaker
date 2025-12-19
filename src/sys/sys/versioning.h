#pragma once
#ifndef VERSIONING_H
#define VERSIONING_H

#include <QString>
#include <QVector>

// Versioned names do not include.xml
// The display name will always include .v0, the filename does not
class VersionedName
{
public:
    VersionedName() {}
    VersionedName(QString name)             { set(name); }
    VersionedName(const VersionedName & other);

    void    set(QString name);
    void    setVersion(uint ver);

    QString get() const                     { return name; }
    QString getUnversioned() const;
    uint    getVersion() const;

    void    clear()                         { name.clear(); }
    bool    isEmpty() const                 { return name.isEmpty(); }

       void operator=(const VersionedName& other)  { name = other.name; }
       bool operator==(const VersionedName& other) { return (name == other.name); }
       bool operator!=(const VersionedName& other) const { return !(*this == other); }

friend bool operator==(const VersionedName & v1, const VersionedName & v2) { return (v1.name == v2.name); }

private:
    QString name;
};

// A version List is an encapsulated list of VersionNames
class VersionList
{
    friend class VersionStepper;

public:
    void        setNames(QStringList & slist);  // sorts by version
    QStringList getNames();

    bool        add(const VersionedName & name);
    bool        remove(const VersionedName & name);
    bool        prepend(const VersionedName & name);

    void        sort(); // case insensitive
    void        removeDuplicates();
    VersionList filter(QString filter, bool lowerCase);

    int         count()                                     { return list.count(); }
    bool        contains(VersionedName & vn);

    void        clear()                                     { list.clear(); }
    bool        isEmpty() const                             { return list.isEmpty(); }

    VersionedName at(qsizetype index)                       { return list.at(index); }

    QVector<VersionedName>::iterator begin()                { return list.begin(); }
    QVector<VersionedName>::const_iterator begin() const    { return list.cbegin(); }
    QVector<VersionedName>::iterator end()                  { return list.end(); }
    QVector<VersionedName>::const_iterator end() const      { return list.cend(); }

    VersionList& operator+=(const VersionList& other)       { list += other.list; return  *this; }

private:
    static bool CompareVersionName(const VersionedName & a, const VersionedName & b);

    QVector<VersionedName>  list;
};

class VersionedFile
{
public:
    VersionedFile() {}
    VersionedFile(QString fullPathname)         { setFromFullPathname(fullPathname); }

    void            setFromFullPathname(QString namedPath);
    void            updateFromVersionedName(VersionedName & vname);

    VersionedName & getVersionedName()          { return name; }
    QString         getPathedName() const       { return full; }
    QString         getPathOnly() const         { return path; }
    QString         getPathDir();

    void            clear()                     { full.clear(); path.clear(); name.clear(); }
    bool            isEmpty()                   { return full.isEmpty(); }

    bool operator==(const VersionedFile& other) { return (name == other.name); }
    bool operator!=(const VersionedFile& other) { return !(*this == other); }

protected:
    static QString  stripPath(QString path);

private:
    QString         full;   // includes .xml
    QString         path;   // path only
    VersionedName   name;   // created using extracted file name
};

class VersionFileList
{
public:
    QStringList     getNames();
    QStringList     getPathedNames();
    VersionList     getVersionList();
    void            sort(); // case insensitive
    VersionFileList filter(QString filter, bool lowerCase);

    void            add(const VersionedFile & vf);
    void            prepend(const VersionedFile & vf);
    void            remove(const VersionedFile & vf);

    void            clear()                                 { list.clear(); }
    int             size()                                  { return list.size(); }
    VersionedFile   at(qsizetype index)                     { return list.at(index); }

    QVector<VersionedFile>::iterator begin()                { return list.begin(); }
    QVector<VersionedFile>::const_iterator begin() const    { return list.cbegin(); }
    QVector<VersionedFile>::iterator end()                  { return list.end(); }
    QVector<VersionedFile>::const_iterator end() const      { return list.cend(); }

    VersionFileList& operator+=(const VersionFileList& other) { list += other.list; return  *this; }

private:
    static bool     CompareVersionName(VersionedFile &a, VersionedFile &b);

    QVector<VersionedFile>  list;
};

#endif // VERSIONING_H
