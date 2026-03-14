#pragma once
#ifndef COLORSET_H
#define COLORSET_H

#include <QColor>

class TPColor
{
public:
    TPColor()                        { color = Qt::yellow;  hidden = false; }
    TPColor(QColor col)              { color = col;         hidden = false; }
    TPColor(QColor col, bool hide)   { color = col;         hidden = hide; }
    TPColor(const TPColor & other)   { color = other.color; hidden = other.hidden; }

    TPColor & operator=(const TPColor & other) { color = other.color; hidden = other.hidden; return *this; }

    bool operator==(const TPColor&) const = default;
    bool operator!=(const TPColor&) const = default;

    void    dump();
    QString info();

    QColor color;
    bool   hidden;
};

class ColorSet : public QVector<TPColor>
{
public:
    ColorSet();

    bool operator==(const ColorSet&) const = default;
    bool operator!=(const ColorSet&) const = default;

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
    TPColor         getTPColor(int index) { if (!size()) return QColor(Qt::red); else return at(index % size()); }
    QColor          getQColor(int index)  { if (!size()) return QColor(Qt::red); else return getTPColor(index %size()).color;}

    void            clear();

    void            hide(bool hide) { hidden = hide; }
    bool            isHidden() { return hidden; }

    QString         info();
    void            dump();
    QString         colorsString() const;

private:
    bool             hidden;
};

class  ColorGroup : public  QVector<ColorSet>
{
public:
    ColorGroup();

    bool operator==(const ColorGroup&) const = default;
    bool operator!=(const ColorGroup&) const = default;

    void          addColorSet(ColorSet & cset);
    void          setColorSet(int idx, ColorSet & cset);
    ColorSet *    getColorSet(int idx);
    void          removeColorSet(int idx);

    void          hide(int idx, bool hide);
    bool          isHidden(int idx);

    void          dump();
};


#endif // COLORSET_H
