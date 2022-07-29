#include <QMessageBox>

#include "mosaic/mosaic_manager.h"
#include "misc/fileservices.h"
#include "mosaic/mosaic_reader.h"
#include "mosaic/mosaic_writer.h"
#include "viewers/viewcontrol.h"
#include "panels/panel.h"
#include "makers/mosaic_maker/mosaic_maker.h"
#include "settings/model_settings.h"
#include "mosaic/mosaic.h"
#include "settings/configuration.h"

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
    mosaicMaker->takeDown(mosaic);

    // size view to mosaic
    view->frameSettings.reInit();
    view->frameSettings.setModelAlignment(M_ALIGN_MOSAIC);

    ModelSettings & model = mosaic->getSettings();
    view->frameSettings.initialiseCommon(model.getSize(),model.getZSize());

    return true;
}

bool MosaicManager::saveMosaic(QString name, QString & savedName, bool forceOverwrite, bool forceTest)
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

            QMessageBox msgBox(ControlPanel::getInstance());
            QString str = QString("The XML design file <%1> already exists").arg(filename);
            msgBox.setText(str);
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
                name = FileServices::getNextVersion(FILE_MOSAIC,name);
                if (forceTest)
                    filename = config->testDesignDir + name + ".xml";
                else if (isOriginal)
                    filename = config->originalDesignDir + name + ".xml";
                else if (isNew)
                    filename = config->newDesignDir + name + ".xml";
                else
                {
                    Q_ASSERT(isTest);
                    filename = config->testDesignDir + name + ".xml";
                }
            }
            // save drops thru
            Q_UNUSED(save)
        }
        else
        {
            if (forceTest)
                filename = config->testDesignDir + name + ".xml";
            else
                filename = config->newDesignDir + name + ".xml";
        }
    }

    savedName = name;

    qDebug() << "Saving XML to:"  << filename;

    // match size of mosaic view
    QSize size  = view->frameSettings.getCropSize(VIEW_MOSAIC);
    QSize zsize = view->frameSettings.getZoomSize(VIEW_MOSAIC);
    mosaic->getSettings().setSize(size);
    mosaic->getSettings().setZSize(zsize);

    // write
    MosaicWriter writer;
    bool rv = writer.writeXML(filename,mosaic);

    if (rv)
        view->frameSettings.setModelAlignment(M_ALIGN_MOSAIC);

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
