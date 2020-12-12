#include "makers/map_editor/map_editor_stash.h"
#include "makers/map_editor/map_editor.h"
#include "base/fileservices.h"
#include "base/configuration.h"

#include <QTimer>

#define STASH_VERSION 3

MapEditorStash::MapEditorStash(MapEditor * me)
{
    editor  = me;
    first   = -1;
    last    = -1;
    current = -1;

    timer = nullptr;
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
    out << editor->constructionLines;

    out << editor->constructionCircles;

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
    return readStashTo(name,editor->constructionLines, editor->constructionCircles);
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
        qWarning() << "invalid stash header"  << hex << magic;
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
        QVector<Circle> tmp;
        in >> tmp;
        for (auto it = tmp.begin(); it != tmp.end(); it++)
        {
            Circle c = *it;
            CirclePtr p = make_shared<Circle>(c);
            circs.push_back(p);
        }
    }

    if (version >= 3)
    {
        in >> circs;
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

    QString designfile = FileServices::getDesignTemplateFile(stashname);
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

    QString designfile = FileServices::getDesignTemplateFile(stashname);

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

    Configuration * config = Configuration::getInstance();
    QString file = config->templateDir + name + ".dat";

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
        editor->constructionLines.push_back(line);
        editor->buildEditorDB();
        editor->forceRedraw();
        return;
    }

    while (!localCircs.isEmpty())
    {
        CirclePtr cp = localCircs.takeFirst();
        editor->constructionCircles.push_back(cp);
        editor->buildEditorDB();
        editor->forceRedraw();
        return;
    }

    timer->stop();
}
