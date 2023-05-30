#include <QComboBox>
#include <QGroupBox>
#include <QStackedLayout>
#include <QButtonGroup>
#include <QMessageBox>

#include "panels/page_borders.h"
#include "geometry/crop.h"
#include "makers/mosaic_maker/mosaic_maker.h"
#include "makers/map_editor/map_editor.h"
#include "misc/border.h"
#include "mosaic/mosaic.h"
#include "panels/panel.h"
#include "settings/configuration.h"
#include "viewers/border_view.h"
#include "viewers/viewcontrol.h"
#include "widgets/layout_sliderset.h"

using std::string;
using std::make_shared;

page_borders::page_borders(ControlPanel * apanel)  : panel_page(apanel,"Border Maker")
{
    brview = BorderView::getInstance();

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

    setMaximumWidth(762);

    // assembly
    vbox->addLayout(hbox);
    vbox->addWidget(borderTypeBox);
    vbox->addSpacing(9);
    vbox->addLayout(hbox2);
    vbox->addWidget(cropTypeBox);
    vbox->addSpacing(9);
    vbox->addLayout(hbox3);
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
    borderWidth[0] = new DoubleSpinSet("Width",10,1,999);

    borderColorLabel[0] = new QLabel("Border Color");
    borderColor[0]      = new QLineEdit();
    borderColorPatch[0] = new ClickableLabel;
    borderColorPatch[0]->setMinimumWidth(75);

    QHBoxLayout * hbox = new QHBoxLayout;
    hbox->addLayout(borderWidth[0]);
    hbox->addWidget(borderColorLabel[0]);
    hbox->addWidget(borderColor[0]);
    hbox->addWidget(borderColorPatch[0]);
    hbox->addStretch();

    QVBoxLayout * vbox = new QVBoxLayout;
    vbox->addLayout(hbox);
    vbox->addStretch();

    // initial values
    borderWidth[0]->setValue(20.0);

    QColor color(Qt::blue);
    borderColor[0]->setText(color.name(QColor::HexArgb));
    QVariant variant = color;
    QString colcode  = variant.toString();
    borderColorPatch[0]->setStyleSheet("QLabel { background-color :"+colcode+" ; border: 1px solid black;}");

    connect(borderWidth[0],         &DoubleSpinSet::valueChanged,   this,   &page_borders::slot_borderWidthChanged);
    connect(borderColorPatch[0],    &ClickableLabel::clicked,       this,   &page_borders::slot_pickBorderColor);
    connect(borderColor[0],         &QLineEdit::textChanged,        this,   [this] { slot_borderColorChanged(borderColor[0]); });

    QWidget * w = new QWidget;
    w->setLayout(vbox);
    return w;
}

QWidget * page_borders::createBorderType2Color()
{
    borderWidth[1] = new DoubleSpinSet("Width",10,1,999);

    borderColorLabel[1] = new QLabel("Border Color 1");
    borderColor[1]      = new QLineEdit();
    borderColorPatch[1] = new ClickableLabel;
    borderColorPatch[1]->setMinimumWidth(75);

    borderColorLabel[2] = new QLabel("Border Color 2");
    borderColor[2]      = new QLineEdit();
    borderColorPatch[2] = new ClickableLabel;
    borderColorPatch[2]->setMinimumWidth(75);

    QHBoxLayout * hbox = new QHBoxLayout;
    hbox->addLayout(borderWidth[1]);
    hbox->addStretch();

    QHBoxLayout * hbox1 = new QHBoxLayout;
    hbox1->addWidget(borderColorLabel[1]);
    hbox1->addWidget(borderColor[1]);
    hbox1->addWidget(borderColorPatch[1]);
    hbox1->addStretch();

    QHBoxLayout * hbox2 = new QHBoxLayout;
    hbox2->addWidget(borderColorLabel[2]);
    hbox2->addWidget(borderColor[2]);
    hbox2->addWidget(borderColorPatch[2]);
    hbox2->addStretch();

    QVBoxLayout * vbox = new QVBoxLayout;
    vbox->addLayout(hbox);
    vbox->addLayout(hbox1);
    vbox->addLayout(hbox2);

    // initial values
    borderWidth[1]->setValue(20);

    QColor color1(0xa2,0x79,0x67);
    borderColor[1]->setText(color1.name(QColor::HexArgb));
    QVariant variant = color1;
    QString colcode  = variant.toString();
    borderColorPatch[1]->setStyleSheet("QLabel { background-color :"+colcode+" ; border: 1px solid black;}");

    QColor color2(TileWhite);
    borderColor[2]->setText(color2.name(QColor::HexArgb));
    variant = color2;
    colcode  = variant.toString();
    borderColorPatch[2]->setStyleSheet("QLabel { background-color :"+colcode+" ; border: 1px solid black;}");

    connect(borderWidth[1],     &DoubleSpinSet::valueChanged,     this, &page_borders::slot_borderWidthChanged);
    connect(borderColorPatch[1],&ClickableLabel::clicked,         this, &page_borders::slot_pickBorderColor);
    connect(borderColorPatch[2],&ClickableLabel::clicked,         this, &page_borders::slot_pickBorderColor2);
    connect(borderColor[1],     &QLineEdit::textChanged,          this,   [this] { slot_borderColorChanged(borderColor[1]); });
    connect(borderColor[2],     &QLineEdit::textChanged,          this,   [this] { slot_borderColorChanged(borderColor[2]); });

    QWidget * w = new QWidget;
    w->setLayout(vbox);
    return w;
}

