#ifndef VIEWPANEL_H
#define VIEWPANEL_H

#include "panels/panel_page.h"

class View;

class ViewPanel : public QWidget
{
    Q_OBJECT

public:
    ViewPanel();
    ~ViewPanel();

    void    setButtonSize(QSize(size));

protected:
    void    setTranslateMode(bool checked);
    void    setRotateMode(bool checked);
    void    setScaleMode(bool checked);

private:
    View * view;

    QPushButton * btn1;
    QPushButton * btn2;
    QPushButton * btn3;
};

#endif // VIEWPANEL_H
