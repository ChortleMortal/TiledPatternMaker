#include <QProcess>
#include <QMessageBox>
#include "engine/image_engine.h"
#include "engine/version_stepper.h"
#include "makers/mosaic_maker/mosaic_maker.h"
#include "makers/tiling_maker/tiling_maker.h"
#include "misc/fileservices.h"
#include "misc/sys.h"
#include "mosaic/mosaic.h"
#include "panels/controlpanel.h"
#include "settings/configuration.h"
#include "viewers/view.h"

////////////////////////////////////////////
///
///  VersionStepper
///
////////////////////////////////////////////

VersionStepper::VersionStepper(ImageEngine * parent) : SteppingEngine(parent) {}

void VersionStepper::connect(QComboBox * mediaA, QComboBox * mediaB, QComboBox * versionsA, QComboBox * versionsB)
{
    this->mediaA    = mediaA;
    this->mediaB    = mediaB;
    this->versionsA = versionsA;
    this->versionsB = versionsB;
}

bool VersionStepper::begin()
{
    if (isStarted())
    {
        end();
        return false;
    }

    start(true);
    setPaused (false);

    qInfo() << "Version Stepper started";

    // get root names
    QString rootDir;
    if (config->vCompTile)
    {
        mediaNames = FileServices::getTilingRootNames(config->versionFilter);
        rootDir    = Sys::rootTileDir;
    }
    else
    {
        mediaNames = FileServices::getMosaicRootNames(config->versionFilter);
        rootDir    = Sys::rootMosaicDir;
    }

    imgList_it = mediaNames.begin();

    // get first name
    auto mediaName = *imgList_it;
    int index = mediaA->findText(mediaName);
    mediaA->setCurrentIndex(index);
    mediaB->setCurrentIndex(index);

    // get versions
    versions = FileServices::getFileVersions(mediaName,rootDir);
    imgListVerA_it = versions.begin();
    imgListVerB_it = imgListVerA_it;
    imgListVerB_it++;
    if (imgListVerB_it == versions.end())
    {
        return next();
    }

    // compare first two versions
    auto vera = *imgListVerA_it;
    index = versionsA->findText(vera);
    versionsA->setCurrentIndex(index);
    auto verb = *imgListVerB_it;
    index = versionsB->findText(verb);
    versionsB->setCurrentIndex(index);

    compareVersions();

    emit parent->sig_ready();
    return true;
}

bool VersionStepper::tick()
{
    return false;
}

bool VersionStepper::next()
{
    if (!isStarted()) return false;

    if (imgListVerB_it == versions.end() || ++imgListVerB_it == versions.end())
    {
    next_mosaic:
        // go to next mosaic
        if (++imgList_it == mediaNames.end())
        {
            // we are done
            QMessageBox box(panel);
            box.setText("Version comparison complete");
            box.setIcon(QMessageBox::Information);
            box.setStandardButtons(QMessageBox::Ok);
            box.exec();

            return false;
        }

        auto mediaName = *imgList_it;
        int index = mediaA->findText(mediaName);
        mediaA->setCurrentIndex(index);
        mediaB->setCurrentIndex(index);

        if (config->vCompTile)
        {
            versions = FileServices::getFileVersions(mediaName,Sys::rootTileDir);
        }
        else
        {
            versions = FileServices::getFileVersions(mediaName,Sys::rootMosaicDir);
        }
        imgListVerA_it = versions.begin();
        imgListVerB_it = imgListVerA_it;
        imgListVerB_it++;
        if (imgListVerB_it == versions.end())
        {
            // there is no other version to compare to
            goto next_mosaic;
        }

        auto vera = *imgListVerA_it;
        index = versionsA->findText(vera);
        versionsA->setCurrentIndex(index);
        auto verb = *imgListVerB_it;
        index = versionsB->findText(verb);
        versionsB->setCurrentIndex(index);

        compareVersions();
    }
    else
    {
        // go to next version
        imgListVerA_it++;

        auto vera = *imgListVerA_it;
        int index = versionsA->findText(vera);
        versionsA->setCurrentIndex(index);
        auto verb = *imgListVerB_it;
        index = versionsB->findText(verb);
        versionsB->setCurrentIndex(index);

        compareNextVersions();
    }

    emit parent->sig_ready();

    return true;
}

bool VersionStepper::prev()
{
    return false;
}

bool VersionStepper:: end()
{
    qInfo() << "Version Stepper ended";
    if (isStarted())
    {
        start(false);
        finish("Version comparisons");
        return true;
    }
    else
    {
        return false;
    }
}

