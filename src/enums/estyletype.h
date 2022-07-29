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
};

enum eDrawOutline
{
    OUTLINE_NONE,
    OUTLINE_DEFAULT,    // backwards compatible
    OUTLINE_SET
};

#endif // ESTYLETYPE_H
