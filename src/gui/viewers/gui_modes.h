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

    void     setTMKbdMode(eTMMode mode);
    bool     getTMKbdMode(eTMMode mode);
    eTMMode  TMKbdMode() { return TMKeyboardMode; }
    QString  getTMKbdModeStr();
    void     resetTMKbdMode();

    void     setLegacyKbdMode(eLegacyMode mode);
    bool     getLegacyKbdMode(eLegacyMode mode);
    eLegacyMode  LegacyKbdMode() { return LegacyKeyboardMode; }
    QString  getLegacyKbdModeStr();
    void     resetLegacyKbdMode();

    void     setMouseMode(eMouseMode newMode, bool set);
    bool     getMouseMode(eMouseMode mode);

signals:
    void    sig_TMKbdMode(eTMMode);
    void    sig_LegacyKbdMode(eLegacyMode);

private:
    uint        iMouseMode;
    uint        iLastMouseMode;
    eTMMode     TMKeyboardMode;
    eLegacyMode LegacyKeyboardMode;
};

#endif // GUI_MODES_H
