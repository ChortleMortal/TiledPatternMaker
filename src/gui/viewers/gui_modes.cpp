#include <QDebug>
#include "gui/viewers/gui_modes.h"
#include "sys/sys.h"
#include "gui/top/view_controller.h"

GuiModes::GuiModes()
{
    iMouseMode   = MOUSE_MODE_NONE;
    keyboardMode = KBD_MODE_XFORM_VIEW;
}

void GuiModes::setKbdMode(eKbdMode mode)
{
    eKbdMode newMode = getValidKbdMode(mode);
    if (newMode != keyboardMode)
    {
        keyboardMode = newMode;
        qDebug().noquote() << "Keyboard Mode is:" << getKbdModeStr();
        emit sig_kbdMode(keyboardMode);
    }
}

void GuiModes::resetKbdMode()
{
    setKbdMode(keyboardMode);   // converts if necessary
    emit sig_kbdMode(keyboardMode);
}

bool GuiModes::getKbdMode(eKbdMode mode)
{
    return (mode == keyboardMode);
}

QString GuiModes::getKbdModeStr()
{
    return sKbdMode[keyboardMode];
}

bool GuiModes::getMouseMode(eMouseMode mode)
{
    if (mode == MOUSE_MODE_NONE)
        return iMouseMode == 0;
    else
        return (iMouseMode & mode) > 0;
}

void GuiModes::setMouseMode(eMouseMode newMode, bool set)
{
    static unsigned int lastMode = 0;

    qDebug() << "MouseMode:" << newMode;

    if (set)
    {
        switch (newMode)
        {
        case MOUSE_MODE_NONE:
            iMouseMode = 0;
            break;

        case MOUSE_MODE_CENTER:
            lastMode = iMouseMode;
            iMouseMode = MOUSE_MODE_CENTER;
            break;
        case MOUSE_MODE_TRANSLATE:
            iMouseMode &= ~MOUSE_MODE_CENTER;
            iMouseMode |= newMode;
            break;

        case MOUSE_MODE_ROTATE:
            iMouseMode &= ~MOUSE_MODE_CENTER;
            iMouseMode &= ~MOUSE_MODE_SCALE;
            iMouseMode |= newMode;
            break;

        case MOUSE_MODE_SCALE:
            iMouseMode &= ~MOUSE_MODE_CENTER;
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

        case MOUSE_MODE_CENTER:
            iMouseMode = lastMode;
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

eKbdMode GuiModes::getValidKbdMode(eKbdMode mode)
{
    if (Sys::viewController->isEnabled(VIEW_DESIGN))
    {
        return getValidDesignMode(mode);
    }
    else
    {
        return getValidMosaicMode(mode);
    }
}

eKbdMode GuiModes::getValidDesignMode(eKbdMode mode)
{
    switch (mode)
    {
    case KBD_MODE_DES_POS:
    case KBD_MODE_DES_LAYER_SELECT:
    case KBD_MODE_DES_ZLEVEL:
    case KBD_MODE_DES_STEP:
    case KBD_MODE_DES_SEPARATION:
    case KBD_MODE_DES_ORIGIN:
    case KBD_MODE_DES_OFFSET:
    case KBD_MODE_MOVE:
        return mode;
    case KBD_MODE_XFORM_VIEW:
    case KBD_MODE_XFORM_SELECTED:
    case KBD_MODE_XFORM_BKGD:
    case KBD_MODE_XFORM_TILING:
    case KBD_MODE_XFORM_UNIQUE_TILE:
    case KBD_MODE_XFORM_PLACED_TILE:
    case KBD_MODE_XFORM_GRID:
        break;
    }
    return KBD_MODE_DES_LAYER_SELECT;
}

eKbdMode GuiModes::getValidMosaicMode(eKbdMode mode)
{
    switch (mode)
    {
    case KBD_MODE_XFORM_VIEW:
    case KBD_MODE_XFORM_SELECTED:
    case KBD_MODE_XFORM_BKGD:
    case KBD_MODE_XFORM_TILING:
    case KBD_MODE_XFORM_UNIQUE_TILE:
    case KBD_MODE_XFORM_PLACED_TILE:
    case KBD_MODE_XFORM_GRID:
    case KBD_MODE_MOVE:
        return mode;
    case KBD_MODE_DES_POS:
    case KBD_MODE_DES_LAYER_SELECT:
    case KBD_MODE_DES_ZLEVEL:
    case KBD_MODE_DES_STEP:
    case KBD_MODE_DES_SEPARATION:
    case KBD_MODE_DES_ORIGIN:
    case KBD_MODE_DES_OFFSET:
        break;
    }
    return KBD_MODE_XFORM_VIEW;
}
