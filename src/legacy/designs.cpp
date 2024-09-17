#include "legacy/legacy_border.h"
#include "legacy/design_maker.h"
#include "legacy/designs.h"
#include "legacy/patterns.h"
#include "model/motifs/tile_color_defs.h"
#include "model/settings/configuration.h"
#include "model/settings/canvas_settings.h"
#include <QDebug>
#include <QtMath>

using std::make_shared;

/////////////////////////////////////////////////////////////////////////////
//
// The designs
//
/////////////////////////////////////////////////////////////////////////////

Design5::Design5(eDesign design, QString atitle) : Design(design,atitle) {}
Design6::Design6(eDesign design, QString atitle) : Design(design,atitle) {}
Design7::Design7(eDesign design, QString atitle) : Design(design,atitle) {}
Design8::Design8(eDesign design, QString atitle) : Design(design,atitle) {}
Design9::Design9(eDesign design, QString atitle) : Design(design,atitle) {}
DesignHuPacked::DesignHuPacked(eDesign design, QString atitle) : Design(design,atitle) {}
DesignHuInsert::DesignHuInsert(eDesign design, QString atitle) : Design(design,atitle) {}
Design11::Design11(eDesign design, QString atitle) : Design(design,atitle) {}
Design12::Design12(eDesign design, QString atitle) : Design(design,atitle) {}
Design13::Design13(eDesign design, QString atitle) : Design(design,atitle) {}
Design14::Design14(eDesign design, QString atitle) : Design(design,atitle) {}
Design16::Design16(eDesign design, QString atitle) : Design(design,atitle) {}
Design17::Design17(eDesign design, QString atitle) : Design(design,atitle) {}
Design18::Design18(eDesign design, QString atitle) : Design(design,atitle) {}
Design19::Design19(eDesign design, QString atitle) : Design(design,atitle) {}
DesignKumiko1::DesignKumiko1(eDesign design, QString atitle) : Design(design,atitle) {}
DesignKumiko2::DesignKumiko2(eDesign design, QString atitle) : Design(design,atitle) {}

bool Design5::build()
{
    QSize size(1800,1100);
    settings.setViewSize(size);
    settings.setBackgroundColor(QColor(TileBlue));

    LegacyBorderPtr bp = make_shared<LegacyBorderTwoColor>(size,QColor(TileGreen),QColor(TileWhite),20.0);
    bp->construct();

    border = bp;

    qreal d = 150.0;                // diameter
    settings.setStartTile(QPointF(200.0,150.0));
    xSeparation = 260.0;
    ySeparation = 453.0;

    QBrush brush1(TileBrown);
    QBrush brush2(TileGreen);

    rows = 5;
    cols = 7;
    bool toggle = false;

    PatternPtr pat;

    // repeat
    for (int row = 0; row < rows; row++)
    {
        for (int col = 0; col < cols; col++)
        {
            if (!toggle)
                pat = make_shared<Pattern5>(d,brush1);
            else
                pat = make_shared<Pattern5>(d,brush2);
            toggle=!toggle;
            pat->setTilePosition(row,col);
            pat->build();
            patterns.push_back(pat);
        }
    }
    return true;
}

bool Design6::build()
{
    QSize size(1800,1100);
    settings.setViewSize(size);
    settings.setBackgroundColor((QColor(TileWhite)));

    LegacyBorderPtr bp = make_shared<LegacyBorderTwoColor>(size,QColor(TileGreen),QColor(TileWhite),20.0);
    bp->construct();
    border = bp;

    qreal d = 150.0;                // diameter
    settings.setStartTile(QPointF(170.0,85.0));
    xSeparation = 300.0;
    ySeparation = 260.0;

    QBrush brush1;
    brush1.setColor(TileBrown2);
    brush1.setStyle(Qt::SolidPattern);

    QBrush brush2;
    brush2.setColor(TileGreen);
    brush2.setStyle(Qt::SolidPattern);

    rows = 5;
    cols = 7;

    bool toggle = false;
    PatternPtr pat;

    // repeat
    QPointF pt = settings.getStartTile();
    for (int row = 0; row < rows; row++)
    {
        for (int col = 0; col < cols; col++)
        {
            if (!toggle)
                pat = make_shared<Pattern6>(d,brush1);
            else
                pat = make_shared<Pattern6>(d,brush2);
            toggle=!toggle;
            pat->setTilePosition(row,col);
            patterns.push_back(pat);
            pat->setLoc(QPointF(pt.x() + (xSeparation * col), pt.y() + (ySeparation * row)));
            pat->build();
        }
    }
    return true;
}