QWidget   * page_borders::createBorderTypeBlocks()
{
    borderWidth[2] = new DoubleSpinSet("Width",150,10,999);
    borderRows     = new SpinSet("Rows",5,0,99);
    borderCols =     new SpinSet("Cols",5,0,99);

    QHBoxLayout * hbox1 = new QHBoxLayout;
    hbox1->addLayout(borderWidth[2]);
    hbox1->addLayout(borderRows);
    hbox1->addLayout(borderCols);
    hbox1->addStretch();

    borderColorLabel[3] = new QLabel("Border Color");
    borderColor[3]      = new QLineEdit();
    borderColorPatch[3] = new ClickableLabel;
    borderColorPatch[3]->setMinimumWidth(75);

    QHBoxLayout * hbox2 = new QHBoxLayout;
    hbox2->addWidget(borderColorLabel[3]);
    hbox2->addWidget(borderColor[3]);
    hbox2->addWidget(borderColorPatch[3]);
    hbox2->addStretch();

    QVBoxLayout * vbox = new QVBoxLayout;
    vbox->addLayout(hbox1);
    vbox->addLayout(hbox2);
    vbox->addStretch();

    // initial values
    borderWidth[2]->setValue(150);
    borderRows->setValue(9);
    borderCols->setValue(11);

    QColor color(0xa2,0x79,0x67);
    borderColor[3]->setText(color.name(QColor::HexArgb));
    QVariant variant = color;
    QString colcode  = variant.toString();
    borderColorPatch[3]->setStyleSheet("QLabel { background-color :"+colcode+" ; border: 1px solid black;}");

    connect(borderWidth[2],     &DoubleSpinSet::valueChanged,     this, &page_borders::slot_borderWidthChanged);
    connect(borderRows,         &SpinSet::valueChanged,           this, &page_borders::slot_borderRowsChanged);
    connect(borderCols,         &SpinSet::valueChanged,           this, &page_borders::slot_borderColsChanged);
    connect(borderColorPatch[3],&ClickableLabel::clicked,         this, &page_borders::slot_pickBorderColor);
    connect(borderColor[3],     &QLineEdit::textChanged,          this,   [this] { slot_borderColorChanged(borderColor[3]); });

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
    rectBoundaryLayout = new LayoutQRectF("Outer Boundary (screen units):");

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
{}

// refresh updates the page from the mosaic
void  page_borders::onRefresh()
{
    auto border = getMosaicBorder();
    if (!border)
    {
        blockPage(true);
        cropTypes->select(CROP_UNDEFINED);
        cropTypeStack->setCurrentIndex(CROP_UNDEFINED);
        borderTypes->select(BORDER_NONE);
        borderTypeStack->setCurrentIndex(BORDER_NONE);
        blockPage(false);
        return;
    }

    blockPage(true);
    refreshBorderType(border);
    refreshBorderCrop(border);
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
        borderWidth[0]->setValue(w);

        QColor qc = border->getColor();
        borderColor[0]->setText(qc.name(QColor::HexArgb));

        QVariant variant = qc;
        QString colcode  = variant.toString();
        borderColorPatch[0]->setStyleSheet("QLabel { background-color :"+colcode+" ; border: 1px solid black;}");
    }
    else if (btype == BORDER_TWO_COLOR)
    {
        qreal w = border->getWidth();
        borderWidth[1]->setValue(w);

        QColor qc = border->getColor();
        borderColor[1]->setText(qc.name(QColor::HexArgb));

        QVariant variant = qc;
        QString colcode  = variant.toString();
        borderColorPatch[1]->setStyleSheet("QLabel { background-color :"+colcode+" ; border: 1px solid black;}");

        auto bp2 = dynamic_cast<BorderTwoColor*>(border.get());
        if (bp2)
        {
            qc = bp2->getColor2();
            borderColor[2]->setText(qc.name(QColor::HexArgb));

            variant = qc;
            colcode  = variant.toString();
            borderColorPatch[2]->setStyleSheet("QLabel { background-color :"+colcode+" ; border: 1px solid black;}");
        }
    }
    else if (btype == BORDER_BLOCKS)
    {
        BorderBlocks * bp3 = dynamic_cast<BorderBlocks*>(border.get());
        if (bp3)
        {
            QColor  qc;
            qreal   width;
            int     rows;
            int     cols;
            bp3->get(qc, width, rows, cols);

            borderRows->setValue(rows);
            borderCols->setValue(cols);
            borderWidth[2]->setValue(width);

            borderColor[3]->setText(qc.name(QColor::HexArgb));

            QVariant variant = qc;
            QString colcode  = variant.toString();
            borderColorPatch[3]->setStyleSheet("QLabel { background-color :"+colcode+" ; border: 1px solid black;}");
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

    // real crops are in model units
    // howeaver border crops are in screen units
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

   CropPtr mosCrop = mosaic->getCrop();
    if (!mosCrop)
    {
        QMessageBox box(panel);
        box.setIcon(QMessageBox::Warning);
        box.setText("Fetch failed: No crop");
        box.setStandardButtons(QMessageBox::Ok);
        box.exec();
        return;
    }

    // convert crop
    auto newCrop = make_shared<Crop>();
    switch (mosCrop->getCropType())
    {
    case CROP_RECTANGLE:
    {
        auto rect = brview->worldToScreen(mosCrop->getRect());
        newCrop->setRect(rect);
    }   break;

    case CROP_CIRCLE:
    {
        Circle c = mosCrop->getCircle();
        c = brview->worldToScreen(c);
        newCrop->setCircle(c);
    }   break;

    case CROP_POLYGON:
    {
        auto poly = brview->worldToScreen(mosCrop->getPolygon());
        newCrop->setPolygon(poly);
    }   break;

    case CROP_UNDEFINED:
        break;
    }

    // new border
    auto border = make_shared<BorderPlain>(newCrop,7,QColor(Qt::red));     // HACK
    border->construct();
    mosaic->setBorder(border);

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
            qreal width  = borderWidth[0]->value();
            QColor color(borderColor[0]->text());
            QRectF rect  = rectBoundaryLayout->get();
            auto bp = make_shared<BorderPlain>(rect,width,color);
            bp->construct();
            mosaic->setBorder(bp);
        }
            break;

        case CROP_CIRCLE:
        {
            QColor color(borderColor[0]->text());
            qreal width  = borderWidth[0]->value();
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
        qreal width  = borderWidth[1]->value();
        QRectF rect  = rectBoundaryLayout->get();
        QColor color1(borderColor[1]->text());
        QColor color2(borderColor[2]->text());

        auto bp = make_shared<BorderTwoColor>(rect,color1,color2,width);
        bp->construct();
        mosaic->setBorder(bp);
    }
        break;

    case BORDER_BLOCKS:
    {
        qreal width  = borderWidth[2]->value();
        QRectF rect  = rectBoundaryLayout->get();
        QColor color1(borderColor[3]->text());
        int rows = borderRows->value();
        int cols = borderCols->value();
        auto bp = make_shared<BorderBlocks>(rect,color1,width,rows,cols);
        bp->construct();
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

            QSize sz = view->size();
            QRectF rect(QPointF(0,0),sz);
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
    mosaicMaker->getMosaic()->setBorder(bp);    // triggers rebuild
    emit sig_refreshView();
    view->update();
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
        mosaicMaker->getMosaic()->setBorder(bp);    // triggers rebuild
        emit sig_refreshView();
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
        emit sig_refreshView();
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
    emit sig_refreshView();
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
    emit sig_refreshView();
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
        borderColor[0]->setText(color.name(QColor::HexArgb));

        QVariant variant = color;
        QString colcode  = variant.toString();
        borderColorPatch[0]->setStyleSheet("QLabel { background-color :"+colcode+" ; border: 1px solid black;}");
        bp->setColor(color);
        emit sig_refreshView();
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
        borderColor[1]->setText(color.name(QColor::HexArgb));

        QVariant variant = color;
        QString colcode  = variant.toString();
        borderColorPatch[1]->setStyleSheet("QLabel { background-color :"+colcode+" ; border: 1px solid black;}");
        bp->setColor2(color);
        emit sig_refreshView();
        view->update();
    }
}


void page_borders::slot_rectBoundaryChanged()
{
    if (pageBlocked()) return;

    qDebug() << "page_borders::rectBoundryChanged";
    BorderPtr bp = getMosaicBorder();
    if (!bp)
        return;

    QRectF rect = rectBoundaryLayout->get();
    bp->setRect(rect);
    bp->construct();
    view->update();
}

void page_borders::slot_centreChanged()
{
    if (pageBlocked()) return;

    BorderPtr bp = getMosaicBorder();
    if (!bp)
        return;

    auto c = bp->getCircle();
    QPointF p = centre->get();
    c.setCenter(p);

    bp->construct();
    view->update();
}

void page_borders::slot_radiusChanged(qreal r)
{
    if (pageBlocked()) return;

    BorderPtr bp = getMosaicBorder();
    if (!bp)
        return;

    auto c = bp->getCircle();
    c.setRadius(r);

    bp->construct();
    view->update();
}
