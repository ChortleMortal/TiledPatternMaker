#include <QTimer>
#include <QFile>
#include <QDebug>

#include "gui/map_editor/map_editor_stash.h"
#include "gui/map_editor/map_editor_db.h"
#include "sys/sys/fileservices.h"
#include "sys/sys.h"
#include "gui/top/view.h"

#define STASH_VERSION 3

MapEditorStash::MapEditorStash()
{
    init();
}

bool MapEditorStash::stash(MapEditorDb * db)
{
    int next = current + 1;
    if (next > MAX_STASH) next = 0;

    QString nextName = getStashName(next);
    bool rv = writeStash(nextName,db);

    if (rv)
    {
        // update local data
        add(next);
    }

    return rv;
}

bool MapEditorStash::writeStash(QString name, MapEditorDb * db)
{
    QFile file(name);
    bool rv = file.open(QIODevice::WriteOnly | QIODevice::Truncate);
    if (!rv)
    {
        qWarning() << "could not open" << file.fileName();
        return false;
    }

    QDataStream out(&file);

    // Write a header with a "magic number" and a version
    out << static_cast<quint64>(0xA0B0C0D00);
    out << static_cast<qint64>(STASH_VERSION);

    // Write the data
    out << db->constructionLines;

    for (const auto & c : std::as_const(db->constructionCircles))
    {
        Circle circle = *c;
        out << circle;
    }

    return true;
}

bool MapEditorStash::destash()
{
#if 0
    QString name = getStashName(current);

    return readStash(name);
#else
    return false;
#endif
}

bool MapEditorStash::animateReadStash(VersionedFile &xfile)
{
    bool rv = readStashTo(xfile, localLines, localCircs);
    return rv;
}

bool MapEditorStash::readStash(VersionedFile & xfile, MapEditorDb * db)
{
    return readStashTo(xfile,db->constructionLines, db->constructionCircles);
}

bool MapEditorStash::readStashTo(VersionedFile & xfile, QVector<QLineF>  & lines, QVector<CirclePtr> & circs)
{
    qInfo() << "Loading template" << xfile.getPathedName();

    QFile file(xfile.getPathedName());
    bool rv = file.open(QIODevice::ReadOnly);
    if (!rv)
    {
        qWarning() << "could not open" << file.fileName();
        return false;
    }

    QDataStream in(&file);

    // Read and check the header
    quint64 magic;
    in >> magic;
    if (magic != 0xA0B0C0D00)
    {
        qWarning() << "invalid stash header"  << magic;
        return false;
    }
    qint64 version = 0;
    in >> version;
    if ( version < 0 || version > STASH_VERSION)
    {
        qWarning() << "invalid vesion" << version;
        return false;
    }
    else
    {
        qInfo() << "Template version =" << version;
    }

    // read data
    lines.clear();
    circs.clear();

    if (version >= 1)
    {
        in >> lines;
    }

    if (version == 2 && !in.atEnd())
    {
        QVector<Circle> circles;
        in >> circles;

        for (const auto & c : circles)
        {
            CirclePtr circle = std::make_shared<Circle>(c);
            circs.push_back(circle);
        }
    }
    else if (version >= 3 && !in.atEnd())
    {
        QVector<Circle> circles;
        in >> circles;

        for (const auto & c : circles)
        {
            CirclePtr circle = std::make_shared<Circle>(c);
            circs.push_back(circle);
        }
    }

    file.close();

    qInfo() << "Template loaded" << xfile.getPathedName();
    return true;
}

bool MapEditorStash::initStash(VersionedName mosaicname, MapEditorDb * db)
{
    VersionedFile xfile = FileServices::getFile(mosaicname,FILE_TEMPLATE);
    if (xfile.isEmpty())
    {
        // there is no existing template assocoated with the newly
        // loaded mosaic
        return false;
    }

    bool rv = readStash(xfile,db);
    stash(db);
    return rv;
}

bool MapEditorStash::saveTemplate(VersionedName vname)
{
    if (current == -1)
        return false;

    VersionedFile xfile = FileServices::getFile(vname,FILE_TEMPLATE);
    if (xfile.isEmpty())
    {
        QString pname = Sys::templateDir + vname.get() + ".dat";
        xfile.setFromFullPathname(pname);
    }
    else
    {
        // delete existing file
        QFile afile(xfile.getPathedName());
        afile.remove(xfile.getPathedName());
    }

    QString currentName = getStashName(current);
    QFile bfile(currentName);
    bool rv = bfile.copy(xfile.getPathedName());

    return rv;
}

int MapEditorStash::getNext()
{
    if (current == last)
        return current;

    current++;
    if (current > MAX_STASH) current = 0;
    return current;
}

int MapEditorStash::getPrev()
{
    if (current == first)
        return current;

    current--;
    if (current < 0)
        current = MAX_STASH;
    return current;
}

void MapEditorStash::add(int index)
{
    current = index;
    last    = index;
    if (first == index)
    {
        first++;
        if (first > MAX_STASH)
            first = 0;
    }
}

QString MapEditorStash::getStashName(int index)
{
    QString name = QString("constructionstash%1.dat").arg(QString::number(index));
    return name;
}

void MapEditorStash::nextAnimationStep(MapEditorDb * db, QTimer * timer)
{
    while (!localLines.isEmpty())
    {
        QLineF line = localLines.takeFirst();
        db->constructionLines.push_back(line);
        return;
    }

    while (!localCircs.isEmpty())
    {
        auto cp = localCircs.takeFirst();
        db->constructionCircles.push_back(cp);
        return;
    }

    timer->stop();
}
