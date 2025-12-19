#include <QDebug>
#include "gui/viewers/gui_modes.h"
#include "sys/sys.h"

GuiModes::GuiModes()
{
    iMouseMode          = MOUSE_MODE_NONE;
    TMKeyboardMode      = TM_MODE_XFORM_ALL;
    LegacyKeyboardMode  = LEGACY_MODE_DES_POS;
}

// Tiling Maker

void GuiModes::setTMKbdMode(eTMMode mode)
{
    if (mode != TMKeyboardMode)
    {
        TMKeyboardMode = mode;
        qDebug().noquote() << "Keyboard Mode is:" << getTMKbdModeStr();
        emit sig_TMKbdMode(mode);
    }
}

void GuiModes::resetTMKbdMode()
{
    setTMKbdMode(TM_MODE_XFORM_ALL);
    emit sig_TMKbdMode(TMKeyboardMode);
}

bool GuiModes::getTMKbdMode(eTMMode mode)
{
    return (mode == TMKeyboardMode);
}

QString GuiModes::getTMKbdModeStr()
{
    return sTMMode[TMKeyboardMode];
}


// Loader

void GuiModes::setLegacyKbdMode(eLegacyMode mode)
{
    if (mode != LegacyKeyboardMode)
    {
        LegacyKeyboardMode = mode;
        qDebug().noquote() << "Keyboard Mode is:" << getLegacyKbdModeStr();
        emit sig_LegacyKbdMode(mode);
    }
}

void GuiModes::resetLegacyKbdMode()
{
    setLegacyKbdMode(LEGACY_MODE_DES_POS);
    emit sig_LegacyKbdMode(LegacyKeyboardMode);
}

bool GuiModes::getLegacyKbdMode(eLegacyMode mode)
{
    return (mode == LegacyKeyboardMode);
}

QString GuiModes::getLegacyKbdModeStr()
{
    return sLegacyMode[LegacyKeyboardMode];
}


// Mouse

bool GuiModes::getMouseMode(eMouseMode mode)
{
    if (mode == MOUSE_MODE_NONE)
        return iMouseMode == 0;
    else
        return (iMouseMode & mode) > 0;
}


void GuiModes::setMouseMode(eMouseMode newMode, bool set)
{
    qDebug() << "MouseMode:" << newMode;

    if (set)
    {
        switch (newMode)
        {
        case MOUSE_MODE_NONE:
            iMouseMode = 0;
            break;

        case MOUSE_MODE_TRANSLATE:
            iMouseMode |= newMode;
            break;

        case MOUSE_MODE_ROTATE:
            iMouseMode &= ~MOUSE_MODE_SCALE;
            iMouseMode |= newMode;
            break;

        case MOUSE_MODE_SCALE:
            iMouseMode &= ~MOUSE_MODE_ROTATE;
            iMouseMode |= newMode;
            break;
        }
    }
    else
    {
        switch (newMode)
        {
        case MOUSE_MODE_NONE:
            break;

        case MOUSE_MODE_TRANSLATE:
            iMouseMode &= ~newMode;
            break;

        case MOUSE_MODE_ROTATE:
            iMouseMode &= ~newMode;
            break;

        case MOUSE_MODE_SCALE:
            iMouseMode &= ~newMode;
            break;
        }
    }

    if (iMouseMode)
        Sys::showCenterMouse = true;
    else
        Sys::showCenterMouse = false;
}

