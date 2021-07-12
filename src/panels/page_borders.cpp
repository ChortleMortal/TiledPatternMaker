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

#include "panels/page_borders.h"
#include "base/border.h"
#include "base/mosaic.h"
#include "base/tiledpatternmaker.h"
#include "base/utilities.h"
#include "designs/design.h"
#include "designs/design_maker.h"
#include "geometry/transform.h"
#include "makers/decoration_maker/decoration_maker.h"
#include "makers/motif_maker/motif_maker.h"
#include "makers/tiling_maker/tiling_maker.h"
#include "makers/map_editor/map_mouseactions.h"
#include "panels/panel.h"
#include "panels/page_map_editor.h"
#include "tapp/prototype.h"
#include "tile/backgroundimage.h"
#include "tile/tiling.h"
#include "base/shared.h"
#include "panels/layout_sliderset.h"
#include "panels/layout_qrectf.h"
#include "viewers/viewcontrol.h"
#include "viewers/view.h"
#include "geometry/crop.h"

using std::string;
using std::make_shared;

page_borders::page_borders(ControlPanel * apanel)  : panel_page(apanel,"Borders")
{
    // border selection
    QLabel * label = new QLabel("Border");

    borderType.addItem("None",              BORDER_NONE);
    borderType.addItem("Solid",             BORDER_PLAIN);
    borderType.addItem("Two color",         BORDER_TWO_COLOR);
    borderType.addItem("Block",             BORDER_BLOCKS);
    borderType.addItem("Integrated Crop",   BORDER_INTEGRATED);

    QHBoxLayout * hbox = new QHBoxLayout;
    hbox->addWidget(label);
    hbox->addWidget(&borderType);
    hbox->addStretch();

    // stacked layout
    stack = new QStackedLayout;

    nullBorderWidget  = new QWidget;
    outerBorderWidget = createOuterBorderWidget();
    innerBorderWidget = createInnerBorderWidget();
    stack->addWidget(nullBorderWidget);
    stack->addWidget(outerBorderWidget);
    stack->addWidget(innerBorderWidget);

    // border box
    borderbox = new QGroupBox("Design Border Settings");
    borderbox->setMinimumWidth(400);
    borderbox->setLayout(stack);

    vbox->addLayout(hbox);
    vbox->addWidget(borderbox);
    adjustSize();

    connect(&borderType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &page_borders::borderChanged);

    connect(theApp,   &TiledPatternMaker::sig_tilingLoaded,      this,   &page_borders::slot_viewUpated);
    connect(theApp,   &TiledPatternMaker::sig_mosaicLoaded,      this,   &page_borders::slot_viewUpated);
    connect(theApp,   &TiledPatternMaker::sig_loadedDesign,      this,   &page_borders::slot_viewUpated);
    connect(vcontrol, &ViewControl::sig_viewUpdated,             this,   &page_borders::slot_viewUpated);
}

QWidget * page_borders::createOuterBorderWidget()
{
    borderWidth = new SpinSet("Width",10,1,999);

    borderColorLabel[0] = new QLabel("Border Color");
    borderColor[0] = new QLineEdit();
    borderColorPatch[0] = new ClickableLabel;
    borderColorPatch[0]->setMinimumWidth(75);

    borderColorLabel[1] = new QLabel("Border Color 2");
    borderColor[1] = new QLineEdit();
    borderColorPatch[1] = new ClickableLabel;
    borderColorPatch[1]->setMinimumWidth(75);

    borderRows = new SpinSet("Rows",5,0,99);
    borderCols = new SpinSet("Cols",5,0,99);

    outerBoundaryLayout = new LayoutQRectF("Outer Boundary (screen units):");

    QGridLayout * bgrid  = new QGridLayout();

    int row = 0;
    bgrid->addLayout(borderWidth,row,0,1,2);
    row++;
    bgrid->addWidget(borderColorLabel[0],row,0);
    bgrid->addWidget(borderColor[0],row,1);
    bgrid->addWidget(borderColorPatch[0],row,2);
    row++;
    bgrid->addWidget(borderColorLabel[1],row,0);
    bgrid->addWidget(borderColor[1],row,1);
    bgrid->addWidget(borderColorPatch[1],row,2);
    row++;
    bgrid->addLayout(borderRows,row,0,1,2);
    row++;
    bgrid->addLayout(borderCols,row,0,1,2);
    row++;
    bgrid->addLayout(outerBoundaryLayout,row,0,1,2);

    connect(borderColorPatch[0],&ClickableLabel::clicked,         this, &page_borders::pickBorderColor);
    connect(borderColorPatch[1],&ClickableLabel::clicked,         this, &page_borders::pickBorderColor2);
    connect(borderWidth,        &SpinSet::valueChanged,           this, &page_borders::borderWidthChanged);
    connect(borderRows,         &SpinSet::valueChanged,           this, &page_borders::borderRowsChanged);
    connect(borderCols,         &SpinSet::valueChanged,           this, &page_borders::borderColsChanged);
    connect(outerBoundaryLayout,&LayoutQRectF::boundaryChanged,   this, &page_borders::slot_outerBoundryChanged);

    QWidget * w = new QWidget;
    w->setLayout(bgrid);
    return w;
}

