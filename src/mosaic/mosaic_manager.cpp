#include <QMessageBox>

#include "mosaic/mosaic_manager.h"
#include "makers/mosaic_maker/mosaic_maker.h"
#include "misc/fileservices.h"
#include "mosaic/mosaic.h"
#include "mosaic/mosaic_reader.h"
#include "mosaic/mosaic_writer.h"
#include "panels/controlpanel.h"
#include "settings/configuration.h"
#include "settings/model_settings.h"
#include "viewers/viewcontrol.h"
#include "widgets/version_dialog.h"

MosaicManager::MosaicManager()
{
    view            = ViewControl::getInstance();
    config          = Configuration::getInstance();
    mosaicMaker     = MosaicMaker::getInstance();
}

bool MosaicManager::loadMosaic(QString name)
{
    qDebug().noquote() << "MosaicManager::loadMosaic()" << name;

    QString file = FileServices::getMosaicXMLFile(name);
    if (file.isEmpty())
    {
        QMessageBox box(ControlPanel::getInstance());
        box.setIcon(QMessageBox::Critical);
        box.setText(QString("File <%1>not found").arg(name));
        box.exec();
        return false;
    }

    QFile afile(file);
    if (!afile.exists())
    {
        QMessageBox box(ControlPanel::getInstance());
        box.setIcon(QMessageBox::Critical);
        box.setText(QString("File <%1>not found").arg(file));
        box.exec();
        return false;
    }

    qDebug().noquote() << "Loading:"  << file;
    LoadUnit & loadunit = view->getLoadUnit();
    loadunit.name = name;
    loadunit.loadTimer.restart();

    // load
    MosaicReader loader;
    MosaicPtr mosaic = loader.readXML(file);

    if (!mosaic)
    {
        QString str = QString("Load ERROR - %1").arg(loader.getFailMessage());
        QMessageBox box(ControlPanel::getInstance());
        box.setIcon(QMessageBox::Critical);
        box.setText(str);
        box.exec();
        return false;
    }

    mosaic->setName(name);

    // starts the chain reaction
    mosaicMaker->sm_takeDown(mosaic);

    // size view to mosaic
    auto & settings = view->getViewSettings();
    settings.reInit();
    settings.setModelAlignment(M_ALIGN_MOSAIC);

    ModelSettings & model = mosaic->getSettings();
    settings.initialiseCommon(model.getSize(),model.getZSize());

    return true;
}

bool MosaicManager::saveMosaic(QString name, QString & savedName, bool forceOverwrite)
{
    MosaicPtr mosaic = mosaicMaker->getMosaic();
    if (!mosaic)
    {
        QMessageBox box(ControlPanel::getInstance());
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

            VersionDialog msgBox(ControlPanel::getInstance());
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
                    filename = config->originalMosaicDir + name + ".xml";
                else if (isNew)
                    filename = config->newMosaicDir + name + ".xml";
                else if (isTest)
                    filename = config->testMosiacDir + name + ".xml";
            } break;
            }
        }
        else
        {
            if (config->saveMosaicTest)
                filename = config->testMosiacDir + name + ".xml";
            else
                filename = config->newMosaicDir + name + ".xml";
        }
    }

    savedName = name;

    qDebug() << "Saving XML to:"  << filename;

    // match size of mosaic view
    auto & settings = view->getViewSettings();
    QSize size      = settings.getCropSize(VIEW_MOSAIC);
    QSize zsize     = settings.getZoomSize(VIEW_MOSAIC);
    mosaic->getSettings().setSize(size);
    mosaic->getSettings().setZSize(zsize);

    // write
    MosaicWriter writer;
    bool rv = writer.writeXML(filename,mosaic);

    if (rv)
       settings.setModelAlignment(M_ALIGN_MOSAIC);

    if (!forceOverwrite)
    {
        QMessageBox box(ControlPanel::getInstance());
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
