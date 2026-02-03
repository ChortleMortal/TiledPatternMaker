#include <QFile>
#include <QTextStream>
#include <QTextEdit>

#include "gui/widgets/dlg_textedit.h"
#include "gui/top/system_view.h"
#include "gui/panels/shortcuts.h"

void  Shortcuts::popup(eViewType view)
{
    DlgTextEdit * ted = new DlgTextEdit(Sys::sysview);
    ted->setWindowTitle("Shortcuts");
    switch(view)
    {
    case VIEW_LEGACY:
        ted->setHtml(getLegacyShortcuts());
        break;

    case VIEW_TILING_MAKER:
        ted->setHtml(getTilingMakerShortcuts());
        break;

    case VIEW_MAP_EDITOR:
        ted->setHtml(getMapEditorShortcuts());
        break;

    case VIEW_MOSAIC:
    default:
        ted->setHtml(getGeneralShortcuts());
        break;
    }
    ted->show();
    ted->exec();
}


QString Shortcuts::getGeneralShortcuts()
{
    QString name(":/txt/mosaic.txt");
    QFile data(name);
    if (data.open(QFile::ReadOnly | QFile::Text))
    {
        QTextStream in(&data);
        QString rv = in.readAll();
        data.close();
        return rv;
    }
    else
    {
        QString astring;
        QTextStream ts(&astring);
        ts << "Coud not open" << name << data.errorString();
        return  astring;
    }
}

QString Shortcuts::getLegacyShortcuts()
{
    QString name(":/txt/legacy.txt");
    QFile data(name);
    if (data.open(QFile::ReadOnly | QFile::Text))
    {
        QTextStream in(&data);
        QString rv = in.readAll();
        data.close();
        return rv;
    }
    else
    {
        QString astring;
        QTextStream ts(&astring);
        ts << "Coud not open" << name << data.errorString();
        return  astring;
    }
}

QString Shortcuts::getTilingMakerShortcuts()
{
    QString name(":/txt/tilingmaker.txt");
    QFile data(name);
    if (data.open(QFile::ReadOnly | QFile::Text))
    {
        QTextStream in(&data);
        QString rv = in.readAll();
        data.close();
        return rv;
    }
    else
    {
        QString astring;
        QTextStream ts(&astring);
        ts << "Coud not open" << name << data.errorString();
        return  astring;
    }
}

QString Shortcuts::getMapEditorShortcuts()
{
    QString name(":/txt/mapeditor.txt");
    QFile data(name);
    if (data.open(QFile::ReadOnly | QFile::Text))
    {
        QTextStream in(&data);
        QString rv = in.readAll();
        data.close();
        return  rv;
    }
    else
    {
        QString astring;
        QTextStream ts(&astring);
        ts << "Coud not open" << name << data.errorString();
        return  astring;
    }
}
