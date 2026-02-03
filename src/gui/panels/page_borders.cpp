#include <QComboBox>
#include <QGroupBox>
#include <QStackedLayout>
#include <QButtonGroup>
#include <QMessageBox>

#include "gui/map_editor/map_editor.h"
#include "gui/panels/page_borders.h"
#include "gui/top/controlpanel.h"
#include "gui/top/system_view_controller.h"
#include "gui/widgets/layout_sliderset.h"
#include "gui/widgets/smx_widget.h"
#include "model/makers/mosaic_maker.h"
#include "model/mosaics/border.h"
#include "model/mosaics/mosaic.h"
#include "model/motifs/tile_color_defs.h"
#include "model/styles/style.h"
#include "sys/enums/eborder.h"
#include "sys/geometry/crop.h"

using std::string;
using std::make_shared;

page_borders::page_borders(ControlPanel * apanel)  : panel_page(apanel,PAGE_BORDER_MAKER,"Border Maker")
{
    pageStatusString = "Left-click inside crop to display and move";

    // create button
    QPushButton * pbLoadCrop  = new QPushButton("Load from Mosaic Crop");
    QPushButton * pbLoadPCrop = new QPushButton("Load from Painter Crop");
    QPushButton * pbRemove    = new QPushButton("Remove Border");

    QHBoxLayout * hbox3 = new QHBoxLayout;
    hbox3->addStretch();
    hbox3->addWidget(pbLoadCrop);
    hbox3->addWidget(pbLoadPCrop);
    hbox3->addStretch();
    hbox3->addWidget(pbRemove);
    hbox3->addStretch();

    connect(pbLoadCrop,  &QPushButton::clicked, this, &page_borders::slot_loadFromCrop);
    connect(pbLoadPCrop, &QPushButton::clicked, this, &page_borders::slot_loadFromPainterCrop);
    connect(pbRemove,    &QPushButton::clicked, this, &page_borders::slot_removeBorder);

    // border selection
    QLabel * label2 = new QLabel("Border Shape");
    chkUseViewSize  = new QCheckBox("Lock to view size");

    connect(chkUseViewSize, &QCheckBox::clicked, this, &page_borders::slot_useViewSzChanged);

    // border shapes
    cropTypes = new AQComboBox();
    cropTypes->addItem("Undefined", CROP_UNDEFINED);
    cropTypes->addItem("Rectangle", CROP_RECTANGLE);
    cropTypes->addItem("Circle",    CROP_CIRCLE);
    cropTypes->addItem("Polygon",   CROP_POLYGON);

    QHBoxLayout * hbox2 = new QHBoxLayout;
    hbox2->addWidget(label2);
    hbox2->addWidget(cropTypes);
    hbox2->addSpacing(21);
    hbox2->addWidget(chkUseViewSize);
    hbox2->addStretch();

    // stacked layout
    cropTypeStack = new QStackedLayout;

    QWidget * w = createUndefinedShapeWidget();
    cropTypeStack->addWidget(w);

    w = createRectShapeWidget();
    cropTypeStack->addWidget(w);

    w = createPolyShapeWidget();
    cropTypeStack->addWidget(w);

    w = createCircleShapeWidget();
    cropTypeStack->addWidget(w);

    connect(cropTypes, &QComboBox::currentIndexChanged, cropTypeStack,  &QStackedLayout::setCurrentIndex);

    // border box
    cropTypeBox = new QGroupBox("Border Shape Settings");
    cropTypeBox->setMinimumWidth(400);
    cropTypeBox->setLayout(cropTypeStack);

    // border types
    QLabel * label = new QLabel("Border Type");

    borderTypes = new AQComboBox();
    borderTypes->addItem("None",      BORDER_NONE);
    borderTypes->addItem("Solid",     BORDER_PLAIN);
    borderTypes->addItem("Two color", BORDER_TWO_COLOR);
    borderTypes->addItem("Block",     BORDER_BLOCKS);

    smxWidget = new SMXWidget(nullptr,false,true);

    QHBoxLayout * hbox = new QHBoxLayout;
    hbox->addWidget(label);
    hbox->addWidget(borderTypes);
    hbox->addStretch();
    hbox->addWidget(smxWidget);

    // stacked layout
    borderTypeStack = new QStackedLayout;

    controlNone = new QWidget();
    borderTypeStack->addWidget(controlNone);

    controlPlain = new PlainControl(this);
    borderTypeStack->addWidget(controlPlain);

    controlTwoColors = new TwoColorControl(this);
    borderTypeStack->addWidget(controlTwoColors);

    controlBlocks = new BlockControl(this);
    borderTypeStack->addWidget(controlBlocks);

    // border box
    borderTypeBox = new QGroupBox("Border Type Settings");
    borderTypeBox->setMinimumWidth(400);
    borderTypeBox->setLayout(borderTypeStack);

    // assembly
    vbox->addLayout(hbox);
    vbox->addWidget(borderTypeBox);
    vbox->addSpacing(9);
    vbox->addLayout(hbox2);
    vbox->addWidget(cropTypeBox);
    vbox->addSpacing(9);
    vbox->addLayout(hbox3);
    vbox->addStretch();
    adjustSize();

    connect(borderTypes, &QComboBox::currentIndexChanged, this, &page_borders::slot_borderTypeChanged);
    connect(cropTypes,   &QComboBox::currentIndexChanged, this, &page_borders::slot_cropTypeChanged);
}

