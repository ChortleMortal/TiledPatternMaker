#include <QGroupBox>
#include <QHBoxLayout>
#include <QButtonGroup>
#include <QRadioButton>
#include <QCheckBox>

#include "gui/panels/page_grid.h"
#include "gui/widgets/layout_sliderset.h"
#include "model/settings/configuration.h"
#include "gui/top/controlpanel.h"
#include "gui/panels/panel_misc.h"
#include "gui/viewers/grid_view.h"
#include "gui/top/view_controller.h"

#if (QT_VERSION >= QT_VERSION_CHECK(6,7,0))
#define CBOX_STATECHANGE &QCheckBox::checkStateChanged
#else
#define CBOX_STATECHANGE &QCheckBox::stateChanged
#endif

page_grid:: page_grid(ControlPanel * cpanel)  : panel_page(cpanel,PAGE_GRID,"Grid")
{

    QRadioButton * gridScreen          = new QRadioButton("Screen Units");
    QCheckBox    * gridScreenCentered  = new QCheckBox("Model Centered");
    SpinSet      * gridScreenSpacing   = new SpinSet("Spacing",100,10,990);
    SpinSet      * gridScreenWidth     = new SpinSet("Width",config->gridScreenWidth,1,9);

    QRadioButton * gridModel           = new QRadioButton("Model Units");
    QCheckBox    * gridModelCentered   = new QCheckBox("Canvas Centered");
    DoubleSpinSet* gridModelSpacing    = new DoubleSpinSet("Spacing",1.0,0.0001,900);
    SpinSet      * gridModelWidth      = new SpinSet("Width",config->gridModelWidth,1,9);

    QRadioButton * gridTiling          = new QRadioButton("Tiling Repeat Units");
    QRadioButton * gridTilingAlgo1     = new QRadioButton("Flood");
    QRadioButton * gridTilingAlgo2     = new QRadioButton("Fill Region ");
    SpinSet      * gridTilingWidth     = new SpinSet("Width",config->gridTilingWidth,1,9);

    QCheckBox    * chkDrawLayerCenter  = new QCheckBox("Show Layer Center");
    QCheckBox    * chkDrawModelCenter  = new QCheckBox("Show Model Center");
    QCheckBox    * chkDrawViewCenter   = new QCheckBox("Show View Center");
    QCheckBox    * chkLockToview       = new QCheckBox("Lock to View");

    chkDrawLayerCenter->setStyleSheet("color: red");
    chkDrawModelCenter->setStyleSheet("color: green");
    chkDrawViewCenter->setStyleSheet("color: blue");

    gridScreen->setFixedWidth(91);
    gridScreenCentered->setFixedWidth(111);
    gridModel->setFixedWidth(91);
    gridModelCentered->setFixedWidth(111);

    gridModelSpacing->setDecimals(8);
    gridModelSpacing->setSingleStep(0.01);

    gridUnitGroup = new QButtonGroup;
    gridUnitGroup->addButton(gridScreen,GRID_UNITS_SCREEN);
    gridUnitGroup->addButton(gridModel,GRID_UNITS_MODEL);
    gridUnitGroup->addButton(gridTiling,GRID_UNITS_TILE);

    QLabel       * glabel    = new QLabel("Grid Type:");
    QRadioButton * btnOrthog = new QRadioButton("Orthogonal");
    QRadioButton * btnIso    = new QRadioButton("Isometric");
    QRadioButton * btnRhom   = new QRadioButton("Rhombic");
    DoubleSpinSet* angleSpin = new DoubleSpinSet("Rhombus angle",30,0,180);

    gridTypeGroup = new QButtonGroup;
    gridTypeGroup->addButton(btnOrthog,GRID_ORTHOGONAL);
    gridTypeGroup->addButton(btnIso,GRID_ISOMETRIC);
    gridTypeGroup->addButton(btnRhom,GRID_RHOMBIC);

    SpinSet * gridZLevel = new SpinSet("Z-level",config->gridZLevel,-10,10);

    // initial values
    gridTypeGroup->button(config->gridType)->setChecked(true);
    gridUnitGroup->button(config->gridUnits)->setChecked(true);
    gridScreenCentered->setChecked(config->gridScreenCenter);
    gridScreenSpacing->setValue(config->gridScreenSpacing);
    gridScreenWidth->setValue(config->gridScreenWidth);
    gridTilingWidth->setValue(config->gridTilingWidth);
    gridModelCentered->setChecked(config->gridModelCenter);
    gridModelSpacing->setValue(config->gridModelSpacing);
    gridModelWidth->setValue(config->gridModelWidth);
    angleSpin->setValue(config->gridAngle);
    if (config->gridTilingAlgo == FLOOD)
        gridTilingAlgo1->setChecked(true);
    else
        gridTilingAlgo2->setChecked(true);

    chkDrawModelCenter->setChecked(config->showGridModelCenter);
    chkDrawViewCenter->setChecked(config->showGridViewCenter);
    chkDrawLayerCenter->setChecked(config->showGridLayerCenter);
    chkLockToview->setChecked(config->lockGridToView);

    // connections
    connect(gridUnitGroup ,     &QButtonGroup::idClicked,     this, &page_grid::slot_gridUnitsChanged);
    connect(gridScreenSpacing,  &SpinSet::valueChanged,       this, &page_grid::slot_gridScreenSpacingChanged);
    connect(gridModelSpacing,   &DoubleSpinSet::valueChanged, this, &page_grid::slot_gridModelSpacingChanged);
    connect(gridScreenWidth,    &SpinSet::valueChanged,       this, &page_grid::slot_gridScreenWidthChanged);
    connect(gridTilingWidth,    &SpinSet::valueChanged,       this, &page_grid::slot_gridTilingWidthChanged);
    connect(gridModelWidth,     &SpinSet::valueChanged,       this, &page_grid::slot_gridModelWidthChanged);
    connect(gridZLevel,         &SpinSet::valueChanged,       this, &page_grid::slot_zValueChanged);
    connect(gridScreenCentered, CBOX_STATECHANGE,             this, &page_grid::slot_gridScreenCenteredChanged);
    connect(gridModelCentered,  CBOX_STATECHANGE,             this, &page_grid::slot_gridModelCenteredChanged);
    connect(gridTilingAlgo1,    &QRadioButton::clicked,       this, [this]() {config->gridTilingAlgo = FLOOD;  emit sig_reconstructView(); });
    connect(gridTilingAlgo2,    &QRadioButton::clicked,       this, [this]() {config->gridTilingAlgo = REGION; emit sig_reconstructView(); });
    connect(gridTypeGroup,      &QButtonGroup::idClicked,     this, &page_grid::slot_gridTypeSelected);
    connect(angleSpin,          &DoubleSpinSet::valueChanged, this, &page_grid::slot_gridAngleChanged);

    connect(chkDrawLayerCenter, CBOX_STATECHANGE,             this, &page_grid::slot_gridLayerCenterChanged);
    connect(chkDrawModelCenter, CBOX_STATECHANGE,             this, &page_grid::slot_drawModelCenterChanged);
    connect(chkDrawViewCenter,  CBOX_STATECHANGE,             this, &page_grid::slot_drawViewCenterChanged);
    connect(chkLockToview,      &QCheckBox::clicked,          this, &page_grid::slot_lockToViewChanged);

    labelT = new ClickableLabel();
    QVariant variant = QColor(config->gridColorTiling);
    QString colcode  = variant.toString();
    labelT->setStyleSheet("QLabel { background-color :"+colcode+" ; border: 1px solid black;}");
    connect(labelT,&ClickableLabel::clicked, this, &page_grid::slot_pickColorTiling);

    labelM = new ClickableLabel();
    variant = QColor(config->gridColorModel);
    colcode  = variant.toString();
    labelM->setStyleSheet("QLabel { background-color :"+colcode+" ; border: 1px solid black;}");
    connect(labelM,&ClickableLabel::clicked, this, &page_grid::slot_pickColorModel);

    labelS = new ClickableLabel();
    variant = QColor(config->gridColorScreen);
    colcode  = variant.toString();
    labelS->setStyleSheet("QLabel { background-color :"+colcode+" ; border: 1px solid black;}");
    connect(labelS,&ClickableLabel::clicked, this, &page_grid::slot_pickColorScreen);

    // fill the layout
    QGridLayout * layout = new QGridLayout();
    int row = 0;

    layout->addWidget(gridTiling,row,0);
    layout->addWidget(labelT,row,2);
    layout->addLayout(gridTilingWidth,row,3);
    QHBoxLayout * hbox = new QHBoxLayout;
    hbox->addWidget(gridTilingAlgo1);
    hbox->addWidget(gridTilingAlgo2);
    layout->addLayout(hbox,row,4);
    row++;

    layout->addWidget(gridScreen,row,0);
    layout->addWidget(gridScreenCentered,row,1);
    layout->addWidget(labelS,row,2);
    layout->addLayout(gridScreenWidth,row,3);
    layout->addLayout(gridScreenSpacing,row,4);
    row++;

    layout->addWidget(gridModel,row,0);
    layout->addWidget(gridModelCentered,row,1);
    layout->addWidget(labelM,row,2);
    layout->addLayout(gridModelWidth,row,3);
    layout->addLayout(gridModelSpacing,row,4);
    row++;

    layout->addWidget(glabel,row,0);
    layout->addWidget(btnIso,row,1);
    layout->addWidget(btnOrthog,row,2);
    layout->addWidget(btnRhom,row,3);
    layout->addLayout(angleSpin,row,4);
    row++;

    layout->addLayout(gridZLevel,row,0);
    layout->addWidget(chkDrawLayerCenter,row,1);
    layout->addWidget(chkDrawModelCenter,row,2);
    layout->addWidget(chkDrawViewCenter,row,3);
    layout->addWidget(chkLockToview,row,4);

    groupBox = new QGroupBox("Show Grid");
    groupBox->setCheckable(true);
    groupBox->setLayout(layout);
    groupBox->setChecked(config->showGrid);


    QPushButton * pbReset = new QPushButton("Reset position");
    QPushButton * pbAlign = new QPushButton("Re-align position");

    QHBoxLayout * hbox2 = new QHBoxLayout();
    hbox2->addStretch();
    hbox2->addWidget(pbReset);
    hbox2->addSpacing(9);
    hbox2->addWidget(pbAlign);

    vbox->addWidget(groupBox);
    vbox->addLayout(hbox2);
    vbox->addStretch();

    connect(groupBox, &QGroupBox::clicked,  this, &page_grid::slot_showGridChanged);
    connect(pbReset, &QPushButton::clicked, this, &page_grid::slot_resetPos);
    connect(pbAlign, &QPushButton::clicked, this, &page_grid::slot_reAlign);
}