bool Design7::build()
{
    cols           = 11;
    rows           = 6;
    qreal diameter = 150.0;
    qreal side     = diameter * qTan(M_PI/8.0);
    qreal piece    = sqrt(side*side*0.5);

    QSize size((diameter*cols),(diameter*rows)+(piece*2.0));
    qDebug() << "pat 7 size:" << size;
    settings.setViewSize(size);
    settings.setBackgroundColor((QColor(TileGreen)));

    LegacyBorderPtr bp = make_shared<LegacyBorderBlocks>(size,QColor(TileWhite),diameter,0,cols);
    bp->construct();
    border = bp;

    settings.setStartTile(QPointF(diameter/2.0,diameter/2.0 + piece));
    qDebug() << "start tile:" << settings.getStartTile();

    xSeparation = diameter;
    ySeparation = diameter;

    QBrush brush1;
    brush1.setColor(TileWhite);
    brush1.setStyle(Qt::SolidPattern);

    PatternPtr  pat;
    QPointF pt = settings.getStartTile();
    for (int row = 0; row < rows; row++)
    {
        for (int col = 0; col < cols; col++)
        {
            pat = make_shared<Pattern7>(diameter,brush1);
            pat->setTilePosition(row,col);
            patterns.push_back(pat);
            pat->setLoc(QPointF(pt.x() + (xSeparation * col), pt.y() + (ySeparation * row)));
            pat->build();
        }
    }
    return true;
}

// The Hu Symbol
bool Design8::build()
{
    // canvas
    QColor canvasColor = QColor(TileBlack);
    settings.setBackgroundColor((canvasColor));
    settings.setViewSize(QSize(1800,1100));

    // patern
    int gridWidth = 41;
    config->gridScreenSpacing = gridWidth;
    qreal diameter = 20.0 * qreal(gridWidth);

    // positioning
    //startTile = sceneRect().center();
    settings.setStartTile(QPointF(901.0,533));
    qDebug() << "start tile:" << settings.getStartTile();
    xSeparation = diameter;
    ySeparation = diameter;

    // pens
    QPen gridPen;
    gridPen.setWidthF((qreal)gridWidth);
    gridPen.setColor(QColor(TileGold));

    QPen innerPen;
    innerPen.setWidthF((qreal)gridWidth - 4.0);
    innerPen.setColor(QColor(Qt::red));

    PatternPtr pat = make_shared<PatternHuSymbol>(gridWidth,gridPen,innerPen, canvasColor,diameter,QBrush(Qt::NoBrush));
    patterns.push_back(pat);
    pat->setTilePosition(0,0);
    pat->setLoc(settings.getStartTile());
    pat->build();

#if 0
    // test overlay
    gridPen.setWidthF(1.0);
    gridPen.setColor(Qt::blue);

    pat = new Pattern8(gridWidth,gridPen,diameter,QBrush(Qt::NoBrush),shared_from_this());
    pat->build();
    pat->setTilePosition(0,0);
    tiles.push_back(pat);
    pat->setLoc(startTile);
    pat->setZValue(2.0);
#endif

    doSteps();

    return true;
}

// A packing of Hu symbols
bool Design9::build()
{
    QColor canvasColor = QColor(TileBlack);
    settings.setBackgroundColor((canvasColor));
    settings.setViewSize(QSize(1776,1100));

    // patern
    int gridWidth = 21;
    config->gridScreenSpacing = gridWidth;
    qreal diameter = 20.0 * qreal(gridWidth);

    // positioning
    //startTile = sceneRect().center();
    settings.setStartTile(QPointF(222,222));
    qDebug() << "start tile:" << settings.getStartTile();
    xSeparation = 444;
    ySeparation = 444;

    // pens
    QPen gridPen;
    gridPen.setWidthF((qreal)gridWidth);
    gridPen.setColor(QColor(TileGold));

    QPen innerPen;
    innerPen.setWidthF((qreal)gridWidth - 4.0);
    innerPen.setColor(QColor(Qt::red));

    rows = 4;
    cols = 6;

    // repeat
    QPointF pt = settings.getStartTile();
    for (int row = 0; row < rows; row++)
    {
        for (int col=0; col < cols; col++)
        {
            PatternPtr pat = make_shared<PatternHuSymbol>(gridWidth,gridPen,innerPen, canvasColor,diameter,QBrush(Qt::NoBrush));
            pat->setTilePosition(row,col);
            patterns.push_back(pat);
            pat->setLoc(QPointF(pt.x() + (xSeparation * col), pt.y() + (ySeparation * row)));
            pat->build();
        }
    }
    doSteps();
    return true;
}