QWidget * page_borders::createInnerBorderWidget()
{
    QPushButton * pbDefine = new QPushButton("Define Border");
    QPushButton * pbEdit   = new QPushButton("Edit Border");

    innerBoundaryLayout = new LayoutQRectF("Inner Boundary (model units):");

    QHBoxLayout * hbox1 = new QHBoxLayout;
    hbox1->addWidget(pbDefine);
    hbox1->addWidget(pbEdit);
    hbox1->addStretch();

    QHBoxLayout * hbox2 = new QHBoxLayout;
    hbox2->addLayout(innerBoundaryLayout);
    hbox2->addStretch();

    // put it together
    QVBoxLayout * vb = new QVBoxLayout;
    vb->addLayout(hbox1);
    vb->addLayout(hbox2);
    vb->addLayout(createAspectLayout());

    connect(pbDefine,            &QPushButton::clicked,          this,   &page_borders::slot_defineBorder);
    connect(pbEdit,              &QPushButton::clicked,          this,   &page_borders::slot_editBorder);
    connect(innerBoundaryLayout, &LayoutQRectF::boundaryChanged, this,   &page_borders::slot_innerBoundryChanged);

    QWidget * w = new QWidget;
    w->setLayout(vb);
    return w;
}

QHBoxLayout * page_borders::createAspectLayout()
{
    QRadioButton *  rad_unc   = new QRadioButton("Unconstrained");
    QRadioButton *  rad_two   = new QRadioButton(QString("1 : %1 2").arg(MathSymbolSquareRoot));
    QRadioButton *  rad_three = new QRadioButton(QString("1 : %1 3").arg(MathSymbolSquareRoot));
    QRadioButton *  rad_four  = new QRadioButton(QString("1 : %1 4").arg(MathSymbolSquareRoot));
    QRadioButton *  rad_five  = new QRadioButton(QString("1 : %1 5").arg(MathSymbolSquareRoot));
    QRadioButton *  rad_six   = new QRadioButton(QString("1 : %1 6").arg(MathSymbolSquareRoot));
    QRadioButton *  rad_seven = new QRadioButton(QString("1 : %1 7").arg(MathSymbolSquareRoot));
    chkVert   = new QCheckBox("Vertical");

    QHBoxLayout * hbox = new QHBoxLayout();
    hbox->addWidget(rad_unc);
    hbox->addWidget(rad_two);
    hbox->addWidget(rad_three);
    hbox->addWidget(rad_four);
    hbox->addWidget(rad_five);
    hbox->addWidget(rad_six);
    hbox->addWidget(rad_seven);
    hbox->addWidget(chkVert);
    hbox->addStretch();

    aspects.addButton(rad_unc,  ASPECT_UNCONSTRAINED);
    aspects.addButton(rad_two,  ASPECT_SQRT_2);
    aspects.addButton(rad_three,ASPECT_SQRT_3);
    aspects.addButton(rad_four, ASPECT_SQRT_4);
    aspects.addButton(rad_five, ASPECT_SQRT_5);
    aspects.addButton(rad_six,  ASPECT_SQRT_6);
    aspects.addButton(rad_seven,ASPECT_SQRT_7);

    connect (&aspects, &QButtonGroup::idClicked, this, &page_borders::slot_cropAspect);
    connect(chkVert,   &QCheckBox::toggled,      this, &page_borders::slot_verticalAspect);

    return hbox;
}

