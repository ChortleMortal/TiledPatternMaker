#include <QMessageBox>
#include "model/tilings/tiling_manager.h"
#include "model/makers/prototype_maker.h"
#include "model/makers/tiling_maker.h"
#include "sys/sys/fileservices.h"
#include "sys/sys.h"
#include "gui/top/controlpanel.h"
#include "model/settings/canvas_settings.h"
#include "model/tilings/tiling.h"
#include "model/tilings/tiling_reader.h"
#include "model/tilings/tiling_writer.h"
#include "gui/top/view_controller.h"

TilingPtr TilingManager::loadTiling(VersionedFile &vfile, eTILM_Event event)
{
    QString name = vfile.getVersionedName().get();

    TilingPtr tiling;
    
    if (vfile.isEmpty())
    {
        qWarning() << "No tiling found with name" << name;
        QMessageBox box(Sys::controlPanel);
        box.setIcon(QMessageBox::Critical);
        box.setText(QString("No tiling found with name <%1>not found").arg(name));
        box.exec();
        return tiling;
    }
    
    qInfo().noquote() << "TilingManager::loadTiling" << vfile.getPathedName() << sTILM_Events[event];
    
    TilingReader tm;
    tiling  = tm.readTilingXML(vfile);

    if (!tiling)
    {
        qWarning().noquote() << "Error loading" << name;

        QMessageBox box(Sys::controlPanel);
        box.setIcon(QMessageBox::Critical);
        box.setText(QString("Load Error: <%1>").arg(name));
        box.exec();
        return tiling;
    }

    qInfo().noquote() << "Loaded  tiling:" << tiling->getName().get();

    return tiling;
}

bool TilingManager::saveTiling(TilingPtr tiling)
{
    // match size to current view
    auto & canvas = Sys::viewController->getCanvas();
    QSize size    = Sys::view->getSize();
    QSize zsize   = canvas.getSize();
    
    const CanvasSettings & cs = tiling->getData().getCanvasSettings();
    if (cs.getViewSize() != size || cs.getCanvasSize() != zsize)
    {
        CanvasSettings settings = tiling->getCanvasSettings();
        settings.setViewSize(size);
        settings.setCanvasSize(zsize);
        tiling->setCanvasSettings(settings);
    }

    if (Sys::tilingMaker->getSelected() == tiling)
    {
        Xform xf = Sys::tilingMakerView->getModelXform();
        tiling->setModelXform(xf,false);
    }

    // write
    TilingWriter writer;
    bool rv = writer.writeTilingXML(tiling);   // uses the name in the tiling
    return rv;
}

bool TilingManager::verifyTilingFiles()
{
    bool rv = true;
    VersionFileList files = FileServices::getTilingFiles(ALL_TILINGS);
    for (VersionedFile & file : files)
    {
        TilingPtr tiling = loadTiling(file,TILM_LOAD_SINGLE);
        if (tiling->getName().get() != file.getVersionedName().get())
        {
            qWarning() << "Error: name does not match filename =" << file.getVersionedName().get() <<"internal name= " << tiling->getName().get();
            rv = false;
        }
        if (!FileServices::verifyTilingFile(file))
        {
            qWarning() << "Error: name does not match filename =" << file.getVersionedName().get();
            rv = false;
        }
    }
    return rv;
}

