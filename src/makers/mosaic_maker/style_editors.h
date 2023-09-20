#pragma once
#ifndef STYLED_EDITORS_H
#define STYLED_EDITORS_H

#include <memory>
#include <QObject>
#include <QPointF>
#include <QColor>

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

typedef std::shared_ptr<class Filled>       FilledPtr;
typedef std::shared_ptr<class Tile>      TilePtr;
typedef std::shared_ptr<class Tiling>       TilingPtr;

class StyleEditor : public QObject
{
    Q_OBJECT

public:
    StyleEditor();

    virtual void onEnter() {};
    virtual void onExit()  {};

signals:
    void    sig_refreshView();
    void    sig_updateView();
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
class StyleColorFillGroup;

// Editor for the filled style.
class FilledEditor : public StyleEditor
{
    Q_OBJECT

public:
    FilledEditor(FilledPtr f, AQTableWidget *table, QVBoxLayout *parmsCtrl );
    ~FilledEditor();

    FilledPtr getFilled() { return filled; }

    virtual void onEnter();
    virtual void onExit();

public slots:
    void slot_colorsChanged();
    void slot_mousePressed(QPointF spt, Qt::MouseButton btn);
    void slot_colorPick(QColor color);

private slots:
    void slot_insideChanged(int state);
    void slot_outsideChanged(int state);
    void slot_editB();
    void slot_editW();

protected:
    void slot_algo(int index);

    void displayParms();
    void displayParms01();
    void displayParms2();
    void displayParms3();

private:
    FilledPtr       filled;

    AQTableWidget * table;

    QCheckBox     * inside_checkbox;
    QCheckBox     * outside_checkbox;
    QComboBox     * algoBox;

    QVBoxLayout   * vbox;

    StyleColorFillSet   * fillSet;
    StyleColorFillGroup * fillGroup;

    class ViewControl   * view;
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

private slots:
    void    slot_edit();
    void    slot_colors_changed();

    void    slot_outlineChanged(int state);
    void    slot_outline_color();
    void    slot_widthChanged(int val);

protected:
    void    buildTable();

private:
    class Configuration     * config;
    class ControlPanel      * panel;
    TilingPtr                 tiling;
    TileColors              * colored;
    AQTableWidget           * table;

    SliderSet               * width_slider;
    QCheckBox               * outline_checkbox;
    AQWidget                * colorwidget;
    QPushButton             * color_button;
    DoubleSliderSet         * transparency;
    QTableWidgetItem        * colorItem;

    QVector<TilePtr> qlfp;

};
#endif
