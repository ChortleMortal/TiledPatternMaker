#include <QDebug>

#include "model/styles/colorset.h"

////////////////////////////////////////////////////
///
///  TPColor
///
////////////////////////////////////////////////////

QString TPColor::info()
{
    QString astring;
    QDebug  deb(&astring);

    deb.noquote() << color.name(QColor::HexArgb) << ((hidden) ? "hide" : "show");
    return astring;
}

void TPColor::dump()
{
    qDebug().noquote() <<  color.name(QColor::HexArgb) << ((hidden) ? "hide" : "show");
}

////////////////////////////////////////////////////
///
///  Color Set
///
////////////////////////////////////////////////////

ColorSet::ColorSet()
{
    hidden = false;
}

void ColorSet::clear()
{
    QVector<TPColor>::clear();
}

void ColorSet::setColor(int idx, QColor color, bool hide)
{
    replace(idx,TPColor(color,hide));
}

void ColorSet::setColor(int idx, TPColor tpcolor)
{
    replace(idx,tpcolor);
}

void ColorSet::addColor(QColor color, bool hidden)
{
    push_back(TPColor(color,hidden));
}

void ColorSet::addColor(TPColor color)
{
    push_back(color);
}

void ColorSet::setColors(QVector<TPColor> &cset)
{
    QVector<TPColor>::clear();
    for (auto & tp : std::as_const(cset))
    {
        push_back(tp);
    }
}

void  ColorSet::setColors(const ColorSet & cset)
{
    QVector<TPColor>::clear();
    *this = cset;
}

void ColorSet::setOpacity(float val)
{
    for (auto & tpcolor : *this)
    {
        QColor c = tpcolor.color;
        c.setAlphaF(val);
        tpcolor.color = c;
    }
}

TPColor ColorSet::getFirstTPColor()
{
    return first();
}

TPColor ColorSet::getLastTPColor()
{
    return last();
}

void ColorSet::removeTPColor(int idx)
{
    idx = idx % size();
    removeAt(idx);
}

QString ColorSet::colorsString() const
{
    QString str("Colors: ");
    for (auto & tpcolor : std::as_const(*this))
    {
        str += tpcolor.color.name(QColor::HexArgb);
        str += " ";
    }
    return str;
}

void ColorSet::dump()
{
    qDebug().noquote() << info();
}

QString ColorSet::info()
{
    QString astring;
    QDebug  deb(&astring);

    deb.noquote() << "(size:" << size() << ")";
    for (auto it = begin(); it != end(); it++)
    {
        auto tpc = *it;
        deb.noquote() << tpc.info();
    }
    return astring;
}

////////////////////////////////////////////////////////
///
///  Color Group
///
////////////////////////////////////////////////////////

ColorGroup::ColorGroup()
{}

void ColorGroup::addColorSet(ColorSet & cset)
{
    push_back(cset);
}

void ColorGroup::setColorSet(int idx, ColorSet & cset)
{
    idx = idx % size();
    replace(idx,cset);
}

ColorSet * ColorGroup::getColorSet(int idx)
{
    idx = idx % size();
    QVector<ColorSet> & cset  = *this;
    return &cset[idx];
}

void ColorGroup::removeColorSet(int idx)
{
    idx = idx % size();
    removeAt(idx);
}

void ColorGroup::hide(int idx, bool hide)
{
    idx = idx % size();
    auto cset = getColorSet(idx);
    cset->hide(hide);
}

bool  ColorGroup::isHidden(int idx)
{
    idx = idx % size();
    auto cset = getColorSet(idx);
    return cset->isHidden();
}

void ColorGroup::dump()
{
    for (auto & cset : *this)
    {
        cset.dump();
    }
}