void  page_grid::onEnter()
{
}

void  page_grid::onRefresh()
{
    groupBox->setChecked(config->showGrid);
}

void page_grid::slot_showGridChanged(bool checked)
{
    config->showGrid = checked;
    if (checked)
        panel->delegateView(VIEW_GRID);
    else
        panel->unDelegateView(VIEW_GRID);
    emit sig_reconstructView();
}

void page_grid::slot_gridModelSpacingChanged(qreal value)
{
    config->gridModelSpacing = value;
    emit sig_reconstructView();
}

void page_grid::slot_gridScreenSpacingChanged(int value)
{
    config->gridScreenSpacing = value;
    emit sig_reconstructView();

}

void page_grid::slot_gridScreenWidthChanged(int value)
{
    config->gridScreenWidth = value;
    emit sig_reconstructView();
}

void page_grid::slot_gridTilingWidthChanged(int value)
{
    config->gridTilingWidth = value;
    emit sig_reconstructView();
}

void page_grid::slot_gridModelWidthChanged(int value)
{
    config->gridModelWidth = value;
    emit sig_reconstructView();
}

void page_grid::slot_gridUnitsChanged(int id)
{
    config->gridUnits = eGridUnits(id);
    emit sig_reconstructView();
}

void page_grid::slot_gridScreenCenteredChanged(int id)
{
    config->gridScreenCenter = (id == Qt::Checked);
    emit sig_reconstructView();
}

