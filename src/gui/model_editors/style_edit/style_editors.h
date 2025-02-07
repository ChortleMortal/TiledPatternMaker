#pragma once
#ifndef STYLED_EDITORS_H
#define STYLED_EDITORS_H

#include <QObject>
#include <QPointF>
#include <QColor>
#include <QWidget>
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
#include <memory>
#endif

class QPushButton;
class QCheckBox;
class QTableWidgetItem;
class QComboBox;
class QVBoxLayout;
class QHBoxLayout;

class DoubleSliderSet;
class SliderSet;
class Emboss;
class TileColors;
class EdgePoly;
class Interlace;
class Colored;
class Thick;
class AQTableWidget;
class View;


using std::shared_ptr;
using std::weak_ptr;

typedef shared_ptr<class Filled>    FilledPtr;
typedef shared_ptr<class Style>     StylePtr;
typedef shared_ptr<class Tiling>    TilingPtr;
typedef shared_ptr<class Tile>      TilePtr;
typedef weak_ptr<class Tile>       wTilePtr;

class StyleEditor : public QWidget
{
    Q_OBJECT

public:
    StyleEditor();

    virtual void onEnter()   {};
    virtual void onExit()    {};
    virtual void onRefresh() {};

signals:
    void    sig_reconstructView();
    void    sig_updateView();
    void    sig_colorsChanged();

protected:
    AQTableWidget * setable;
};


// Editor for the colored style: allow choosing the color.
class ColoredEditor : public StyleEditor
{
    Q_OBJECT

public:
    ColoredEditor(StylePtr style);

public slots:
    void slot_colorsChanged();

private slots:
    void slot_opacityChanged(qreal val);
    void slot_pickColor();

protected:
    int             rows;

private:
    void updateOpacity();

    weak_ptr<Colored>  wcolored;
    QWidget         * colorwidget;
    QPushButton     * color_button;
    DoubleSliderSet * opacitySlider;
};

// Editor for the thick style.
class ThickEditor : public ColoredEditor
{
    Q_OBJECT

public:
    ThickEditor(StylePtr style);

 private slots:
    void  slot_outlineChanged(bool checked);
    void  slot_widthChanged(int width);
    void  slot_outlineWidthChanged(int width);
    void  slot_outlineColor();
    void  slot_joinStyle(int index);
    void  slot_capStyle(int index);

private:
    weak_ptr<Thick>    wthick;
    SliderSet        * width_slider;
    QCheckBox        * outline_checkbox;
    SliderSet        * outline_width_slider;
    QTableWidgetItem * outline_color;
    QPushButton      * outline_color_button;
    QComboBox        * join_style;
    QComboBox        * cap_style;
};

class StyleColorFillSet;
class StyleColorFillFace;
class StyleColorFillGroup;
class StyleColorFillOriginal;

// Editor for the filled style.
class FilledEditor : public StyleEditor
{
    Q_OBJECT

public:
    FilledEditor(StylePtr style);
    ~FilledEditor();

    FilledPtr getFilled() { return wfilled.lock(); }

    void onEnter() override;
    void onExit() override;
    void onRefresh() override;

    void displayParms();

public slots:
    void slot_colorsChanged();
    void slot_mousePressed(QPointF spt, Qt::MouseButton btn);
    void slot_colorPick(QColor color);
    void slot_editW();
    void slot_editB();

protected:
    void slot_algo(int index);

    void displayParms1();
    void displayParms2();
    void displayParms3();
    void displayParmsPalette();

private:
    weak_ptr<Filled>    wfilled;

    QComboBox         * algoBox;
    QVBoxLayout       * vbox;
    QHBoxLayout       * hbox;

    StyleColorFillSet       * fillSet;
    StyleColorFillGroup     * fillGroup;
    StyleColorFillFace      * fillFaces;
    StyleColorFillOriginal  * fillOriginal;

    class ControlPanel  * panel;
};


// Editor for the embossed style.
class EmbossEditor : public ThickEditor
{
    Q_OBJECT

public:
    EmbossEditor(StylePtr style);

private slots:
    void slot_anlgeChanged(int angle);
    void slot_colorsChanged();

private:
    weak_ptr<Emboss>    wemboss;
    SliderSet         * angle_slider;
};

// Editor for teh interlaced style.
class InterlaceEditor : public ThickEditor
{
    Q_OBJECT

public:
    InterlaceEditor(StylePtr style);

private slots :
    void slot_gapChanged(qreal gap);
    void slot_shadowChanged(qreal shadow);
    void slot_startUnderChanged(bool checked);
    void slot_includeTipVerticesChanged(bool checked);
    void slot_colorsChanged();

private:
    weak_ptr<Interlace> winterlace;

    DoubleSliderSet * gap_slider;
    DoubleSliderSet * shadow_slider;
    QCheckBox       * tipVert_checkbox;
    QCheckBox       * sunder_checkbox;      // start under
};

// Editor for the colored style: allow choosing the color.
class TileColorsEditor : public StyleEditor
{
    Q_OBJECT
    enum eTileCol
    {
        TILE_COLORS_ADDR    = 0,
        TILE_COLORS_SIDES   = 1,
        TILE_COLORS_BTN     = 2,
        TILE_COLORS_COLORS  = 3
    };

public:
    TileColorsEditor(StylePtr style);

public slots:
    void    slot_colors_changed();

private slots:
    void    slot_edit();

    void    slot_outlineChanged(bool checked);
    void    slot_outline_color();
    void    slot_widthChanged(int val);

protected:
    void    buildTable();

private:
    class Configuration     * config;
    class ControlPanel      * panel;

    weak_ptr<TileColors>      wtilecolors;
    weak_ptr<Tiling>          wtiling;

    SliderSet               * width_slider;
    QCheckBox               * outline_checkbox;
    QPushButton             * color_button;
    DoubleSliderSet         * transparency;
    QTableWidgetItem        * colorItem;

    QVector<wTilePtr>        uniqueTiles;

};
#endif
