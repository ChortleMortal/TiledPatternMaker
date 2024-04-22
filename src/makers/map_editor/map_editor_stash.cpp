#include <QTimer>
#include <QFile>
#include <QDebug>

#include "makers/map_editor/map_editor_stash.h"
#include "makers/map_editor/map_editor_db.h"
#include "misc/fileservices.h"
#include "misc/sys.h"
#include "viewers/view.h"

#define STASH_VERSION 3

MapEditorStash::MapEditorStash(MapEditorDb * db)
{
    this->db = db;
    first    = -1;
    last     = -1;
    current  = -1;

    timer    = nullptr;
}

bool MapEditorStash::stash()
{
    int next = current + 1;
    if (next > MAX_STASH) next = 0;

    QString nextName = getStashName(next);
    bool rv = writeStash(nextName);

    if (rv)
    {
        // update local data
        add(next);
    }

    return rv;
}

bool MapEditorStash::writeStash(QString name)
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
    QString name = getStashName(current);

    return readStash(name);
}

bool MapEditorStash::animateReadStash(QString name)
{
    bool rv = readStashTo(name, localLines, localCircs);
    if (!rv) return false;

    // animate
    if (!timer)
    {
        timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, &MapEditorStash::slot_nextAnimationStep);
    }
    timer->start(500);

    return true;
}

bool MapEditorStash::readStash(QString name)
{
    return readStashTo(name,db->constructionLines, db->constructionCircles);
}


bool MapEditorStash::readStashTo(QString name, QVector<QLineF>  & lines, QVector<CirclePtr> & circs)
{
    QFile file(name);
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

    // read data
    lines.clear();
    circs.clear();

    if (version >= 1)
    {
        in >> lines;
    }

    if (version == 2)
    {
        QVector<Circle> circles;
        in >> circles;
        for (const auto & c : circles)
        {
            CirclePtr circle = std::make_shared<Circle>(c);
            circs.push_back(circle);
        }
    }

    if (version >= 3)
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

    return true;
}


bool  MapEditorStash::initStash(QString stashname)
{
    QString nextName = getStashName(0);
    QFile afile(nextName);
    if (afile.exists())
    {
        afile.remove(nextName);
    }

    QString designfile = FileServices::getMosaicTemplateFile(stashname);
    if (designfile.isEmpty())
    {
        return false;
    }
    QFile bfile(designfile);
    bool rv = bfile.copy(nextName);
    if (rv)
    {
        first   = 0;
        last    = 0;
        current = 0;
    }

    return rv;
}

bool MapEditorStash::keepStash(QString stashname)
{
    if (current == -1)
        return false;

    QString currentName = getStashName(current);

    QString designfile = FileServices::getMosaicTemplateFile(stashname);

    QFile afile(designfile);
    if (afile.exists())
    {
        afile.remove(designfile);
    }

    QFile bfile(currentName);
    bool rv = bfile.copy(designfile);

    return rv;
}

bool MapEditorStash::saveTemplate(QString name)
{
    if (current == -1)
        return false;

    QString currentName = getStashName(current);

    QString file = Sys::templateDir + name + ".dat";

    QFile afile(file);
    if (afile.exists())
    {
        afile.remove(file);
    }

    QFile bfile(currentName);
    bool rv = bfile.copy(file);

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

void MapEditorStash::slot_nextAnimationStep()
{
    while (!localLines.isEmpty())
    {
        QLineF line = localLines.takeFirst();
        db->constructionLines.push_back(line);
        Sys::view->update();
        return;
    }

    while (!localCircs.isEmpty())
    {
        auto cp = localCircs.takeFirst();
        db->constructionCircles.push_back(cp);
        Sys::view->update();
        return;
    }

    timer->stop();
}
