#include <QComboBox>
#include <QGroupBox>
#include <QStackedLayout>
#include <QButtonGroup>
#include <QMessageBox>

#include "panels/page_borders.h"
#include "enums/eborder.h"
#include "geometry/crop.h"
#include "makers/mosaic_maker/mosaic_maker.h"
#include "makers/map_editor/map_editor.h"
#include "misc/border.h"
#include "misc/tile_color_defs.h"
#include "mosaic/mosaic.h"
#include "panels/controlpanel.h"
#include "viewers/border_view.h"
#include "viewers/view_controller.h"
#include "widgets/layout_sliderset.h"

using std::string;
using std::make_shared;

page_borders::page_borders(ControlPanel * apanel)  : panel_page(apanel,PAGE_BORDER_MAKER,"Border Maker")
{
    borderView = BorderView::getInstance();

    // create button
    QPushButton * pbLoadCrop = new QPushButton("Load from Crop");
    QPushButton * pbRemove  =  new QPushButton("Remove Border");

    QHBoxLayout * hbox3 = new QHBoxLayout;
    hbox3->addStretch();
    hbox3->addWidget(pbLoadCrop);
    hbox3->addStretch();
    hbox3->addWidget(pbRemove);
    hbox3->addStretch();

    connect(pbLoadCrop, &QPushButton::clicked, this, &page_borders::slot_loadFromCrop);
    connect(pbRemove,   &QPushButton::clicked, this, &page_borders::slot_removeBorder);

    // border selection
    QLabel * label2 = new QLabel("Border Shape");
    chkUseViewSize  = new QCheckBox("Use view size");

    connect(chkUseViewSize,   &QCheckBox::clicked, this, &page_borders::slot_useViewSzChanged);

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

    connect(cropTypes, QOverload<int>::of(&QComboBox::currentIndexChanged), cropTypeStack,  &QStackedLayout::setCurrentIndex);

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

    QHBoxLayout * hbox = new QHBoxLayout;
    hbox->addWidget(label);
    hbox->addWidget(borderTypes);
    hbox->addStretch();

    // stacked layout
    borderTypeStack = new QStackedLayout;

    w = createBorderTypeNone();
    borderTypeStack->addWidget(w);

    w = createBorderTypePlain();
    borderTypeStack->addWidget(w);

    w = createBorderType2Color();
    borderTypeStack->addWidget(w);

    w = createBorderTypeBlocks();
    borderTypeStack->addWidget(w);

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

    connect(borderTypes, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &page_borders::slot_borderTypeChanged);
    connect(cropTypes,   QOverload<int>::of(&QComboBox::currentIndexChanged), this, &page_borders::slot_cropTypeChanged);
}

QWidget * page_borders::createBorderTypeNone()
{
    QWidget * w = new QWidget;
    return w;
}

QWidget * page_borders::createBorderTypePlain()
{
    borderWidth[BORDER_PLAIN]      = new DoubleSpinSet("Width",10,1,999);
    borderColorLabel[BORDER_PLAIN] = new QLabel("Border Color");
    borderColor[BORDER_PLAIN]      = new QLineEdit();
    borderColorPatch[BORDER_PLAIN] = new ClickableLabel;
    borderColorPatch[BORDER_PLAIN]->setMinimumWidth(75);

    QHBoxLayout * hbox = new QHBoxLayout;
    hbox->addLayout(borderWidth[BORDER_PLAIN]);
    hbox->addWidget(borderColorLabel[BORDER_PLAIN]);
    hbox->addWidget(borderColor[BORDER_PLAIN]);
    hbox->addWidget(borderColorPatch[BORDER_PLAIN]);
    hbox->addStretch();

    QVBoxLayout * vbox = new QVBoxLayout;
    vbox->addLayout(hbox);
    vbox->addStretch();

    // initial values
    borderWidth[BORDER_PLAIN]->setValue(60.0);

    QColor color(Qt::blue);
    borderColor[BORDER_PLAIN]->setText(color.name(QColor::HexArgb));
    QVariant variant = color;
    QString colcode  = variant.toString();
    borderColorPatch[BORDER_PLAIN]->setStyleSheet("QLabel { background-color :"+colcode+" ; border: 1px solid black;}");

    connect(borderWidth[BORDER_PLAIN],         &DoubleSpinSet::valueChanged,   this,   &page_borders::slot_borderWidthChanged);
    connect(borderColorPatch[BORDER_PLAIN],    &ClickableLabel::clicked,       this,   &page_borders::slot_pickBorderColor);
    connect(borderColor[BORDER_PLAIN],         &QLineEdit::textChanged,        this,   [this] { slot_borderColorChanged(borderColor[BORDER_PLAIN]); });

    QWidget * w = new QWidget;
    w->setLayout(vbox);
    return w;
}

