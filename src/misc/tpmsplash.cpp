#include "misc/tpmsplash.h"
#include "panels/controlpanel.h"

/*
 * QSplashScreen() has bugs
 *   - does not display (refresh correctly) on some Linux systems
 *   - the repaint() call in its code can be recursive calling crashes
 *  Both problems are probably caused by its hidden calls to repaint() and to
 *  QCoreApplication::processEvents()
 *
 */

TPMSplash::TPMSplash() : QSplashScreen()
{

    QPixmap pm(":/tpm.png");
    setPixmap(pm);

    QFont f = font();
    f.setPointSize(16);
    setFont(f);

    QSize qs = pm.size();
    w = qs.width();
    h = qs.height();

    hide();
}

void TPMSplash::display(QString &  txt)
{
    msgStack.push(txt);

    draw();
}

void TPMSplash::remove()
{
    if (!msgStack.isEmpty())
    {
        msgStack.pop();
    }

    draw();
}

void TPMSplash::draw()
{
    if (msgStack.isEmpty())
    {
        hide();
    }
    else
    {
        ControlPanel * p = ControlPanel::getInstance();
        QPoint pos       = p->rect().center();
        pos              = p->mapToGlobal(pos);
        QPoint p2        = pos - QPoint(w/2,h/2);
        move(p2);

        show();

       QString txt = msgStack.top();
       showMessage(txt, Qt::AlignCenter);
    }
}

