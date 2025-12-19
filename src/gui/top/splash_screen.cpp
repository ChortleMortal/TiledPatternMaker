#include <qapplication.h>
#include "gui/top/controlpanel.h"
#include "gui/top/splash_screen.h"
#include "gui/top/system_view.h"
#include "sys/sys.h"

SplashScreen::SplashScreen() : QLabel(Sys::sysview)
{
    QPixmap pm(":/tpm.png");
    QSize qs = pm.size();
    resize(qs);

    setAlignment(Qt::AlignCenter);
    setStyleSheet("color: black; background-image: url(:/tpm.png);");

    QFont f = font();
    f.setPointSize(16);
    setFont(f);

    disable(false);

    hide();
}

void SplashScreen::display(QString &  txt, bool panelToo)
{
    if (_disable) return;

    msgStack.push(txt);

    draw();

    if (panelToo)
    {
        Sys::controlPanel->overridePagelStatus(txt);
    }
}

void SplashScreen::replace(QString &  txt, bool panelToo)
{
    if (!msgStack.isEmpty())
    {
        msgStack.pop();
    }
    msgStack.push(txt);
    draw();

    if (panelToo)
    {
        Sys::controlPanel->overridePagelStatus(txt);
    }
}

void SplashScreen::display(QString &&  txt)
{
    if (_disable) return;

    msgStack.push(txt);

    draw();
}

void SplashScreen::remove(bool panelToo)
{
    if (_disable) return;

    if (!msgStack.isEmpty())
    {
        msgStack.pop();
    }

    draw();

    if (panelToo)
    {
        Sys::controlPanel->restorePageStatus();
    }
}

void SplashScreen::draw()
{
    if (msgStack.isEmpty())
    {
        hide();
    }
    else
    {
        if (Sys::sysview)
        {
            this->setParent(Sys::sysview);
            QPoint pos = Sys::viewController->viewRect().center() - rect().center();
            move(pos);
        }

        show();
        raise();

        QString txt;
        for (auto & t : msgStack)
        {
            txt += t;
            txt += "\n";
        }
        setText(txt);

        if (Sys::viewController)
            Sys::viewController->appSuspendPaint(true);

        //repaint();
        update();
        qApp->processEvents();

        if (Sys::viewController)
            Sys::viewController->appSuspendPaint(false);
    }
}

void SplashScreen::disable(bool disable)
{
    _disable = disable;

    if (disable)
    {
        msgStack.clear();
    }
}
