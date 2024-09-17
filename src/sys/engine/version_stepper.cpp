#include <QProcess>
#include <QMessageBox>
#include "sys/engine/image_engine.h"
#include "sys/engine/version_stepper.h"
#include "model/makers/mosaic_maker.h"
#include "model/makers/tiling_maker.h"
#include "sys/sys/fileservices.h"
#include "sys/sys.h"
#include "model/mosaics/mosaic.h"
#include "gui/top/controlpanel.h"
#include "model/settings/configuration.h"
#include "gui/top/view.h"

////////////////////////////////////////////
///
///  VersionStepper
///
////////////////////////////////////////////

VersionStepper::VersionStepper(ImageEngine * parent) : SteppingEngine(parent)
{
}

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
    int index      = mediaA->findText(mediaName);
    mediaA->setCurrentIndex(index);
    mediaB->setCurrentIndex(index);

    // get versions
    bool use_wlist = (config->versionFilter == WORKLIST);
    versions = FileServices::getFileVersions(mediaName,rootDir,use_wlist);
    imgListVerA_it = versions.begin();
    imgListVerB_it = imgListVerA_it;
    imgListVerB_it++;
    if (imgListVerB_it == versions.end())
    {
        return next();
    }

    // compare first two versions
    VersionedName vera = *imgListVerA_it;
    index = versionsA->findText(vera.get());
    versionsA->setCurrentIndex(index);
    VersionedName verb = *imgListVerB_it;
    index = versionsB->findText(verb.get());
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

        bool use_wlist = (config->versionFilter == WORKLIST);
        if (config->vCompTile)
        {
            versions = FileServices::getFileVersions(mediaName,Sys::rootTileDir,use_wlist);
        }
        else
        {
            versions = FileServices::getFileVersions(mediaName,Sys::rootMosaicDir,use_wlist);
        }
        imgListVerA_it = versions.begin();
        imgListVerB_it = imgListVerA_it;
        imgListVerB_it++;
        if (imgListVerB_it == versions.end())
        {
            // there is no other version to compare to
            goto next_mosaic;
        }

        VersionedName vera = *imgListVerA_it;
        index = versionsA->findText(vera.get());
        versionsA->setCurrentIndex(index);
        VersionedName verb = *imgListVerB_it;
        index = versionsB->findText(verb.get());
        versionsB->setCurrentIndex(index);

        compareVersions();
    }
    else
    {
        // go to next version
        imgListVerA_it++;

        VersionedName vera = *imgListVerA_it;
        int index = versionsA->findText(vera.get());
        versionsA->setCurrentIndex(index);
        VersionedName verb = *imgListVerB_it;
        index = versionsB->findText(verb.get());
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
    VersionedName vna(nameA);

    QString nameB = versionsB->currentText();
    VersionedName vnb(nameB);

    if (config->vCompTile)
    {
        // tiling
        if (config->vCompXML)
        {
            VersionedFile tileA = FileServices::getFile(vna,FILE_TILING);
            VersionedFile tileB = FileServices::getFile(vnb,FILE_TILING);
            QStringList qsl;
            qsl << tileA.getPathedName() << tileB.getPathedName();
            QProcess::startDetached(config->diffTool,qsl);
        }
        else
        {
            auto maker = Sys::tilingMaker;
            VersionedFile vfa = FileServices::getFile(vna,FILE_TILING);
            maker->loadTiling(vfa,TILM_LOAD_SINGLE);
            auto pixA = Sys::view->grab();

            VersionedFile vfb = FileServices::getFile(vnb,FILE_TILING);
            maker->loadTiling(vfb,TILM_LOAD_SINGLE);
            auto pixB = Sys::view->grab();

            imgA = pixA.toImage();
            imgB = pixB.toImage();
            parent->closeAllImageViewers();
            VersionedFile fileA;
            fileA.updateFromVersionedName(vna);
            VersionedFile fileB;
            fileB.updateFromVersionedName(vnb);
            parent->compareImages(imgA,imgB,fileA,fileB);
        }
    }
    else
    {
        // mosaic
        if (config->vCompXML)
        {
            VersionedFile mosA = FileServices::getFile(vna,FILE_MOSAIC);
            VersionedFile mosB = FileServices::getFile(vnb,FILE_MOSAIC);
            QStringList qsl;
            qsl << mosA.getPathedName() << mosB.getPathedName();
            QProcess::startDetached(config->diffTool,qsl);
        }
        else
        {
            auto maker   = Sys::mosaicMaker;

            VersionedFile vfa = FileServices::getFile(vna,FILE_MOSAIC);
            MosaicPtr mosaicA = maker->loadMosaic(vfa);
            QPixmap pixA      = Sys::view->grab();

            VersionedFile vfb = FileServices::getFile(vnb,FILE_MOSAIC);
            MosaicPtr mosaicB = maker->loadMosaic(vfb);
            QPixmap pixB      = Sys::view->grab();

            if (mosaicA)
            {
                mosaicA->dumpMotifs();
                mosaicA->dumpStyles();
            }
            if (mosaicB)
            {
                mosaicB->dumpMotifs();
                mosaicB->dumpStyles();
            }
            imgA = pixA.toImage();
            imgB = pixB.toImage();

            VersionedFile fileA;
            fileA.updateFromVersionedName(vna);
            VersionedFile fileB;
            fileB.updateFromVersionedName(vnb);

            parent->closeAllImageViewers();
            parent->compareImages(imgA,imgB,fileA,fileB);
        }
    }
}

