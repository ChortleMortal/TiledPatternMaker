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

#include "page_workspace.h"
#include "tile/Tiling.h"
#include "tapp/Prototype.h"
#include "style/Sketch.h"
#include "base/qtapplog.h"

using std::string;

page_workspace::page_workspace(ControlPanel * cpanel)  : panel_page(cpanel,"Workspace Info")
{
    ws = workspace;

    QPushButton * pbDump = new QPushButton("Dump");
    pbDump->setFixedWidth(101);

    QPushButton * pbExpand = new QPushButton("Expand All");
    pbExpand->setFixedWidth(101);

    lockCheck = new QCheckBox("lock");
    lockCheck->setChecked(false);

    QPushButton * pbRefresh = new QPushButton("Refresh");
    pbRefresh->setFixedWidth(101);

    QHBoxLayout * hbox = new QHBoxLayout();
    hbox->addWidget(pbDump);
    hbox->addSpacing(15);
    hbox->addWidget(pbExpand);
    hbox->addStretch();
    hbox->addWidget(lockCheck);
    hbox->addWidget(pbRefresh);

    tree = new QTreeWidget();
    tree->setColumnCount(3);
    tree->setHeaderHidden(true);
    tree->setMinimumWidth(601);
    tree->setMinimumHeight(750);
    tree->setRootIsDecorated(true);
    tree->setItemsExpandable(true);

    AQVBoxLayout * box = new AQVBoxLayout();
    box->setSizeConstraint(QLayout::SetFixedSize);
    box->addLayout(hbox);
    box->addSpacing(7);
    box->addWidget(tree);

    AQWidget * widget = new AQWidget();
    widget->setLayout(box);

    vbox->addWidget(widget);

    connect(pbRefresh, &QPushButton::clicked,     this, &page_workspace::populateTree);
    connect(pbDump,    &QPushButton::clicked,     this, &page_workspace::dumpTree);
    connect(pbExpand,  &QPushButton::clicked,     this, &page_workspace::expandTree);
    connect(tree,      &QTreeWidget::itemClicked, this, &page_workspace::slot_itemClicked);
}


void  page_workspace::refreshPage()
{
}

void page_workspace::onEnter()
{
    if (!lockCheck->isChecked())
    {
        populateTree();
    }
}

void page_workspace::populateTree(bool expandAll)
{
    tree->clear();

    // loaded style
    loadedStyle = new QTreeWidgetItem();
    loadedStyle->setText(0,"++ loadedStyles");
    int size = ws->getStyledDesign(WS_LOADED).getStyleSet().size();
    loadedStyle->setText(1,QString("Num=%1").arg(size));
    loadedStyle->setText(2,ws->getStyledDesign(WS_LOADED).getName());
    tree->addTopLevelItem(loadedStyle);
    loadedStyle->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
    if (expandAll)
        slot_itemClicked(loadedStyle,0);

    // working style
    workspaceStyle = new QTreeWidgetItem();
    workspaceStyle->setText(0,"++ workspaceStyles");
    size = ws->getStyledDesign(WS_TILING).getStyleSet().size();
    workspaceStyle->setText(1,QString("Num=%1").arg(size));
    workspaceStyle->setText(2,ws->getStyledDesign(WS_TILING).getName());
    tree->addTopLevelItem(workspaceStyle);
    workspaceStyle->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
        if (expandAll)
        slot_itemClicked(workspaceStyle,0);

    workspacePrototype = new QTreeWidgetItem;
    workspacePrototype->setText(0,"++ workspacePrototype");
    workspacePrototype->setText(1,addr(ws->getPrototype(WS_TILING).get()));
    tree->addTopLevelItem(workspacePrototype);
    workspacePrototype->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
    if (expandAll)
        slot_itemClicked(workspacePrototype,0);

    workspaceTiling= new QTreeWidgetItem;
    workspaceTiling->setText(0,"++ workspaceTiling");
    workspaceTiling->setText(1,addr(ws->getTiling(WS_TILING).get()));
    if (ws->getTiling(WS_TILING))
        workspaceTiling->setText(2,ws->getTiling(WS_TILING)->getName());
    tree->addTopLevelItem(workspaceTiling);
    workspaceTiling->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);

    if (expandAll)
        slot_itemClicked(workspaceTiling,0);

    workingFigure = new QTreeWidgetItem;
    workingFigure->setText(0,"++ workspaceFigure");
    QString str;
    DesignElementPtr dep = ws->getSelectedDesignElement(WS_TILING);
    if (dep)
    {
        str = addr(dep->getFigure().get());
    }
    workingFigure->setText(1,str);
    tree->addTopLevelItem(workingFigure);
    workingFigure->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);

    if (expandAll)
        slot_itemClicked(workingFigure,0);

    tree->setColumnWidth(0,201);
    tree->resizeColumnToContents(2);

    if (expandAll)
        tree->expandAll();
}

