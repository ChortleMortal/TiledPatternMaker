#include <QWindow>
#include "gui/top/splash_screen.h"
#include "gui/top/view_controller.h"
#include "sys/sys.h"

SplashScreen::SplashScreen() : QWidget()
{
    setWindowFlags(Qt::FramelessWindowHint);

    QSize qs = m_pixmap.size();
    resize(qs);

    QFont f = font();
    f.setPointSize(16);
    setFont(f);

    disable(false);

    hide();
}

void SplashScreen::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.drawPixmap(rect(), m_pixmap);
    p.setPen(Qt::black);

    // first line stays where a single centred line sits; more lines grow downward
    int top = (height() - fontMetrics().height()) / 2;
    QRect textRect(0, top, width(), height() - top);
    p.drawText(textRect, Qt::AlignHCenter | Qt::AlignTop, m_text);
}

void SplashScreen::display(QString &  txt)
{
    if (_disable) return;

    msgStack.push(txt);

    draw();
}

void SplashScreen::replace(QString &  txt)
{
    if (!msgStack.isEmpty())
    {
        msgStack.pop();
    }

    msgStack.push(txt);

    draw();
}

void SplashScreen::display(QString &&  txt)
{
    if (_disable) return;

    msgStack.push(txt);

    draw();
}

void SplashScreen::remove()
{
    if (_disable) return;

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
        if (Sys::systemViewController)
        {
            QPoint pos = Sys::systemViewController->getViewRect().center() - rect().center();
            pos = Sys::systemViewController->mapToGlobal(pos);
            move(pos);
        }

        m_text.clear();
        for (auto & t : msgStack)
        {
            m_text += t;
            m_text += "\n";
        }

        if (Sys::systemViewController)
            Sys::systemViewController->appSuspendPaint(true);

#ifdef Q_OS_LINUX
        show();
        raise();
        if (windowHandle())
        {
            while (!windowHandle()->isExposed())
            {
                qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
            }
        }
        repaint();
        qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
#else
        show();
        raise();
        update();
        qApp->processEvents();
#endif
        if (Sys::systemViewController)
            Sys::systemViewController->appSuspendPaint(false);
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

void SplashScreen::keyPressEvent(QKeyEvent *)
{
    remove();
}

void SplashScreen::mousePressEvent(QMouseEvent *)
{
    remove();
}