QWidget * page_borders::createBorderTypeNone()
{
    QWidget * w = new QWidget;
    return w;
}

PlainControl::PlainControl(page_borders *parent)
{
    this->parent = parent;

    borderWidth      = new DoubleSpinSet("Width",10,1,999);
    borderColorLabel = new QLabel("Border Color");
    borderColor      = new QLineEdit();
    borderColorPatch = new ClickableLabel;
    borderColorPatch->setMinimumWidth(75);

    QHBoxLayout * hbox = new QHBoxLayout;
    hbox->addLayout(borderWidth);
    hbox->addWidget(borderColorLabel);
    hbox->addWidget(borderColor);
    hbox->addWidget(borderColorPatch);
    hbox->addStretch();

    QVBoxLayout * vbox = new QVBoxLayout;
    vbox->addLayout(hbox);
    vbox->addStretch();

    // initial values
    borderWidth->setValue(20.0);

    QColor color(Qt::blue);
    borderColor->setText(color.name(QColor::HexArgb));
    QVariant variant = color;
    QString colcode  = variant.toString();
    borderColorPatch->setStyleSheet("QLabel { background-color :"+colcode+" ; border: 1px solid black;}");

    connect(borderWidth,         &DoubleSpinSet::valueChanged,   parent,   &page_borders::slot_borderWidthChanged);
    connect(borderColorPatch,    &ClickableLabel::clicked,       parent,   &page_borders::slot_pickBorderColor);
    connect(borderColor,         &QLineEdit::textChanged,        parent,   &page_borders::slot_borderColorChanged);

    setLayout(vbox);
}

