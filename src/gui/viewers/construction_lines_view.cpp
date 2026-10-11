#include <QMenu>

#include "gui/top/view_controller.h"
#include "gui/viewers/construction_lines_view.h"
#include "gui/viewers/geo_graphics.h"
#include "gui/viewers/tiling_maker_view.h"
#include "model/makers/tiling_maker.h"
#include "model/tilings/tiling.h"
#include "sys/geometry/geo.h"
#include "sys/sys.h"

using std::make_shared;

ConstructionLinesView::ConstructionLinesView()
    : LayerController(Type::ConstructionLines,Model::Derived,ZLevel::Standard_Z0,"Construction Lines")
{}

ConstructionLinesView::~ConstructionLinesView()
{}

int ConstructionLinesView::findLineNear(const QVector<QLineF> & lines, const QTransform & t, QPointF spt)
{
    for (int i = 0; i < lines.size(); i++)
    {
        if (Geo::distToLine(spt, t.map(lines[i])) < 7.0)
        {
            return i;
        }
    }
    return -1;
}

int ConstructionLinesView::findEndpointNear(const QLineF & line, const QTransform & t, QPointF spt)
{
    qreal d1 = Geo::dist2(spt, t.map(line.p1()));
    qreal d2 = Geo::dist2(spt, t.map(line.p2()));

    constexpr qreal tolerance2 = 25.0 * 25.0;
    if (d1 > tolerance2 && d2 > tolerance2)
        return -1;

    return (d1 <= d2) ? 0 : 1;
}

void ConstructionLinesView::paint(QPainter * painter)
{
    draw(painter);
}

void ConstructionLinesView::draw(QPainter * painter)
{
    auto tiling = Sys::tilingMaker->getSelectedTiling();
    if (!tiling || !tiling->isViewable())
    {
        return;
    }

    GeoGraphics gg(painter,getUndistortedLayerTransform());

    auto view = Sys::tilingMakerView;
    const QVector<QLineF> & constructionLines = view->getConstructionLines();
    int selected = view->getSelectedConstructionLine();

    QPen apen(Qt::red,3);
    QPen selectedPen(Qt::yellow,3);
    for (int i = 0; i < constructionLines.size(); i++)
    {
        const QLineF & line = constructionLines[i];
        gg.drawLine(line, (i == selected) ? selectedPen : apen);
        gg.drawCircle(line.pointAt(0.5), 5, QPen(TilingMakerView::construction_mid_color), QBrush(TilingMakerView::construction_mid_color));

        QPointF ends[2] = { line.p1(), line.p2() };
        for (QPointF pt : ends)
        {
            auto state = view->classifyConstructionEndpoint(pt,i);
            if (state == TilingMakerView::ConstructionEndpointState::Match)
                gg.drawCircle(pt, 5, QPen(Qt::green), QBrush(Qt::green));
            else if (state == TilingMakerView::ConstructionEndpointState::Near)
                gg.drawCircle(pt, 5, QPen(Qt::red), QBrush(Qt::red));
        }
    }

    // hovering in the grab-zone of the selected line's endpoint (not mid-drag - that has its own indicator)
    if (!mouse_interaction && selected >= 0 && hoverEndpoint >= 0 && selected < constructionLines.size())
    {
        const QLineF & selLine = constructionLines[selected];
        QPointF pt = (hoverEndpoint == 0) ? selLine.p1() : selLine.p2();
        gg.drawCircle(pt, 6, QPen(Qt::yellow,2), QBrush());
    }

    // mark plain interior crossings between two lines (away from either's endpoints)
    for (int i = 0; i < constructionLines.size(); i++)
    {
        for (int j = i+1; j < constructionLines.size(); j++)
        {
            QPointF ipt;
            if (constructionLines[i].intersects(constructionLines[j], &ipt) == QLineF::BoundedIntersection)
            {
                gg.drawCircle(ipt, 5, QPen(TilingMakerView::construction_crossing_color), QBrush(TilingMakerView::construction_crossing_color));
            }
        }
    }

    if (mouse_interaction)
    {
        mouse_interaction->draw(&gg);
    }
}

bool ConstructionLinesView::isEditing()
{
    return viewControl()->isEnabled(type())
        && Sys::tilingMaker->getMouseMode() == TilingMaker::MouseMode::ConstructionLines;
}

void ConstructionLinesView::slot_mousePressed(QPointF spt, Qt::MouseButton btn)
{
    if (!isEditing()) return;

    auto view = Sys::tilingMakerView;
    QVector<QLineF> & constructionLines = view->getConstructionLines();
    int selected = view->getSelectedConstructionLine();

    if (btn == Qt::RightButton)
    {
        int  endpoint = (selected >= 0) ? view->findConstructionEndpointNear(selected, spt) : -1;
        bool onBody   = (selected >= 0) && (view->findConstructionLineNear(spt) == selected);

        // a line shorter on screen than twice the endpoint tolerance has both endpoints
        // within reach of any click near it, so "near an endpoint" would always win and
        // the delete menu below would never show - too short to drag-extend, still deletable
        constexpr qreal shortLineLength = 50.0;
        bool tooShortToExtend = (selected >= 0)
            && (view->undistortedModelToScreen(constructionLines[selected]).length() < shortLineLength);

        if (endpoint >= 0 && !tooShortToExtend)
        {
            mouse_interaction = make_shared<TilingExtendConstructionLine>(selected, endpoint, spt);
        }
        else if (endpoint >= 0 || onBody)
        {
            // right-click on the already-selected line - always offer Delete
            QMenu myMenu;
            myMenu.addAction("Delete", Sys::tilingMaker, SLOT(slot_deleteConstructionLine()));
            myMenu.exec(Sys::systemViewController->mapToGlobal(spt.toPoint()));
        }
        else
        {
            view->setSelectedConstructionLine(view->findConstructionLineNear(spt));
            forceRedraw();
        }
    }
    else if (btn == Qt::LeftButton)
    {
        int endpoint = (selected >= 0) ? view->findConstructionEndpointNear(selected, spt) : -1;
        if (endpoint >= 0)
        {
            mouse_interaction = make_shared<TilingMoveConstructionEndpoint>(selected, endpoint, spt);
            return;
        }

        if (selected >= 0)
        {
            view->setSelectedConstructionLine(-1);
            forceRedraw();
        }
        mouse_interaction = make_shared<TilingConstructionLine>(PlacedTileSelectorPtr(), spt);
    }
}

void ConstructionLinesView::slot_mouseDragged(QPointF spt)
{
    if (!isEditing()) return;

    if (mouse_interaction)
        mouse_interaction->updateDragging(spt);
}

void ConstructionLinesView::slot_mouseMoved(QPointF spt)
{
    if (!isEditing()) return;

    auto view    = Sys::tilingMakerView;
    int selected = view->getSelectedConstructionLine();
    int hover    = (selected >= 0) ? view->findConstructionEndpointNear(selected, spt) : -1;
    if (hover != hoverEndpoint)
    {
        hoverEndpoint = hover;
        forceRedraw();
    }
}

void ConstructionLinesView::slot_mouseReleased(QPointF spt)
{
    if (!isEditing()) return;

    if (mouse_interaction)
    {
        mouse_interaction->endDragging(spt);
        mouse_interaction.reset();
    }
    forceRedraw();
}

void ConstructionLinesView::slot_mouseDoublePressed(QPointF spt)
{
    Q_UNUSED(spt);
}
