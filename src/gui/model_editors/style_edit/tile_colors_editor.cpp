#include <QHeaderView>
#include <QComboBox>
#include <QCheckBox>

#include "gui/model_editors/style_edit/tile_colors_editor.h"
#include "gui/widgets/panel_misc.h"
#include "gui/viewers/motif_maker_view.h"
#include "gui/widgets/colorset_widget.h"
#include "gui/widgets/dlg_colorSet.h"
#include "gui/widgets/layout_sliderset.h"
#include "model/prototypes/prototype.h"
#include "model/styles/tile_colors.h"
#include "model/tilings/tile.h"
#include "model/tilings/tiling.h"
#include "sys/sys.h"

#define ROW_HEIGHT 39

///////////////////////////////////////////////////////////////
///
///   Til eColors Editor
///
///////////////////////////////////////////////////////////////

TileColorsEditor::TileColorsEditor(StylePtr style, eStyleType user) : StyleEditor(style,user)
{
    auto tileColors = std::dynamic_pointer_cast<TileColors>(style);
    wtilecolors     = tileColors;
    Q_ASSERT(wtilecolors.lock());

    wtiling = tileColors->getTiling();

    config  = Sys::config;
    panel   = Sys::controlPanel;

    if (!setable)
    {
        setable = new AQTableWidget(this);
        AQVBoxLayout * vbox = new AQVBoxLayout();
        vbox->addWidget(setable);
        setLayout(vbox);
    }

    buildTable();
}

void TileColorsEditor::buildTable()
{
    qDebug() << "TileColorsEditor::buildTable()";

    auto tileColors = wtilecolors.lock();
    if (!tileColors) return;

    tileColors->createStyleRepresentation();  // this sets up the colors if needed

    setable->clear();
    setable->setColumnCount(3);
    setable->setRowCount(2);        // 2 rows for outline
    setable->setColumnWidth(TILE_COLORS_COLORS, 400);
    setable->setSelectionMode(AQTableWidget::NoSelection);

    QStringList qslH;
    qslH << "Show" << "Colors" << "Edit";
    setable->setHorizontalHeaderLabels(qslH);
    setable->verticalHeader()->setVisible(false);

    QVector<TilePtr> tiles;
    auto tiling = wtiling.lock();
    if (tiling)
    {
        tiles = tiling->unit().getUniqueTiles();
        for (auto & tile : std::as_const(tiles))
        {
            uniqueTiles.push_back(tile);
        }
    }

    int     outline_width;
    QColor  outline_color;
    bool    outlineEnb = tileColors->getOutline(outline_color,outline_width);
    int     row     = 0;

    // outline
    outline_checkbox = new AQCheckBox("Outline");
    outline_checkbox->setChecked(outlineEnb);
    setable->setCellWidget(row,TILE_COLORS_SHOW,outline_checkbox);

    outline_color_patch = new QPushButton;
    outline_color_patch->setText("");
    QVariant variant = outline_color;
    QString colcode  = variant.toString();
    outline_color_patch->setStyleSheet("QPushButton { background-color :"+colcode+" ;}");
    setable->setCellWidget(row,TILE_COLORS_COLORS,outline_color_patch);
    connect(outline_color_patch,&QPushButton::clicked, this, &TileColorsEditor::slot_outline_color);

    outline_color_button = new QPushButton("Color");
    setable->setCellWidget(row,TILE_COLORS_EDIT,outline_color_button);

    row++;

    // outline width
    auto item = new QTableWidgetItem("Outline Width");
    setable->setItem(row,TILE_COLORS_SHOW,item);

    outline_width_slider = new SliderSet("", outline_width, 1, 100);
    auto widget = new QWidget();
    widget->setLayout(outline_width_slider);
    setable->setCellWidget(row,TILE_COLORS_COLORS,widget);

    connect(outline_checkbox,     &QCheckBox::clicked,        this, &TileColorsEditor::slot_outlineChanged);
    connect(outline_color_button, &QPushButton::clicked,      this, &TileColorsEditor::slot_outline_color);
    connect(outline_width_slider, &SliderSet::valueChanged,   this, &TileColorsEditor::slot_widthChanged);
}