TwoColorControl::TwoColorControl(page_borders *parent)
{
    this->parent = parent;

    borderWidth = new DoubleSpinSet("Width",10,0.1,99);
    borderWidth->setSingleStep(0.1);

    borderLength  = new DoubleSpinSet("Segement len",1.5,0.1,9.9);
    borderLength->setSingleStep(0.1);

    borderColorLabel = new QLabel("Border Color 1");
    borderColor      = new QLineEdit();
    borderColorPatch = new ClickableLabel;
    borderColorPatch->setMinimumWidth(75);

    borderColorLabel2 = new QLabel("Border Color 2");
    borderColor2      = new QLineEdit();
    borderColorPatch2 = new ClickableLabel;
    borderColorPatch2->setMinimumWidth(75);

    QHBoxLayout * hbox = new QHBoxLayout;
    hbox->addLayout(borderWidth);
    hbox->addLayout(borderLength);
    hbox->addStretch();

    QHBoxLayout * hbox1 = new QHBoxLayout;
    hbox1->addWidget(borderColorLabel);
    hbox1->addWidget(borderColor);
    hbox1->addWidget(borderColorPatch);
    hbox1->addStretch();

    QHBoxLayout * hbox2 = new QHBoxLayout;
    hbox2->addWidget(borderColorLabel2);
    hbox2->addWidget(borderColor2);
    hbox2->addWidget(borderColorPatch2);
    hbox2->addStretch();

    QVBoxLayout * vbox = new QVBoxLayout;
    vbox->addLayout(hbox);
    vbox->addLayout(hbox1);
    vbox->addLayout(hbox2);

    // initial values
    borderWidth->setValue(0.3);
    borderLength->setValue(1.0);

    QColor color1(0xa2,0x79,0x67);
    borderColor->setText(color1.name(QColor::HexArgb));
    QVariant variant = color1;
    QString colcode  = variant.toString();
    borderColorPatch->setStyleSheet("QLabel { background-color :"+colcode+" ; border: 1px solid black;}");

    QColor color2(TileWhite);
    borderColor2->setText(color2.name(QColor::HexArgb));
    variant = color2;
    colcode  = variant.toString();
    borderColorPatch2->setStyleSheet("QLabel { background-color :"+colcode+" ; border: 1px solid black;}");

    connect(borderWidth,        &DoubleSpinSet::valueChanged,     parent, &page_borders::slot_borderWidthChanged);
    connect(borderLength,       &DoubleSpinSet::valueChanged,     parent, &page_borders::slot_borderLengthChanged);
    connect(borderColorPatch,   &ClickableLabel::clicked,         parent, &page_borders::slot_pickBorderColor);
    connect(borderColorPatch2,  &ClickableLabel::clicked,         parent, &page_borders::slot_pickBorderColor2);
    connect(borderColor,        &QLineEdit::textChanged,          parent, &page_borders::slot_borderColorChanged);
    connect(borderColor2,       &QLineEdit::textChanged,          parent, &page_borders::slot_borderColor2Changed);

    setLayout(vbox);
}

BlockControl::BlockControl(page_borders *parent)
{
    this->parent = parent;

    borderWidth = new DoubleSpinSet("Width",10.0,0.1,99);
    borderWidth->setSingleStep(0.1);
    borderRows                 = new SpinSet("Rows",5,0,99);
    borderColumns              = new SpinSet("Cols",5,0,99);

    QHBoxLayout * hbox1 = new QHBoxLayout;
    hbox1->addLayout(borderWidth);
    hbox1->addLayout(borderRows);
    hbox1->addLayout(borderColumns);
    hbox1->addStretch();

    borderColorLabel = new QLabel("Border Color");
    borderColor      = new QLineEdit();
    borderColorPatch = new ClickableLabel;
    borderColorPatch->setMinimumWidth(75);

    QHBoxLayout * hbox2 = new QHBoxLayout;
    hbox2->addWidget(borderColorLabel);
    hbox2->addWidget(borderColor);
    hbox2->addWidget(borderColorPatch);
    hbox2->addStretch();

    QVBoxLayout * vbox = new QVBoxLayout;
    vbox->addLayout(hbox1);
    vbox->addLayout(hbox2);
    vbox->addStretch();

    // initial values
    borderWidth->setValue(1.5);
    borderRows->setValue(9);
    borderColumns->setValue(11);

    QColor color(0xa2,0x79,0x67);
    borderColor->setText(color.name(QColor::HexArgb));
    QVariant variant = color;
    QString colcode  = variant.toString();
    borderColorPatch->setStyleSheet("QLabel { background-color :"+colcode+" ; border: 1px solid black;}");

    connect(borderWidth,        &DoubleSpinSet::valueChanged,     parent, &page_borders::slot_borderWidthChanged);
    connect(borderRows,         &SpinSet::valueChanged,           parent, &page_borders::slot_borderRowsChanged);
    connect(borderColumns,      &SpinSet::valueChanged,           parent, &page_borders::slot_borderColumnsChanged);
    connect(borderColorPatch,   &ClickableLabel::clicked,         parent, &page_borders::slot_pickBorderColor);
    connect(borderColor,        &QLineEdit::textChanged,          parent, &page_borders::slot_borderColorChanged);

    setLayout(vbox);
}

