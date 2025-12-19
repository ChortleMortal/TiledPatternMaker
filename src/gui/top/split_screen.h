#pragma once
#ifndef SPLITSCREEN_H
#define SPLITSCREEN_H

#include <QFrame>
#include <QBoxLayout>

class ControlPanel;
class panel_page;
class AQHBoxLayout;
class AQVBoxLayout;

class SplitScreen : public QFrame
{
    Q_OBJECT

public:
    SplitScreen(QWidget *parent = nullptr);

    void        addFloater(panel_page * pp);
    panel_page* removeFloater();
    panel_page* getFloater() { return floater; }

    void        setLHSWidth(int width);

signals:
    void        sig_adjust();

public slots:
    void        adjustMe(QSize,QSize);
    void        adjustMe2();

protected:private:
    AQHBoxLayout  * hbox;
    AQVBoxLayout  * vboxL;
    QWidget       * LHS;

    panel_page   * floater;
    QWidget      * dummy;
};

#endif // SPLITSCREEN_H