bool DesignHuPacked::build()
{
    QColor canvasColor = QColor(TileBlack);
    settings.setBackgroundColor((canvasColor));
    settings.setViewSize(QSize(1794,1100));

    // patern
    int igridWidth = 21;
    config->gridScreenSpacing = igridWidth;

    qreal gridWidth = 21.0;
    qreal diameter  = 20.0 * gridWidth;

    // positioning
    //startTile = sceneRect().center();
    settings.setStartTile(QPointF(224,224));
    qDebug() << "start tile:" << settings.getStartTile();
    xSeparation = 448;
    ySeparation = 448;

    // pens
    QPen gridPen;
    gridPen.setWidthF(gridWidth);
    gridPen.setColor(QColor(TileGold));

    QPen innerPen;
    innerPen.setWidthF(gridWidth - 4.0);
    innerPen.setColor(QColor(Qt::red));

    rows = 4;
    cols = 6;

    // repeat
    DesignMaker * designMaker = Sys::designMaker;
    designMaker->setStep(0);
    for (int row = 0; row < rows; row++)
    {
        for (int col=0; col < cols; col++)
        {
            gridPen.setWidthF(gridWidth);
            innerPen.setWidthF(gridWidth - 4.0);
            PatternPtr pat = make_shared<PatternHuSymbol>(gridWidth,gridPen,innerPen, canvasColor,diameter,QBrush(Qt::NoBrush));
            pat->build();
            pat->setTilePosition(row,col);
            patterns.push_back(pat);
            //pat->setLoc(startTile.x() + (xSeparation * col),startTile.y() + (ySeparation * row));
        }
    }
    doSteps();

    DesignPtr dp = designMaker->getDesign(DESIGN_HU_INSERT);
    dp->build();
    dp->repeat();
    designMaker->addDesign(dp);

    return true;
}


bool DesignHuInsert::build()
{
    QColor canvasColor = QColor(TileBlack);
    settings.setBackgroundColor((canvasColor));
    settings.setViewSize(QSize(1794,1100));

    // patern
    int igridWidth = 21;
    config->gridScreenSpacing = igridWidth;

    qreal gridWidth = 21.0;
    qreal diameter  = 20.0 * gridWidth;

    // positioning
    //startTile = sceneRect().center();
    settings.setStartTile(QPointF(224,224));
    qDebug() << "start tile:" << settings.getStartTile();
    xSeparation = 448;
    ySeparation = 448;

    // pens
    QPen gridPen;
    gridPen.setWidthF(gridWidth);
    gridPen.setColor(QColor(TileGold));

    QPen innerPen;
    innerPen.setWidthF(gridWidth - 4.0);
    innerPen.setColor(QColor(Qt::blue));

    rows = 4;
    cols = 6;

    for (int row = 0; row < rows; row++)
    {
        for (int col=0; col < cols; col++)
        {
            // smaller repeat, offset
            gridWidth = 11.0;
            gridPen.setWidthF(gridWidth);
            innerPen.setWidthF(gridWidth - 4.0);
            PatternPtr pat = make_shared<PatternHuSymbol>(gridWidth,gridPen,innerPen, canvasColor,diameter/2.0,QBrush(Qt::NoBrush));
            pat->build();
            pat->setTilePosition(row,col);
            patterns.push_back(pat);
            //pat->setLoc(startTile.x() + (xSeparation * col) - 224,
            //            startTile.y() + (ySeparation * row) - 224);
        }
    }
    doSteps(19);    // no cirumscribing octagon
    return true;
}