QWidget * page_borders::createUndefinedShapeWidget()
{
    QWidget * w = new QWidget;
    //w->setLayout(rectBoundaryLayout);
    return w;
}

QWidget * page_borders::createRectShapeWidget()
{
    rectBoundaryLayout = new LayoutQRectF("Model Units:");

    // initial value
    QSize sz = Sys::viewController->viewSize();
    QRectF rect(QPointF(0,0),sz);
    rectBoundaryLayout->set(rect);

    connect(rectBoundaryLayout, &LayoutQRectF::rectChanged, this, &page_borders::slot_rectBoundaryChanged);

    QWidget * w = new QWidget;
    w->setLayout(rectBoundaryLayout);
    return w;
}

QWidget * page_borders::createCircleShapeWidget()
{
    QLabel * label = new QLabel("Circle Shape:");

    radius = new DoubleSpinSet("Radius",20,1,999);
    centre = new LayoutQPointF("Centre",4);

    QHBoxLayout * hbox = new QHBoxLayout;
    hbox->addWidget(label);
    hbox->addSpacing(9);
    hbox->addLayout(radius);
    hbox->addLayout(centre);
    hbox->addStretch();

    // initial values
    radius->setValue(4);
    centre->set(QPointF(0,0));

    connect(centre, &LayoutQPointF::pointChanged,  this, &page_borders::slot_centreChanged);
    connect(radius, &DoubleSpinSet::valueChanged,  this, &page_borders::slot_radiusChanged);

    QWidget * w = new QWidget;
    w->setLayout(hbox);
    return w;
}

QWidget * page_borders::createPolyShapeWidget()
{
    QWidget * w = new QWidget;
    //w->setLayout(rectBoundaryLayout);
    return w;
}


//////////////////////////////////
///
///
////////////////////////////////

void page_borders::onExit()
{
    clearPageStatus();
}

// refresh updates the page from the mosaic
void  page_borders::onRefresh()
{
    BorderPtr border = getMosaicBorder();

    blockPage(true);
    if (border)
    {
        chkUseViewSize->setChecked(border->getUseViewSize());
        refreshBorderType(border);
        refreshBorderCrop(border);
        smxWidget->setLayer(border.get());
        smxWidget->refresh();
        smxWidget->show();
    }
    else
    {
        cropTypes->select(CROP_UNDEFINED);
        cropTypeStack->setCurrentIndex(CROP_UNDEFINED);
        borderTypes->select(BORDER_NONE);
        borderTypeStack->setCurrentIndex(BORDER_NONE);
        chkUseViewSize->setChecked(false);
        smxWidget->hide();
    }
    blockPage(false);

}

