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

#include "base/configuration.h"
#include "panels/style_editors.h"
#include "viewers/workspaceviewer.h"
#include "panels/dlg_colorSet.h"
#include "panels/style_colorFillGroup.h"
#include "panels/style_colorFillSet.h"

StyleEditor::StyleEditor()
{
    viewer = WorkspaceViewer::getInstance();

    setObjectName("StyleEditor");

    connect(this, &StyleEditor::sig_viewWS,  viewer, &WorkspaceViewer::slot_viewWorkspace);
    connect(this, &StyleEditor::sig_update,  viewer, &WorkspaceViewer::slot_update);
}

///////////////////////////////////////////////////////////////
///   Colored
///////////////////////////////////////////////////////////////

ColoredEditor::ColoredEditor(Colored * c, QTableWidget * table)
{
    colored = c;
    this->table = table;

    TPColor tpcolor = colored->getColorSet().getFirstColor();
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

    colorwidget = colored->getColorSet().createWidget();
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
    ColorSet & cset = colored->getColorSet();
    cset.setOpacity(val);

    colorwidget = cset.createWidget();
    table->setCellWidget(rows,1,colorwidget);

    emit sig_colorsChanged();
    emit sig_viewWS();
}

void ColoredEditor::slot_pickColor()
{
    DlgColorSet dlg(colored->getColorSet());

    connect(&dlg, &DlgColorSet::sig_colorsChanged, this, &ColoredEditor::slot_colorsChanged);

    dlg.exec();
}

void ColoredEditor::slot_colorsChanged()
{
    colorwidget = colored->getColorSet().createWidget();
    table->setCellWidget(rows,1,colorwidget);
    emit sig_colorsChanged();
    emit sig_viewWS();
}

///////////////////////////////////////////////////////////////
///   Thick
///////////////////////////////////////////////////////////////
ThickEditor::ThickEditor(Thick * o, QTableWidget * table) : ColoredEditor(o,table)
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
    emit sig_viewWS();
}

void  ThickEditor::slot_outlineChanged(int state)
{
    bool checked = (state == Qt::Checked);
    thick->setDrawOutline(checked);
    emit sig_viewWS();
}


////////////////////////////////////////////////////////////////////////////
// Filled
////////////////////////////////////////////////////////////////////////////
FilledEditor::FilledEditor(Filled * f, QTableWidget * table , QVBoxLayout *parmsCtrl) : StyleEditor()
{
    filled      = f;
    this->table = table;
    vbox        = parmsCtrl;
    fillSet     = nullptr;
    fillGroup   = nullptr;

    table->clear();
    table->setColumnCount(3);
    table->setRowCount(3);
    table->setColumnWidth(0,131);
    table->setColumnWidth(1,410);
    table->setColumnWidth(2,120);
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
    fillSet   = nullptr;
    fillGroup = nullptr;

    // clear the table
    table->clearContents();

    int algo = filled->getAlgorithm();
    int row  = 0;

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
    connect(algoBox,SIGNAL(currentIndexChanged(int)), this, SLOT(slot_algo(int)));

    QPushButton * pbViewFaces = new QPushButton("View faces");
    pbViewFaces->setFixedWidth(61);
    QSpinBox * faceSetSelect = new QSpinBox;
    faceSetSelect->setRange(0,9999);
    AQWidget * w = new AQWidget;
    AQHBoxLayout * l = new AQHBoxLayout;
    l->addWidget(pbViewFaces);
    l->addWidget(faceSetSelect);
    w->setLayout(l);
    table->setCellWidget(row,2,w);
    connect(pbViewFaces,&QPushButton::clicked, this, &FilledEditor::slot_viewFaces);
    connect(faceSetSelect,SIGNAL(valueChanged(int)), this, SLOT(slot_setSelect(int)));

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
}

void FilledEditor::displayParms01()
{
    int row = 1;

    // whites
    outside_checkbox = new QCheckBox("Inside (Whites)");   // is truth but does not match code
    outside_checkbox->setStyleSheet("padding-left:3px;");
    outside_checkbox->setChecked(filled->getDrawOutsideWhites());
    table->setCellWidget(row,0,outside_checkbox);

    ColorSet & colorSetW    = filled->getWhiteColorSet();
    AQWidget * widget       = colorSetW.createWidget();
    table->setCellWidget(row,1,widget);

    QPushButton * btnW = new QPushButton("Edit");
    table->setCellWidget(row,2,btnW);

    row++;

    // blacks
    inside_checkbox = new QCheckBox("Outside (Blacks)");   // is truth but does not match code
    inside_checkbox->setStyleSheet("padding-left:3px;");
    inside_checkbox->setChecked(filled->getDrawInsideBlacks());
    table->setCellWidget(row,0,inside_checkbox);

    ColorSet & colorSetB    = filled->getBlackColorSet();
    widget                  = colorSetB.createWidget();
    table->setCellWidget(row,1,widget);

    QPushButton * btnB = new QPushButton("Edit");
    table->setCellWidget(row,2,btnB);

    connect(inside_checkbox,  &QCheckBox::stateChanged, this, &FilledEditor::slot_insideChanged);
    connect(outside_checkbox, &QCheckBox::stateChanged, this, &FilledEditor::slot_outsideChanged);
    connect(btnB,             &QPushButton::clicked,    this, &FilledEditor::slot_editB);
    connect(btnW,             &QPushButton::clicked,    this, &FilledEditor::slot_editW);
}

