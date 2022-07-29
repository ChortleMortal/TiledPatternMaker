#ifndef COLORSET_H
#define COLORSET_H

#include <QColor>
#include "widgets/panel_misc.h"

class ColorSet;

class TPColor
{
public:
    TPColor() { hidden = false; }
    TPColor(QColor color) { this->color = color; hidden = false; }
    TPColor(QColor color, bool hide) { this->color = color; this->hidden = hide; }

    QColor color;
    bool   hidden;
};

class  ColorGroup
{
public:
    ColorGroup();

    void          addColorSet(ColorSet & cset);
    void          setColorSet(int idx, ColorSet & cset);
    ColorSet *    getNextColorSet();
    ColorSet *    getColorSet(int idx);
    void          removeColorSet(int idx);

    void          hide(int idx, bool hide);
    bool          isHidden(int idx);

    void          resetIndex();
    void          clear() { colorgroup.clear(); }

    int           size() { return static_cast<int>(colorgroup.size()); }
    void          resize(int num) { colorgroup.resize(num); }

private:
    QVector<ColorSet>           colorgroup;
    int                         ipos;
};

class ColorSet
{
public:
    ColorSet();

    void            addColor(QColor color, bool hide=false);
    void            addColor(TPColor color);
    void            setColor(int idx, QColor color, bool hide=false);
    void            setColor(int idx, TPColor tpcolor);

    void            setColors(QVector<TPColor> &cset);
    void            setColors(const ColorSet &cset);

    void            setOpacity(float val);

    void            removeColor(int idx);

    TPColor         getFirstColor();
    TPColor         getNextColor();
    TPColor         getColor(int index) { return colorset[index % colorset.size()]; }
    QString         colorsString();

    void            resetIndex() { pos = colorset.begin(); }
    void            clear();
    int             size() { return colorset.size(); }
    void            resize(int num) { colorset.resize(num); }

    void            hide(bool hide) { hidden = hide; }
    bool            isHidden() { return hidden; }

    AQHBoxLayout *  createLayout();
    AQWidget     *  createWidget();

protected:

private:
    QVector<TPColor> colorset;
    bool             hidden;
    QVector<TPColor>::iterator pos;
};

#endif // COLORSET_H
