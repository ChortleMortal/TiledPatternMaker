#pragma once
#ifndef COLORED_H
#define COLORED_H

#include <QColor>

#include "model/styles/style.h"
#include "model/styles/colorset.h"

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
    Colored(const ProtoPtr & proto);
    Colored(const StylePtr &other);
    virtual ~Colored();

    ColorSet colors;
};
#endif

