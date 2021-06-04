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

    void displayMosaic(QString txt);
    void displayTiling(QString txt);
    void removeMosaic();
    void removeTiling();

protected:
    void draw();

private:
    QString design;
    QString tiling;
    int     w;
    int     h;
};

#endif
