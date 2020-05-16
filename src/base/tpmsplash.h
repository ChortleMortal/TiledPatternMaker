#ifndef TPMSPLASH_H
#define TPMSPLASH_H

#include <QtWidgets>

#ifdef __linux__
#undef  TPMSPLASH
#else
#define TPMSPLASH
#endif

class TPMSplash : public QSplashScreen
{
public:
    TPMSplash(QWidget * parent);

    void display(QString txt);
    void hide();

protected:

private:
    QStack<QString>  msgStack;
    int     w;
    int     h;

};

#endif
