#include <QDir>
#include <QFile>
#include <QDebug>
#include <QStandardPaths>

#include "gui/map_editor/map_editor_stash.h"
#include "gui/map_editor/map_editor_db.h"
#include "sys/sys/fileservices.h"
#include "sys/sys.h"
#include "sys/qt/tpm_io.h"
#include "sys/sys/pugixml.hpp"

using namespace pugi;

QString MapEditorStash::workingFile()
{
    QString dir = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    QDir().mkpath(dir);
    return dir + "/maped_construction.xml";
}

bool MapEditorStash::stash(MapEditorDb * db)
{
    return write(workingFile(),db);
}

bool MapEditorStash::restore(MapEditorDb * db)
{
    QString name = workingFile();
    if (!QFile::exists(name))
        return false;
    return read(name,db);
}

bool MapEditorStash::saveTemplate(VersionedName vname, MapEditorDb * db)
{
    VersionedFile xfile = FileServices::getFile(vname,FILE_TEMPLATE);
    QString name = xfile.isEmpty() ? Sys::templateDir + vname.get() + ".xml" : xfile.getPathedName();
    return write(name,db);
}

bool MapEditorStash::loadTemplate(VersionedFile xfile, MapEditorDb * db)
{
    bool rv = read(xfile.getPathedName(),db);
    if (rv)
        stash(db);
    return rv;
}

bool MapEditorStash::write(QString name, MapEditorDb * db)
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
    xfile.setFromFullPathname(name);
    return FileServices::reformatXML(xfile);
}

bool MapEditorStash::read(QString name, MapEditorDb * db)
{
    qInfo() << "Loading construction lines" << name;

    xml_document doc;

    xml_parse_result result = doc.load_file(name.toStdString().c_str());
    if (result == false)
    {
        qWarning() << result.description();
        return false;
    }

    xml_node node = doc.child("template");
    if (!node)
    {
        qWarning() << "Invalid template XML file";
        return false;
    }

    db->constructionLines.clear();
    db->constructionCircles.clear();
    db->selectedCLine = -1;

    xml_node node2 = node.child("lines");
    if (node2)
    {
        for (xml_node node3 = node2.child("line"); node3; node3 = node3.next_sibling())
        {
            QString line = node3.child_value();
            QStringList l = line.split(',');
            QLineF aline(l[0].toDouble(),l[1].toDouble(),l[2].toDouble(),l[3].toDouble());
            db->constructionLines.push_back(aline);
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
            db->constructionCircles.push_back(std::make_shared<Circle>(QPointF(x,y),radius));
        }
    }

    return true;
}