void  page_borders::refreshPage()
{
    // does not call display
}

void page_borders::onEnter()
{
    display();
}

void page_borders::slot_viewUpated()
{
    if (panel->isVisiblePage(this))
    {
        display();
    }
}

void page_borders::display()
{
    blockPage(true);

    BorderPtr bp = getMosaicOrDesignBorder();
    if (bp)
    {
        int index = borderType.findData(bp->getType());
        borderType.blockSignals(true);
        borderType.setCurrentIndex(index);
        borderType.blockSignals(false);

        switch(bp->getType())
        {
        case BORDER_NONE:
            stack->setCurrentIndex(0);
            break;

        case BORDER_INTEGRATED:
            stack->setCurrentIndex(2);
            break;

        case BORDER_BLOCKS:
        case BORDER_PLAIN:
        case BORDER_TWO_COLOR:
        default:
            stack->setCurrentIndex(1);
            break;
        }

        displayBorder(bp);
    }
    else
    {
        borderType.blockSignals(true);
        borderType.setCurrentIndex(0);
        borderType.blockSignals(false);
        stack->setCurrentIndex(0);
    }

    blockPage(false);
}

BorderPtr page_borders::getMosaicOrDesignBorder()
{
    if (config->getViewerType() == VIEW_DESIGN)
    {
        DesignMaker * designMaker = DesignMaker::getInstance();
        QVector<DesignPtr> & designs = designMaker->getDesigns();
        if (designs.count())
        {
            DesignPtr dp = designs.first();
            return dp->border;
        }
    }
    else
    {
        MosaicPtr m = decorationMaker->getMosaic();
        if (m)
        {
            return m->getBorder();
        }
    }
    BorderPtr bp;
    return bp;
}

void page_borders::displayBorder(BorderPtr bp)
{

    if (bp->getType() == BORDER_INTEGRATED)
    {
        InnerBorder * ib = dynamic_cast<InnerBorder*>(bp.get());
        Q_ASSERT(ib);

        CropPtr crop = ib->getInnerBoundary();
        aspects.button(crop->getAspect())->setChecked(true);
        chkVert->setChecked(ib->getInnerBoundary()->getAspectVertical());
        innerBoundaryLayout->set(crop->getRect());
    }
    else
    {
        OuterBorder * ob = dynamic_cast<OuterBorder*>(bp.get());
        Q_ASSERT(ob);

        outerBoundaryLayout->set(ob->getOuterBoundary());

        borderWidth->show();
        qreal w = ob->getWidth();
        borderWidth->setValue(w);

        borderColorLabel[0]->show();

        QColor qc = ob->getColor();
        borderColor[0]->setText(qc.name(QColor::HexArgb));
        borderColor[0]->show();

        QVariant variant = qc;
        QString colcode  = variant.toString();
        borderColorPatch[0]->setStyleSheet("QLabel { background-color :"+colcode+" ; border: 1px solid black;}");
        borderColorPatch[0]->show();

        if (bp->getType() == BORDER_TWO_COLOR)
        {
            borderColorLabel[1]->show();

            BorderTwoColor * bp2 = dynamic_cast<BorderTwoColor*>(bp.get());
            Q_ASSERT(bp2);
            qc = bp2->getColor2();
            borderColor[1]->setText(qc.name(QColor::HexArgb));
            borderColor[1]->show();

            variant = qc;
            colcode  = variant.toString();
            borderColorPatch[1]->setStyleSheet("QLabel { background-color :"+colcode+" ; border: 1px solid black;}");
            borderColorPatch[1]->show();

            borderRows->hide();
            borderCols->hide();
        }
        else if (bp->getType() == BORDER_BLOCKS)
        {
            borderColorLabel[1]->hide();
            borderColor[1]->hide();
            borderColorPatch[1]->hide();

            borderRows->show();
            borderCols->show();

            BorderBlocks * bp3 = dynamic_cast<BorderBlocks*>(bp.get());

            QColor c;
            qreal  d;
            int  rows;
            int cols;
            bp3->get(c, d, rows, cols);
            borderRows->setValue(rows);
            borderCols->setValue(cols);
        }
        else
        {
            Q_ASSERT(bp->getType() == BORDER_PLAIN);
            borderColorLabel[1]->hide();
            borderColor[1]->hide();
            borderColorPatch[1]->hide();

            borderRows->hide();
            borderCols->hide();
        }
    }
}

