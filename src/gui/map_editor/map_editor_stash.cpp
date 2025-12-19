#include <QTimer>
#include <QFile>
#include <QDebug>

#include "gui/map_editor/map_editor_stash.h"
#include "gui/map_editor/map_editor_db.h"
#include "model/settings/configuration.h"
#include "sys/sys/fileservices.h"
#include "sys/sys.h"
#include "sys/qt/tpm_io.h"
#include "sys/sys/pugixml.hpp"

using namespace pugi;

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

    QTextStream ts(&file);
    ts.setRealNumberPrecision(16);

    ts << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl;
    ts << "<template>" << endl;

    ts << "<lines>" << endl;

    for (const auto & line : std::as_const(db->constructionLines))
    {
        ts << "<line>" << line.p1().x() << "," << line.p1().y() << "," << line.p2().x() << "," << line.p2().y() << "</line>" << endl;
    }
    ts << "</lines>" << endl;

    ts << "<circles>" << endl;

    for (const auto & c : std::as_const(db->constructionCircles))
    {
        QString str = QString("<circle radius=\"%1\" centreX=\"%2\" centreY=\"%3\" />").arg(c->radius).arg(c->centre.x()).arg(c->centre.y());
        ts << str << endl;
    }
    ts << "</circles>" << endl;

    ts << "</template>" << endl;
    file.close();

    // reformat
    VersionedFile xfile;
    xfile.setFromFullPathname(name);    // name works for current directory
    return FileServices::reformatXML(xfile);
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
    if (Sys::config->mapedOldTemplates)
        return readStashDat(xfile, localLines, localCircs);
    else
        return readStashXML(xfile, localLines, localCircs);
}

bool MapEditorStash::readStash(VersionedFile & xfile, MapEditorDb * db)
{
     if (Sys::config->mapedOldTemplates)
         return readStashDat(xfile,db->constructionLines, db->constructionCircles);
     else
         return readStashXML(xfile,db->constructionLines, db->constructionCircles);
}

bool MapEditorStash::readStashDat(VersionedFile & xfile ,QVector<QLineF>  & lines, QVector<CirclePtr> & circs)
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
        qWarning() << "invalid stash header"  << Qt::hex <<  magic;
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

bool MapEditorStash::readStashXML(VersionedFile &xfile,QVector<QLineF>  & lines, QVector<CirclePtr> & circs)
{
    qInfo() << "Loading template" << xfile.getPathedName();

    xml_document doc;

    xml_parse_result result = doc.load_file(xfile.getPathedName().toStdString().c_str());
    if (result == false)
    {
        qWarning() << result.description();
        return false;
    }

    xml_node node = doc.child("template");
    if (!node)
    {
        qWarning() << "Invalid templa4e XML file";
        return false;
    }

    xml_node node2 = node.child("lines");
    if (node2)
    {
        for (xml_node node3 = node2.child("line"); node3; node3 = node3.next_sibling())
        {
            QString line = node3.child_value();
            QStringList l = line.split(',');
            QLineF aline(l[0].toDouble(),l[1].toDouble(),l[2].toDouble(),l[3].toDouble());
            lines.push_back(aline);
        }
    }

    node2 = node.child("circles");
    if (node2)
    {
        for (xml_node node3 = node2.child("circle"); node3; node3 = node3.next_sibling())
        {
            qreal x = 0;
            qreal y = 0;
            qreal radius = 1;
            xml_attribute attr = node3.attribute("radius");
            if (attr)
            {
                QString str = attr.value();
                radius = str.toDouble();
            }
            attr = node3.attribute("centreX");
            if (attr)
            {
                QString str = attr.value();
                x = str.toDouble();
            }
            attr = node3.attribute("centreY");
            if (attr)
            {
                QString str = attr.value();
                y = str.toDouble();
            }
            CirclePtr c = std::make_shared<Circle>(QPointF(x,y),radius);
            circs.push_back(c);
        }
    }

    qInfo() << "Template loaded" << xfile.getPathedName();
    return true;
}

bool MapEditorStash::initStash(VersionedName mosaicname, MapEditorDb * db)
{
    VersionedFile xfile = FileServices::getFile(mosaicname,FILE_TEMPLATE2);
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

    VersionedFile xfile = FileServices::getFile(vname,FILE_TEMPLATE2);
    if (xfile.isEmpty())
    {
        QString pname = Sys::templateDir + vname.get() + ".xml";
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
    QString name = QString("constructionstash%1.xml").arg(QString::number(index));
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
