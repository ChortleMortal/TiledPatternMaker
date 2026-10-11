#include <QGroupBox>
#include <QHBoxLayout>
#include <QButtonGroup>
#include <QRadioButton>
#include <QCheckBox>

#include "gui/panels/page_grid.h"
#include "gui/widgets/panel_misc.h"
#include "gui/top/controlpanel.h"
#include "gui/top/view_controller.h"
#include "gui/top/system_view.h"
#include "gui/viewers/grid_view.h"
#include "gui/widgets/layout_sliderset.h"
#include "gui/widgets/smx_widget.h"
#include "model/settings/configuration.h"
#include "sys/sys.h"

page_grid:: page_grid(ControlPanel * cpanel)  : panel_page(cpanel,PAGE_GRID,"Grid")
{
    QRadioButton * gridScreen          = new QRadioButton("Screen Units");
    QRadioButton * gridModel           = new QRadioButton("Model Units");
    QRadioButton * gridTiling          = new QRadioButton("Repeat Unit");
    QRadioButton * gridNone            = new QRadioButton("Centers Only");

    SpinSet      * gridScreenSpacing   = new SpinSet("Spacing",100,10,990);
    SpinSet      * gridScreenWidth     = new SpinSet("Width",config->gridScreenWidth,1,9);

    DoubleSpinSet* gridModelSpacing    = new DoubleSpinSet("Spacing",1.0,0.0001,900);
    SpinSet      * gridModelWidth      = new SpinSet("Width",config->gridModelWidth,1,9);

    QRadioButton * gridTilingAlgo1     = new QRadioButton("Flood");
    QRadioButton * gridTilingAlgo2     = new QRadioButton("Fill Region ");
    SpinSet      * gridTilingWidth     = new SpinSet("Width",config->gridTilingWidth,1,9);

    QCheckBox    * chkDrawModelCenter  = new QCheckBox("Show Model Center");
    QCheckBox    * chkDrawViewCenter   = new QCheckBox("Show Window Center");

    chkDrawModelCenter->setStyleSheet("color: green");
    chkDrawViewCenter->setStyleSheet("color: blue");

    gridScreen->setFixedWidth(91);
    gridModel->setFixedWidth(91);

    screenPosGroup = new QButtonGroup;
    QHBoxLayout * screenPos = createPositionButtons(screenPosGroup,GRID_UNITS_SCREEN);

    modelPosGroup = new QButtonGroup;
    QHBoxLayout * modelPos  = createPositionButtons(modelPosGroup,GRID_UNITS_MODEL);

    btnMove = new QPushButton("Move Grid Only");
    btnMove->setCheckable(true);
    btnMove->setStyleSheet("QPushButton:checked { background-color: LightGreen; color: black; }");

    gridModelSpacing->setDecimals(8);
    gridModelSpacing->setSingleStep(0.01);

    gridUnitGroup = new QButtonGroup;
    gridUnitGroup->addButton(gridScreen,    GRID_UNITS_SCREEN);
    gridUnitGroup->addButton(gridModel,     GRID_UNITS_MODEL);
    gridUnitGroup->addButton(gridTiling,    GRID_UNITS_TILE);
    gridUnitGroup->addButton(gridNone,      GRID_UNITS_OFF);

    QRadioButton * btnOrthog = new QRadioButton("Orthogonal");
    QRadioButton * btnIso    = new QRadioButton("Isometric");
    QRadioButton * btnRhom   = new QRadioButton("Rhombic");
    angleSpin = new DoubleSpinSet("Rhombus angle",30,0,180);

    gridTypeGroup = new QButtonGroup;
    gridTypeGroup->addButton(btnOrthog, GRID_ORTHOGONAL);
    gridTypeGroup->addButton(btnIso,    GRID_ISOMETRIC);
    gridTypeGroup->addButton(btnRhom,   GRID_RHOMBIC);

    SpinSet * gridZLevel = new SpinSet("Z-level",(int)config->gridZLevel,-10,10);

    // initial values
    gridTypeGroup->button(config->gridType)->setChecked(true);
    gridUnitGroup->button(config->gridUnits)->setChecked(true);
    screenPosGroup->button(config->gridScreenPos)->setChecked(true);
    gridScreenSpacing->setValue(config->gridScreenSpacing);
    gridScreenWidth->setValue(config->gridScreenWidth);
    gridTilingWidth->setValue(config->gridTilingWidth);
    gridModelSpacing->setValue(config->gridModelSpacing);
    modelPosGroup->button(config->gridModelPos)->setChecked(true);
    gridModelWidth->setValue(config->gridModelWidth);
    angleSpin->setValue(config->gridAngle);
    angleSpin->setEnabled(config->gridType == GRID_RHOMBIC);
    if (config->gridTilingAlgo == FLOOD)
        gridTilingAlgo1->setChecked(true);
    else
        gridTilingAlgo2->setChecked(true);

    chkDrawModelCenter->setChecked(config->showGridModelCenter);
    chkDrawViewCenter->setChecked(config->showGridViewCenter);

    // connections
    connect(gridUnitGroup ,     &QButtonGroup::idClicked,     this, &page_grid::slot_gridUnitsChanged);
    connect(gridScreenSpacing,  &SpinSet::valueChanged,       this, &page_grid::slot_gridScreenSpacingChanged);
    connect(gridModelSpacing,   &DoubleSpinSet::valueChanged, this, &page_grid::slot_gridModelSpacingChanged);
    connect(gridScreenWidth,    &SpinSet::valueChanged,       this, &page_grid::slot_gridScreenWidthChanged);
    connect(gridTilingWidth,    &SpinSet::valueChanged,       this, &page_grid::slot_gridTilingWidthChanged);
    connect(gridModelWidth,     &SpinSet::valueChanged,       this, &page_grid::slot_gridModelWidthChanged);
    connect(gridZLevel,         &SpinSet::valueChanged,       this, &page_grid::slot_zValueChanged);
    connect(gridTilingAlgo1,    &QRadioButton::clicked,       this, [this]() {config->gridTilingAlgo = FLOOD;  emit sig_reconstructView(); });
    connect(gridTilingAlgo2,    &QRadioButton::clicked,       this, [this]() {config->gridTilingAlgo = REGION; emit sig_reconstructView(); });
    connect(gridTypeGroup,      &QButtonGroup::idClicked,     this, &page_grid::slot_gridTypeSelected);
    connect(btnMove,            &QPushButton::clicked,        this, [](bool checked)
    {
        Sys::gridViewer->setMoving(checked);
        Sys::systemViewController->getView()->setFocus(Qt::MouseFocusReason);
    });
    connect(angleSpin,          &DoubleSpinSet::valueChanged, this, &page_grid::slot_gridAngleChanged);

    connect(chkDrawModelCenter, &QCheckBox::clicked,          this, &page_grid::slot_drawModelCenterChanged);
    connect(chkDrawViewCenter,  &QCheckBox::clicked,          this, &page_grid::slot_drawViewCenterChanged);

    labelT = createColorPatch(config->gridColorTiling);
    labelT->setMinimumWidth(121);
    labelM = createColorPatch(config->gridColorModel);
    labelS = createColorPatch(config->gridColorScreen);

    // fill the layout
    QGridLayout * layout = new QGridLayout();

    int row = 0;
    layout->addWidget(gridNone,row,0);
    row++;

    layout->addWidget(gridScreen,row,0);
    layout->addLayout(screenPos,row,1);
    layout->addWidget(labelS,row,2);
    layout->addLayout(gridScreenWidth,row,3);
    layout->addLayout(gridScreenSpacing,row,4);
    row++;

    layout->addWidget(gridModel,row,0);
    layout->addLayout(modelPos,row,1);
    layout->addWidget(labelM,row,2);
    layout->addLayout(gridModelWidth,row,3);
    layout->addLayout(gridModelSpacing,row,4);
    row++;

    layout->addWidget(gridTiling,row,0);
    layout->addWidget(labelT,row,2);
    layout->addLayout(gridTilingWidth,row,3);
    QHBoxLayout * hbox = new QHBoxLayout;
    hbox->addWidget(gridTilingAlgo1);
    hbox->addWidget(gridTilingAlgo2);
    hbox->addStretch();
    layout->addLayout(hbox,row,4);
    row++;

    QGroupBox * gridBox = new QGroupBox("Grid Units");
    gridBox->setLayout(layout);

    smxWidget = new SMXWidget(Sys::gridViewer.get(),false,true);
    smxWidget->setEnabled(false);   // display only

    QHBoxLayout * hb5 = new QHBoxLayout;
    hb5->addWidget(btnOrthog);
    hb5->addWidget(btnIso);
    hb5->addWidget(btnRhom);
    hb5->addLayout(angleSpin);
    hb5->addStretch();

    QGroupBox * styleBox = new QGroupBox("Grid Type");
    styleBox->setLayout(hb5);

    QHBoxLayout * hbox4 = new QHBoxLayout();
    hbox4->addLayout(gridZLevel);
    hbox4->addSpacing(15);
    hbox4->addWidget(btnMove);
    hbox4->addStretch();
    hbox4->addWidget(smxWidget);

    viewBox = new QGroupBox("Grid View");
    viewBox->setCheckable(true);
    viewBox->setChecked(viewControl->isEnabled(Layer::Type::Grid));
    viewBox->setLayout(hbox4);

    QHBoxLayout * hbox3 = new QHBoxLayout();
    hbox3->addWidget(chkDrawModelCenter);
    hbox3->addWidget(chkDrawViewCenter);
    hbox3->addStretch();

    centerBox = new QGroupBox("Centers");
    centerBox->setLayout(hbox3);

    vbox->addWidget(viewBox);
    vbox->addWidget(gridBox);
    vbox->addWidget(styleBox);
    vbox->addWidget(centerBox);
    vbox->addStretch();

    connect(viewBox,      &QGroupBox::clicked, this, &page_grid::slot_showGridChanged);
}

