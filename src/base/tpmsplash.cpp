#include "base/tpmsplash.h"
#include "panels/panel.h"

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

void TPMSplash::display(QString txt)
{
    //qDebug()  << "splash:" << txt;

    if (!isHidden())
    {
        msgStack.push(message());
    }

    ControlPanel * p = ControlPanel::getInstance();
    QPoint pos       = p->rect().center();
    pos              = p->mapToGlobal(pos);
    QPoint p2        = pos - QPoint(w/2,h/2);
    move(p2);
    show();
    showMessage(txt, Qt::AlignCenter);
}

void TPMSplash::remove()
{
    if (msgStack.count())
    {
        QString str = msgStack.pop();
        //qDebug() << "redisplay:" << str;
        showMessage(str, Qt::AlignCenter);
    }
    else
    {
        hide();
    }
}
