#include <QMessageBox>

#include "gui/top/controlpanel.h"       // required
#include "gui/widgets/version_dialog.h"
#include "model/makers/mosaic_maker.h"
#include "model/mosaics/mosaic.h"
#include "model/mosaics/mosaic_manager.h"
#include "model/mosaics/mosaic_reader.h"
#include "model/mosaics/mosaic_writer.h"
#include "model/settings/configuration.h"
#include "sys/sys.h"
#include "sys/sys/fileservices.h"

// This is a GUI wropper for MosaciReader
// Called by MosaicMaker which handles signals/slots
MosaicPtr MosaicManager::loadMosaic(VersionedFile vfile)
{
    qDebug().noquote() << "MosaicManager::loadMosaic()" << vfile.getVersionedName().get();

    MosaicPtr mosaic;

    if (vfile.isEmpty())
    {
        QMessageBox box(Sys::controlPanel);
        box.setIcon(QMessageBox::Critical);
        box.setText(QString("File <%1>not found").arg(vfile.getVersionedName().get()));
        box.exec();
        return mosaic;
    }

    QFile afile(vfile.getPathedName());
    if (!afile.exists())
    {
        QMessageBox box(Sys::controlPanel);
        box.setIcon(QMessageBox::Critical);
        box.setText(QString("File <%1>not found").arg(vfile.getPathedName()));
        box.exec();
        return mosaic;
    }

    qDebug().noquote() << "Loading:"  << vfile.getPathedName();

    // load
    MosaicReader reader(Sys::viewController);
    mosaic = reader.readXML(vfile);

    if (!mosaic)
    {
        QString str = QString("Load ERROR - %1").arg(reader.getFailMessage());
        QMessageBox box(Sys::controlPanel);
        box.setIcon(QMessageBox::Critical);
        box.setText(str);
        box.exec();
        return mosaic;
    }

    mosaic->setName(vfile.getVersionedName());

    mosaic->dumpTransforms();

    return mosaic;
}

bool MosaicManager::saveMosaic(MosaicPtr mosaic, VersionedFile & rvSavedFile, bool forceOverwrite)
{
    VersionedName vname = mosaic->getName();

    VersionedFile mosaicFile = FileServices::getFile(vname,FILE_MOSAIC);
    if (!forceOverwrite)
    {
        if (!mosaicFile.isEmpty())
        {
            QString fname   = mosaicFile.getPathedName();
            bool isOriginal = fname.contains("original");
            bool isNew      = fname.contains("new_");
            bool isTest     = fname.contains("tests");

            VersionDialog msgBox(Sys::controlPanel);
            QString str = QString("Mosaic <%1> already exists.").arg(mosaicFile.getVersionedName().get());
            msgBox.setText1(str);
            int rv = msgBox.exec();

            switch (rv)
            {
            case QDialog::Rejected:
                return false;

            case QDialog::Accepted:
                break;

            default:
            {
                // bumps version
                int ver = msgBox.combo->currentIndex();
                if (ver == 0)
                {
                    vname = FileServices::getNextVersion(FILE_MOSAIC,vname);
                }
                else
                {
                    vname.set(vname.getUnversioned() + ".v" + QString::number(ver));
                }
                if (isOriginal)
                    mosaicFile.setFromFullPathname(Sys::originalMosaicDir + vname.get() + ".xml");
                else if (isNew)
                    mosaicFile.setFromFullPathname(Sys::newMosaicDir + vname.get() + ".xml");
                else if (isTest)
                    mosaicFile.setFromFullPathname(Sys::testMosiacDir + vname.get() + ".xml");
            } break;
            }
        }
        else
        {
            if (Sys::config->saveMosaicTest)
            {
                mosaicFile.setFromFullPathname(Sys::testMosiacDir + vname.get() + ".xml");
            }
            else
            {
                mosaicFile.setFromFullPathname(Sys::newMosaicDir + vname.get() + ".xml");
            }
        }
    }

    rvSavedFile = mosaicFile;

    qDebug() << "Saving XML to:"  << mosaicFile.getPathedName();

    // write
    MosaicWriter writer;
    bool rv = writer.writeXML(mosaicFile,mosaic);

    if (!forceOverwrite)
    {
        QMessageBox box(Sys::controlPanel);
        QString astring;
        if (rv)
        {
            astring = "Mosaic (" + mosaicFile.getVersionedName().get() + ") - saved OK";
            mosaic->setName(vname);
        }
        else
        {
            QString str = writer.getFailMsg();
            astring = QString("Save File (%1) FAILED %2").arg(mosaicFile.getPathedName()).arg(str);
            box.setIcon(QMessageBox::Critical);
        }
        box.setText(astring);
        box.exec();
    }
    return rv;
}
