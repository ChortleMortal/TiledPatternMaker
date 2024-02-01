#ifndef IMAGEENGINE_H
#define IMAGEENGINE_H

#include <QObject>
#include <QKeyEvent>
#include <QImage>
#include <QMetaType>
#include <QTimer>

class ImageWidget;
class TransparentImageWidget;
class Configuration;

class ImageEngine : public QObject
{
    Q_OBJECT

    friend class page_image_tools;

public:
    ~ImageEngine();

    void    compareBMPsByName(QString leftName, QString rightName, bool autoMode);
    void    compareBMPsByFilename(QString filename1, QString filename2);
    void    compareBMPwithLoaded(QString leftName, bool autoMode);
    void    compareImages(QImage &imgA, QImage &imgB, QString filenameA, QString filenameB, QString nameA, QString nameB, bool automode);

    static QPixmap createTransparentPixmap(QImage img);
    QPixmap makeTextPixmap(QString txt,QString txt2=QString(),QString txt3=QString());

    ImageWidget *            popupPixmap(QPixmap & pixmap,QString title);
    TransparentImageWidget * popupTransparentPixmap(QPixmap & pixmap,QString title);
    void                     view_image(QString filename, bool transparent, bool popup);

    void    imageKeyPressed(QKeyEvent * k);

signals:
    void sig_tick();
    void sig_next();
    void sig_end();
    void sig_pause();
    void sig_ready();

    void sig_compareResult(QString);
    void sig_image0(QString name);
    void sig_image1(QString name);
    void sig_closeAllImageViewers();

    void sig_colorPick(QColor color);
    void sig_deleteCurrentInWorklist(bool confirm);

    void cycle_sig_LoadTiling(QString name);
    void cycle_sig_workList();

public slots:
    void slot_stepperKey(int key);

protected:
    ImageEngine();      // only the friend class page_image_tools can create this

    void ping_pong_images(bool transparent, bool popup);

    class WorklistBMPStepper    * wlStepper;
    class VersionStepper        * verStepper;
    class MosaicStepper         * mosaicStepper;
    class TilingStepper         * tilingStepper;
    class PNGStepper            * pngStepper;

private:
    Configuration       * config;
    QTimer              * timer;

    // commpare data
    QImage  _imageA;
    QImage  _imageB;
    QString _filenameA;
    QString _filenameB;
    QString _nameA;
    QString _nameB;

    bool    _showA;
};

#endif // IMAGEENGINE_H
