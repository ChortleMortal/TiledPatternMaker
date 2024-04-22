#pragma once
#ifndef ESTYLETYPE_H
#define ESTYLETYPE_H

enum eStyleType
{
    STYLE_FILLED,
    STYLE_EMBOSSED,
    STYLE_INTERLACED,
    STYLE_OUTLINED,
    STYLE_PLAIN,
    STYLE_SKETCHED,
    STYLE_STYLE,
    STYLE_THICK,
    STYLE_TILECOLORS,
    STYLE_BORDER
};

enum eDrawOutline
{
    OUTLINE_NONE,
    OUTLINE_DEFAULT,    // backwards compatible - color = black, width = 1
    OUTLINE_SET
};

#endif // ESTYLETYPE_H
