#ifndef IMAGEENGINE_H
#define IMAGEENGINE_H

#include <QObject>
#include <QKeyEvent>
#include <QImage>
#include <QMetaType>
#include <QTimer>
#include "sys/sys/versioning.h"

class ImageWidget;
class TransparentImageWidget;
class Configuration;

class ImageEngine : public QObject
{
    Q_OBJECT

    friend class page_image_tools;

public:
    ~ImageEngine();

    void    compareBMPs(VersionedName & nameA,  VersionedName & nameB);
    void    compareBMPs(VersionedFile & fileA,  VersionedFile & fileB);
    void    compareBMPwithLoaded(VersionedName & name);
    void    compareImages(QImage & imgA, QImage & imgB, VersionedFile & fileA, VersionedFile & fileB);


    static QPixmap createTransparentPixmap(QImage img);
    static bool    validViewKey(QKeyEvent * k);

    QPixmap makeTextPixmap(QString txt,QString txt2=QString(),QString txt3=QString());

    void    closeAllImageViewers(bool force = true);

signals:
    void sig_tick();
    void sig_next();
    void sig_end();
    void sig_pause();
    void sig_ready();

    void sig_compareResult(QString);
    void sig_image0(QString name);
    void sig_image1(QString name);
    void sig_closeAll();

    void sig_colorPick(QColor color);
    void sig_deleteCurrentInWorklist(bool confirm);

    void cycle_sig_LoadTiling(QString name);
    void cycle_sig_workList();

    void sig_reconstructView();

public slots:
    void slot_stepperKey(int key);
    void slot_imageKeyPressed(QKeyEvent * k);

protected:
    ImageEngine();      // only the friend class page_image_tools can create this

    class WorklistBMPStepper    * wlStepper;
    class VersionStepper        * verStepper;
    class MosaicStepper         * mosaicStepper;
    class TilingStepper         * tilingStepper;
    class PNGStepper            * pngStepper;

private:

    void view_image(VersionedFile & file);
    void ping_pong_images();

    void                     viewPixmap( QPixmap & pixmap,QString title);
    ImageWidget *            popupPixmap(QPixmap & pixmap,QString title);
    TransparentImageWidget * popupTransparentPixmap(QPixmap & pixmap,QString title);

    Configuration       * config;
    QTimer              * timer;

    // commpare data
    QImage  _imageA;
    QImage  _imageB;
    VersionedFile _fileA;
    VersionedFile _fileB;

    bool    _showA;

    ImageWidget * currentImgWidget;
};

#endif // IMAGEENGINE_H
