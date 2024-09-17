#define pragma once
#ifndef GUI_MODES_H
#define GUI_MODES_H

#include <QObject>
#include "sys/enums/ekeyboardmode.h"
#include "sys/enums/emousemode.h"


class GuiModes : public QObject
{
    Q_OBJECT

public:
    GuiModes();

    void     setKbdMode(eKbdMode mode);
    bool     getKbdMode(eKbdMode mode);
    QString  getKbdModeStr();
    void     resetKbdMode();
    eKbdMode getValidKbdMode(eKbdMode mode);

    void     setMouseMode(eMouseMode newMode, bool set);
    bool     getMouseMode(eMouseMode mode);

signals:
    void sig_kbdMode(eKbdMode);

protected:
    eKbdMode getValidMosaicMode(eKbdMode mode);
    eKbdMode getValidDesignMode(eKbdMode mode);

private:
    uint      iMouseMode;
    uint      iLastMouseMode;
    eKbdMode  keyboardMode;
};

#endif // GUI_MODES_H
