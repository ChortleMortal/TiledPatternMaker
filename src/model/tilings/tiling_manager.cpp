#include <QMessageBox>
#include "gui/top/controlpanel.h"
#include "gui/top/system_view_controller.h"
#include "model/makers/tiling_maker.h"
#include "model/mosaics/reader_base.h"
#include "model/settings/canvas_settings.h"
#include "model/settings/configuration.h"
#include "model/tilings/tiling.h"
#include "model/tilings/tiling_manager.h"
#include "model/tilings/tiling_reader.h"
#include "model/tilings/tiling_writer.h"
#include "sys/sys.h"
#include "sys/sys/fileservices.h"

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
    
    TilingReader tm(Sys::viewController);
    ReaderBase mrbase;
    tiling  = tm.readTilingXML(vfile,&mrbase);

    if (!tiling)
    {
        qWarning().noquote() << "Error loading" << name;

        QMessageBox box(Sys::controlPanel);
        box.setIcon(QMessageBox::Critical);
        box.setText(QString("Load Error: <%1>").arg(name));
        box.exec();
        return tiling;
    }

    qInfo().noquote() << "Loaded  tiling:" << tiling->getVName().get();

    return tiling;
}

// called by TilingMaker::saveTiling (and for debug reprocessing of files)
bool TilingManager::saveTiling(TilingPtr tiling,VersionedFile & retSavedFile, bool forceOverwrite)
{
    // match size to current view
    auto & canvas = Sys::viewController->getCanvas();
    QSize size    = canvas.getViewSize();
    QSize zsize   = canvas.getCanvasSize();
    
    const CanvasSettings & cs = tiling->hdr().getCanvasSettings();
    if (cs.getViewSize() != size || cs.getCanvasSize() != zsize)
    {
        CanvasSettings settings = tiling->hdr().getCanvasSettings();
        settings.setViewSize(size);
        settings.setCanvasSize(zsize);
        tiling->hdr().setCanvasSettings(settings);
    }

    if (Sys::tilingMaker->getSelected() == tiling)
    {
        Xform xf = Sys::tilingMakerView->getModelXform();
        tiling->setModelXform(xf,false,Sys::nextSigid());
    }

    VersionedFile vfile = FileServices::getFile(tiling->getVName(),FILE_TILING);
    if (!forceOverwrite)
    {
        if (!vfile.isEmpty())
        {
            // file already exists
            bool isOriginal  = vfile.getPathOnly().contains("original");
            bool isNewTiling = vfile.getPathOnly().contains("new_tilings");

            QMessageBox msgBox(Sys::controlPanel);
            msgBox.setText(QString("The tiling %1 already exists").arg(tiling->getVName().get()));
            msgBox.setInformativeText("Do you want to bump version (Bump) or overwrite (Save)?");
            QPushButton * bump   = msgBox.addButton("Bump",QMessageBox::ApplyRole);
            QPushButton * save   = msgBox.addButton(QMessageBox::Save);
            QPushButton * cancel = msgBox.addButton(QMessageBox::Cancel);
            msgBox.setDefaultButton(bump);
            msgBox.exec();

            if (msgBox.clickedButton() == cancel)
            {
                return false;
            }
            else if (msgBox.clickedButton() == bump)
            {
                // appends a version
                VersionedName vname = FileServices::getNextVersion(FILE_TILING,tiling->getVName());
                tiling->setVName(vname);
                if (isOriginal)
                {
                    vfile.setFromFullPathname(Sys::originalTileDir + vname.get() + ".xml");
                }
                else if (isNewTiling)
                {
                    vfile.setFromFullPathname(Sys::newTileDir + vname.get() + ".xml");
                }
                else
                {
                    vfile.setFromFullPathname(Sys::testTileDir + vname.get() + ".xml");
                }
            }
            // save drops thru
            Q_UNUSED(save)
        }
        else
        {
            // new file
            if (Sys::config->saveTilingTest)
            {
                vfile.setFromFullPathname(Sys::testTileDir + tiling->getVName().get() + ".xml");
            }
            else
            {
                vfile.setFromFullPathname(Sys::newTileDir + tiling->getVName().get() + ".xml");
            }
        }
    }

    // write
    TilingWriter writer;
    bool rv = writer.writeTilingXML(vfile,tiling);   // uses the name in the tiling
    if (rv)
    {
        QMessageBox box(Sys::controlPanel);
        box.setIcon(QMessageBox::Information);
        box.setText(QString("Saved: %1 - OK").arg(vfile.getPathedName()));
        box.exec();
        retSavedFile = vfile;
    }
    else
    {
        QMessageBox box(Sys::controlPanel);
        box.setIcon(QMessageBox::Critical);
        box.setText(QString("Error saving: %1 - FAILED").arg(vfile.getPathedName()));
        box.exec();
    }
    return rv;
}

bool TilingManager::verifyTilingFiles()
{
    bool rv = true;
    VersionFileList files = FileServices::getTilingFiles(ALL_TILINGS);
    for (VersionedFile & file : files)
    {
        TilingPtr tiling = loadTiling(file,TILM_LOAD_SINGLE);
        if (tiling->getVName().get() != file.getVersionedName().get())
        {
            qWarning() << "Error: name does not match filename =" << file.getVersionedName().get() <<"internal name= " << tiling->getVName().get();
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

