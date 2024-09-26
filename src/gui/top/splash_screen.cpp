#include <qapplication.h>
#include "gui/top/splash_screen.h"
#include "gui/top/view.h"
#include "sys/sys.h"

SplashScreen::SplashScreen() : QLabel(Sys::view)
{
    QPixmap pm(":/tpm.png");
    QSize qs = pm.size();
    resize(qs);

    setAlignment(Qt::AlignCenter);
    setStyleSheet("color: black; background-image: url(:/tpm.png);");

    QFont f = font();
    f.setPointSize(16);
    setFont(f);

    hide();
}

void SplashScreen::display(QString &  txt)
{
    msgStack.push(txt);

    draw();
}

void SplashScreen::display(QString &&  txt)
{
    msgStack.push(txt);

    draw();
}

void SplashScreen::remove()
{
    if (!msgStack.isEmpty())
    {
        msgStack.pop();
    }

    draw();
}

void SplashScreen::draw()
{
    if (msgStack.isEmpty())
    {
        hide();
    }
    else
    {
        if (Sys::view)
        {
            this->setParent(Sys::view);
            QPoint pos = Sys::view->rect().center() - rect().center();
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

        if (Sys::view)
        {
            Sys::view->appSuspendPaint(true);
        }

        //repaint();
        update();
        qApp->processEvents();

        if (Sys::view)
        {
            Sys::view->appSuspendPaint(false);
        }
    }
}

