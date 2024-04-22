#include <QLabel>
#include <QGridLayout>
#include "engine/png_stepper.h"
#include "misc/sys.h"
#include "viewers/view_controller.h"

////////////////////////////////////////////
///
///  PNGStepper runs in GUI thread
///
////////////////////////////////////////////

PNGStepper::PNGStepper(ImageEngine * parent) : SteppingEngine(parent)
{
    connect(this,  &PNGStepper::cycle_sig_unloadView, Sys::viewController,  &ViewController::slot_unloadView);
}

bool PNGStepper::begin()
{
    if (isStarted())
    {
        end();
        return false;
    }

    qDebug() << "Starting PNG viewing";

    setPaused(false);
    start(true);

    pngRow   = 0;
    pngCol   = 0;
    pngIndex = 0;

    cydata.fileFilter << "*.png";

    QString path = Sys::examplesDir;
    QDir adir(path);
    cydata.files = adir.entryList(cydata.fileFilter);
    qDebug() << "num pngs =" << cydata.files.size();

    return next();
}

bool PNGStepper::tick()
{
    return false;
}

bool PNGStepper::next()
{
    if (!isStarted()) return false;

    qDebug() << "page starting: " << pngIndex;

    emit cycle_sig_unloadView();

    while (pngIndex < cydata.files.size())
    {
        cycleShowPNG(cydata.files.at(pngIndex),pngRow, pngCol);
        pngIndex++;

        if (++pngCol > 4)
        {
            pngCol = 0;
            if (++pngRow > 4)
            {
                // pause
                pngRow = 0;
                return false;   // TODO maybe true
            }
        }
    }
    pngIndex = 0;
    pngRow   = 0;
    pngCol   = 0;

    return true;
}

void PNGStepper::cycleShowPNG(QString file, int row, int col)
{
    QString name = Sys::examplesDir + file;
    QPixmap pix(name);
    QLabel  * label = new QLabel("Put PNG Here");
    label->setPixmap(pix);

    QLayout * l = Sys::view->layout();
    QGridLayout * grid = dynamic_cast<QGridLayout*>(l);
    grid->addWidget(label,row,col);

    Sys::view->show();
}

bool PNGStepper::prev()
{
    return false;
}

bool PNGStepper:: end()
{
    setPaused(false);

    if (isStarted())
    {
        start(false);
        finish("PNG viewing");
        return true;
    }
    else
    {
        return false;
    }
}