void FilledEditor::displayParms2()
{
    if (!fillSet)
    {
        fillSet = new StyleColorFillSet(this,filled->getWhiteColorSet(),vbox);
    }
    fillSet->displayColors(filled->getWhiteColorSet());
}


void FilledEditor::displayParms3()
{
    if (!fillGroup)
    {
        fillGroup = new StyleColorFillGroup(this,filled->getColorGroup(),vbox);
    }
    fillGroup->display(filled->getColorGroup());
}

void FilledEditor::slot_insideChanged(int state)
{
    bool checked = (state == Qt::Checked);
    filled->setDrawInsideBlacks(checked);

    slot_colorsChanged();
    displayParms();
}

void FilledEditor::slot_outsideChanged(int state)
{
    bool checked = (state == Qt::Checked);
    filled->setDrawOutsideWhites(checked);

    slot_colorsChanged();
    displayParms();
}

void FilledEditor::slot_algo(int index)
{
    filled->setAlgorithm(index);

    slot_colorsChanged();
    displayParms();
}

void FilledEditor::slot_editB()
{
    ColorSet & colorSet = filled->getBlackColorSet();

    DlgColorSet dlg(colorSet);

    connect(&dlg, &DlgColorSet::sig_colorsChanged, this, &FilledEditor::slot_colorsChanged);

    dlg.exec();

    displayParms();
}

void FilledEditor::slot_editW()
{
    DlgColorSet dlg(filled->getWhiteColorSet());

    connect(&dlg, &DlgColorSet::sig_colorsChanged, this, &FilledEditor::slot_colorsChanged);

    dlg.exec();

    displayParms();
}

void FilledEditor::slot_colorsChanged()
{
    filled->whiteColorSet.resetIndex();
    filled->blackColorSet.resetIndex();
    filled->colorGroup.resetIndex();

    switch(filled->getAlgorithm())
    {
    case 3:
        filled->assignColorsNew3(filled->colorGroup);
        break;
    case 2:
        filled->assignColorsNew2(filled->whiteColorSet);
        break;
    case 1:
    case 0:
        filled->resetStyleRepresentation();
        break;
    }

    emit sig_viewWS();
}

void FilledEditor::slot_viewFaces()
{
    static eViewType vtype;
    static bool selected = false;

    Configuration * config = Configuration::getInstance();

    QVector<Layer*> layers = viewer->getActiveLayers();
    if (layers.empty())
    {
        return;
    }
    Layer * l = layers[0];
    Xform xf = l->getDeltas();

    if (!selected)
    {
        selected = true;
        vtype    = config->viewerType;
        config->viewerType = VIEW_FACE_SET;
    }
    else
    {
        selected = false;
        config->viewerType = vtype;
    }

    emit sig_viewWS();

    // match the settings for the two views
    layers = viewer->getActiveLayers();
    for (auto it = layers.begin(); it != layers.end(); it++)
    {
        Layer * l2 = *it;
        l2->setDeltas(xf);
    }
    emit sig_update();
}

void FilledEditor::slot_setSelect(int face)
{
    FaceSet & set = filled->allFaces;
    if (face >=0 && face < set.size())
    {
        FacePtr fp  = set[face];
        Configuration * config = Configuration::getInstance();
        config->selectedFace = fp;
        emit sig_update();
    }
}

////////////////////////////////////////////////////////////////////////////
// Embossed
////////////////////////////////////////////////////////////////////////////
EmbossEditor::EmbossEditor(Emboss * e, QTableWidget * table) : ThickEditor(e,table)
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

    rows++;

    connect(angle_slider, &SliderSet::valueChanged, this, &EmbossEditor::slot_anlgeChanged);
}

void EmbossEditor::slot_anlgeChanged(int angle)
{
    qDebug() << "angle=" << angle;
    emboss->setAngle( angle * M_PI / 180.0 );
    emit sig_viewWS();
}

////////////////////////////////////////////////////////////////////////////
// Interlaced
////////////////////////////////////////////////////////////////////////////

InterlaceEditor::InterlaceEditor(Interlace * i, QTableWidget * table) : ThickEditor(i,table)
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

    rows++;

    connect(gap_slider,    &DoubleSliderSet::valueChanged, this, &InterlaceEditor::slot_gapChanged);
    connect(shadow_slider, &DoubleSliderSet::valueChanged, this, &InterlaceEditor::slot_shadowChanged);
    connect(tipVert_checkbox, &QCheckBox::stateChanged, this, &InterlaceEditor::slot_includeTipVerticesChanged);

}

void InterlaceEditor::slot_gapChanged(qreal gap)
{
    interlace->setGap(gap);
    emit sig_viewWS();
}

void InterlaceEditor::slot_shadowChanged(qreal shadow)
{
    interlace->setShadow(shadow);
    emit sig_viewWS();
}

void InterlaceEditor::slot_includeTipVerticesChanged(int state)
{
    Q_UNUSED(state)
    interlace->setIncludeTipVertices(tipVert_checkbox->isChecked());
    emit sig_viewWS();
}

///////////////////////////////////////////////////////////////
///   TileColors
///////////////////////////////////////////////////////////////
TileColorsEditor::TileColorsEditor(TileColors * c, QTableWidget * table)
{
    colored = c;
    table->clear();
}
