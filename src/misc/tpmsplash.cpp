#include "misc/tpmsplash.h"
#include "misc/sys.h"
#include "viewers/view.h"

TPMSplash::TPMSplash() : QLabel(Sys::view)
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
        QPoint pos = Sys::view->rect().center() - rect().center();
        move(pos);

        show();

        QString txt = msgStack.top();
        setText(txt);
        repaint();
    }
}