void page_workspace::slot_itemClicked(QTreeWidgetItem * item, int col)
{
    Q_UNUSED(col)

    if (item == loadedStyle)
    {
        removeChildren(loadedStyle);
        populateStyles(loadedStyle,workspace->getStyledDesign(WS_LOADED));
        tree->expandItem(loadedStyle);
    }
    else if (item == workspaceStyle)
    {
        removeChildren(workspaceStyle);
        populateStyles(workspaceStyle,workspace->getStyledDesign(WS_TILING));
        tree->expandItem(workspaceStyle);
    }
    else if (item == workspacePrototype)
    {
        if (ws->getPrototype(WS_TILING))
        {
            removeChildren(workspacePrototype);
            populatePrototype(workspacePrototype,ws->getPrototype(WS_TILING));
            tree->expandItem(workspacePrototype);
        }
    }
    else if (item == workspaceTiling)
    {
        if (ws->getTiling(WS_TILING))
        {
            removeChildren(workspaceTiling);
            populateTiling(workspaceTiling,ws->getTiling(WS_TILING));
            tree->expandItem(workspaceTiling);
        }
    }
    else
    {
       // some other - just toggle expansion
       if (item->isExpanded())
       {
           tree->collapseItem(item);
       }
       else
       {
           tree->expandItem(item);
       }
    }

    tree->resizeColumnToContents(2);
}

void page_workspace::populateStyles(QTreeWidgetItem * parent, StyledDesign & design)
{
    QTreeWidgetItem * item;

    TilingPtr tp = design.getTiling();
    if (tp)
    {
        item = new QTreeWidgetItem();
        item->setText(0,"Tiling");
        item->setText(1,addr(tp.get()));
        item->setText(2,tp->getName());
        parent->addChild(item);
        populateTiling(item,tp);
    }

    int size = design.getStyleSet().size();
    const StyleSet & sset = design.getStyleSet();
    for (int i=0; i < size; i++)
    {
        item = new QTreeWidgetItem;

        StylePtr s = sset[i];
        item->setText(0,"Style");
        item->setText(1,addr(s.get()));
        item->setText(2,s->getStyleDesc());
        parent->addChild(item);

        populateMap(item,s->getMap());
        populatePrototype(item,s->getPrototype());

        QTreeWidgetItem * item2 = new QTreeWidgetItem;
        item2->setText(0,"Boundary");
        PolyPtr pp = s->getBoundary();
        QString astring;
        for (int i=0; i < pp->size(); i++)
        {
            QPointF pt = pp->at(i);
            QString bstring = "[" + QString::number(pt.x()) + "," + QString::number(pt.y()) + "]  ";
            astring += bstring;
        }
        item2->setText(2,astring);
        item->addChild(item2);
    }
}

void page_workspace::populateMap(QTreeWidgetItem *parent, MapPtr mp)
{
    //mp->dump();

    QTreeWidgetItem * item;

    item = new QTreeWidgetItem;
    item->setText(0,"Map");
    item->setText(1,addr((mp.get())));
    parent->addChild(item);

    if (!mp)
    {
        item->setText(2,"EMPTY Map");
        return;
    }

    const QVector<VertexPtr> & vertices = mp->getVertices();
    const QVector<EdgePtr>   & edges    = mp->getEdges();

    QTreeWidgetItem * item2;
    item2 = new QTreeWidgetItem;
    item2->setText(0,"Vertices");
    item2->setText(1,QString("count=%1").arg(mp->numVertices()));
    item->addChild(item2);

    NeighbourMap & nmap = mp->getNeighbourMap();

    for (int i=0; i< mp->numVertices(); i++)
    {
        VertexPtr v = vertices.at(i);
        QPointF pos = v->getPosition();
        QTreeWidgetItem * item3 = new QTreeWidgetItem;
        item3->setText(0,QString("(%1) %2").arg(i).arg(addr(v.get())));
        item3->setText(1,QString::number(pos.x()));
        item3->setText(2,QString::number(pos.y()));
        item2->addChild(item3);

        NeighboursPtr np        = nmap.getNeighbours(v);
        QVector<EdgePtr> & qvep = np->getNeighbours();
        for (auto edge : qvep)
        {
            QTreeWidgetItem * item4 = new QTreeWidgetItem;
            item4->setText(0,"edge");
            item4->setText(1,QString::number(edges.indexOf(edge)));
            item3->addChild(item4);
        }
    }

    item2 = new QTreeWidgetItem;
    item2->setText(0,"Edges");
    item2->setText(1,QString("count=%1").arg(mp->numEdges()));
    item->addChild(item2);

    for (int i=0; i< mp->numEdges(); i++)
    {
        EdgePtr e = edges.at(i);
        QTreeWidgetItem * item3 = new QTreeWidgetItem;
        item3->setText(0,QString("(%1) %2").arg(i).arg(addr(e.get())));
        item3->setText(1, QString("from %1 to %2").arg(vertices.indexOf(e->getV1())).arg(vertices.indexOf(e->getV2())));
        item2->addChild(item3);
    }
}

