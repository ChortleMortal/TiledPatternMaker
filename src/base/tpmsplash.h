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
    TPMSplash();

    void display(QString txt);
    void remove();

protected:

private:
    QStack<QString>  msgStack;
    int     w;
    int     h;

};

#endif
