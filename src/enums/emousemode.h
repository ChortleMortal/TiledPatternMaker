#ifndef EMOUSEMODE_H
#define EMOUSEMODE_H

enum eMouseMode
{
    MOUSE_MODE_NONE         = 0x00,
    MOUSE_MODE_TRANSLATE    = 0x01,
    MOUSE_MODE_ROTATE       = 0x02,
    MOUSE_MODE_SCALE        = 0x04,
    MOUSE_MODE_CENTER       = 0x08
};

#endif // EMOUSEMODE_H