void TileColorsEditor::onRefresh()
{
    auto tileColors = wtilecolors.lock();
    if (!tileColors) return;

    // update firsts two rows
    int     outline_width;
    QColor  outline_color;
    bool    outlineEnb = tileColors->getOutline(outline_color,outline_width);

    outline_checkbox->setChecked(outlineEnb);

    outline_width_slider->setValue(outline_width);

    QVariant variant = outline_color;
    QString colcode  = variant.toString();
    outline_color_patch->setStyleSheet("QPushButton { background-color :"+colcode+" ;}");

    // update table
    int currentRows = setable->rowCount();

    ColorGroup & group  = tileColors->getColorGroup();
    const int baseRows   = 2;
    const int neededRows = baseRows + group.size();

    if (currentRows != neededRows)
    {
        setable->setRowCount(neededRows);

        // Update existing rows
        int minRows = std::min(currentRows, neededRows);
        for (int row = baseRows; row < minRows; ++row)
        {
            populateRow(row, group[row - baseRows]);
        }

        // Populate new rows (if any)
        for (int row = minRows; row < neededRows; ++row)
        {
            populateRow(row, group[row - baseRows]);
        }
    }

    for (int row = 2; row < neededRows; row++)
    {
        refreshRow(row);
    }

    setable->adjustTableSize();
}

void TileColorsEditor::populateRow(int row, ColorSet & cset)
{
    auto tiling = wtiling.lock();
    if (!tiling) return;

    auto tile = tiling->unit().getUniqueTiles()[row-2];

    QString str = QString("%1 %2").arg(tile->numPoints()).arg((tile->isRegular()) ? "Regular" : "Not-regular");
    auto item = new QTableWidgetItem(str);
    setable->setItem(row,TILE_COLORS_SHOW,item);

    QPushButton * btn = new QPushButton("Edit");
    setable->setCellWidget(row,TILE_COLORS_EDIT,btn);
    connect(btn, &QPushButton::clicked, this, &TileColorsEditor::slot_edit);

    //ColorSet * cset = tileColors->getColorSet(tile);
    ColorSetWidget * widget = new ColorSetWidget(this,&cset);
    widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setable->setCellWidget(row,TILE_COLORS_COLORS,widget);
}

void TileColorsEditor::refreshRow(int row)
{
    auto tiling = wtiling.lock();
    if (!tiling) return;

    auto tile = tiling->unit().getUniqueTiles()[row-2];
    QString str = QString("%1 %2").arg(tile->numPoints()).arg((tile->isRegular()) ? "Regular" : "Not-regular");
    auto item = setable->item(row,TILE_COLORS_SHOW);
    item->setText(str);

    auto * widget = setable->cellWidget(row,TILE_COLORS_COLORS);
    ColorSetWidget * cwidget = static_cast<ColorSetWidget*>(widget);
    cwidget->updateFromColorSet();
}

void TileColorsEditor::slot_edit()
{
    auto tileColors = wtilecolors.lock();
    if (!tileColors) return;

    int row = setable->currentRow();
    int index = row - 2;        // first 2 rows are outline

    if (index < 0 || index >= (uniqueTiles.size() ))
        return;

    TilePtr tile = uniqueTiles[index].lock();
    if (!tile)
    {
        qWarning() << "tile not found";
        return;
    }

    ColorSet * cset = tileColors->getColorSet(index);
    DlgColorSet dlg(cset,setable);
    connect(&dlg, &DlgColorSet::sig_dlg_colorsChanged, this, &::TileColorsEditor::slot_colors_changed);

    dlg.exec();
}

void  TileColorsEditor::slot_colors_changed()
{
    auto tileColors = wtilecolors.lock();
    if (!tileColors) return;

    tileColors->resetStyleRepresentation();     // need to calc _colorSets and _coloredPlacements
    emit sig_reconstructView();
}

void TileColorsEditor::slot_outlineChanged(bool checked)
{
    auto tileColors = wtilecolors.lock();
    if (!tileColors) return;

    QColor color;
    int width;

    tileColors->getOutline(color,width);
    tileColors->setOutline(checked,color,width);

    emit sig_updateView();
}

void  TileColorsEditor::slot_outline_color()
{
    auto tileColors = wtilecolors.lock();
    if (!tileColors) return;

    QColor color;
    int width;
    bool outlineEnb = tileColors->getOutline(color,width);

    AQColorDialog dlg(color,setable);
    dlg.setOption(QColorDialog::ShowAlphaChannel);
    int rv = dlg.exec();
    if (rv != QDialog::Accepted)
    {
        return;
    }
    color = dlg.selectedColor();

    tileColors->setOutline(outlineEnb,color,width);

    emit sig_updateView();
}

void TileColorsEditor::slot_widthChanged(int val)
{
    auto tileColors = wtilecolors.lock();
    if (!tileColors) return;

    QColor color;
    int width;
    bool outlineEnb = tileColors->getOutline(color,width);
    tileColors->setOutline(outlineEnb,color,val);

    emit sig_updateView();
}