bool Design11::build()
{
    QColor canvasColor = QColor(TileBlack);
    settings.setBackgroundColor((canvasColor));
    settings.setViewSize(QSize(1800,1100));

    // patern
    int gridWidth = 41;
    config->gridScreenSpacing = gridWidth;
    qreal diameter = 20.0 * qreal(gridWidth);

    // positioning
    //startTile = sceneRect().center();
    //settings.setStartTile(QPointF(901.0,533));
    qDebug() << "start tile:" << settings.getStartTile();
    xSeparation = diameter;
    ySeparation = diameter;

    // pens
    QPen gridPen;
    gridPen.setWidthF((qreal)gridWidth);
    gridPen.setColor(QColor(TileGold));

    QPen innerPen;
    innerPen.setWidthF((qreal)gridWidth - 4.0);
    innerPen.setColor(QColor(Qt::red));

    PatternPtr pat = make_shared<PatternHuInterlace>(gridWidth,gridPen,innerPen, canvasColor,diameter,QBrush(Qt::NoBrush));
    pat->setTilePosition(0,0);
    pat->setLoc(QPointF(901,533));
    pat->build();
    patterns.push_back(pat);

#if 0
    // test overlay
    gridPen.setWidthF(1.0);
    gridPen.setColor(Qt::blue);

    pat = new Pattern8(gridWidth,gridPen,diameter,QBrush(Qt::NoBrush),this);
    pat->build();
    pat->setTilePosition(0,0);
    tiles.push_back(pat);
    pat->setLoc(startTile);
    pat->setZValue(2.0);
#endif
    doSteps();

    return true;
}

bool Design12::build()
{
    QSize size(1000,1100);
    settings.setViewSize(size);
    settings.setBackgroundColor((TileBlack));

    LegacyBorderPtr bp = make_shared<LegacyBorderTwoColor>(size,QColor(Qt::green),QColor(Qt::red),20.0);
    bp->construct();
    border = bp;

    qreal diameter = 400.0;
    settings.setStartTile(settings.getCenter());
    PatternPtr pat = make_shared<Pattern10>(diameter, QBrush(Qt::NoBrush));
    patterns.push_back(pat);
    pat->setTilePosition(0,0);
    pat->setLoc(settings.getStartTile());
    pat->build();
    return true;
}


// Alhambra
// This is hexagoal based. The variants are
// - Hex packing (separtion/overlap)
// - Hex orientation (0 or 90)
// - Arcs to points or arcs to midoints
// - Arcs CW/CCW
// - Coloration

bool Design13::build()
{
    qreal rotation   = 90.0;
    eDirection turn  = CW;
    
    settings.setViewSize(QSize(1800,1100));
    settings.setBackgroundColor((TileBlack));
    qreal diameter = 400.0;
    settings.setStartTile(settings.getCenter());

    settings.setStartTile(QPointF(0,75));

    if (rotation == 0.0)
    {
        xSeparation = 299;
        ySeparation = 260;
    }
    else
    {
        Q_ASSERT(rotation == 90.0);
        xSeparation = 346;
        ySeparation = 299;
        if (turn == CW)
        {
            xSeparation++;
            ySeparation++;
        }
    }

    if (config->repeatMode != REPEAT_SINGLE)
    {
        rows = 8;
        cols = 12;
    }
    else
    {
        rows = 1;
        cols = 1;
        settings.setStartTile(settings.getStartTile() + QPointF(200,200));
    }

    // create Patterns
    for (int row = 0; row < rows; row++)
    {
        for (int col=0; col < cols; col++)
        {
            PatternPtr pat = make_shared<Pattern11>(diameter,  QBrush(Qt::NoBrush), rotation, turn);
            pat->setTilePosition(row,col);
            pat->build();
            patterns.push_back(pat);
        }
    }
    return true;
}


// Alhambra
// This is hexagoal based. The variants are
// - Hex packing (separtion/overlap)
// - Hex orientation (0 or 90)
// - Arcs to points or arcs to midoints
// - Arcs CW/CCW
// - Coloration

