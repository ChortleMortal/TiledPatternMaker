#include <QMessageBox>
#include "tile/tiling_manager.h"
#include "misc/fileservices.h"
#include "makers/prototype_maker/prototype_maker.h"
#include "makers/tiling_maker/tiling_maker.h"
#include "misc/sys.h"
#include "panels/controlpanel.h"
#include "settings/canvas_settings.h"
#include "settings/configuration.h"
#include "tile/tiling.h"
#include "tile/tiling_reader.h"
#include "tile/tiling_writer.h"
#include "viewers/backgroundimageview.h"
#include "viewers/view_controller.h"

TilingManager::TilingManager()
{
    viewController  = Sys::viewController;
    config          = Configuration::getInstance();
    tilingMaker     = TilingMaker::getInstance();
    prototypeMaker  = PrototypeMaker::getInstance();
}

TilingPtr TilingManager::loadTiling(QString name, eTILM_Event event)
{
    TilingPtr tiling;
    
    QString filename = FileServices::getTilingXMLFile(name);
    if (filename.isEmpty())
    {
        qWarning() << "No tiling found with name" << name;
        QMessageBox box(ControlPanel::getInstance());
        box.setIcon(QMessageBox::Critical);
        box.setText(QString("No tiling found with name <%1>not found").arg(name));
        box.exec();
        return tiling;
    }
    
    qInfo().noquote() << "TilingManager::loadTiling" << filename << sTILM_Events[event];
    
    TilingReader tm;
    tiling  = tm.readTilingXML(filename);

    if (!tiling)
    {
        qWarning().noquote() << "Error loading" << filename;

        QMessageBox box(ControlPanel::getInstance());
        box.setIcon(QMessageBox::Critical);
        box.setText(QString("Load Error: <%1>").arg(name));
        box.exec();
        return tiling;
    }

    qInfo().noquote() << "Loaded  tiling:" << filename << tiling->getTitle();

    return tiling;
}

bool TilingManager::saveTiling(QString name, TilingPtr tiling)
{
    if (tiling->getTitle() != name)
    {
        tiling->setTitle(name);
    }

    // match size to current view
    auto & canvas = viewController->getCanvas();
    QSize size    = Sys::view->getCurrentSize();
    QSizeF zsize  = canvas.getSize();
    
    const CanvasSettings & cs = tiling->getData().getSettings();
    if (cs.getViewSize() != size || cs.getCanvasSize() != zsize)
    {
        CanvasSettings settings = tiling->getCanvasSettings();
        settings.setViewSize(size);
        settings.setCanvasSize(zsize);
        tiling->setCanvasSettings(settings);
    }

    if (tilingMaker->getSelected() == tiling)
    {
        Xform xf = TilingMakerView::getInstance()->getModelXform();
        tiling->setModelXform(xf,false);
    }

    // write
    TilingWriter writer(tiling);
    bool rv = writer.writeTilingXML();   // uses the name in the tiling
    return rv;
}

bool TilingManager::verifyNameFiles()
{
    bool rv = true;
    QStringList files = FileServices::getTilingNames(ALL_TILINGS);
    for (int i=0; i < files.size(); i++)
    {
        QString name = files[i];
        TilingPtr tiling = loadTiling(name,TILM_LOAD_SINGLE);
        if (tiling->getTitle() != name)
        {
            qWarning() << "Error: name does not match filename =" << name <<"internal name= " << tiling->getTitle();
            rv = false;
        }
        if (!FileServices::verifyTilingName(name))
        {
            qWarning() << "Error: name does not match filename =" << name;
            rv = false;
        }
    }
    return rv;
}