void page_borders::refreshBorderType(BorderPtr border)
{
    // types
    eBorderType btype = border->getBorderType();
    borderTypes->select(btype);
    borderTypeStack->setCurrentIndex(btype);

    if (btype == BORDER_PLAIN)
    {
        qreal w = border->getWidth();
        controlPlain->borderWidth->setValue(w);

        QColor qc = border->getColor();
        controlPlain->borderColor->setText(qc.name(QColor::HexArgb));

        QVariant variant = qc;
        QString colcode  = variant.toString();
        controlPlain->borderColorPatch->setStyleSheet("QLabel { background-color :"+colcode+" ; border: 1px solid black;}");
    }
    else if (btype == BORDER_TWO_COLOR)
    {
        qreal w = border->getWidth();
        controlTwoColors->borderWidth->setValue(w);

        QColor qc = border->getColor();
        controlTwoColors->borderColor->setText(qc.name(QColor::HexArgb));

        QVariant variant = qc;
        QString colcode  = variant.toString();
        controlTwoColors->borderColorPatch->setStyleSheet("QLabel { background-color :"+colcode+" ; border: 1px solid black;}");

        auto bp2 = dynamic_cast<BorderTwoColor*>(border.get());
        if (bp2)
        {
            qreal len = bp2->getLength();
            controlTwoColors->borderLength->setValue(len);

            qc = bp2->getColor2();
            controlTwoColors->borderColor2->setText(qc.name(QColor::HexArgb));

            variant = qc;
            colcode  = variant.toString();
            controlTwoColors->borderColorPatch2->setStyleSheet("QLabel { background-color :"+colcode+" ; border: 1px solid black;}");
        }
    }
    else if (btype == BORDER_BLOCKS)
    {
        BorderBlocks * bp3 = dynamic_cast<BorderBlocks*>(border.get());
        if (bp3)
        {
            QColor  qc;
            int     rows;
            int     cols;
            qreal   width;
            bp3->get(qc, rows, cols, width);

            controlBlocks->borderRows->setValue(rows);
            controlBlocks->borderColumns->setValue(cols);
            controlBlocks->borderWidth->setValue(width);

            controlBlocks->borderColor->setText(qc.name(QColor::HexArgb));

            QVariant variant = qc;
            QString colcode  = variant.toString();
            controlBlocks->borderColorPatch->setStyleSheet("QLabel { background-color :"+colcode+" ; border: 1px solid black;}");
        }
    }
}

void page_borders::refreshBorderCrop(BorderPtr border)
{
    // shape
    eCropType ctype = border->getCropType();
    cropTypes->select(ctype);
    cropTypeStack->setCurrentIndex(ctype);

    if (ctype == CROP_RECTANGLE)
    {
        QRectF rect = border->getRect();
        rectBoundaryLayout->set(rect);
    }
    else if (ctype == CROP_CIRCLE)
    {
        auto c = border->getCircle();
        QPointF p = c.centre;
        centre->set(p);
        qreal r = c.radius;
        radius->setValue(r);
    }
}

void page_borders::slot_loadFromCrop()
{
    if (pageBlocked()) return;

    auto mosaic = mosaicMaker->getMosaic();
    if (!mosaic)
    {
        QMessageBox box(panel);
        box.setIcon(QMessageBox::Warning);
        box.setText("Load failed: No mosaic");
        box.setStandardButtons(QMessageBox::Ok);
        box.exec();
        return;
    }

   CropPtr mosaicCrop = mosaic->getCrop();
    if (!mosaicCrop)
    {
        QMessageBox box(panel);
        box.setIcon(QMessageBox::Warning);
        box.setText("Fetch failed: No crop");
        box.setStandardButtons(QMessageBox::Ok);
        box.exec();
        return;
    }

    BorderPtr border =mosaic->getBorder();
    if (!border || border->getBorderType() == BORDER_NONE)
    {
        QMessageBox box(panel);
        box.setIcon(QMessageBox::Warning);
        box.setText("Please select a Border Type before applying crop dimensions");
        box.setStandardButtons(QMessageBox::Ok);
        box.exec();
        return;
    }

    // convert crop
    CropPtr borderCrop = std::dynamic_pointer_cast<Crop>(border);
    Q_ASSERT(borderCrop);

    if (mosaicCrop->getCropType() != borderCrop->getCropType())
    {
        QMessageBox box(panel);
        box.setIcon(QMessageBox::Warning);
        box.setText("Border Crop Type <%1> does not match Mosaic Crop Type <%2>");
        box.setStandardButtons(QMessageBox::Ok);
        box.exec();
        return;
    }

    switch (mosaicCrop->getCropType())
    {
    case CROP_RECTANGLE:
    {
        auto rect = mosaicCrop->getRect();
        borderCrop->setRect(rect);
    }   break;

    case CROP_CIRCLE:
    {
        Circle c = mosaicCrop->getCircle();
        borderCrop->setCircle(c);
    }   break;

    case CROP_POLYGON:
    {
        APolygon poly = mosaicCrop->getAPolygon();
        borderCrop->setPolygon(poly);
    }   break;

    case CROP_UNDEFINED:
        break;
    }

    border->resetStyleRepresentation();

    emit sig_reconstructView();

    QMessageBox box(panel);
    box.setIcon(QMessageBox::Information);
    box.setText("Load OK");
    box.setStandardButtons(QMessageBox::Ok);
    box.exec();
}

