#include <QFile>
#include <QTextStream>
#include <QTextEdit>

#include "gui/widgets/dlg_textedit.h"
#include "gui/top/view_controller.h"
#include "gui/panels/shortcuts.h"
#include "sys/sys.h"

void  Shortcuts::popup(Layer::Type type)
{
    DlgTextEdit * ted = new DlgTextEdit((QWidget*)Sys::systemViewController->getView());
    ted->setWindowTitle("Shortcuts");
    switch(type)
    {
    case Layer::Type::LegacyDesign:
        ted->setHtml(getLegacyShortcuts());
        break;

    case Layer::Type::TilingMaker:
        ted->setHtml(getTilingMakerShortcuts());
        break;

    case Layer::Type::MapEditor:
        ted->setHtml(getMapEditorShortcuts());
        break;

    case Layer::Type::Mosaic:
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
