#include <QImage>
#include <QPixmap>
#include <QThread>
#include <QMessageBox>
#include <QScreen>

#include "gui/top/controlpanel.h"
#include "gui/top/system_view.h"
#include "gui/top/system_view_controller.h"
#include "gui/viewers/image_view.h"
#include "gui/widgets/image_widget.h"
#include "gui/widgets/memory_combo.h"
#include "gui/widgets/transparent_widget.h"
#include "model/settings/configuration.h"
#include "sys/engine/image_engine.h"
#include "sys/engine/mosaic_stepper.h"
#include "sys/engine/png_stepper.h"
#include "sys/engine/tiling_stepper.h"
#include "sys/engine/version_stepper.h"
#include "sys/engine/compare_bmp_stepper.h"
#include "sys/engine/view_bmp_stepper.h"
#include "sys/sys.h"

qreal ImageEngine::popupScale = 1.0;

ImageEngine::ImageEngine()
{
    config          = Sys::config;
    _showA          = false;
    _compareMode    = false;
    currentImgWidget = nullptr;

    timer             = new QTimer();
    compareBMPStepper = new CompareBMPStepper(this);  // steps thru list of BMPs and compares them
    viewBMPStepper    = new ViewBMPStepper(this);     // steps thru list of BMPs and shows them
    verStepper        = new VersionStepper(this);     // doesn't really step - but syncs versions for comparison or viewing
    mosaicStepper     = new MosaicStepper(this);      // steps thru list and loads mosaic
    tilingStepper     = new TilingStepper(this);      // steps thru list and loads tiling
    pngStepper        = new PNGStepper(this);         // steps thru exisitsng pngs (with timer)

    connect(this, &ImageEngine::sig_reconstructView, Sys::viewController, &SystemViewController::slot_reconstructView);
    connect(this, &ImageEngine::sig_updateView,      Sys::viewController, &SystemViewController::slot_updateView);

    connect(this,         &ImageEngine::sig_ready,            this,   &ImageEngine::sig_next, Qt::QueuedConnection);
    connect(timer,        &QTimer::timeout,                   this,   &ImageEngine::sig_tick);
    connect(Sys::sysview, &SystemView::sig_stepperEnd,        this,   &ImageEngine::sig_end);
    connect(Sys::sysview, &SystemView::sig_stepperPause,      this,   &ImageEngine::sig_pause);
    connect(Sys::sysview, &SystemView::sig_stepperKey,        this,   &ImageEngine::slot_stepperKey);

    timer->start(1000);
}

ImageEngine::~ImageEngine()
{
    delete compareBMPStepper;
    delete viewBMPStepper;
    delete verStepper;
    delete mosaicStepper;
    delete tilingStepper;
    delete pngStepper;
}