void page_borders::slot_loadFromPainterCrop()
{
    if (pageBlocked()) return;

    auto mosaic = mosaicMaker->getMosaic();
    if (!mosaic)
    {
        QMessageBox box(panel);
        box.setIcon(QMessageBox::Warning);
        box.setText("Load failed: No mosaic");
        box.setStandardButtons(QMessageBox::Ok);
        box.exec();
        return;
    }

    CropPtr painterCrop = mosaic->getPainterCrop();
    if (!painterCrop)
    {
        QMessageBox box(panel);
        box.setIcon(QMessageBox::Warning);
        box.setText("Fetch failed: No crop");
        box.setStandardButtons(QMessageBox::Ok);
        box.exec();
        return;
    }

    BorderPtr border =mosaic->getBorder();
    if (!border || border->getBorderType() == BORDER_NONE)
    {
        QMessageBox box(panel);
        box.setIcon(QMessageBox::Warning);
        box.setText("Please select a Border Type before applying crop dimensions");
        box.setStandardButtons(QMessageBox::Ok);
        box.exec();
        return;
    }

    // convert crop
    CropPtr borderCrop = std::dynamic_pointer_cast<Crop>(border);
    Q_ASSERT(borderCrop);

    if (painterCrop->getCropType() != borderCrop->getCropType())
    {
        QMessageBox box(panel);
        box.setIcon(QMessageBox::Warning);
        box.setText("Border Crop Type <%1> does not match Mosaic Crop Type <%2>");
        box.setStandardButtons(QMessageBox::Ok);
        box.exec();
        return;
    }

    switch (painterCrop->getCropType())
    {
    case CROP_RECTANGLE:
    {
        auto rect = painterCrop->getRect();
        borderCrop->setRect(rect);
    }   break;

    case CROP_CIRCLE:
    {
        Circle c = painterCrop->getCircle();
        borderCrop->setCircle(c);
    }   break;

    case CROP_POLYGON:
    {
        APolygon poly = painterCrop->getAPolygon();
        borderCrop->setPolygon(poly);
    }   break;

    case CROP_UNDEFINED:
        break;
    }

    border->resetStyleRepresentation();

    emit sig_reconstructView();

    QMessageBox box(panel);
    box.setIcon(QMessageBox::Information);
    box.setText("Load OK");
    box.setStandardButtons(QMessageBox::Ok);
    box.exec();
}

