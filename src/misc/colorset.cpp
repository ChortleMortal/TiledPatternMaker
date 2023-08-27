#include "misc/colorset.h"

ColorSet::ColorSet()
{
    pos = colorset.end();
    hidden = false;
}

void ColorSet::clear()
{
    colorset.clear();
    pos = colorset.begin();
}

void ColorSet::setColor(int idx, QColor color, bool hide)
{
    colorset.replace(idx,TPColor(color,hide));
    pos = colorset.begin();
}

void ColorSet::setColor(int idx, TPColor tpcolor)
{
    colorset.replace(idx,tpcolor);
    pos = colorset.begin();
}

void ColorSet::addColor(QColor color, bool hidden)
{
    colorset.push_back(TPColor(color,hidden));
    pos = colorset.begin();
}

void ColorSet::addColor(TPColor color)
{
    colorset.push_back(color);
    pos = colorset.begin();
}

void ColorSet::setColors(QVector<TPColor> &cset)
{
    clear();
    for (auto it = cset.begin(); it != cset.end(); it++)
    {
        TPColor tp = *it;
        colorset.push_back(tp);
    }
    pos = colorset.begin();
}

void  ColorSet::setColors(const ColorSet & cset)
{
    clear();
    colorset = cset.colorset;
    pos = colorset.begin();
}

void ColorSet::setOpacity(float val)
{
    for (auto & tpcolor : colorset)
    {
        QColor c = tpcolor.color;
        c.setAlphaF(val);
        tpcolor.color = c;
    }
}

TPColor ColorSet::getFirstColor()
{
    pos = colorset.begin();
    return getNextColor();
}

TPColor ColorSet::getNextColor()
{
    if (size() == 0)
    {
        colorset.push_back(TPColor(Qt::yellow,false));
        pos = colorset.begin();
    }
    Q_ASSERT(pos != colorset.end());

    TPColor color = *pos;
    if (++pos == colorset.end())
    {
        pos = colorset.begin();
    }
    return color;
}

void ColorSet::removeColor(int idx)
{
    colorset.removeAt(idx);
    pos = colorset.begin();
}

QString ColorSet::colorsString() const
{
    QString str("Colors: ");
    for (auto & tpcolor : colorset)
    {
        str += tpcolor.color.name(QColor::HexArgb);
        str += " ";
    }
    return str;
}

QWidget * ColorSet::createWidget()
{
    AQHBoxLayout * hbox = createLayout();
    QWidget    * widget = new QWidget();
    widget->setLayout(hbox);
    return widget;
}

AQHBoxLayout * ColorSet::createLayout()
{
    AQHBoxLayout * hbox = new AQHBoxLayout;
    for (auto it = colorset.begin(); it != colorset.end(); it++)
    {
        TPColor tpcolor  = *it;
        QColor color     = tpcolor.color;
        QColor fullColor = color;
        fullColor.setAlpha(255);
        QLabel * label   = new QLabel;
        label->setMinimumWidth(50);
        QVariant variant = fullColor;
        QString colcode  = variant.toString();
        label->setStyleSheet("QLabel { background-color :"+colcode+" ;}");
        hbox->addWidget(label);
    }
    return hbox;
}

////////////////////////////////////////////////////////
///
////////////////////////////////////////////////////////

ColorGroup::ColorGroup()
{
    ipos = -1;
}

void ColorGroup::addColorSet(ColorSet & cset)
{
    colorgroup.push_back(cset);
    ipos = 0;
}

void ColorGroup::setColorSet(int idx, ColorSet & cset)
{
    colorgroup.replace(idx,cset);
    ipos = 0;
}

ColorSet * ColorGroup::getColorSet(int idx)
{
    return &colorgroup[idx];
}

void ColorGroup::removeColorSet(int idx)
{
    colorgroup.removeAt(idx);
    ipos = 9;
}

void  ColorGroup::resetIndex()
{
    if (colorgroup.size())
        ipos = 0;
    else
        ipos = -1;

    for (auto & fset : colorgroup)
    {
        fset.resetIndex();
    }
}

ColorSet * ColorGroup::getNextColorSet()
{
    if (size() == 0 || ipos == -1)
    {
        ColorSet colorSet;
        colorgroup.push_back(colorSet);
        ipos = 0;
    }
    ColorSet * cset = &colorgroup[ipos++];
    if (ipos >= colorgroup.size())
    {
        ipos = 0;
    }
    return cset;
}

void ColorGroup::hide(int idx, bool hide)
{
    if (idx >= 0 && idx < size())
    {
        colorgroup[idx].hide(hide);
    }

}
bool  ColorGroup::isHidden(int idx)
{
    if (idx >= 0 && idx < size())
    {
        return colorgroup[idx].isHidden();
    }
    return false;
}
