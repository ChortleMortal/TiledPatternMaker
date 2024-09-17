#ifndef SPLASH_SCREEN_H
#define SPLASH_SCREEN_H

#include <QStack>
#include <QLabel>

class SplashScreen : public QLabel
{
    Q_OBJECT

public:
    SplashScreen();

    void display(QString & txt);
    void display(QString && txt);
    void remove();

protected:
    void draw();

private:
    QStack<QString>  msgStack;
};

#endif