bool Design14::build()
{
    qreal rotation   = 90.0;
    eDirection turn  = CW;
    
    settings.setViewSize(QSize(1800,1100));
    settings.setBackgroundColor((Qt::yellow));
    qreal diameter = 400.0 / 2.0;
    settings.setStartTile(settings.getCenter());

  //startTile = QPointF(0,75);  // for full-size (400)
    settings.setStartTile(QPointF(0,50));  // for half-size (200)
  //startTile = QPointF(115,172);

    if (rotation == 0.0)
    {
        xSeparation = 299 /2.0;
        ySeparation = 260 /2.0;
    }
    else
    {
        Q_ASSERT(rotation == 90.0);
        xSeparation = 346 /2.0;
        ySeparation = 299 /2.0;
        if (turn == CW)
        {
            //xSeparation = 172.5;    // emprical
            //ySeparation = 150.5;    // empirical
            //xSeparation++;
            //ySeparation++;
            xSeparation = 86.6025 * 2.0;
            ySeparation = 150.0;
        }
    }

    if (config->repeatMode != REPEAT_SINGLE)
    {
        rows = 8;
        cols = 12;
    }
    else
    {
        rows = 1;
        cols = 1;
        settings.setStartTile(settings.getStartTile() + QPointF(200,200));
    }

    // create Patterns
    for (int row = 0; row < rows; row++)
    {
        for (int col=0; col < cols; col++)
        {
            PatternPtr pat = make_shared<Pattern12>(diameter,  QBrush(Qt::NoBrush), rotation, turn, row, col);
            pat->setTilePosition(row,col);
            pat->build();
            patterns.push_back(pat);
        }
    }
    return true;
}

bool Design16::build()
{
    settings.setViewSize(QSize(1800,1100));
    settings.setBackgroundColor((QColor(TileBlack)));

    qreal diameter = 200.0;
    settings.setStartTile(settings.getCenter());

    xSeparation = 206;
    ySeparation = 206;
    settings.setStartTile(QPointF(126,126));

    if (config->repeatMode != REPEAT_SINGLE)
    {
        rows = 8;
        cols = 12;
    }
    else
    {
        rows = 1;
        cols = 1;
        settings.setStartTile(settings.getStartTile() + QPointF(600,300));
    }

    // create Patterns
    QColor c1(TileBlue);
    QColor c2(AlhambraBrown);
    for (int row = 0; row < rows; row++)
    {
        for (int col=0; col < cols; col++)
        {
            QColor c = ((row+col)&1) ? c1 : c2;
            PatternPtr pat = make_shared<Pattern14>(diameter, QBrush(c));
            pat->build();
            pat->setTilePosition(row,col);
            patterns.push_back(pat);

        }
    }
    return true;
}

bool Design17::build()
{
    settings.setViewSize(QSize(1800,1100));
    settings.setBackgroundColor((QColor(TileBlack)));

    qreal diameter = 200.0;
    settings.setStartTile(settings.getCenter());

    xSeparation = 145;
    ySeparation = 145;
    settings.setStartTile(QPointF(-16,-16));

     if (config->repeatMode != REPEAT_SINGLE)
    {
        rows = 8;
        cols = 14;
    }
    else
    {
        rows = 1;
        cols = 1;
        settings.setStartTile(settings.getStartTile() + QPointF(600,300));
    }

    // create Patterns
    QColor c1(TileBlue);
    QColor c2(AlhambraBrown);
    for (int row = 0; row < rows; row++)
    {
        for (int col=0; col < cols; col++)
        {
            if ((col == 0) && (row & 1))
            {
                col++; // indent
            }

            QColor c = ((row+col)&3) ? c1 : c2;
            PatternPtr pat = make_shared<Pattern14>(diameter, QBrush(c));
            pat->setTilePosition(row,col);
            pat->build();
            patterns.push_back(pat);
            col++; // skip every other

        }
    }
    return true;
}


bool Design18::build()
{
    settings.setViewSize(QSize(1800,1100));
    settings.setBackgroundColor((QColor(TileBlack)));

    qreal diameter = 200.0;
    settings.setStartTile(settings.getCenter());

    xSeparation = 136;
    ySeparation = 117;
    settings.setStartTile(QPointF(-24,33));

     if (config->repeatMode != REPEAT_SINGLE)
    {
        rows = 12;
        cols = 14;
    }
    else
    {
        rows = 1;
        cols = 1;
        settings.setStartTile(settings.getStartTile() + QPointF(600,300));
    }

    // create Patterns
    QColor c1(TileBlue);
    QColor c2(AlhambraBrown);
    for (int row = 0; row < rows; row++)
    {
        for (int col=0; col < cols; col++)
        {
            QColor c = ((row+col)& 1) ? c1 : c2;
            PatternPtr pat = make_shared<Pattern15>(diameter, QBrush(c));
            pat->build();
            pat->setTilePosition(row,col);
            patterns.push_back(pat);
        }
    }
    return true;
}