void page_workspace::populatePrototype(QTreeWidgetItem *parent, PrototypePtr pp)
{
    TilingPtr tiling                 = pp->getTiling();
    QVector<DesignElementPtr> & dels = pp->getDesignElements();
    QVector<QTransform> & tforms     = pp->getLocations();

    // summary
    QTreeWidgetItem * pitem = new QTreeWidgetItem;
    pitem->setText(0,"Prototype");
    pitem->setText(1,addr(pp.get()));
    QString astring = "Num Locations=" + QString::number(pp->getLocations().size()) + "  Num Design Elements =" + QString::number(dels.size());
    pitem->setText(2,astring);
    parent->addChild(pitem);

    // tiling
    if (tiling)
    {
        QTreeWidgetItem * item = new QTreeWidgetItem();
        item->setText(0,"Tiling");
        item->setText(1,addr(tiling.get()));
        item->setText(2,tiling->getName());
        pitem->addChild(item);
        populateTiling(item,tiling);
    }

    // design elements
    QTreeWidgetItem * item = new QTreeWidgetItem();
    item->setText(0,"Design Elements");
    item->setText(1,QString("Num=%1").arg(dels.count()));
    pitem->addChild(item);
    for (auto del : dels)
    {
        populateDEL(item,del);
    }

    // locations
    QTreeWidgetItem * item2 = new QTreeWidgetItem();
    item2->setText(0,"Locations");
    item2->setText(2,QString("Num transforms = %1").arg(tforms.count()));
    pitem->addChild(item2);
    for (auto it = tforms.begin(); it != tforms.end(); it++)
    {
        QTransform tr = *it;
        QTreeWidgetItem * item = new QTreeWidgetItem;
        item->setText(0,"Transform");
        item->setText(2,Transform::toInfoString(tr));
        item2->addChild(item);
    }
}

