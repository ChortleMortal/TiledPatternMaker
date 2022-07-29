#include "style/colored.h"

////////////////////////////////////////////////////////////////////////////
//
// A style encapsulates drawing a map with colors.


////////////////////////////////////////////////////////////////////////////
//
// Creation.

Colored::Colored(const PrototypePtr & proto) : Style(proto)
{
    colors.addColor(QColor( 20, 150, 210 ));
}

Colored::Colored(const StylePtr & other) : Style(other)
{
    std::shared_ptr<Colored> otherColored = std::dynamic_pointer_cast<Colored>(other);
    if (otherColored)
    {
        colors.setColors(otherColored->colors);
    }
    else
    {
        colors.addColor(QColor( 20, 150, 210 ));
    }
}

Colored::~Colored()
{
#ifdef EXPLICIT_DESTRUCTOR
    qDebug() << "deleting colored";
#endif
}

void Colored::setColor(QColor color)
{
    colors.clear();
    colors.addColor(color);
}