void ImageEngine::compareImages(QImage & imgA, QImage & imgB, VersionedFile & fileA, VersionedFile & fileB)
{
    qDebug() << "imgA" << imgA;
    qDebug() << "imgB" << imgB;

    _imageA = imgA;
    _imageB = imgB;
    _fileA  = fileA;
    _fileB  = fileB;

    if (imgA.isNull())
    {
        qWarning() << "Image not found" << fileA.getVersionedName().get();
        QString str = "Image not found";
        emit sig_compareResult(str);
        QPixmap  pm = makeTextPixmap(str,fileA.getPathedName());
        if (config->compare_popup)
            currentImgWidget = popupPixmap(pm,str);
        else
            viewPixmap(pm,str);
        return;
    }

    if (imgB.isNull())
    {
        qWarning() << "different (no match)" << fileA.getVersionedName().get();
        QString str = "No matching image found";
        emit sig_compareResult(str);
        QPixmap  pm = makeTextPixmap(str,fileB.getPathedName());
        if (config->compare_popup)
            currentImgWidget = popupPixmap(pm,str);
        else
            viewPixmap(pm,str);
        return;
    }

    if (imgA == imgB)
    {
        qInfo() << "same     " << fileA.getVersionedName().get();
        QString str = "Images are the same";
        emit sig_compareResult(str);
        QPixmap  pm = makeTextPixmap(str,fileA.getPathedName(),fileB.getPathedName());
        if (config->compare_popup)
            currentImgWidget = popupPixmap(pm,str);
        else
            viewPixmap(pm,str);
        return;
    }

    qWarning() << "different" << fileA.getVersionedName().get();

    //
    // files are different
    //

    QString str = "Images are different";
    emit sig_compareResult(str);    // sets page_debug status

    if (imgA.size() != imgB.size())
    {
        QString str1 = QString("%1 = %2 x %3").arg(fileA.getPathedName()).arg(imgA.width()).arg(imgA.height());
        QString str2 = QString("%1 = %2 x %3").arg(fileB.getPathedName()).arg(imgB.width()).arg(imgB.height());
        QPixmap  pm = makeTextPixmap("Images different sizes:",str1,str2);
        QString str = "Images are different sizes";
        emit sig_compareResult(str);    // sets page_debug status
        QString title = QString("Image Differences (%1) (%2)").arg(fileA.getPathedName()).arg(fileB.getPathedName());
        if (config->compare_popup)
        {
            currentImgWidget = popupPixmap(pm, title);
        }
        else
        {
            viewPixmap(pm,title);
        }
        return;
    }

    // calculate differences
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

    // display differences
    QPixmap pixmap;
    pixmap.convertFromImage(result,Qt::NoFormatConversion);

    QString title = QString("Image Differences (%1) (%2)").arg(fileA.getPathedName()).arg(fileB.getPathedName());
    if (config->compare_popup)
    {
        if (config->compare_transparent)
        {
            currentImgWidget = popupTransparentPixmap(pixmap, title);
        }
        else
        {
            currentImgWidget = popupPixmap(pixmap, title);
        }
    }
    else
    {
        viewPixmap(pixmap,title);
    }
}

// called by worklist cycler and prev/next/compare
// called with BMP name
void ImageEngine::compareBMPs(VersionedName & nameA, VersionedName & nameB)
{
    emit sig_image0(nameA.get());      // sets page_debug status
    emit sig_image1(nameB.get());     // sets page_debug status

    VersionedFile left( MemoryCombo::getTextFor("leftDir")  + "/" + nameA.get()  + ".bmp");
    VersionedFile right(MemoryCombo::getTextFor("rightDir") + "/" + nameB.get() + ".bmp");

    if (nameA.isEmpty())
    {
        qWarning() << "different (no match)" << nameA.get();
        QString str = "No matching image found ";
        emit sig_compareResult(str);
        QPixmap  pm = makeTextPixmap(str,left.getPathedName());
        popupPixmap(pm,str);
        return;
    }

    QImage img_left(left.getPathedName());
    QImage img_right(right.getPathedName());

    compareImages(img_left,img_right,left,right);
}

// called with BMP path
void ImageEngine::compareBMPs(VersionedFile & fileA, VersionedFile & fileB)
{
    if (fileA.isEmpty() || fileB.isEmpty())
    {
        QString str = "No matching image found ";
        QPixmap  pm = makeTextPixmap(str,fileA.getPathedName(),fileB.getPathedName());
        popupPixmap(pm,str);
        return;
    }

    QImage img_left(fileA.getPathedName());
    QImage img_right(fileB.getPathedName());

    compareImages(img_left,img_right,fileA,fileB);
}

void ImageEngine::compareBMPwithLoaded(VersionedName & name)
{
    QPixmap pixmap    = Sys::viewController->grabView();  // do this first
    QImage  img_right = pixmap.toImage();

    closeAllImageViewers();

    emit sig_image0(name.get());      // sets page_debug status

    VersionedFile fileA(MemoryCombo::getTextFor("leftDir") + "/" + name.get()  + ".bmp");

    QImage img_left(fileA.getPathedName());

    VersionedFile fileB;
    VersionedName nameB("Loaded Mosaic Bitmap");
    fileB.updateFromVersionedName(nameB);

    compareImages(img_left,img_right,fileA,fileB);
}