bool Design19::build()
{
    settings.setViewSize(QSize(1800,1100));
    settings.setBackgroundColor((QColor(TileBlack)));

    qreal diameter = 400.0;
    settings.setStartTile(settings.getCenter());

    xSeparation = 400.0;
    ySeparation = 400.0;
    settings.setStartTile(QPointF(222,24));

     if (config->repeatMode != REPEAT_SINGLE)
    {
        rows = 12;
        cols = 14;
    }
    else
    {
        rows = 1;
        cols = 1;
        settings.setStartTile(settings.getStartTile() + QPointF(600,300));
    }

    // create Patterns
    QColor c1(TileBlue);
    QColor c2(AlhambraBrown);
    for (int row = 0; row < rows; row++)
    {
        for (int col=0; col < cols; col++)
        {
            QColor c = ((row+col)& 1) ? c1 : c2;
            PatternPtr pat = make_shared<Pattern16>(diameter, QBrush(c));
            pat->build();
            pat->setTilePosition(row,col);
            patterns.push_back(pat);
        }
    }
    return true;
}

bool DesignKumiko1::build()
{
    QSize size(1395,1000);
    settings.setViewSize(size);
    settings.setBackgroundColor((QColor(0x58, 0x39, 0x3e)));

    LegacyBorderPtr bp = make_shared<LegacyBorderTwoColor>(size,QColor(0xa2,0x79,0x67),QColor(0xa2,0x79,0x67),20);
    bp->construct();
    border = bp;

    qreal diameter = 200.0;
    settings.setStartTile(settings.getCenter());

    xSeparation = 200.0;
    ySeparation = 346.41;   // ypos * 2

    if (config->repeatMode != REPEAT_SINGLE)
    {
        settings.setStartTile(QPointF(-2.0,-197.0));
        rows = 6;
        cols = 9;
    }
    else
    {
        rows = 1;
        cols = 1;
        settings.setStartTile(QPointF(400,400));
    }

    // create Patterns
    for (int row = 0; row < rows; row++)
    {
        for (int col=0; col < cols; col++)
        {
            PatternPtr pat = make_shared<PatternKumiko1>(diameter, QBrush());
            pat->build();
            pat->setTilePosition(row,col);
            patterns.push_back(pat);
        }
    }
    return true;
}

////////////////////////////////////////

void DesignKumiko2::init()
{
    Design::init();

    QSize size(1395,1000);
    settings.setViewSize(size);

    settings.setBackgroundColor((QColor(0x58, 0x39, 0x3e)));

    //LegacyBorderPtr bp = make_shared<BorderTwoColor>(QColor(0xa2,0x79,0x67),QColor(0xa2,0x79,0x67),20.0);
    //info->setBorder(bp);

    xSeparation = 200.0/100.0;
    ySeparation = 346.41/100.0;   // ypos * 2

    if (config->repeatMode != REPEAT_SINGLE)
    {
        //info->setStartTile(QPointF(-2.0,-197.0));
        //info->setStartTile(QPointF(-2.0,282.0));
        settings.setStartTile(QPointF(0.0,0.0));

        rows = 6;
        cols = 9;
    }
    else
    {
        rows = 1;
        cols = 1;
        settings.setStartTile(QPointF(400,400));
    }
}

bool DesignKumiko2::build()
{
    PatternPtr pat = make_shared<PatternKumiko2>(2.0, QBrush());
    pat->setLoc(settings.getStartTile());        // needs to be done here since repeat is called after the map is created
    pat->fd.set(false,-5,5,-5,5);
    pat->trans1 = QPointF(2.0,0);
    pat->trans2 = QPointF(0,3.4641);
    pat->build();
    //pat->setTilePosition(row,col);
    patterns.push_back(pat);

    LegacyBorderPtr bp = make_shared<LegacyBorderTwoColor>(settings.getViewSize(),QColor(0xa2,0x79,0x67),QColor(0xa2,0x79,0x67),20.0);
    bp->construct();
    border = bp;

    settings.setBackgroundColor((QColor(0x58, 0x39, 0x3e)));

    return true;
}
