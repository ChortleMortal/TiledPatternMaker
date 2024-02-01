#include <QImage>
#include <QPixmap>
#include <QThread>
#include <QMessageBox>
#include <QScreen>
#include "engine/image_engine.h"
#include "engine/mosaic_stepper.h"
#include "engine/tiling_stepper.h"
#include "engine/png_stepper.h"
#include "engine/version_stepper.h"
#include "engine/worklist_stepper.h"
#include "misc/sys.h"
#include "panels/controlpanel.h"
#include "settings/configuration.h"
#include "viewers/view.h"
#include "viewers/view_controller.h"
#include "widgets/image_layer.h"
#include "widgets/image_widget.h"
#include "widgets/memory_combo.h"
#include "widgets/transparent_widget.h"

typedef std::shared_ptr<ImageLayerView> ImgLayerPtr;

ImageEngine::ImageEngine()
{
    config = Configuration::getInstance();

    _showA = false;

    timer           = new QTimer();
    wlStepper       = new WorklistBMPStepper(this);
    verStepper      = new VersionStepper(this);
    mosaicStepper   = new MosaicStepper(this);
    tilingStepper   = new TilingStepper(this);
    pngStepper      = new PNGStepper(this);

    connect(this,      &ImageEngine::sig_ready,      this,   &ImageEngine::sig_next, Qt::QueuedConnection);
    connect(timer,     &QTimer::timeout,             this,   &ImageEngine::sig_tick);
    connect(Sys::view, &View::sig_stepperEnd,        this,   &ImageEngine::sig_end);
    connect(Sys::view, &View::sig_stepperPause,      this,   &ImageEngine::sig_pause);
    connect(Sys::view, &View::sig_stepperKey,        this,   &ImageEngine::slot_stepperKey);

    timer->start(1000);
}

ImageEngine::~ImageEngine()
{
    delete wlStepper;
    delete verStepper;
    delete mosaicStepper;
    delete tilingStepper;
    delete pngStepper;
}

