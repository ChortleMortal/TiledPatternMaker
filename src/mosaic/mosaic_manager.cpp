#include <QMessageBox>

#include "mosaic/mosaic_manager.h"
#include "makers/mosaic_maker/mosaic_maker.h"
#include "misc/fileservices.h"
#include "mosaic/mosaic.h"
#include "mosaic/mosaic_reader.h"
#include "mosaic/mosaic_writer.h"
#include "misc/sys.h"
#include "panels/controlpanel.h"
#include "settings/configuration.h"
#include "viewers/view.h"
#include "widgets/version_dialog.h"

MosaicManager::MosaicManager()
{
    view            = Sys::view;
    viewControl     = Sys::viewController;
    config          = Sys::config;
    mosaicMaker     = Sys::mosaicMaker;
}

// This is a GUI wropper for MosaciReader
// which also calls sm_takeDown
MosaicPtr MosaicManager::loadMosaic(QString name)
{
    qDebug().noquote() << "MosaicManager::loadMosaic()" << name;

    MosaicPtr mosaic;

    QString file = FileServices::getMosaicXMLFile(name);
    if (file.isEmpty())
    {
        QMessageBox box(Sys::controlPanel);
        box.setIcon(QMessageBox::Critical);
        box.setText(QString("File <%1>not found").arg(name));
        box.exec();
        return mosaic;
    }

    QFile afile(file);
    if (!afile.exists())
    {
        QMessageBox box(Sys::controlPanel);
        box.setIcon(QMessageBox::Critical);
        box.setText(QString("File <%1>not found").arg(file));
        box.exec();
        return mosaic;
    }

    qDebug().noquote() << "Loading:"  << file;
    LoadUnit & loadunit = view->getLoadUnit();
    loadunit.loadTimer.restart();

    // load
    MosaicReader reader;
    mosaic = reader.readXML(file);

    if (!mosaic)
    {
        QString str = QString("Load ERROR - %1").arg(reader.getFailMessage());
        QMessageBox box(Sys::controlPanel);
        box.setIcon(QMessageBox::Critical);
        box.setText(str);
        box.exec();
        return mosaic;
    }

    mosaic->setName(name);

    return mosaic;
}

bool MosaicManager::saveMosaic(QString name, QString & savedName, bool forceOverwrite)
{
    MosaicPtr mosaic = mosaicMaker->getMosaic();
    if (!mosaic)
    {
        QMessageBox box(Sys::controlPanel);
        box.setIcon(QMessageBox::Critical);
        box.setText("Save FAILED: There is no mosaic to save");
        box.exec();
        return false;
    }

    QString filename = FileServices::getMosaicXMLFile(name);
    if (!forceOverwrite)
    {
        if (!filename.isEmpty())
        {
            bool isOriginal = filename.contains("original");
            bool isNew      = filename.contains("new_");
            bool isTest     = filename.contains("tests");

            VersionDialog msgBox(Sys::controlPanel);
            QString str = QString("The XML design file <%1> already exists.").arg(filename);
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
                    name = FileServices::getNextVersion(FILE_MOSAIC,name);
                }
                else
                {
                    name = name + ".v" + QString::number(ver);
                }
                if (isOriginal)
                    filename = Sys::originalMosaicDir + name + ".xml";
                else if (isNew)
                    filename = Sys::newMosaicDir + name + ".xml";
                else if (isTest)
                    filename = Sys::testMosiacDir + name + ".xml";
            } break;
            }
        }
        else
        {
            if (config->saveMosaicTest)
                filename = Sys::testMosiacDir + name + ".xml";
            else
                filename = Sys::newMosaicDir + name + ".xml";
        }
    }

    savedName = name;

    qDebug() << "Saving XML to:"  << filename;

    // write
    MosaicWriter writer;
    bool rv = writer.writeXML(filename,mosaic);

    if (!forceOverwrite)
    {
        QMessageBox box(Sys::controlPanel);
        QString astring;
        if (rv)
        {
            astring ="File (" +  filename + ") - saved OK";
            mosaic->setName(name);
        }
        else
        {
            QString str = writer.getFailMsg();
            astring = QString("Save File (%1) FAILED %2").arg(filename).arg(str);
            box.setIcon(QMessageBox::Critical);
        }
        box.setText(astring);
        box.exec();
    }
    return rv;
}
