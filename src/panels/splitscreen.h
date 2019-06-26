#ifndef SPLITSCREEN_H
#define SPLITSCREEN_H

#include <QtWidgets>

class SplitScreen : public QFrame
{
public:
    SplitScreen(QWidget *parent = nullptr);
    void addWidgets(QWidget * w1, QWidget * w2);

protected:

private:
    QGridLayout * grid;
};

#endif // SPLITSCREEN_H