QHBoxLayout * page_grid::createPositionButtons(QButtonGroup * group, eGridUnits units)
{
    QRadioButton * centered = new QRadioButton("Centered");
    QRadioButton * offset   = new QRadioButton("Offset");
    QRadioButton * fixed    = new QRadioButton("Fixed");

    group->addButton(centered,GRID_POS_CENTERED);
    group->addButton(offset,  GRID_POS_OFFSET);
    group->addButton(fixed,   GRID_POS_FIXED);

    connect(group, &QButtonGroup::idClicked, this, [units](int id)
    {
        Sys::gridViewer->setPosition(units,eGridPosition(id));
        Sys::systemViewController->getView()->setFocus(Qt::MouseFocusReason);
    });

    QHBoxLayout * hbox = new QHBoxLayout;
    hbox->addWidget(centered);
    hbox->addWidget(offset);
    hbox->addWidget(fixed);
    return hbox;
}

void  page_grid::onEnter()
{
}

void  page_grid::onRefresh()
{
    viewBox->setChecked(viewControl->isEnabled(Layer::Type::Grid));

    Sys::gridViewer->syncPosition();
    screenPosGroup->button(config->gridScreenPos)->setChecked(true);
    modelPosGroup->button(config->gridModelPos)->setChecked(true);
    btnMove->setChecked(Sys::gridViewer->isSolo());

    smxWidget->refresh();
}

