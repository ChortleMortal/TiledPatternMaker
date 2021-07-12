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

#ifndef FIGUREVIEW_H
#define FIGUREVIEW_H

//#include "base/shared.h"
#include "base/layer.h"
//#include "tapp/infer.h"

typedef std::shared_ptr<class FigureView>       FigureViewPtr;
typedef std::shared_ptr<class Figure>           FigurePtr;
typedef std::shared_ptr<class DesignElement>    DesignElementPtr;
typedef std::shared_ptr<class Map>              MapPtr;
typedef std::shared_ptr<class Feature>          FeaturePtr;

class FigureView : public Layer
{
public:
    static FigureViewPtr getSharedInstance();
    FigureView();
    virtual ~FigureView() override;

    virtual void paint(QPainter *painter) override;

    void    setDebugContacts(bool enb, QPolygonF pts, QVector<class contact*> contacts);

public slots:
    virtual void slot_mousePressed(QPointF spt, enum Qt::MouseButton btn) override;
    virtual void slot_mouseDragged(QPointF spt)       override;
    virtual void slot_mouseTranslate(QPointF pt)      override;
    virtual void slot_mouseMoved(QPointF spt)         override;
    virtual void slot_mouseReleased(QPointF spt)      override;
    virtual void slot_mouseDoublePressed(QPointF spt) override;

    virtual void slot_wheel_scale(qreal delta)  override;
    virtual void slot_wheel_rotate(qreal delta) override;

    virtual void slot_scale(int amount)  override;
    virtual void slot_rotate(int amount) override;
    virtual void slot_moveX(int amount)  override;
    virtual void slot_moveY(int amount)  override;

protected:
    void paintExplicitFigureMap(QPainter *painter, QPen pen);
    void paintRadialFigureMap(QPainter *painter, QPen pen);
    void paintFeatureBoundary(QPainter *painter);
    void paintRadialFigureBoundary(QPainter *painter);
    void paintExtendedBoundary(QPainter *painter);

private:
    static FigureViewPtr spThis;

    void paintMap(QPainter * painter, MapPtr map, QPen pen);

    class MotifMaker * motifMaker;

    DesignElementPtr _dep;
    FigurePtr        _fig;
    FeaturePtr       _feat;
    QTransform       _T;

    bool                debugContacts;
    QPolygonF           debugPts;
    QVector<contact *>  debugContactPts;
};

#endif // FIGUREVIEW_H
