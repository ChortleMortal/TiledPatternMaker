/* TiledPatternMaker - a tool for exploring geometric patterns as found in Andalusian and Islamic art
 *
 *  Copyright 2019 David A. Casper  email: david.casper@gmail.com
 *
 *  This file is part of TiledPatternMaker
 *
 *  TiledPatternMaker is based on the Java application taprats, which is:
 *  Copyright 2000 Craig S. Kaplan.      email: csk at cs.washington.edu
 *  Copyright 2010 Pierre Baillargeon.   email: pierrebai at hotmail.com
 *
 *  TiledPatternMaker is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  TiledPatternMaker is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with TiledPatternMaker.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "makers/decoration_maker/style_editors.h"
#include "base/utilities.h"
#include "viewers/view.h"
#include "viewers/viewcontrol.h"
#include "panels/dlg_colorSet.h"
#include "makers/decoration_maker/style_color_fill_group.h"
#include "makers/decoration_maker//style_color_fill_set.h"
#include "base/configuration.h"
#include "panels/panel.h"

StyleEditor::StyleEditor()
{

    ViewControl * vcontrol = ViewControl::getInstance();
    connect(this, &StyleEditor::sig_refreshView,  vcontrol, &ViewControl::slot_refreshView);
    View * view = View::getInstance();
    connect(this, SIGNAL(sig_update()),  view, SLOT(update()));
}

///////////////////////////////////////////////////////////////
///   Colored
///////////////////////////////////////////////////////////////

ColoredEditor::ColoredEditor(Colored * c, AQTableWidget * table)
{
    colored = c;
    this->table = table;

    TPColor tpcolor = colored->getColorSet()->getFirstColor();
    qreal  opacity  = tpcolor.color.alphaF();
    qDebug() << "color=" << tpcolor.color << "opacity=" << opacity;

    table->clear();
    table->setColumnCount(3);
    table->setRowCount(3);
    table->setColumnWidth(1,301);
    table->horizontalHeader()->setVisible(false);
    table->verticalHeader()->setVisible(false);
    rows     = 0;

    QTableWidgetItem * item = new QTableWidgetItem("Color");
    table->setItem(rows,0,item);

    colorwidget = colored->getColorSet()->createWidget();
    table->setCellWidget(rows,1,colorwidget);

    color_button = new QPushButton("Select Color");
    table->setCellWidget(rows,2,color_button);

    rows++;

    item = new QTableWidgetItem("Transparency");
    table->setItem(rows,0,item);

    transparency = new DoubleSliderSet("", opacity, 0.0, 1.0, 100);
    QWidget * widget = new QWidget();
    widget->setContentsMargins(0,0,0,0);
    widget->setLayout(transparency);

    table->setCellWidget(rows,1,widget);
    table->setRowHeight(rows,41);

    rows++;

    connect(color_button,  &QPushButton::clicked, this, &ColoredEditor::slot_pickColor);
    connect(transparency,  &DoubleSliderSet::valueChanged, this, &ColoredEditor::slot_transparencyChanged);
}

void ColoredEditor::slot_transparencyChanged(qreal val)
{
    ColorSet * cset = colored->getColorSet();
    cset->setOpacity(val);

    colorwidget = cset->createWidget();
    table->setCellWidget(rows,1,colorwidget);

    emit sig_colorsChanged();
    emit sig_refreshView();
}

void ColoredEditor::slot_pickColor()
{
    DlgColorSet dlg(colored->getColorSet());

    connect(&dlg, &DlgColorSet::sig_colorsChanged, this, &ColoredEditor::slot_colorsChanged);

    dlg.exec();
}

void ColoredEditor::slot_colorsChanged()
{
    colorwidget = colored->getColorSet()->createWidget();
    table->setCellWidget(0,1,colorwidget);
    colored->resetStyleRepresentation();
    emit sig_colorsChanged();
    emit sig_refreshView();
}

///////////////////////////////////////////////////////////////
///   Thick
///////////////////////////////////////////////////////////////
ThickEditor::ThickEditor(Thick * o, AQTableWidget * table) : ColoredEditor(o,table)
{
    thick = o;
    table->setRowCount(rows + 2);

    QTableWidgetItem * item;
    item = new QTableWidgetItem("Width");
    table->setItem(rows,0,item);

    qreal width = thick->getLineWidth();
    int val = static_cast<int>(width * 100);
    width_slider = new SliderSet("", val, 1, 100);
    QWidget * widget = new QWidget();
    widget->setContentsMargins(0,0,0,0);
    widget->setLayout(width_slider);
    table->setCellWidget(rows,1,widget);
    table->setRowHeight(rows,41);

    rows++;

    item = new QTableWidgetItem("Outline");
    table->setItem(rows,0,item);

    outline_checkbox = new QCheckBox();
    outline_checkbox->setStyleSheet("padding-left:21px;");
    outline_checkbox->setChecked(thick->getDrawOutline());
    table->setCellWidget(rows,1,outline_checkbox);

    rows++;

    connect(outline_checkbox, &QCheckBox::stateChanged, this, &ThickEditor::slot_outlineChanged);
    connect(width_slider, &SliderSet::valueChanged, this, &ThickEditor::slot_widthChanged);
}


void  ThickEditor::slot_widthChanged(int width)
{
    qreal val = width/100.0;
    thick->setLineWidth(val);
    emit sig_refreshView();
}

void  ThickEditor::slot_outlineChanged(int state)
{
    bool checked = (state == Qt::Checked);
    thick->setDrawOutline(checked);
    emit sig_refreshView();
}


////////////////////////////////////////////////////////////////////////////
// Filled
////////////////////////////////////////////////////////////////////////////
FilledEditor::FilledEditor(FilledPtr f, AQTableWidget * table , QVBoxLayout *parmsCtrl) : StyleEditor()
{
    filled      = f;
    this->table = table;
    vbox        = parmsCtrl;
    fillSet     = nullptr;
    fillGroup   = nullptr;
    view        = View::getInstance();

    table->clear();

    table->horizontalHeader()->setVisible(false);
    table->verticalHeader()->setVisible(false);

    displayParms();
}

FilledEditor::~FilledEditor()
{
    if (fillSet)
        delete fillSet;
    if (fillGroup)
        delete fillGroup;
}

void FilledEditor::displayParms()
{
    eraseLayout(dynamic_cast<QLayout*>(vbox));
    if (fillSet)
    {
        delete fillSet;
        fillSet   = nullptr;
    }
    if (fillGroup)
    {
        delete fillGroup;
        fillGroup = nullptr;
    }

    // clear the table
    table->clearContents();
    table->setColumnCount(4);
    table->setRowCount(1);

    int algo         = filled->getAlgorithm();
    int cleanseLevel = filled->getCleanseLevel();
    int row          = 0;

    // algorithm
    QTableWidgetItem * item = new QTableWidgetItem("Algorithm");
    table->setItem(row,0,item);

    QComboBox * algoBox = new QComboBox();
    algoBox->addItem("Original: two face sets, two color sets",0);
    algoBox->addItem("New 1: two face sets, two color sets",1);
    algoBox->addItem("New 2: multi face sets, one color each",2);
    algoBox->addItem("New 3: multi face sets, color sets for each",3);
    table->setCellWidget(row,1,algoBox);

    algoBox->setCurrentIndex(algo);
    connect(algoBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index){ slot_algo(index);} );

    cleanseBox = new QComboBox();
    cleanseBox->addItem("No Cleanse",0);
    cleanseBox->addItem("Cleanse V",2);
    cleanseBox->addItem("Cleanse E",3);
    cleanseBox->addItem("Cleanse V&E",1);
    table->setCellWidget(row,2,cleanseBox);

    cleanseBox->setCurrentIndex(cleanseBox->findData(cleanseLevel));
    connect(cleanseBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index){ slot_cleanse(index);} );

    switch (algo)
    {
    case 0:
    case 1:
        displayParms01();
        break;
    case 2:
        displayParms2();
        break;
    case 3:
        displayParms3();
        break;
    }

    table->adjustTableSize();
    table->update();
}

void FilledEditor::displayParms01()
{
    int row = 0;

    table->setRowCount(3);
    table->setColumnCount(4);

    QTableWidgetItem * item = new QTableWidgetItem(QString("Number"));
    table->setItem(row,3,item);
    row++;

    // whites
    outside_checkbox = new QCheckBox("Inside (Whites)");   // is truth but does not match code
    outside_checkbox->setStyleSheet("padding-left:3px;");
    outside_checkbox->setChecked(filled->getDrawOutsideWhites());
    table->setCellWidget(row,0,outside_checkbox);

    item = new QTableWidgetItem(QString("%1 faces").arg(filled->whiteFaces.size()));
    table->setItem(row,3,item);

    ColorSet * colorSetW    = filled->getWhiteColorSet();
    AQWidget * widget       = colorSetW->createWidget();
    table->setCellWidget(row,1,widget);

    QPushButton * btnW = new QPushButton("Edit");
    table->setCellWidget(row,2,btnW);

    row++;

    // blacks
    inside_checkbox = new QCheckBox("Outside (Blacks)");   // is truth but does not match code
    inside_checkbox->setStyleSheet("padding-left:3px;");
    inside_checkbox->setChecked(filled->getDrawInsideBlacks());
    table->setCellWidget(row,0,inside_checkbox);

    item = new QTableWidgetItem(QString("%1 faces").arg(filled->blackFaces.size()));
    table->setItem(row,3,item);

    ColorSet * colorSetB    = filled->getBlackColorSet();
    widget                  = colorSetB->createWidget();
    table->setCellWidget(row,1,widget);

    QPushButton * btnB = new QPushButton("Edit");
    table->setCellWidget(row,2,btnB);

    connect(inside_checkbox,  &QCheckBox::stateChanged, this, &FilledEditor::slot_insideChanged);
    connect(outside_checkbox, &QCheckBox::stateChanged, this, &FilledEditor::slot_outsideChanged);
    connect(btnB,             &QPushButton::clicked,    this, &FilledEditor::slot_editB);
    connect(btnW,             &QPushButton::clicked,    this, &FilledEditor::slot_editW);

    table->setColumnWidth(0,131);
    table->setColumnWidth(1,410);
    table->setColumnWidth(2,120);
    table->setColumnWidth(3,120);
}

void FilledEditor::displayParms2()
{
//    if (!filled->dcel)
//        return;

    fillSet = new StyleColorFillSet(filled,vbox);
    fillSet->display();
    connect(fillSet, &StyleColorFillSet::sig_colorsChanged,     this, &FilledEditor::slot_colorsChanged,     Qt::UniqueConnection);
}


void FilledEditor::displayParms3()
{
//    if (!filled->dcel)
//        return;

    fillGroup = new StyleColorFillGroup(filled,vbox);
    connect(fillGroup, &StyleColorFillGroup::sig_colorsChanged,     this, &FilledEditor::slot_colorsChanged,     Qt::UniqueConnection);
    fillGroup->display();
}

void FilledEditor::slot_insideChanged(int state)
{
    bool checked = (state == Qt::Checked);
    filled->setDrawInsideBlacks(checked);

    slot_colorsChanged();
    //displayParms();
    //emit sig_refreshView();
}

void FilledEditor::slot_outsideChanged(int state)
{
    bool checked = (state == Qt::Checked);
    filled->setDrawOutsideWhites(checked);

    slot_colorsChanged();
    //displayParms();
    //emit sig_refreshView();
}

void FilledEditor::slot_algo(int index)
{
    filled->setAlgorithm(index);
    filled->resetStyleRepresentation();
    filled->createStyleRepresentation();
    displayParms();
    emit sig_update();
    emit sig_refreshView();
}

void FilledEditor::slot_cleanse(int index)
{
    int level = cleanseBox->itemData(index).toInt();
    filled->setCleanseLevel(level);
    filled->eraseProtoMap();
    filled->resetStyleRepresentation();
    filled->createStyleRepresentation();
    displayParms();
    emit sig_update();
    emit sig_refreshView();
}


void FilledEditor::slot_editB()
{
    qDebug() << "DLG B" << filled.get() << filled->getBlackColorSet();

    DlgColorSet dlg(filled->getBlackColorSet());

    connect(&dlg, &DlgColorSet::sig_colorsChanged, this, &FilledEditor::slot_colorsChanged);

    dlg.exec();

    displayParms();
}

void FilledEditor::slot_editW()
{
    qDebug() << "DLG W" << filled.get() << filled->getWhiteColorSet();

    DlgColorSet dlg(filled->getWhiteColorSet());

    connect(&dlg, &DlgColorSet::sig_colorsChanged, this, &FilledEditor::slot_colorsChanged);

    dlg.exec();

    displayParms();
}

void FilledEditor::slot_colorsChanged()
{
    view->update();     // that's all
}


////////////////////////////////////////////////////////////////////////////
// Embossed
////////////////////////////////////////////////////////////////////////////
EmbossEditor::EmbossEditor(Emboss * e, AQTableWidget * table) : ThickEditor(e,table)
{
    emboss = e;

    table->setRowCount(rows + 1);

    QTableWidgetItem * item;

    item = new QTableWidgetItem("Azimuth Angle");
    table->setItem(rows,0,item);

    qreal angle = e->getAngle() * 180.0 / M_PI;
    int iangle = static_cast<int>(angle);
    angle_slider = new SliderSet("", iangle, 0, 360);
    QWidget * widget = new QWidget();
    widget->setContentsMargins(0,0,0,0);
    widget->setLayout(angle_slider);
    table->setCellWidget(rows,1,widget);
    table->setRowHeight(rows,41);
    table->adjustTableSize();

    rows++;

    connect(angle_slider, &SliderSet::valueChanged, this, &EmbossEditor::slot_anlgeChanged);
}

void EmbossEditor::slot_anlgeChanged(int angle)
{
    qDebug() << "angle=" << angle;
    emboss->setAngle( angle * M_PI / 180.0 );
    emit sig_refreshView();
}

////////////////////////////////////////////////////////////////////////////
// Interlaced
////////////////////////////////////////////////////////////////////////////

InterlaceEditor::InterlaceEditor(Interlace * i, AQTableWidget * table) : ThickEditor(i,table)
{
    interlace = i;

    table->setRowCount(rows + 3);

    QTableWidgetItem * item;

    item = new QTableWidgetItem("Gap Width");
    table->setItem(rows,0,item);

    qreal gap = i->getGap();
    gap_slider = new DoubleSliderSet("", gap, 0.0, 1.0, 100);
    QWidget * widget = new QWidget();
    widget->setContentsMargins(0,0,0,0);
    widget->setLayout(gap_slider);
    table->setCellWidget(rows,1,widget);
    table->setRowHeight(rows,41);

    rows++;
    item = new QTableWidgetItem("Shadow Width");
    table->setItem(rows,0,item);

    qreal shadow = i->getShadow();
    shadow_slider = new DoubleSliderSet("", shadow, 0.0, 0.7, 100);
    widget = new QWidget();
    widget->setContentsMargins(0,0,0,0);
    widget->setLayout(shadow_slider);
    table->setCellWidget(rows,1,widget);
    table->setRowHeight(rows,41);

    rows++;
    item = new QTableWidgetItem("Include Tip Vertices");
    table->setItem(rows,0,item);

    tipVert_checkbox = new QCheckBox();
    tipVert_checkbox->setStyleSheet("padding-left:21px;");
    tipVert_checkbox->setChecked(i->getIncludeTipVertices());
    table->setCellWidget(rows,1,tipVert_checkbox);
    table->resizeColumnToContents(0);
    table->adjustTableSize();

    rows++;

    connect(gap_slider,    &DoubleSliderSet::valueChanged, this, &InterlaceEditor::slot_gapChanged);
    connect(shadow_slider, &DoubleSliderSet::valueChanged, this, &InterlaceEditor::slot_shadowChanged);
    connect(tipVert_checkbox, &QCheckBox::stateChanged, this, &InterlaceEditor::slot_includeTipVerticesChanged);


}

void InterlaceEditor::slot_gapChanged(qreal gap)
{
    interlace->setGap(gap);
    emit sig_refreshView();
}

void InterlaceEditor::slot_shadowChanged(qreal shadow)
{
    interlace->setShadow(shadow);
    emit sig_refreshView();
}

void InterlaceEditor::slot_includeTipVerticesChanged(int state)
{
    Q_UNUSED(state)
    interlace->setIncludeTipVertices(tipVert_checkbox->isChecked());
    emit sig_refreshView();
}

///////////////////////////////////////////////////////////////
///   TileColors
///////////////////////////////////////////////////////////////
TileColorsEditor::TileColorsEditor(TileColors * c, AQTableWidget * table, TilingPtr tiling)
{
    config = Configuration::getInstance();
    panel   = ControlPanel::getInstance();

    colored     = c;
    this->table = table;
    this->tiling = tiling;
    buildTable();
}

void TileColorsEditor::buildTable()
{
    table->clear();
    table->setColumnCount(4);
    table->setColumnWidth(TILE_COLORS_ADDR,  100);
    table->setColumnWidth(TILE_COLORS_SIDES, 130);
    table->setColumnWidth(TILE_COLORS_BTN,   100);
    table->setColumnWidth(TILE_COLORS_COLORS,400);

    QStringList qslH;
    qslH << "Feature" << "Sides" << "Btn" << "Colors" ;
    table->setHorizontalHeaderLabels(qslH);
    table->verticalHeader()->setVisible(false);

    qlfp = tiling->getUniqueFeatures();
    table->setRowCount(qlfp.size() + 1);

    QColor color;
    int width;;
    bool outline = colored->getOutline(color,width);

    int row = 0;

    outline_checkbox = new QCheckBox("Outline");
    outline_checkbox->setStyleSheet("padding-left:21px;");
    outline_checkbox->setChecked(outline);
    table->setCellWidget(row,0,outline_checkbox);

    color_button = new QPushButton("Color");
    table->setCellWidget(row,2,color_button);

    colorItem = new QTableWidgetItem();
    colorItem->setBackground(color);
    table->setItem(row,1,colorItem);

    width_slider = new SliderSet("Width", width, 1, 10);
    AQWidget * widget = new AQWidget();
    widget->setLayout(width_slider);
    table->setCellWidget(row,3,widget);
    table->setRowHeight(row,41);
    row++;

    connect(outline_checkbox,   &QCheckBox::stateChanged,   this, &TileColorsEditor::slot_outlineChanged);
    connect(color_button,       &QPushButton::clicked,      this, &TileColorsEditor::slot_outline_color);
    connect(width_slider,       &SliderSet::valueChanged,   this, &TileColorsEditor::slot_widthChanged);

    for (auto it = qlfp.begin(); it != qlfp.end(); it++)
    {
        FeaturePtr fp = *it;

        QTableWidgetItem * twi = new QTableWidgetItem(Utils::addr(fp.get()));
        table->setItem(row,TILE_COLORS_ADDR,twi);

        QString str = QString("%1 %2 num=%3").arg(fp->numPoints()).arg((fp->isRegular()) ? "Regular" : "Not-regular").arg(tiling->numPlacements(fp));
        twi = new QTableWidgetItem(str);
        table->setItem(row,TILE_COLORS_SIDES,twi);

        QPushButton * btn = new QPushButton("Edit");
        table->setCellWidget(row,TILE_COLORS_BTN,btn);
        connect(btn, &QPushButton::clicked, this, &TileColorsEditor::slot_edit);

        ColorSet * bkgdColors = fp->getBkgdColors();
        AQWidget * widget = bkgdColors->createWidget();
        table->setCellWidget(row,TILE_COLORS_COLORS,widget);

        row++;
    }
    table->adjustTableSize();
    table->selectRow(0);
}

void TileColorsEditor::slot_edit()
{
    int row = table->currentRow();
    row--;  // first row is outline

    if (row < 0 || row >= (qlfp.size() ))
        return;

    FeaturePtr fp = qlfp[row];
    ColorSet * colorSet = fp->getBkgdColors();
    DlgColorSet dlg(colorSet,table);

    connect(&dlg, &DlgColorSet::sig_colorsChanged, this, &TileColorsEditor::slot_colors_changed);

    dlg.exec();

    buildTable();
}

void  TileColorsEditor::slot_colors_changed()
{
    emit panel->sig_render();
    buildTable();
}

void TileColorsEditor::slot_outlineChanged(int state)
{
     bool checked = (state == Qt::Checked);

     QColor color;
     int width;

     colored->getOutline(color,width);
     colored->setOutline(checked,color,width);

     emit sig_refreshView();
}

void  TileColorsEditor::slot_outline_color()
{
    QColor color;
    int width;
    bool outlineEnb = colored->getOutline(color,width);

    AQColorDialog dlg(color,table);
    dlg.setOption(QColorDialog::ShowAlphaChannel);
    int rv = dlg.exec();
    if (rv != QDialog::Accepted)
    {
        return;
    }
    color = dlg.selectedColor();

    colored->setOutline(outlineEnb,color,width);

    colorItem->setBackground(color);

    emit sig_refreshView();

}

void TileColorsEditor::slot_widthChanged(int val)
{
    QColor color;
    int width;
    bool outlineEnb = colored->getOutline(color,width);
    colored->setOutline(outlineEnb,color,val);

    emit sig_refreshView();
}
