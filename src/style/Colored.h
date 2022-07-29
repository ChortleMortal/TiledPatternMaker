#ifndef COLORED_H
#define COLORED_H

#include <QColor>

#include "style/style.h"
#include "misc/colorset.h"

////////////////////////////////////////////////////////////////////////////
//
// A style encapsulates drawing a map with colors.


class Colored : public Style
{
public:
    void   setColor(QColor color);
    void   setColorSet(ColorSet & cset) { colors = cset; }

    ColorSet * getColorSet() { return &colors; }

protected:
    Colored(const PrototypePtr & proto);
    Colored(const StylePtr &other);
    ~Colored();

    ColorSet colors;
};
#endif

