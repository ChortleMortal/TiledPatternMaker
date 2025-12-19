#pragma once
#ifndef COLORSET_H
#define COLORSET_H

#include <QColor>
#include "gui/panels/panel_misc.h"

class ColorSet;

class TPColor
{
public:
    TPColor()                        { color = Qt::cyan;    hidden = false; }
    TPColor(QColor col)              { color = col;         hidden = false; }
    TPColor(QColor col, bool hide)   { color = col;         hidden = hide; }
    TPColor(const TPColor & other)   { color = other.color; hidden = other.hidden; }

    TPColor & operator=(const TPColor & other) { color = other.color; hidden = other.hidden; return *this; }

    QColor color;
    bool   hidden;
};

class  ColorGroup : public  QVector<ColorSet>
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

    void          dump();

private:
    QVector<ColorSet>::iterator  pos;
};

class ColorSet : public QVector<TPColor>
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

    void            removeTPColor(int idx);

    TPColor         getFirstTPColor();
    TPColor         getLastTPColor();
    TPColor         getNextTPColor();
    TPColor         getTPColor(int index) { return at(index % size()); }
    QColor          getQColor(int index)  { if (!size()) return QColor(Qt::red); else return getTPColor(index).color;}

    void            resetIndex();
    void            clear();

    void            hide(bool hide) { hidden = hide; }
    bool            isHidden() { return hidden; }

    AQHBoxLayout *  createLayout();
    QWidget      *  createWidget();

    void            dump();
    QString         colorsString() const;

private:
    bool             hidden;
    QVector<TPColor>::iterator pos;
};

#endif // COLORSET_H
