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

#ifndef TILE_PATTERNS_H
#define TILE_PATTERNS_H

#include "base/tile.h"
#include "designs/shapefactory.h"
#include "base/configuration.h"
#include "base/shared.h"
#include "tile/tiling_loader.h"

enum eDirection
{
    CW,
    CCW
};

class Workspace;

class Pattern : public Tile
{
public:
    Pattern(qreal Diameter, QBrush brush, int Row = 0, int Col = 0);
    ~Pattern();

    virtual void build() = 0;

    void setHeight(qreal h) {height = h;}

    static int refs;

    QPointF trans1;
    QPointF trans2;
    FillData fd;

protected:
    void    nope();

    qreal   diameter;
    qreal   radius;
    qreal   height;     //used for FigureView
    QPen    linePen;
    QBrush  centerBrush;

    QPen    nopen;
    QBrush  nobrush;

    Configuration * config;
    Workspace     * workspace;

};


class Pattern5: public Pattern
{
public:
    Pattern5(qreal Diameter, QBrush brush);
    void build() override;

private:
    LayerPtr layer1;
    LayerPtr layer2;
    LayerPtr layer3;
};

class Pattern6: public Pattern
{
public:
    Pattern6(qreal Diameter, QBrush brush);
    void build() override;
private:
    LayerPtr layer1;
    LayerPtr layer2;
};


class Pattern7 : public Pattern
{
public:
    Pattern7(qreal Diameter, QBrush brush);
    void build() override;
    bool doStep(int Index) Q_DECL_OVERRIDE;
private:
    LayerPtr layer1;
};

class PatternHuSymbol : public Pattern
{
public:
    PatternHuSymbol(int gridWidth, QPen GridPen, QPen InnerPen, QColor canvasColor, qreal Diameter, QBrush brush);
    void build() override;
    bool doStep(int Index) Q_DECL_OVERRIDE;

private:
private:
    LayerPtr layer1;
    LayerPtr layer3;
    ShapeFPtr s;
    ShapeFPtr s3;
    qreal width;
    QPen  gridPen;
    QPen  innerPen;
    QColor canvasColor;
};

class PatternHuInterlace : public Pattern
{
public:
    PatternHuInterlace(int gridWidth, QPen GridPen, QPen InnerPen, QColor canvasColor, qreal Diameter, QBrush brush);
    void build() override;
    bool doStep(int Index) Q_DECL_OVERRIDE;

private:
    LayerPtr layer1;
    LayerPtr layer2;
    LayerPtr layer3;
    LayerPtr layer4;

    ShapeFPtr l1; // level 1
    ShapeFPtr l2; // level 2
    ShapeFPtr l3; // level 3
    ShapeFPtr l4; // level 4

    qreal width;
    QPen  gridPen;
    QPen  innerPen;
    QColor canvasColor;
};

class Pattern10 : public Pattern
{
public:
    Pattern10(qreal Diameter, QBrush Brush);
    void build() override;
    bool doStep(int Index) Q_DECL_OVERRIDE;

private:
    LayerPtr layer1;
};

class Pattern11 : public Pattern
{
public:
    Pattern11(qreal Diameter, QBrush Brush, qreal rotation, eDirection direction);
    void build() override;
protected:
    qreal Rotation;
    eDirection Direction;
private:
    LayerPtr layer1;
    LayerPtr layer2;
};

class Pattern12 : public Pattern
{
public:
    Pattern12(qreal Diameter, QBrush Brush, qreal rotation, eDirection direction, int Row = 0, int Col = 0);
    void build() override;
protected:
    qreal Rotation;
    eDirection Direction;
    Polygon2 * hexagon;
private:
    LayerPtr layer1;
    LayerPtr layer2;
    LayerPtr layer3;
};

class PatternIncompleteA : public Pattern
{
public:
    PatternIncompleteA(qreal Diameter, QBrush Brush);
    void build() override;
private:
    LayerPtr layer1;
    LayerPtr layer2;
    LayerPtr layer3;
    LayerPtr layer4;
};

class Pattern14 : public Pattern
{
public:
    Pattern14(qreal Diameter, QBrush Brush);
    void build() override;
private:
};

class Pattern15 : public Pattern
{
public:
    Pattern15(qreal Diameter, QBrush Brush);
    void build() override;
};

class Pattern16 : public Pattern
{
public:
    Pattern16(qreal Diameter, QBrush Brush);
    void build();
};


class PatternKumiko1 : public Pattern
{
public:
    PatternKumiko1(qreal Diameter, QBrush Brush);
    void build() override;

protected:
    ShapeFPtr s2;
    ShapeFPtr s3;
    ShapeFPtr s4;
    ShapeFPtr s5;
};

class PatternKumiko2 : public Pattern
{
public:
    PatternKumiko2(qreal Diameter, QBrush Brush);
    void build() override;

protected:
    ShapeFPtr s2;
    ShapeFPtr s3;
    ShapeFPtr s4;
    ShapeFPtr s5;

    LayerPtr layer2;
};

#endif