void ImageEngine::compareImages(QImage & imgA, QImage & imgB, QString filenameA, QString filenameB, QString nameA, QString nameB, bool autoMode)
{
    qDebug() << imgA << imgB;

    _imageA     = imgA;
    _imageB     = imgB;
    _filenameA  = filenameA;
    _filenameB  = filenameB;
    _nameA      = nameA;
    _nameB      = nameB;

    if (imgA.isNull())
    {
        qWarning() << "Image not found" << nameA;
        QString str = "Image not found";
        emit sig_compareResult(str);
        QPixmap  pm = makeTextPixmap(str,filenameA);
        if (!autoMode || config->stopIfDiff)
        {
            popupPixmap(pm,str);
        }
        else
        {
            emit sig_ready();
        }
        return;
    }

    if (imgB.isNull())
    {
        qWarning() << "different (no match)" << nameB;
        QString str = "No matching image found";
        emit sig_compareResult(str);
        QPixmap  pm = makeTextPixmap(str,filenameB);
        if (!autoMode || config->stopIfDiff)
        {
            popupPixmap(pm,str);
        }
        else
        {
            emit sig_ready();
        }
        return;
    }

    if (imgA == imgB)
    {
        qInfo() << "same     " << nameA;
        QString str = "Images are the same";
        emit sig_compareResult(str);
        QPixmap  pm = makeTextPixmap(str,filenameA,filenameB);
        if (!autoMode)
        {
            popupPixmap(pm,str);
        }
        else
        {
            emit sig_ready();
        }
        return;
    }

    qWarning() << "different" << nameA;

    //
    // files are different
    //

    QString str = "Images are different";
    emit sig_compareResult(str);    // sets page_debug status

    if (autoMode && !config->stopIfDiff)
    {
        emit sig_ready();
        return;
    }

    if  (config->display_differences)
    {
        // display differences
        if (imgA.size() != imgB.size())
        {
            QString str1 = QString("%1 = %2 x %3").arg(filenameA).arg(imgA.width()).arg(imgA.height());
            QString str2 = QString("%1 = %2 x %3").arg(filenameB).arg(imgB.width()).arg(imgB.height());
            QPixmap  pm = makeTextPixmap("Images different sizes:",str1,str2);
            QString str = "Images are different sizes";
            emit sig_compareResult(str);    // sets page_debug status
            popupPixmap(pm, QString("Image Differences (%1) (%2)").arg(filenameA).arg(filenameB));
        }
        else
        {
            int w = qMin(imgA.width(),imgB.width());
            int h = qMin(imgA.height(),imgB.height());
            QImage result(QSize(w,h),imgA.format());

            for(int i=0; i<h; i++)
            {
                QRgb *rgbLeft   = reinterpret_cast<QRgb*>(imgA.scanLine(i));
                QRgb *rgbRigth  = reinterpret_cast<QRgb*>(imgB.scanLine(i));
                QRgb *rgbResult = reinterpret_cast<QRgb*>(result.scanLine(i));
                for( int j=0; j<w; j++)
                {
                    rgbResult[j] = rgbLeft[j]-rgbRigth[j];
                }
            }

            QPixmap pixmap;
            pixmap.convertFromImage(result);

            if (config->compare_transparent)
            {
                popupTransparentPixmap(pixmap, QString("Image Differences (%1) (%2)").arg(filenameA).arg(filenameB));
            }
            else
            {
                popupPixmap(pixmap, QString("Image Differences (%1) (%2)").arg(filenameA).arg(filenameB));
            }
        }
    }
    else
    {
        // show images
        QPixmap pm(filenameA);
        ImageWidget * widget = popupPixmap(pm,filenameA);
        widget->move(widget->pos().x()-200,widget->pos().y());

        QPixmap pm2;
        if (!filenameB.isEmpty())
        {
            pm2 = QPixmap(filenameB);
        }
        else
        {
            pm2 = Sys::view->grab();
        }
        widget = popupPixmap(pm2,filenameB);
        widget->move(widget->pos().x()+200,widget->pos().y());
    }
}

// called by worklist cycler and prev/next/compare
// called with BMP name
void ImageEngine::compareBMPsByName(QString leftName, QString rightName, bool autoMode)
{
    emit sig_image0(leftName);      // sets page_debug status
    emit sig_image1(rightName);     // sets page_debug status

    QString pathLeft  = MemoryCombo::getTextFor("leftDir")  + "/" + leftName  + ".bmp";
    QString pathRight = MemoryCombo::getTextFor("rightDir") + "/" + leftName + ".bmp";

    if (leftName.isEmpty())
    {
        qWarning() << "different (no match)" << leftName;
        QString str = "No matching image found ";
        emit sig_compareResult(str);
        QPixmap  pm = makeTextPixmap(str,pathLeft);
        if (!autoMode || config->stopIfDiff)
        {
            popupPixmap(pm,str);
        }
        else
        {
            emit sig_ready();
        }
        return;
    }

    QImage img_left(pathLeft);
    QImage img_right(pathRight);

    compareImages(img_left,img_right,pathLeft,pathRight,leftName,rightName,autoMode);
}

// called with BMP path
void ImageEngine::compareBMPsByFilename(QString filename1, QString filename2)
{
    if (filename1.isEmpty() || filename2.isEmpty())
    {
        QString str = "No matching image found ";
        QPixmap  pm = makeTextPixmap(str,filename1);
        popupPixmap(pm,str);
        return;
    }

    QImage img_left(filename1);
    QImage img_right(filename2);

    compareImages(img_left,img_right,filename1,filename2,filename1,filename2,false);
}