void VersionStepper::compareVersions()
{
    QString nameA = versionsA->currentText();
    QString nameB = versionsB->currentText();

    if (config->vCompTile)
    {
        // tiling
        if (config->vCompXML)
        {
            auto tileA = FileServices::getTilingXMLFile(nameA);
            auto tileB = FileServices::getTilingXMLFile(nameB);
            QStringList qsl;
            qsl << tileA << tileB;
            QProcess::startDetached(Configuration::getInstance()->diffTool,qsl);
        }
        else
        {
            auto maker = TilingMaker::getInstance();
            maker->loadTiling(nameA,TILM_LOAD_SINGLE);
            auto pixA = Sys::view->grab();

            maker->loadTiling(nameB,TILM_LOAD_SINGLE);
            auto pixB = Sys::view->grab();

            imgA = pixA.toImage();
            imgB = pixB.toImage();
            emit parent->sig_closeAllImageViewers();
            parent->compareImages(imgA,imgB,nameA,nameB,nameA,nameB,false);
        }
    }
    else
    {
        // mosaic
        if (config->vCompXML)
        {
            auto mosA = FileServices::getMosaicXMLFile(nameA);
            auto mosB = FileServices::getMosaicXMLFile(nameB);
            QStringList qsl;
            qsl << mosA << mosB;
            QProcess::startDetached(config->diffTool,qsl);
        }
        else
        {
            auto maker   = MosaicMaker::getInstance();

            auto mosaicA = maker->loadMosaic(nameA);
            auto pixA    = Sys::view->grab();

            auto mosaicB = maker->loadMosaic(nameB);
            auto pixB    = Sys::view->grab();

            if (mosaicA)
            {
                mosaicA->reportMotifs();
                mosaicA->reportStyles();
            }
            if (mosaicB)
            {
                mosaicB->reportMotifs();
                mosaicB->reportStyles();
            }
            imgA = pixA.toImage();
            imgB = pixB.toImage();
            emit parent->sig_closeAllImageViewers();
            parent->compareImages(imgA,imgB,nameA,nameB,nameA,nameB,false);
        }
    }
}

void VersionStepper::compareNextVersions()
{
    // this assumes imgB can bes used as the new img
    imgA = imgB;

    auto maker        = MosaicMaker::getInstance();
    MosaicPtr mosaicA = maker->getMosaic();
    QString mosA      = versionsA->currentText();
    QString mosB      = versionsB->currentText();

    maker->loadMosaic(mosB);
    QPixmap pixB = Sys::view->grab();
    MosaicPtr mosaicB = maker->getMosaic();

    mosaicA->reportMotifs();
    mosaicA->reportStyles();
    mosaicB->reportMotifs();
    mosaicB->reportStyles();

    imgB = pixB.toImage();

    emit parent->sig_closeAllImageViewers();
    parent->compareImages(imgA,imgB,mosA,mosB,mosA,mosB,false);
}

void VersionStepper::loadVersionCombos()
{
    eLoadType loadType = config->versionFilter;

    mediaA->blockSignals(true);
    mediaB->blockSignals(true);

    mediaA->clear();
    mediaB->clear();

    if (config->vCompTile)
    {
        mediaNames = FileServices::getTilingRootNames(loadType);
    }
    else
    {
        mediaNames = FileServices::getMosaicRootNames(loadType);
    }

    mediaA->addItems(mediaNames);
    mediaB->addItems(mediaNames);

    int index = mediaA->findText(config->lastCompareName);
    if (index < 0) index = 0;

    mediaA->setCurrentIndex(index);
    mediaB->setCurrentIndex(index);

    mediaA->blockSignals(false);
    mediaB->blockSignals(false);

    mediaAChanged();
    if (!config->vCompLock)
    {
        mediaBChanged();
    }
}

void VersionStepper::mediaAChanged()
{
    // the media a selection has changed, do sets versions and set index to 0
    versionsA->clear();

    auto name = mediaA->currentText();

    config->lastCompareName = name;

    if (config->vCompTile)
    {
        versions = FileServices::getFileVersions(name,Sys::rootTileDir);
    }
    else
    {
        versions = FileServices::getFileVersions(name,Sys::rootMosaicDir);
    }

    if (versions.isEmpty())
        return;

    versionsA->addItems(versions);
    versionsA->setCurrentIndex(0);

    // if there is a lock, similary select the media, add its versions and set it to 1
    if (config->vCompLock)
    {
        mediaB->blockSignals(true);
        mediaB->setCurrentIndex(mediaA->currentIndex());
        mediaB->blockSignals(false);

        versionsB->clear();
        versionsB->addItems(versions);
        versionsB->setCurrentIndex(1);
    }
}

void VersionStepper::mediaBChanged()
{
    // media be has changed, set its versions and if there is no lock select index 0 else select index 1
    versionsB->clear();

    auto name = mediaB->currentText();

    QStringList qsl;
    if (config->vCompTile)
    {
        qsl = FileServices::getFileVersions(name,Sys::rootTileDir);
    }
    else
    {
        qsl = FileServices::getFileVersions(name,Sys::rootMosaicDir);
    }

    if (qsl.isEmpty())
        return;

    versionsB->addItems(qsl);
    versionsB->setCurrentIndex(0);

    // if there is a lock put the same versions into A amd select version 0
    if (config->vCompLock)
    {
        versions = qsl;

        versionsB->setCurrentIndex(1);

        mediaA->blockSignals(true);
        mediaA->setCurrentIndex(mediaB->currentIndex());
        mediaA->blockSignals(false);

        versionsA->clear();
        versionsA->addItems(versions);
        versionsA->setCurrentIndex(0);
    }
}
