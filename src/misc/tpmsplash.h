#ifndef TPMSPLASH_H
#define TPMSPLASH_H

#include <QSplashScreen>
#include <QStack>

class TPMSplash : public QSplashScreen
{
    Q_OBJECT

public:
    TPMSplash();

    void display(QString & txt);
    void remove();

protected:
    void draw();

private:
    int     w;
    int     h;

    QStack<QString>  msgStack;
};

#endif
