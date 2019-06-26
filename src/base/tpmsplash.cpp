#include "tpmsplash.h"

TPMSplash * TPMSplash::mpThis = nullptr;

TPMSplash * TPMSplash::getInstance()
{
    if (mpThis == nullptr)
    {
        mpThis = new TPMSplash;
    }
    return mpThis;
}

TPMSplash::TPMSplash()
{
    QPixmap pm(":/tpm.png");
    setPixmap(pm);
    QFont f = font();
    f.setPointSize(16);
    setFont(f);
}

void TPMSplash::display(QString txt)
{
    if (isVisible())
    {
        msgStack.push(message());
    }
    QSplashScreen::show();
    clearMessage();
    showMessage(txt, Qt::AlignCenter);
    repaint();
}

void TPMSplash::show()
{
    qFatal("don't use");
}

void TPMSplash::hide()
{
    if (!isVisible())
    {
        return;
    }

    if (msgStack.count())
    {
        QString str = msgStack.pop();
        clearMessage();
        showMessage(str, Qt::AlignCenter);
    }
    else
    {
        QSplashScreen::hide();
    }
}