void page_grid::slot_gridModelCenteredChanged(int id)
{
    config->gridModelCenter = (id == Qt::Checked);
    emit sig_reconstructView();
}

void page_grid::slot_gridLayerCenterChanged(int id)
{
    config->showGridLayerCenter = (id == Qt::Checked);
    emit sig_reconstructView();
}

void page_grid::slot_drawModelCenterChanged(int id)
{
    config->showGridModelCenter = (id == Qt::Checked);
    emit sig_reconstructView();
}

void page_grid::slot_drawViewCenterChanged(int id)
{
    config->showGridViewCenter = (id == Qt::Checked);
    emit sig_reconstructView();
}

void page_grid::slot_lockToViewChanged(bool enb)
{
    config->lockGridToView = enb;
}

void page_grid::slot_gridTypeSelected(int type)
{
    config->gridType = eGridType(type);
    emit sig_reconstructView();
}

void page_grid::slot_gridAngleChanged(qreal angle)
{
    config->gridAngle = angle;
    emit sig_reconstructView();
}

void page_grid::slot_zValueChanged(int value)
{
    config->gridZLevel = value;
    Sys::gridViewer->setZValue(value);
    emit sig_reconstructView();
}

void  page_grid::slot_pickColorTiling()
{
    QColor color(config->gridColorTiling);
    AQColorDialog dlg(color,this);
    dlg.setCurrentColor(color);
    int rv = dlg.exec();
    if (rv != QDialog::Accepted)
        return;

    QColor acolor = dlg.selectedColor();
    if (acolor.isValid())
    {
        config->gridColorTiling = acolor.name(QColor::HexArgb);
        QVariant variant = QColor(config->gridColorTiling);
        QString colcode  = variant.toString();
        labelT->setStyleSheet("QLabel { background-color :"+colcode+" ; border: 1px solid black;}");
        emit sig_reconstructView();
    }
}