void ImageEngine::showBMP(VersionedName & name)
{
    emit sig_image0(name.get());

    VersionedFile file(MemoryCombo::getTextFor("leftDir")  + "/" + name.get()  + ".bmp");
    QPixmap pm;
    pm.load(file.getPathedName());
    if (config->compare_popup)
    {
        currentImgWidget = popupPixmap(pm, file.getVersionedName().get());
    }
    else
    {
        viewPixmap(pm,file.getVersionedName().get());
    }
}

QPixmap  ImageEngine::createTransparentPixmap(QImage img)
{
    Configuration * config  = Sys::config;
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

void ImageEngine::ping_pong_images()
{
    _showA = !_showA;

    QImage & img  =  (_showA) ? _imageA    : _imageB;
    QString title =  (_showA) ? _fileA.getPathedName() : _fileB.getPathedName();

    QPixmap pixmap;
    if (config->compare_filterColor)
    {
        pixmap = createTransparentPixmap(img);
    }
    else
    {
        pixmap = QPixmap::fromImage(img);
    }

    if (config->compare_popup)
    {
        if (config->compare_transparent)
        {
            currentImgWidget = popupTransparentPixmap(pixmap,title);
        }
        else
        {
            currentImgWidget = popupPixmap(pixmap,title);
        }
    }
    else
    {
        viewPixmap(pixmap,title);
    }
}

void ImageEngine::view_image(VersionedFile & file)
{
    Q_ASSERT(!file.isEmpty());

    QPixmap pm;
    if (config->compare_popup)
    {
        if (config->compare_transparent)
        {
            Q_ASSERT(config->compare_transparent);
            if (config->compare_filterColor)
            {
                pm = createTransparentPixmap(QImage(file.getPathedName()));
            }
            else
            {
                pm.load(file.getPathedName());
            }

            popupTransparentPixmap(pm,file.getPathedName());
        }
        else
        {
            pm.load(file.getPathedName());
            popupPixmap(pm,file.getPathedName());
        }
    }
    else
    {
        Q_ASSERT(!config->compare_popup);
        if (config->compare_filterColor)
        {
            pm = createTransparentPixmap(QImage(file.getPathedName()));
        }
        else
        {
            pm.load(file.getPathedName());
        }

        viewPixmap(pm,file.getVersionedName().get());
    }
}

void ImageEngine::viewPixmap(QPixmap& pixmap, QString title)
{
    Sys::imageViewer->load(pixmap);
    Sys::viewController->setWindowTitle(title);
    Sys::viewController->setSize(pixmap.size());

    if (!Sys::viewController->isEnabled(VIEW_BMP_IMAGE))
    {
        Sys::controlPanel->deselectGangedViewers();
        Sys::controlPanel->delegateView(VIEW_BMP_IMAGE,true);
        emit sig_reconstructView();
    }
    else
    {
        emit sig_updateView();
    }
}

/////////////////////////////////////////////////////////////////////
///
///     Top Level Widgets
///
/////////////////////////////////////////////////////////////////////

ImageWidget *  ImageEngine::popupPixmap(const QPixmap & pixmap,QString title)
{
    qDebug() << "Popup pixmap" << title;

    ImageWidget * widget;
    if (currentImgWidget)
    {
        if (currentImgWidget->gettypename() == "image")
        {
            widget = currentImgWidget;
        }
        else
        {
            currentImgWidget->slot_closeMe();
            currentImgWidget = nullptr;
        }
    }
    if (currentImgWidget == nullptr)
    {

        widget =  new ImageWidget();
        connect(widget, &ImageWidget::sig_keyPressed,   this,   &ImageEngine::slot_imageKeyPressed);
        connect(widget, &ImageWidget::sig_closed,       this,   &ImageEngine::slot_imageWidgetClosed);
        connect(this,   &ImageEngine::sig_closeAll,     widget, &ImageWidget::slot_closeMe);

        QSettings s;
        QPoint pos = s.value("imageWidgetPos").toPoint();
        QScreen *screen = QGuiApplication::primaryScreen();
        QRect r = screen->availableGeometry();
        if (r.contains(pos))
        {
            widget->move(pos);
        }
    }

#ifdef __linux__
    widget->setPixmap(widget->removeAlphaChannel(pixmap));
#else
    if (popupScale == 1.0)
        widget->setPixmap(pixmap);
    else
        widget->setPixmap(pixmap,popupScale);
#endif
    widget->setWindowTitle(title);
    widget->setContentSize(pixmap.size());
    widget->show();
    return widget;
}

ImageWidget * ImageEngine::popupTransparentPixmap(QPixmap & pixmap,QString title)
{
    qDebug() << "Popup Transparent" << title;

    ImageWidget * widget;
    if (currentImgWidget)
    {
        if (currentImgWidget->gettypename() == "transp")
        {
            widget = currentImgWidget;
        }
        else
        {
            currentImgWidget->slot_closeMe();
            currentImgWidget = nullptr;
        }
    }
    if (currentImgWidget == nullptr)
    {
        widget =  new TransparentImageWidget(title);

        connect(widget, &ImageWidget::sig_keyPressed, this,   &ImageEngine::slot_imageKeyPressed);
        connect(widget, &ImageWidget::sig_closed,     this,   &ImageEngine::slot_imageWidgetClosed);
        connect(this,   &ImageEngine::sig_closeAll,   widget, &ImageWidget::slot_closeMe);

        QSettings s;
        QPoint pos = s.value("imageWidgetPos").toPoint();
        QScreen *screen = QGuiApplication::primaryScreen();
        QRect r = screen->availableGeometry();
        if (r.contains(pos))
        {
            widget->move(pos);
        }
    }

#ifdef __linux__
    widget->setPixmap(widget->removeAlphaChannel(pixmap));
#else
    widget->setPixmap(pixmap);
#endif
    widget->setWindowTitle(title);
    widget->setContentSize(pixmap.size());
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

void ImageEngine::closeAllImageViewers(bool force)
{
    if (force)
    {
        currentImgWidget = nullptr;
        emit sig_closeAll();
    }
    Sys::imageViewer->unloadLayerContent();
    Sys::controlPanel->delegateView(VIEW_BMP_IMAGE,false);
    Sys::lastViewTitle.clear();
    Sys::controlPanel->slot_poll();
}

/////////////////////////////////////////////////////////////////////
///
///     Key Press events from ImageWidgets
///
/////////////////////////////////////////////////////////////////////

void ImageEngine::slot_imageWidgetClosed(ImageWidget * widget)
{
    delete widget;
    if (widget == currentImgWidget)
    {
        currentImgWidget = nullptr;
    }
}

bool ImageEngine::slot_imageKeyPressed(QKeyEvent * k)
{
    int key = k->key();

    switch(key)
    {
    case Qt::Key_Space:
        if (k->modifiers() & Qt::ControlModifier)
            emit sig_prev();
        else
            emit sig_next();
        return true;

    case 'C':
        // compare
        if (_compareMode)
        {
            compareImages(_imageA,_imageB,_fileA,_fileB);
            return true;
        }
        break;

    case  'D':
        // delete (from current worklist)
        emit sig_deleteCurrentInWorklist(false);  // this sends a sig_ready()
        return true;

    case 'Q':
        // quit
        closeAllImageViewers();
        Sys::localCycle = false;
        emit sig_end();
        return true;

    case'P':
        // ping-pong
        if (_compareMode)
        {
            ping_pong_images();
            return true;
        }
      break;

    default:
        break;

    }
    return false;  // return is ignored by popups
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

void ImageEngine::setPopupScale(qreal scale)
{
    popupScale = scale;
    if (currentImgWidget)
    {
        QPixmap p   = currentImgWidget->getPixmap();
        QString txt = currentImgWidget->windowTitle();
        popupPixmap(p,txt);
    }
}