void VersionStepper::compareNextVersions()
{
    // this assumes imgB can bes used as the new img
    imgA = imgB;

    auto maker        = Sys::mosaicMaker;
    MosaicPtr mosaicA = maker->getMosaic();

    VersionedName vna(versionsA->currentText());

    VersionedName vnb(versionsB->currentText());
    VersionedFile vfb = FileServices::getFile(vnb,FILE_MOSAIC);
    maker->loadMosaic(vfb);

    QPixmap pixB = Sys::view->grab();

    MosaicPtr mosaicB = maker->getMosaic();

    mosaicA->dumpMotifs();
    mosaicA->dumpStyles();
    mosaicB->dumpMotifs();
    mosaicB->dumpStyles();

    imgB = pixB.toImage();

    VersionedFile fileA;
    fileA.updateFromVersionedName(vna);

    parent->closeAllImageViewers();
    parent->compareImages(imgA,imgB,fileA,vfb);
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

    bool use_wlist = (config->versionFilter == WORKLIST);
    if (config->vCompTile)
    {
        versions = FileServices::getFileVersions(name,Sys::rootTileDir,use_wlist);
    }
    else
    {
        versions = FileServices::getFileVersions(name,Sys::rootMosaicDir,use_wlist);
    }

    if (versions.isEmpty())
        return;

    versionsA->addItems(versions.getNames());
    versionsA->setCurrentIndex(0);

    // if there is a lock, similary select the media, add its versions and set it to 1
    if (config->vCompLock)
    {
        mediaB->blockSignals(true);
        mediaB->setCurrentIndex(mediaA->currentIndex());
        mediaB->blockSignals(false);

        versionsB->clear();
        versionsB->addItems(versions.getNames());
        versionsB->setCurrentIndex(1);
    }
}

void VersionStepper::mediaBChanged()
{
    // media be has changed, set its versions and if there is no lock select index 0 else select index 1
    versionsB->clear();

    QString name = mediaB->currentText();

    VersionList vl;
    bool use_wlist = (config->versionFilter == WORKLIST);
    if (config->vCompTile)
    {
        vl = FileServices::getFileVersions(name,Sys::rootTileDir,use_wlist);
    }
    else
    {
        vl = FileServices::getFileVersions(name,Sys::rootMosaicDir,use_wlist);
    }

    if (vl.isEmpty())
        return;

    versionsB->addItems(vl.getNames());
    versionsB->setCurrentIndex(0);

    // if there is a lock put the same versions into A amd select version 0
    if (config->vCompLock)
    {
        versions = vl;

        versionsB->setCurrentIndex(1);

        mediaA->blockSignals(true);
        mediaA->setCurrentIndex(mediaB->currentIndex());
        mediaA->blockSignals(false);

        versionsA->clear();
        versionsA->addItems(versions.getNames());
        versionsA->setCurrentIndex(0);
    }
}