void ImageEngine::compareBMPwithLoaded(QString leftName, bool autoMode)
{
    emit sig_closeAllImageViewers();

    emit sig_image0(leftName);      // sets page_debug status

    QString pathLeft  = MemoryCombo::getTextFor("leftDir") + "/" + leftName  + ".bmp";

    QImage img_left(pathLeft);

    QPixmap pixmap    = Sys::view->grab();
    QImage  img_right = pixmap.toImage();

    compareImages(img_left,img_right,pathLeft,"Loaded Mosasic",leftName, "Loaded Mosaic",autoMode);
}

QPixmap  ImageEngine::createTransparentPixmap(QImage img)
{
    Configuration * config = Configuration::getInstance();
    QColor transparentColor = config->transparentColor;
    int r,g,b,a;
    transparentColor.getRgb(&r,&g,&b,&a);
    qint32 color = (r << 16) + (g << 8) + b;

    // create image from bitmap
    qDebug() << img.width() << img.height() << img.format();

    // add alpha
    QImage img2 = img.convertToFormat(QImage::Format_ARGB32);
    qDebug() << img2.width() << img2.height() << img2.format();

    // make color black transparent
    int w = img2.width();
    int h = img2.height();
    for(int i=0; i<h; i++)
    {
        QRgb *rgb   = reinterpret_cast<QRgb*>(img2.scanLine(i));
        for( int j=0; j<w; j++)
        {
            qint32 pixel = rgb[j];
            //if ((pixel & 0x00ffffff) == 0)
            if ((pixel & 0x00ffffff) == color)
            {
                rgb[j] = 0x00000000;
            }
        }
    }

    // create pixmap
    QPixmap pixmap;
    pixmap.convertFromImage(img2, Qt::NoFormatConversion);
    qDebug() << pixmap.width() << pixmap.height() << pixmap.depth() << pixmap.hasAlpha() << pixmap.hasAlphaChannel();

    return pixmap;
}

void ImageEngine::ping_pong_images(bool transparent, bool popup)
{
    _showA = !_showA;

    QImage & img  =  (_showA) ? _imageA    : _imageB;
    QString title =  (_showA) ? _filenameA : _filenameB;

    QPixmap pixmap;
    if (config->filterColor)
    {
        pixmap = createTransparentPixmap(img);
    }
    else
    {
        pixmap = QPixmap::fromImage(img);
    }

    if (popup)
    {
        if (!transparent)
        {
            popupPixmap(pixmap,title);
        }
        else
        {
            popupTransparentPixmap(pixmap,title);
        }
    }
    else
    {
        ImgLayerPtr ilp = std::make_shared<ImageLayerView>(title);
        ilp->setPixmap(pixmap);
        Sys::viewController->addImage(ilp);
        Sys::viewController->slot_reconstructView();
    }
}

void ImageEngine::view_image(QString filename, bool transparent, bool popup)
{
    Q_ASSERT(!filename.isEmpty());

    QPixmap pm;
    if (popup)
    {
        if (!transparent)
        {
            pm.load(filename);
            popupPixmap(pm,filename);
        }
        else
        {
            Q_ASSERT(transparent);
            if (config->filterColor)
            {
                pm = createTransparentPixmap(QImage(filename));
            }
            else
            {
                pm.load(filename);
            }

            popupTransparentPixmap(pm,filename);
        }
    }
    else
    {
        Q_ASSERT(!popup);
        if (config->filterColor)
        {
            pm = createTransparentPixmap(QImage(filename));
        }
        else
        {
            pm.load(filename);
        }
        
        ImgLayerPtr ilp = std::make_shared<ImageLayerView>(filename);
        ilp->setPixmap(pm);
        Sys::viewController->addImage(ilp);
        Sys::viewController->slot_reconstructView();
    }
}


/////////////////////////////////////////////////////////////////////
///
///     Top Level Widgets
///
/////////////////////////////////////////////////////////////////////

