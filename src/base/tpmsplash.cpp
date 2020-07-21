#include "base/tpmsplash.h"
#include "base/view.h"

TPMSplash::TPMSplash(QWidget * parent) : QSplashScreen(parent)
{
    QPixmap pm(":/tpm.png");
    setPixmap(pm);
    QFont f = font();
    f.setPointSize(16);
    setFont(f);
    QSize qs = pm.size();
    w = qs.width();
    h = qs.height();
}

void TPMSplash::display(QString txt)
{
    msgStack.push(message());

    View * view = View::getInstance();
    QPoint pos = view->rect().center();
    pos = view->mapToGlobal(pos);
    QPoint p2  = pos - QPoint(w/2,h/2);
    move(p2);
    show();
    showMessage(txt, Qt::AlignCenter);
}

void TPMSplash::hide()
{
    if (msgStack.count())
    {
        msgStack.pop();
    }
    if (msgStack.count())
    {
        QString str = msgStack.top();
        showMessage(str, Qt::AlignCenter);
    }
    else
    {
        QSplashScreen::hide();
    }
}