void page_borders::borderWidthChanged(int width)
{
    BorderPtr bp = getMosaicOrDesignBorder();
    if (!bp) return;

    OuterBorder * ob = dynamic_cast<OuterBorder*>(bp.get());
    if (ob)
    {
        ob->setWidth(width);
        decorationMaker->getMosaic()->setBorder(bp);    // triggers rebuild
        emit sig_refreshView();
        view->update();
    }
}

void page_borders::borderRowsChanged(int rows)
{
    BorderPtr bp = getMosaicOrDesignBorder();
    if (!bp) return;

    BorderBlocks * bp3 = dynamic_cast<BorderBlocks*>(bp.get());
    if (bp3)
    {
        bp3->setRows(rows);
        decorationMaker->getMosaic()->setBorder(bp);    // triggers rebuild
        emit sig_refreshView();
        view->update();
    }
}

void page_borders::borderColsChanged(int cols)
{
    BorderPtr bp = getMosaicOrDesignBorder();
    if (!bp) return;

    BorderBlocks * bp3 = dynamic_cast<BorderBlocks*>(bp.get());
    if (bp3)
    {
        bp3->setCols(cols);
        emit sig_refreshView();
        view->update();
    }
}

void page_borders::pickBorderColor()
{
    BorderPtr bp = getMosaicOrDesignBorder();
    if (!bp) return;

    OuterBorder * ob = dynamic_cast<OuterBorder*>(bp.get());
    if (ob)
    {
        QColor color = ob->getColor();

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
            ob->setColor(color);
            emit sig_refreshView();
        }
    }
}

void page_borders::pickBorderColor2()
{
    BorderPtr bp = getMosaicOrDesignBorder();
    if (!bp) return;

    BorderTwoColor * bp2 = dynamic_cast<BorderTwoColor*>(bp.get());
    if (!bp2) return;

    QColor color = bp2->getColor2();

    AQColorDialog dlg(color,this);
    dlg.setCurrentColor(color);
    int rv = dlg.exec();
    if (rv != QDialog::Accepted) return;

    color = dlg.selectedColor();
    if (color.isValid())
    {
        borderColor[1]->setText(color.name(QColor::HexArgb));
        BorderPtr bp = getMosaicOrDesignBorder();

        QVariant variant = color;
        QString colcode  = variant.toString();
        borderColorPatch[1]->setStyleSheet("QLabel { background-color :"+colcode+" ; border: 1px solid black;}");
        bp2->setColor2(color);
        emit sig_refreshView();
        view->update();
    }
}

void page_borders::borderChanged(int row)
{
    Q_UNUSED(row);

    MosaicPtr mosaic = decorationMaker->getMosaic();
    if (!mosaic)
    {
        borderType.blockSignals(true);
        borderType.setCurrentIndex(0);
        borderType.blockSignals(false);
        return;
    }

    QSize sz = view->size();    // FIXME - size

    eBorderType type = static_cast<eBorderType>(borderType.currentData().toInt());

    BorderPtr bp;
    switch(type)
    {
    case BORDER_NONE:
        break;
    case BORDER_PLAIN:
        bp = make_shared<BorderPlain>(sz,20,Qt::blue);
        bp->construct();
        break;
    case BORDER_TWO_COLOR:
        bp = make_shared<BorderTwoColor>(sz,QColor(0xa2,0x79,0x67),QColor(TileWhite),20);
        bp->construct();
        break;
    case BORDER_BLOCKS:
        bp = make_shared<BorderBlocks>(sz,QColor(0xa2,0x79,0x67),150,11,6);
        bp->construct();
        break;
    case BORDER_INTEGRATED:
        bp = make_shared<InnerBorder>();
        break;
    }
    mosaic->setBorder(bp);
    display();
    emit sig_refreshView();
    view->update();
}

