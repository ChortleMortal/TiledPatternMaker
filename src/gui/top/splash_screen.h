#ifndef SPLASH_SCREEN_H
#define SPLASH_SCREEN_H

#include <QStack>
#include <QLabel>

class SplashScreen : public QLabel
{
    Q_OBJECT

public:
    SplashScreen();

    void display(QString & txt, bool panelToo = false);
    void replace(QString & txt, bool panelToo = false);
    void display(QString && txt);
    void remove(bool panelToo = false);

    void disable(bool disable);

protected:
    void draw();

private:
    QStack<QString>  msgStack;
    bool            _disable;
};

#endif
