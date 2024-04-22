#include <QDebug>
#include "misc/colorset.h"

ColorSet::ColorSet()
{
    pos = begin();
    hidden = false;
}

void ColorSet::clear()
{
    QVector<TPColor>::clear();
    pos = begin();
}

void ColorSet::setColor(int idx, QColor color, bool hide)
{
    replace(idx,TPColor(color,hide));
    pos = begin();
}

void ColorSet::setColor(int idx, TPColor tpcolor)
{
    replace(idx,tpcolor);
    pos = begin();
}

void ColorSet::addColor(QColor color, bool hidden)
{
    push_back(TPColor(color,hidden));
    pos = begin();
}

void ColorSet::addColor(TPColor color)
{
    push_back(color);
    pos = begin();
}

void ColorSet::setColors(QVector<TPColor> &cset)
{
    QVector<TPColor>::clear();
    for (auto & tp : std::as_const(cset))
    {
        push_back(tp);
    }
    pos = begin();
}

void  ColorSet::setColors(const ColorSet & cset)
{
    QVector<TPColor>::clear();
    *this = cset;
    pos = begin();
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

void ColorSet::resetIndex()
{
    pos = begin();
}

TPColor ColorSet::getFirstTPColor()
{
    pos = begin();
    return getNextTPColor();
}

TPColor ColorSet::getNextTPColor()
{
    if (size() == 0)
    {
        push_back(TPColor(Qt::yellow,false));
        pos = begin();
    }

    TPColor tpcolor = *pos;
    pos++;
    if (pos == end())
    {
        pos = begin();
    }
    return tpcolor;
}

void ColorSet::removeTPColor(int idx)
{
    idx = idx &size();
    removeAt(idx);
    pos = begin();
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
    for (auto & tpcolor :  * this)
    {
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
    QString astring;
    QDebug  deb(&astring);

    deb << "size:" << size();
    for (auto it = begin(); it != end(); it++)
    {
        auto tpc = *it;
        deb << tpc.color.name(QColor::HexArgb);
    }
    qDebug().noquote() << astring;
}

////////////////////////////////////////////////////////
///
////////////////////////////////////////////////////////

ColorGroup::ColorGroup()
{
    pos = end();
}

void ColorGroup::addColorSet(ColorSet & cset)
{
    push_back(cset);
    pos = begin();
}

void ColorGroup::setColorSet(int idx, ColorSet & cset)
{
    idx = idx % size();
    replace(idx,cset);
    pos = begin();
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
    pos = begin();
}

void  ColorGroup::resetIndex()
{
    pos = begin();
    for (auto & cset : *this)
    {
        cset.resetIndex();
    }
}

ColorSet * ColorGroup::getNextColorSet()
{
    if (size() == 0)
    {
        ColorSet colorSet;
        push_back(colorSet);
        pos = begin();
    }

    ColorSet * cset = &*pos;
    pos++;
    if (pos == end())
    {
        pos = begin();
    }
    return cset;
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