void page_grid::slot_showGridChanged(bool checked)
{
    panel->userDelegateView(Layer::Type::Grid,checked);
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
    Sys::gridViewer->restorePosition();
    emit sig_reconstructView();
    Sys::systemViewController->getView()->setFocus(Qt::MouseFocusReason);
}

void page_grid::slot_drawModelCenterChanged(bool checked)
{
    config->showGridModelCenter = checked;
    emit sig_reconstructView();
}

void page_grid::slot_drawViewCenterChanged(bool checked)
{
    config->showGridViewCenter = checked;
    emit sig_reconstructView();
}

void page_grid::slot_gridTypeSelected(int type)
{
    config->gridType = eGridType(type);
    angleSpin->setEnabled(config->gridType == GRID_RHOMBIC);
    emit sig_reconstructView();
}

void page_grid::slot_gridAngleChanged(qreal angle)
{
    config->gridAngle = angle;
    emit sig_reconstructView();
}

void page_grid::slot_zValueChanged(int value)
{
    config->gridZLevel = (Layer::ZLevel)value;
    Sys::gridViewer->setZLevel((Layer::ZLevel)value);
    emit sig_reconstructView();
}

ClickableLabel * page_grid::createColorPatch(QString & color)
{
    ClickableLabel * patch = new ClickableLabel();
    patch->setStyleSheet("QLabel { background-color :" + QColor(color).name(QColor::HexArgb) + " ; border: 1px solid black;}");
    connect(patch, &ClickableLabel::clicked, this, [this, &color, patch] { pickColor(color,patch); });
    return patch;
}

void page_grid::pickColor(QString & color, ClickableLabel * patch)
{
    AQColorDialog dlg(QColor(color),this);
    dlg.setCurrentColor(QColor(color));
    if (dlg.exec() != QDialog::Accepted)
        return;

    QColor acolor = dlg.selectedColor();
    if (acolor.isValid())
    {
        color = acolor.name(QColor::HexArgb);
        patch->setStyleSheet("QLabel { background-color :" + color + " ; border: 1px solid black;}");
        emit sig_reconstructView();
    }
}