void page_workspace::populateDEL(QTreeWidgetItem * parent, DesignElementPtr de)
{
    QTreeWidgetItem * item2 = new QTreeWidgetItem;
    item2->setText(0,"DEL");
    item2->setText(1,addr(de.get()));
    parent->addChild(item2);

    FeaturePtr feature = de->getFeature();
    QTreeWidgetItem * item = new QTreeWidgetItem;
    item->setText(0,"Feature");
    item->setText(1, addr(feature.get()));
    item->setText(2, QString("Num points = %1 Rot = %2").arg(feature->numPoints()).arg(feature->getRotation()));
    item2->addChild(item);

    FigurePtr figure = de->getFigure();
    item = new QTreeWidgetItem;
    item->setText(0,"Figure");
    item->setText(1,addr(figure.get()));
    QString astring = figure->getFigureDesc() + " " + figure->getFigTypeString();
    item->setText(2,astring);
    item2->addChild(item);

#if 0   // DAC - what was I thinking?
    // find matching vector of placed features in tiling
    TilingPtr tp = ws->GetTiling();
    if (!tp)
    {
        item = new QTreeWidgetItem;
        item->setText(0,"** ERROR - tiling is null");
        parent->addChild(item);
        return;
    }
    QMap<FeaturePtr,QVector<PlacedFeaturePtr>> pfm = tp->regroupFeatures();
    QVector<PlacedFeaturePtr> pfpv = pfm.value(feap);
    for (auto it = pfpv.begin(); it != pfpv.end(); it++)
    {
        PlacedFeaturePtr pfp = *it;
        Transform tr = pfp->getTransform();
        PlacedDesignElement pdel(de,tr);

        item = new QTreeWidgetItem;
        item->setText(0,"Placed Feature");
        item->setText(1,addr(pfp.get()));
        item->setText(2,pdel.getTransform().toString());
        parent->addChild(item);

        QTreeWidgetItem * item2;

        // combine design element with transform to make placed design element
        FeaturePtr feap2 = pdel.getFeature();
        FigurePtr  figp2 = pdel.getFigure();
        if (feap2 != feap)
        {
            //  FEAP error
            item2 = new QTreeWidgetItem;
            item2->setText(0,"** FEAP ERRROR");
            item2->setText(1, addr(feap.get()));
            item2->setText(2, addr(feap2.get()));
            item->addChild(item2);
            return;
        }
        if (figp2 != figp)
        {
            //  FIGP error
            item2 = new QTreeWidgetItem;
            item2->setText(0,"** FIGP ERRROR");
            item2->setText(1, addr(figp.get()));
            item2->setText(2, addr(figp2.get()));
            item->addChild(item2);
            return;
        }

        if (!feap)
        {
            item2 = new QTreeWidgetItem;
            item2->setText(0,"Feature");
            item2->setText(1,"null");
            item->addChild(item2);
        }
        else
        {
            item2 = new QTreeWidgetItem;
            item2->setText(0,"Feature");
            item2->setText(1, addr(feap.get()));
            item2->setText(2, QString("Num points = %1").arg(feap->numPoints()));
            item->addChild(item2);
        }

        if (!figp)
        {
            item2 = new QTreeWidgetItem;
            item2->setText(0,"Figure");
            item2->setText(1,"nullptr");
            item->addChild(item2);
        }
        else
        {
            item2 = new QTreeWidgetItem;
            item2->setText(0,"Figure");
            item2->setText(1,addr(figp.get()));
            item2->setText(2,figp->getFigureDesc());
            item->addChild(item2);

            ExplicitPtr efp = std::dynamic_pointer_cast<ExplicitFigure>(figp);
            if (efp)
            {
                MapPtr map = efp->getMap();
                map->dump();
                populateMap(item2,map);
            }
        }
    }
#endif
}

void page_workspace::populateTiling(QTreeWidgetItem * parent, TilingPtr tp)
{
    QList <PlacedFeaturePtr> & qlpf = tp->getPlacedFeatures();
    QList <PlacedFeaturePtr>::iterator it;
    for (it = qlpf.begin(); it != qlpf.end(); it++)
    {
        PlacedFeaturePtr pfp = *it;
        QTransform tr        = pfp->getTransform();
        FeaturePtr fp        = pfp->getFeature();

        QTreeWidgetItem * item = new QTreeWidgetItem;
        item->setText(0,"Placed Feature");
        item->setText(1,addr(pfp.get()));
        item->setText(2,Transform::toInfoString(tr));
        parent->addChild(item);

        QTreeWidgetItem * item2 = new QTreeWidgetItem;
        item2->setText(0,"Feature");
        item2->setText(1, addr(fp.get()));
        item2->setText(2, QString("Num points = %1 Rot = %2").arg(fp->numPoints()).arg(fp->getRotation()));
        item->addChild(item2);
    }
}

void page_workspace::expandTree()
{
    populateTree(true);
}

void page_workspace::dumpTree()
{
    QDate date = QDate::currentDate();
    QString stamp =  QString::number(date.toJulianDay()) + QTime::currentTime().toString();
    stamp = stamp.replace(':', '.');
    QString fileName  = qtAppLog::getInstance()->logDir() + "wsDumpFile" + stamp + ".txt";
    qDebug() << "saving:" << fileName;

    QFile afile(fileName);
    if (!afile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qDebug() << "Could not write to file:"  << fileName;
        return;
    }

    QTextStream ts(&afile);
    ts.setRealNumberPrecision(16);

    ts << "== START TREE ==  " <<  qApp->applicationDirPath() << endl;
    dumpWalkTree(ts,tree->invisibleRootItem());
    ts << "== END   TREE ==" << endl;

    afile.close();
}

void  page_workspace::dumpWalkTree(QTextStream & ts,  QTreeWidgetItem * item )
{
    ts << item->text(0) << "  " << item->text(1) << "  " << item->text(2) << endl;

    for( int i = 0; i < item->childCount(); ++i )
    {
        dumpWalkTree(ts, item->child(i));
    }
}

void page_workspace::removeChildren(QTreeWidgetItem * parent)
{
    // delete all children
    QList<QTreeWidgetItem*> list = parent->takeChildren();
    for (auto item : list)
    {
        delete item;
    }
}
