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
    static TPMSplash * getInstance();

    void display(QString txt);

    void show();
    void hide();

protected:
    TPMSplash();

private:
    static TPMSplash * mpThis;

    QStack<QString>  msgStack;

};

#endif // TPMSPLASH_H