ImageWidget *  ImageEngine::popupPixmap(QPixmap & pixmap,QString title)
{
    ImageWidget * widget = new ImageWidget();
    widget->resize(pixmap.size());
    widget->setPixmap(pixmap);
    widget->setWindowTitle(title);

    connect(widget, &ImageWidget::sig_keyPressed,          this,   &ImageEngine::imageKeyPressed);
    connect(this,  &ImageEngine::sig_closeAllImageViewers, widget, &ImageWidget::slot_closeMe);

    QSettings s;
    QPoint pos = s.value("imageWidgetPos").toPoint();
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect r = screen->availableGeometry();
    if (r.contains(pos))
    {
        widget->move(pos);
    }

    widget->show();

    return widget;
}

TransparentImageWidget * ImageEngine::popupTransparentPixmap(QPixmap & pixmap,QString title)
{
    TransparentImageWidget * widget = new TransparentImageWidget(title);
    widget->resize(pixmap.size());
    widget->setPixmap(pixmap);

    connect(widget, &TransparentImageWidget::sig_keyPressed, this,   &ImageEngine::imageKeyPressed);
    connect(this,   &ImageEngine::sig_closeAllImageViewers,  widget, &TransparentImageWidget::slot_closeMe);

    QSettings s;
    QPoint pos = s.value("imageWidgetPos").toPoint();
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect r = screen->availableGeometry();
    if (r.contains(pos))
    {
        widget->move(pos);
    }

    widget->show();

    return widget;
}

QPixmap ImageEngine::makeTextPixmap(QString txt,QString txt2,QString txt3)
{
    QPixmap pixmap(1600,500);
    pixmap.fill(Qt::white);

    QPainter painter(&pixmap);

    QFont serifFont("Times", 18, QFont::Normal);
    painter.setFont(serifFont);
    painter.setPen(Qt::black);

    painter.drawText(20,100,txt);
    painter.drawText(20,150,txt2);
    painter.drawText(20,200,txt3);

    return pixmap;
}


/////////////////////////////////////////////////////////////////////
///
///     Key Press events from ImageWidgets
///
/////////////////////////////////////////////////////////////////////

void ImageEngine::imageKeyPressed(QKeyEvent * k)
{
    int key = k->key();

    if (key == Qt::Key_Space)
    {
        emit sig_closeAllImageViewers();
        emit sig_next();
    }
    else if (key == 'C')
    {
        // compare
        emit sig_closeAllImageViewers();
        compareImages(_imageA,_imageB,_filenameA,_filenameB,_nameA,_nameB,false);
    }
    else if (key == 'D')
    {
        // delete (from current worklist)
        emit sig_closeAllImageViewers();
        emit sig_deleteCurrentInWorklist(false);  // this sends a sig_ready()
    }
    else if (key == 'Q')
    {
        // quit
        emit sig_closeAllImageViewers();
        Sys::localCycle = false;
        emit sig_end();
    }
    else if (key == 'P')
    {
        // ping-pong
        emit sig_closeAllImageViewers();
        ping_pong_images(config->compare_transparent,true);
    }
    else if (key == 'S')
    {
        // side-by-side
        QPixmap pm(_filenameA);
        ImageWidget * widget = popupPixmap(pm,_filenameA);
        widget->move(widget->pos().x()-100,widget->pos().y());

        QPixmap pm2;
        if (!_filenameB.isEmpty())
        {
            pm2 = QPixmap(_filenameB);
        }
        else
        {
            pm2 = Sys::view->grab();
        }
        widget = popupPixmap(pm2,_filenameB);
        widget->move(widget->pos().x()+100,widget->pos().y());
    }
    else if (key == 'L')
    {
        // log
        qWarning() << "FILE LOGGED (needs attention)";
        emit sig_closeAllImageViewers();
        emit sig_next();
    }
}

void ImageEngine::slot_stepperKey(int key )
{
    qDebug() << "key=" << key;

    if (key ==  Qt::Key_Space)
    {
        emit sig_next();
        pngStepper->next();
    }
}