void  page_grid::slot_pickColorModel()
{
    QColor color(config->gridColorModel);
    AQColorDialog dlg(color,this);
    dlg.setCurrentColor(color);
    int rv = dlg.exec();
    if (rv != QDialog::Accepted)
        return;

    QColor acolor = dlg.selectedColor();
    if (acolor.isValid())
    {
        config->gridColorModel = acolor.name(QColor::HexArgb);
        QVariant variant = QColor(config->gridColorModel);
        QString colcode  = variant.toString();
        labelM->setStyleSheet("QLabel { background-color :"+colcode+" ; border: 1px solid black;}");
        emit sig_reconstructView();
    }
}
void page_grid::slot_pickColorScreen()
{
    QColor color(config->gridColorScreen);
    AQColorDialog dlg(color,this);
    dlg.setCurrentColor(color);
    int rv = dlg.exec();
    if (rv != QDialog::Accepted)
        return;

    QColor acolor = dlg.selectedColor();
    if (acolor.isValid())
    {
        config->gridColorScreen = acolor.name(QColor::HexArgb);
        QVariant variant = QColor(config->gridColorScreen);
        QString colcode  = variant.toString();
        labelS->setStyleSheet("QLabel { background-color :"+colcode+" ; border: 1px solid black;}");
        emit sig_reconstructView();
    }
}

void  page_grid::slot_resetPos()
{
    Xform xf;
    Sys::gridViewer->setModelXform(xf,false);
    emit sig_reconstructView();
}

void  page_grid::slot_reAlign()
{
    Sys::gridViewer->setModelXform(viewControl->getCurrentModelXform(),false);
    emit sig_reconstructView();
}