QWidget * page_borders::createBorderType2Color()
{
    borderWidth[BORDER_TWO_COLOR] = new DoubleSpinSet("Width",10,0.1,99);
    borderWidth[BORDER_TWO_COLOR]->setSingleStep(0.1);

    borderLength                  = new DoubleSpinSet("Segement len",1.5,0.1,9.9);
    borderLength->setSingleStep(0.1);

    borderColorLabel[BORDER_PLAIN] = new QLabel("Border Color 1");
    borderColor[BORDER_PLAIN]      = new QLineEdit();
    borderColorPatch[BORDER_PLAIN] = new ClickableLabel;
    borderColorPatch[BORDER_PLAIN]->setMinimumWidth(75);

    borderColorLabel[BORDER_TWO_COLOR] = new QLabel("Border Color 2");
    borderColor[BORDER_TWO_COLOR]      = new QLineEdit();
    borderColorPatch[BORDER_TWO_COLOR] = new ClickableLabel;
    borderColorPatch[BORDER_TWO_COLOR]->setMinimumWidth(75);

    QHBoxLayout * hbox = new QHBoxLayout;
    hbox->addLayout(borderWidth[BORDER_TWO_COLOR]);
    hbox->addLayout(borderLength);
    hbox->addStretch();

    QHBoxLayout * hbox1 = new QHBoxLayout;
    hbox1->addWidget(borderColorLabel[BORDER_PLAIN]);
    hbox1->addWidget(borderColor[BORDER_PLAIN]);
    hbox1->addWidget(borderColorPatch[BORDER_PLAIN]);
    hbox1->addStretch();

    QHBoxLayout * hbox2 = new QHBoxLayout;
    hbox2->addWidget(borderColorLabel[BORDER_TWO_COLOR]);
    hbox2->addWidget(borderColor[BORDER_TWO_COLOR]);
    hbox2->addWidget(borderColorPatch[BORDER_TWO_COLOR]);
    hbox2->addStretch();

    QVBoxLayout * vbox = new QVBoxLayout;
    vbox->addLayout(hbox);
    vbox->addLayout(hbox1);
    vbox->addLayout(hbox2);

    // initial values
    borderWidth[BORDER_TWO_COLOR]->setValue(0.3);
    borderLength->setValue(1.0);

    QColor color1(0xa2,0x79,0x67);
    borderColor[BORDER_PLAIN]->setText(color1.name(QColor::HexArgb));
    QVariant variant = color1;
    QString colcode  = variant.toString();
    borderColorPatch[BORDER_PLAIN]->setStyleSheet("QLabel { background-color :"+colcode+" ; border: 1px solid black;}");

    QColor color2(TileWhite);
    borderColor[BORDER_TWO_COLOR]->setText(color2.name(QColor::HexArgb));
    variant = color2;
    colcode  = variant.toString();
    borderColorPatch[BORDER_TWO_COLOR]->setStyleSheet("QLabel { background-color :"+colcode+" ; border: 1px solid black;}");

    connect(borderWidth[BORDER_TWO_COLOR],      &DoubleSpinSet::valueChanged,     this, &page_borders::slot_borderWidthChanged);
    connect(borderLength,                       &DoubleSpinSet::valueChanged,     this, &page_borders::slot_borderLengthChanged);
    connect(borderColorPatch[BORDER_PLAIN],     &ClickableLabel::clicked,         this, &page_borders::slot_pickBorderColor);
    connect(borderColorPatch[BORDER_TWO_COLOR], &ClickableLabel::clicked,         this, &page_borders::slot_pickBorderColor2);
    connect(borderColor[BORDER_PLAIN],          &QLineEdit::textChanged,          this,   [this] { slot_borderColorChanged(borderColor[BORDER_PLAIN]); });
    connect(borderColor[BORDER_TWO_COLOR],      &QLineEdit::textChanged,          this,   [this] { slot_borderColor2Changed(borderColor[BORDER_TWO_COLOR]); });

    QWidget * w = new QWidget;
    w->setLayout(vbox);
    return w;
}

