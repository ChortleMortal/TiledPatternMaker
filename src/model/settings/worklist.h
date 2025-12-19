#ifndef WORKLIST_H
#define WORKLIST_H

#include <QString>
#include <QSettings>

#include "sys/sys/versioning.h"

class Worklist
{
public:
    Worklist();

    void                    set(QString name, VersionList &list);
    VersionList &           get();

    void                    setName(QString name) { listname = name; }
    const QString &         getName()   { return listname; }

    void                    add(VersionedName version);
    void                    remove(VersionedName version);
    void                    clear();
    void                    removeDuplicates() { vlist.removeDuplicates(); }

    void                    load(QSettings & s);
    void                    save(QSettings & s);

    int                     count() { return vlist.count(); }

private:
    QString                 listname;
    VersionList             vlist;
};

#endif // WORKLIST_H
