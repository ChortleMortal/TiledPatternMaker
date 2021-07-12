#ifndef VIEWPANEL_H
#define VIEWPANEL_H

#include <QWidget>
#include <QPushButton>
#include "panels/panel_misc.h"

class ViewPanel : public AQWidget
{
    Q_OBJECT

public:
    ViewPanel();
    ~ViewPanel();

    void    setButtonSize(QSize(size));

    void    setSetCenterMode(bool checked);
    void    setTranslateMode(bool checked);
    void    setRotateMode(bool checked);
    void    setScaleMode(bool checked);

    QPushButton * btnCenter;
    QPushButton * btnPan;
    QPushButton * btnRot;
    QPushButton * btnZoom;

private:
    class View * view;
};

#endif // VIEWPANEL_H