QWidget   * page_borders::createBorderTypeBlocks()
{
    borderWidth[BORDER_BLOCKS] = new DoubleSpinSet("Width",10.0,0.1,99);
    borderWidth[BORDER_BLOCKS]->setSingleStep(0.1);
    borderRows                 = new SpinSet("Rows",5,0,99);
    borderCols                 = new SpinSet("Cols",5,0,99);

    QHBoxLayout * hbox1 = new QHBoxLayout;
    hbox1->addLayout(borderWidth[BORDER_BLOCKS]);
    hbox1->addLayout(borderRows);
    hbox1->addLayout(borderCols);
    hbox1->addStretch();

    borderColorLabel[BORDER_BLOCKS] = new QLabel("Border Color");
    borderColor[BORDER_BLOCKS]      = new QLineEdit();
    borderColorPatch[BORDER_BLOCKS] = new ClickableLabel;
    borderColorPatch[BORDER_BLOCKS]->setMinimumWidth(75);

    QHBoxLayout * hbox2 = new QHBoxLayout;
    hbox2->addWidget(borderColorLabel[BORDER_BLOCKS]);
    hbox2->addWidget(borderColor[BORDER_BLOCKS]);
    hbox2->addWidget(borderColorPatch[BORDER_BLOCKS]);
    hbox2->addStretch();

    QVBoxLayout * vbox = new QVBoxLayout;
    vbox->addLayout(hbox1);
    vbox->addLayout(hbox2);
    vbox->addStretch();

    // initial values
    borderWidth[BORDER_BLOCKS]->setValue(1.5);
    borderRows->setValue(9);
    borderCols->setValue(11);

    QColor color(0xa2,0x79,0x67);
    borderColor[BORDER_BLOCKS]->setText(color.name(QColor::HexArgb));
    QVariant variant = color;
    QString colcode  = variant.toString();
    borderColorPatch[BORDER_BLOCKS]->setStyleSheet("QLabel { background-color :"+colcode+" ; border: 1px solid black;}");

    connect(borderWidth[BORDER_BLOCKS],     &DoubleSpinSet::valueChanged,     this, &page_borders::slot_borderWidthChanged);
    connect(borderRows,                     &SpinSet::valueChanged,           this, &page_borders::slot_borderRowsChanged);
    connect(borderCols,                     &SpinSet::valueChanged,           this, &page_borders::slot_borderColsChanged);
    connect(borderColorPatch[BORDER_BLOCKS],&ClickableLabel::clicked,         this, &page_borders::slot_pickBorderColor);
    connect(borderColor[BORDER_BLOCKS],     &QLineEdit::textChanged,          this,   [this] { slot_borderColorChanged(borderColor[BORDER_BLOCKS]); });

    QWidget * w = new QWidget;
    w->setLayout(vbox);
    return w;
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
    QSize sz = view->size();
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

    // initalvalues
    radius->setValue(100);

    QSize sz = view->size();
    QRectF rect(QPointF(0,0),sz);
    QPointF pt = rect.center();
    centre->set(pt);

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

void page_borders::onEnter()
{
    panel->pushPanelStatus("Left-click inside crop to display and move");
}

void page_borders::onExit()
{
    panel->popPanelStatus();
}

// refresh updates the page from the mosaic
void  page_borders::onRefresh()
{
    auto border = getMosaicBorder();

    blockPage(true);
    if (border)
    {
        chkUseViewSize->setChecked(border->getUseViewSize());
        refreshBorderType(border);
        refreshBorderCrop(border);
    }
    else
    {
        cropTypes->select(CROP_UNDEFINED);
        cropTypeStack->setCurrentIndex(CROP_UNDEFINED);
        borderTypes->select(BORDER_NONE);
        borderTypeStack->setCurrentIndex(BORDER_NONE);
        chkUseViewSize->setChecked(false);
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
        borderWidth[BORDER_PLAIN]->setValue(w);

        QColor qc = border->getColor();
        borderColor[BORDER_PLAIN]->setText(qc.name(QColor::HexArgb));

        QVariant variant = qc;
        QString colcode  = variant.toString();
        borderColorPatch[BORDER_PLAIN]->setStyleSheet("QLabel { background-color :"+colcode+" ; border: 1px solid black;}");
    }
    else if (btype == BORDER_TWO_COLOR)
    {
        qreal w = border->getWidth();
        borderWidth[BORDER_TWO_COLOR]->setValue(w);

        QColor qc = border->getColor();
        borderColor[BORDER_PLAIN]->setText(qc.name(QColor::HexArgb));

        QVariant variant = qc;
        QString colcode  = variant.toString();
        borderColorPatch[BORDER_PLAIN]->setStyleSheet("QLabel { background-color :"+colcode+" ; border: 1px solid black;}");

        auto bp2 = dynamic_cast<BorderTwoColor*>(border.get());
        if (bp2)
        {
            qreal len = bp2->getLength();
            borderLength->setValue(len);

            qc = bp2->getColor2();
            borderColor[BORDER_TWO_COLOR]->setText(qc.name(QColor::HexArgb));

            variant = qc;
            colcode  = variant.toString();
            borderColorPatch[BORDER_TWO_COLOR]->setStyleSheet("QLabel { background-color :"+colcode+" ; border: 1px solid black;}");
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

            borderRows->setValue(rows);
            borderCols->setValue(cols);
            borderWidth[BORDER_BLOCKS]->setValue(width);

            borderColor[BORDER_BLOCKS]->setText(qc.name(QColor::HexArgb));

            QVariant variant = qc;
            QString colcode  = variant.toString();
            borderColorPatch[BORDER_BLOCKS]->setStyleSheet("QLabel { background-color :"+colcode+" ; border: 1px solid black;}");
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
        auto poly = mosaicCrop->getPolygon();
        borderCrop->setPolygon(poly);
    }   break;

    case CROP_UNDEFINED:
        break;
    }

    border->setRequiresConstruction(true);

    emit sig_refreshView();

    QMessageBox box(panel);
    box.setIcon(QMessageBox::Information);
    box.setText("Load OK");
    box.setStandardButtons(QMessageBox::Ok);
    box.exec();
}

void page_borders::createBorder()
{
    MosaicPtr mosaic = mosaicMaker->getMosaic();
    if (!mosaic)
    {
        QMessageBox box(panel);
        box.setIcon(QMessageBox::Warning);
        box.setText("There is no mosaic - cannot create border");
        box.exec();
        return;
    }

    eBorderType  btype = static_cast<eBorderType>(borderTypes->currentData().toInt());
    eCropType ctype    = static_cast<eCropType>(cropTypes->currentData().toInt());

    switch (btype)
    {
    case BORDER_PLAIN:
    {
        switch (ctype)
        {
        case CROP_RECTANGLE:
        {
            qreal width  = borderWidth[BORDER_PLAIN]->value();
            QColor color(borderColor[BORDER_PLAIN]->text());
            QRectF rect  = rectBoundaryLayout->get();

            auto bp = make_shared<BorderPlain>(rect,width,color);
            bp->construct();
            mosaic->setBorder(bp);
        }
            break;

        case CROP_CIRCLE:
        {
            QColor color(borderColor[BORDER_PLAIN]->text());
            qreal width  = borderWidth[BORDER_PLAIN]->value();
            QPointF pt   = centre->get();
            qreal   r    = radius->value();
            Circle c(pt,r);

            auto bp = make_shared<BorderPlain>(c,width,color);
            bp->construct();
            mosaic->setBorder(bp);
        }
            break;
        case CROP_POLYGON:      // for now
            break;
        case CROP_UNDEFINED:
            break;
        }
        break;
    }
    case BORDER_TWO_COLOR:
    {
        qreal width  = borderWidth[BORDER_TWO_COLOR]->value();
        qreal length = borderLength->value();
        QRectF rect  = rectBoundaryLayout->get();
        QColor color1(borderColor[BORDER_PLAIN]->text());
        QColor color2(borderColor[BORDER_TWO_COLOR]->text());

        auto bp = make_shared<BorderTwoColor>(rect,color1,color2,width,length);
        bp->setRequiresConstruction(true);
        mosaic->setBorder(bp);
    }
        break;

    case BORDER_BLOCKS:
    {
        QRectF rect = rectBoundaryLayout->get();
        QColor color1(borderColor[BORDER_BLOCKS]->text());
        int rows    = borderRows->value();
        int cols    = borderCols->value();
        qreal width = borderWidth[BORDER_BLOCKS]->value();

        auto bp = make_shared<BorderBlocks>(rect,color1,rows,cols,width);
        bp->setRequiresConstruction(true);
        mosaic->setBorder(bp);
    }

    case BORDER_NONE:
        break;
    }

    emit sig_refreshView();
    view->update();
}

void page_borders::slot_removeBorder()
{
    if (pageBlocked()) return;

    BorderPtr bp;
    auto mosaic = mosaicMaker->getMosaic();
    if (mosaic)
    {
        mosaic->setBorder(bp);
        emit sig_refreshView();
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

            QRectF rect(view->rect());
            rect = borderView->screenToWorld(rect);
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
    view->update();
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
        view->update();
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
        view->update();
    }
}

void page_borders::slot_borderColsChanged(int cols)
{
    if (pageBlocked()) return;

    BorderPtr bp = getMosaicBorder();
    if (!bp) return;

    BorderBlocks * bp3 = dynamic_cast<BorderBlocks*>(bp.get());
    if (bp3)
    {
        bp3->setCols(cols);
        view->update();
    }
}

void page_borders::slot_borderColorChanged(QLineEdit * le)
{
    if (pageBlocked()) return;

    BorderPtr bp = getMosaicBorder();
    if (!bp) return;

    QString color = le->text();
    QColor qc(color);
    bp->setColor(qc);
    view->update();
}

void page_borders::slot_borderColor2Changed(QLineEdit * le)
{
    if (pageBlocked()) return;

    BorderPtr bp = getMosaicBorder();
    if (!bp) return;

    QString color = le->text();
    QColor qc(color);
    bp->setColor2(qc);
    view->update();
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
        borderColor[BORDER_PLAIN]->setText(color.name(QColor::HexArgb));

        QVariant variant = color;
        QString colcode  = variant.toString();
        borderColorPatch[BORDER_PLAIN]->setStyleSheet("QLabel { background-color :"+colcode+" ; border: 1px solid black;}");
        bp->setColor(color);
        view->update();
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
        borderColor[2]->setText(color.name(QColor::HexArgb));

        QVariant variant = color;
        QString colcode  = variant.toString();
        borderColorPatch[2]->setStyleSheet("QLabel { background-color :"+colcode+" ; border: 1px solid black;}");
        bp->setColor2(color);
        view->update();
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

    bp->setRequiresConstruction(true);
    view->update();
}

void page_borders::slot_useViewSzChanged(bool checked)
{
    if (pageBlocked()) return;

    qDebug() << "page_borders::slot_useViewSzChanged";
    BorderPtr bp = getMosaicBorder();
    if (!bp)
        return;

    bp->setUseViewSize(checked);
    view->update();
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

    bp->setRequiresConstruction(true);
    view->update();
}

void page_borders::slot_radiusChanged(qreal r)
{
    if (pageBlocked()) return;

    BorderPtr bp = getMosaicBorder();
    if (!bp)
        return;

    Circle & c = bp->getCircle();
    c.setRadius(r);

    bp->setRequiresConstruction(true);
    view->update();
}