void page_borders::slot_innerBoundryChanged()
{
    qDebug() << "page_borders::slot_innerBoundryChanged";
    BorderPtr bp = getMosaicOrDesignBorder();
    if (!bp)
        return;

    InnerBorder * ib = dynamic_cast<InnerBorder*>(bp.get());
    if (ib)
    {
        CropPtr crop = ib->getInnerBoundary();
        if (crop && crop->getState() != CROP_NONE)
        {
            QRectF rect = innerBoundaryLayout->get();
            crop->setRect(rect,crop->getState());           // state unchanaged
            decorationMaker->getMosaic()->setBorder(bp);    // triggers rebuild
            emit sig_refreshView();
        }
    }
}

void page_borders::slot_outerBoundryChanged()
{
    qDebug() << "page_borders::slot_outerBoundryChanged";
    BorderPtr bp = getMosaicOrDesignBorder();
    if (!bp)
        return;

    OuterBorder * ob = dynamic_cast<OuterBorder*>(bp.get());
    if (ob)
    {
        ob->setOuterBoundary(outerBoundaryLayout->get());
        decorationMaker->getMosaic()->setBorder(bp);    // triggers rebuild
        emit sig_refreshView();
        view->update();
    }
}

void page_borders::slot_defineBorder()
{
    BorderPtr bp = getMosaicOrDesignBorder();
    if (bp)
    {
        InnerBorder * ib = dynamic_cast<InnerBorder*>(bp.get());
        if (ib)
        {
            if (ib->getInnerBoundary()->getState() != CROP_NONE)
            {
                // already exists so need to edit
                slot_editBorder();
                return;
            }
        }
    }

    // Note: there may not be border yet
    panel->setCurrentPage("Map Editor");

    panel_page      * pp  = panel->getCurrentPage();
    page_map_editor * pme = dynamic_cast<page_map_editor *>(pp);
    if (pme)
    {
        pme->slot_mapEdMode_pressed(MAPED_MODE_PROTO,true);
        pme->slot_setModes(MAPED_MOUSE_CREATE_BORDER,true);
    }

    QString msg = QString("To create the boundary rectangle: left click on boundary corner and drag.\n")
                + QString("You can now edit the rectangle by clicking and dragging.\n")
                + QString("Then:\n")
                + QString("    Press 'Apply Border' to embed the border in the map\n")
                + QString("    Press 'Apply Mask' to remove everything outside of the border");
    QMessageBox box(panel);
    box.setIcon(QMessageBox::Information);
    box.setText(msg);
    box.setStandardButtons(QMessageBox::Ok);
    box.exec();
}

void page_borders::slot_editBorder()
{
    BorderPtr bp = getMosaicOrDesignBorder();
    if (!bp)
        return;

    InnerBorder * ib = dynamic_cast<InnerBorder*>(bp.get());
    if (!ib)
        return;

    CropPtr crop = ib->getInnerBoundary();
    if (crop->getState() == CROP_NONE)
    {
        QMessageBox box(panel);
        box.setIcon(QMessageBox::Warning);
        box.setText("There is no border to edit.\nPlease create a border first.");
        box.setStandardButtons(QMessageBox::Ok);
        box.exec();
        return;
    }

    panel->setCurrentPage("Map Editor");

    panel_page      * pp  = panel->getCurrentPage();
    page_map_editor * pme = dynamic_cast<page_map_editor *>(pp);
    if (pme)
    {
        pme->slot_mapEdMode_pressed(MAPED_MODE_PROTO,true);
        pme->slot_setModes(MAPED_MOUSE_EDIT_BORDER,true);
    }
}


void page_borders::slot_cropAspect(int id)
{
    BorderPtr bp = getMosaicOrDesignBorder();
    if (!bp)
        return;

    InnerBorder * ib = dynamic_cast<InnerBorder*>(bp.get());
    if (ib)
    {
        qDebug() <<  "aspect ratio =" << id;
        ib->getInnerBoundary()->setAspect(eAspectRatio(id));
        slot_innerBoundryChanged();
    }
}

void page_borders::slot_verticalAspect(bool checked)
{
    BorderPtr bp = getMosaicOrDesignBorder();
    if (!bp)
        return;

    InnerBorder * ib = dynamic_cast<InnerBorder*>(bp.get());
    if (ib)
    {
        ib->getInnerBoundary()->setAspectVertical(checked);
        slot_innerBoundryChanged();
    }
}