void page_borders::createBorder()
{
    MosaicPtr mosaic = mosaicMaker->getMosaic();
    Q_ASSERT(mosaic);

    eBorderType  btype = static_cast<eBorderType>(borderTypes->currentData().toInt());
    eCropType ctype    = static_cast<eCropType>(cropTypes->currentData().toInt());

    BorderPtr bp;
    switch (btype)
    {
    case BORDER_PLAIN:
    {
        switch (ctype)
        {
        case CROP_RECTANGLE:
        {
            qreal width  = controlPlain->borderWidth->value();
            QColor color(controlPlain->borderColor->text());
            QRectF rect  = rectBoundaryLayout->get();

            bp = make_shared<BorderPlain>(mosaic.get(),rect,width,color);
        }   break;

        case CROP_CIRCLE:
        {
            QColor color(controlPlain->borderColor->text());
            qreal width  = controlPlain->borderWidth->value();
            QPointF pt   = centre->get();
            qreal   r    = radius->value();
            Circle c(pt,r);

            bp = make_shared<BorderPlain>(mosaic.get(),c,width,color);
        } break;

        case CROP_POLYGON:      // for now
            break;
        case CROP_UNDEFINED:
            break;
        }
    }   break;

    case BORDER_TWO_COLOR:
    {
        qreal width  = controlTwoColors->borderWidth->value();
        qreal length = controlTwoColors->borderLength->value();
        QRectF rect  = rectBoundaryLayout->get();
        QColor color1(controlTwoColors->borderColor->text());
        QColor color2(controlTwoColors->borderColor2->text());

        bp = make_shared<BorderTwoColor>(mosaic.get(),rect,color1,color2,width,length);
    }   break;

    case BORDER_BLOCKS:
    {
        QRectF rect = rectBoundaryLayout->get();
        QColor color1(controlBlocks->borderColor->text());
        int rows    = controlBlocks->borderRows->value();
        int cols    = controlBlocks->borderColumns->value();
        qreal width = controlBlocks->borderWidth->value();

        bp = make_shared<BorderBlocks>(mosaic.get(),rect,color1,rows,cols,width);
    }
    case BORDER_NONE:
        break;
    }

    if (bp)
    {
        bp->setModelXform(Sys::viewController->getSystemModelXform(),true,Sys::nextSigid());
        bp->createStyleRepresentation();
        mosaic->setBorder(bp);

        emit sig_reconstructView();
        emit sig_updateView();
    }
}

void page_borders::slot_removeBorder()
{
    if (pageBlocked()) return;

    auto mosaic = mosaicMaker->getMosaic();
    if (mosaic)
    {
        BorderPtr bp = mosaic->getBorder();
        if (bp)
        {
            mosaic->deleteStyle(bp);
            emit sig_reconstructView();
        }
    }
}

void page_borders::slot_borderTypeChanged(int row)
{
    if (pageBlocked()) return;

    borderTypeStack->setCurrentIndex(row);
    if (borderTypes->currentData() == BORDER_NONE)
    {
        slot_removeBorder();
    }
    else
    {
        auto border = getMosaicBorder();
        if (!border)
        {
            cropTypes->select(CROP_RECTANGLE); // default
            cropTypeStack->setCurrentIndex(cropTypes->currentIndex());

            auto mosaic = mosaicMaker->getMosaic();
            Q_ASSERT(mosaic);
            auto style = mosaic->getFirstStyle();
            Q_ASSERT(style);
            QRectF rect(Sys::viewController->viewRect());
            rect = style->screenToModel(rect);
            rectBoundaryLayout->blockSignals(true);
            rectBoundaryLayout->set(rect);
            rectBoundaryLayout->blockSignals(false);
        }
        createBorder();
    }
}

void page_borders::slot_cropTypeChanged(int row)
{
    if (pageBlocked()) return;

    cropTypeStack->setCurrentIndex(row);

    createBorder();
}

BorderPtr page_borders::getMosaicBorder()
{
    BorderPtr bp;
    MosaicPtr m = mosaicMaker->getMosaic();
    if (m)
    {
        bp = m->getBorder();
    }
    return bp;
}

void page_borders::slot_borderWidthChanged(qreal width)
{
    if (pageBlocked()) return;

    BorderPtr bp = getMosaicBorder();
    if (!bp) return;

    bp->setWidth(width);
    emit sig_updateView();
}

