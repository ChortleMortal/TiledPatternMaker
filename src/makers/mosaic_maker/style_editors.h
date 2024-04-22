#pragma once
#ifndef STYLED_EDITORS_H
#define STYLED_EDITORS_H

#include <QObject>
#include <QPointF>
#include <QColor>
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
#include <memory>
#endif

class QPushButton;
class QCheckBox;
class QTableWidgetItem;
class QComboBox;
class QVBoxLayout;

class DoubleSliderSet;
class SliderSet;
class Emboss;
class TileColors;
class EdgePoly;
class Interlace;
class Colored;
class Thick;
class AQTableWidget;
class AQWidget;
class View;

typedef std::shared_ptr<class Filled>    FilledPtr;
typedef std::shared_ptr<class Tile>      TilePtr;
typedef std::weak_ptr<class Tile>       wTilePtr;
typedef std::shared_ptr<class Tiling>    TilingPtr;

class StyleEditor : public QObject
{
    Q_OBJECT

public:
    StyleEditor();

    virtual void onEnter()   {};
    virtual void onExit()    {};
    virtual void onRefresh() {};

signals:
    void    sig_refreshView();
    void    sig_colorsChanged();

protected:
};


// Editor for the colored style: allow choosing the color.
class ColoredEditor : public StyleEditor
{
    Q_OBJECT

public:
    ColoredEditor(Colored * c, AQTableWidget *table);

public slots:
    void slot_colorsChanged();

private slots:
    void slot_opacityChanged(qreal val);
    void slot_pickColor();

protected:
    int             rows;
    AQTableWidget   * table;

private:
    void updateOpacity();

    Colored         * colored;
    QWidget         * colorwidget;
    QPushButton     * color_button;
    DoubleSliderSet * opacitySlider;
};

// Editor for the thick style.
class ThickEditor : public ColoredEditor
{
    Q_OBJECT

public:
    ThickEditor(Thick * thick, AQTableWidget *table);

 private slots:
    void  slot_outlineChanged(int state);
    void  slot_widthChanged(int width);
    void  slot_outlineWidthChanged(int width);
    void  slot_outlineColor();
    void  slot_joinStyle(int index);
    void  slot_capStyle(int index);

private:
    Thick            * thick;
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
    FilledEditor(FilledPtr f, AQTableWidget *table, QVBoxLayout *parmsCtrl);
    ~FilledEditor();

    FilledPtr getFilled() { return filled; }

    void onEnter() override;
    void onExit() override;
    void onRefresh() override;

public slots:
    void slot_colorsChanged();
    void slot_mousePressed(QPointF spt, Qt::MouseButton btn);
    void slot_colorPick(QColor color);
    void slot_editW();
    void slot_editB();

protected:
    void slot_algo(int index);

    void displayParms();
    void displayParms1();
    void displayParms2();
    void displayParms3();
    void displayParmsPalette();

private:
    FilledPtr       filled;

    AQTableWidget * table;

    QComboBox     * algoBox;

    QVBoxLayout   * vbox;

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
    EmbossEditor(Emboss * e, AQTableWidget *table);

private slots:
    void slot_anlgeChanged(int angle);
    void slot_colorsChanged();

private:
    Emboss      * emboss;
    SliderSet   * angle_slider;
};

// Editor for teh interlaced style.
class InterlaceEditor : public ThickEditor
{
    Q_OBJECT

public:
    InterlaceEditor(Interlace * i, AQTableWidget * table);

private slots :
    void slot_gapChanged(qreal gap);
    void slot_shadowChanged(qreal shadow);
    void slot_startUnderChanged(int state);
    void slot_includeTipVerticesChanged(int state);
    void slot_colorsChanged();

private:
    Interlace * interlace;

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
    TileColorsEditor(TileColors * c, AQTableWidget * table, TilingPtr tiling);

public slots:
    void    slot_colors_changed();

private slots:
    void    slot_edit();

    void    slot_outlineChanged(int state);
    void    slot_outline_color();
    void    slot_widthChanged(int val);

protected:
    void    buildTable();

private:
    class Configuration     * config;
    class ControlPanel      * panel;

    TilingPtr                 tiling;
    TileColors              * colStyle;

    AQTableWidget           * table;
    SliderSet               * width_slider;
    QCheckBox               * outline_checkbox;
    AQWidget                * colorwidget;
    QPushButton             * color_button;
    DoubleSliderSet         * transparency;
    QTableWidgetItem        * colorItem;

    QVector<wTilePtr>        uniqueTiles;

};
#endif
