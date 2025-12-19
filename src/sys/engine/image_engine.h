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
    void    showBMP(VersionedName & name);

    static QPixmap createTransparentPixmap(QImage img);

    QPixmap makeTextPixmap(QString txt,QString txt2=QString(),QString txt3=QString());

    void    closeAllImageViewers(bool force = true);

    void    setPopupScale(qreal scale);

    ImageWidget * currentPopup() { return currentImgWidget; }

    void    setCompareMode(bool set) { _compareMode = set; }

signals:
    void sig_tick();
    void sig_next();
    void sig_prev();
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
    void sig_updateView();

public slots:
    void slot_stepperKey(int key);
    bool slot_imageKeyPressed(QKeyEvent * k);
    void slot_imageWidgetClosed(ImageWidget * widget);

protected:
    ImageEngine();      // only the friend class page_image_tools can create this

    class CompareBMPStepper     * compareBMPStepper;
    class ViewBMPStepper        * viewBMPStepper;
    class VersionStepper        * verStepper;
    class MosaicStepper         * mosaicStepper;
    class TilingStepper         * tilingStepper;
    class PNGStepper            * pngStepper;

private:
    void view_image(VersionedFile & file);
    void ping_pong_images();

    void          viewPixmap            (QPixmap & pixmap,QString title);
    ImageWidget * popupPixmap     (const QPixmap & pixmap,QString title);
    ImageWidget * popupTransparentPixmap(QPixmap & pixmap,QString title);

    Configuration       * config;
    QTimer              * timer;

    // commpare data
    QImage  _imageA;
    QImage  _imageB;
    VersionedFile _fileA;
    VersionedFile _fileB;

    bool    _showA;
    bool    _compareMode;

    ImageWidget * currentImgWidget;

    static qreal popupScale;
};

#endif // IMAGEENGINE_H