void page_borders::slot_borderLengthChanged(qreal length)
{
    if (pageBlocked()) return;

    BorderPtr bp = getMosaicBorder();
    if (!bp) return;

    BorderTwoColor * bp2 = dynamic_cast<BorderTwoColor*>(bp.get());
    if (bp2)
    {
        bp2->setLength(length);
        emit sig_updateView();
    }
}

void page_borders::slot_borderRowsChanged(int rows)
{
    if (pageBlocked()) return;

    BorderPtr bp = getMosaicBorder();
    if (!bp) return;

    BorderBlocks * bp3 = dynamic_cast<BorderBlocks*>(bp.get());
    if (bp3)
    {
        bp3->setRows(rows);
        emit sig_updateView();
    }
}

void page_borders::slot_borderColumnsChanged(int cols)
{
    if (pageBlocked()) return;

    BorderPtr bp = getMosaicBorder();
    if (!bp) return;

    BorderBlocks * bp3 = dynamic_cast<BorderBlocks*>(bp.get());
    if (bp3)
    {
        bp3->setCols(cols);
        emit sig_updateView();
    }
}

void page_borders::slot_borderColorChanged(const QString &text)
{
    if (pageBlocked()) return;

    BorderPtr bp = getMosaicBorder();
    if (!bp) return;

    QColor qc(text);
    bp->setColor(qc);
    emit sig_updateView();
}

void page_borders::slot_borderColor2Changed(const QString &text)
{
    if (pageBlocked()) return;

    BorderPtr bp = getMosaicBorder();
    if (!bp) return;

    QColor qc(text);
    bp->setColor2(qc);
    emit sig_updateView();
}

void page_borders::slot_pickBorderColor()
{
    if (pageBlocked()) return;

    BorderPtr bp = getMosaicBorder();
    if (!bp) return;

    QColor color = bp->getColor();

    AQColorDialog dlg(color,this);
    dlg.setCurrentColor(color);
    int rv = dlg.exec();
    if (rv != QDialog::Accepted) return;

    color = dlg.selectedColor();
    if (color.isValid())
    {
        bp->setColor(color);
        emit sig_updateView();
    }
}

void page_borders::slot_pickBorderColor2()
{
    if (pageBlocked()) return;
    BorderPtr bp = getMosaicBorder();
    if (!bp) return;

    QColor color = bp->getColor2();

    AQColorDialog dlg(color,this);
    dlg.setCurrentColor(color);
    int rv = dlg.exec();
    if (rv != QDialog::Accepted) return;

    color = dlg.selectedColor();
    if (color.isValid())
    {
        bp->setColor2(color);
        emit sig_updateView();
    }
}


void page_borders::slot_rectBoundaryChanged()
{
    if (pageBlocked()) return;

    qDebug() << "page_borders::slot_rectBoundaryChanged";
    BorderPtr bp = getMosaicBorder();
    if (!bp)
        return;

    QRectF rect = rectBoundaryLayout->get();
    bp->setRect(rect);

    bp->resetStyleRepresentation();
    emit sig_updateView();
}

void page_borders::slot_useViewSzChanged(bool checked)
{
    if (pageBlocked()) return;

    qDebug() << "page_borders::slot_useViewSzChanged";
    BorderPtr bp = getMosaicBorder();
    if (!bp)
        return;

    bp->setUseViewSize(checked);
    emit sig_updateView();
}

void page_borders::slot_centreChanged()
{
    if (pageBlocked()) return;

    BorderPtr bp = getMosaicBorder();
    if (!bp)
        return;

    Circle & c = bp->getCircle();
    QPointF p = centre->get();
    c.setCenter(p);

    bp->resetStyleRepresentation();
    emit sig_updateView();
}

void page_borders::slot_radiusChanged(qreal r)
{
    if (pageBlocked()) return;

    BorderPtr bp = getMosaicBorder();
    if (!bp)
        return;

    Circle & c = bp->getCircle();
    c.setRadius(r);

    bp->resetStyleRepresentation();
    emit sig_updateView();
}
