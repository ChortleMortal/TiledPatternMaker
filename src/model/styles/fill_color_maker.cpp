#include <QDebug>
#include <QStack>

#include "model/styles/fill_color_maker.h"

///////////////////////////////////////////////
///
///  Color Maker
///
///////////////////////////////////////////////
ColorMaker::ColorMaker(Filled * filled)
{
    this->filled = filled;
    initialised = false;
}

ColorMaker::~ColorMaker()
{
#ifdef EXPLICIT_DESTRUCTOR
    qDebug() "ColorMaker - destructor";
#endif
}





