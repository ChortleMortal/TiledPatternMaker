#pragma once
#ifndef TILE_COLOR_EDITOR_H
#define TILE_COLOR_EDITOR_H

#include <QObject>
#include <QPointF>
#include <QColor>
#include <QWidget>

#include "model/styles/colorset.h"
#include "sys/enums/estyletype.h"
#include "gui/model_editors/style_edit/style_editor.h"

class QPushButton;
class QCheckBox;
class QTableWidgetItem;
class QComboBox;
class QVBoxLayout;
class QHBoxLayout;

class DoubleSliderSet;
class SliderSet;
class EdgePoly;
class AQTableWidget;

class TileColors;

using std::shared_ptr;
using std::weak_ptr;

typedef shared_ptr<class Tiling>    TilingPtr;
typedef shared_ptr<class Style>     StylePtr;
typedef shared_ptr<class Tile>      TilePtr;
typedef weak_ptr<class Tile>       wTilePtr;
typedef weak_ptr<class Style>      wStylePtr;

// Editor for the colored style: allow choosing the color.
class TileColorsEditor : public StyleEditor
{
    Q_OBJECT
    enum eTileCol
    {
        TILE_COLORS_SHOW    = 0,
        TILE_COLORS_COLORS  = 1,
        TILE_COLORS_EDIT    = 2
    };

public:
    TileColorsEditor(StylePtr style, eStyleType user);

    void onRefresh() override;

public slots:
    void    slot_colors_changed();

private slots:
    void    slot_edit();

    void    slot_outlineChanged(bool checked);
    void    slot_outline_color();
    void    slot_widthChanged(int val);

protected:
    void    buildTable();
    void    populateRow(int row, ColorSet & cset);
    void    refreshRow(int row);

private:
    class Configuration     * config;
    class ControlPanel      * panel;

    weak_ptr<TileColors>      wtilecolors;
    weak_ptr<Tiling>          wtiling;

    SliderSet               * outline_width_slider;
    QCheckBox               * outline_checkbox;
    QPushButton             * outline_color_button;
    QPushButton             * outline_color_patch;

    QVector<wTilePtr>        uniqueTiles;

};
#endif
