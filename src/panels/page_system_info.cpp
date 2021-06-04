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

#include "page_system_info.h"
#include "base/qtapplog.h"
#include "base/utilities.h"
#include "base/mosaic.h"
#include "geometry/transform.h"
#include "makers/tiling_maker/tiling_maker.h"
#include "makers/motif_maker/motif_maker.h"
#include "makers/decoration_maker/decoration_maker.h"
#include "viewers/viewcontrol.h"
#include "style/filled.h"

page_system_info::page_system_info(ControlPanel * cpanel)  : panel_page(cpanel,"System Info")
{
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
    tree->setMinimumWidth(701);
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

    connect(pbRefresh, &QPushButton::clicked,     this, &page_system_info::populateTree);
    connect(pbDump,    &QPushButton::clicked,     this, &page_system_info::dumpTree);
    connect(pbExpand,  &QPushButton::clicked,     this, &page_system_info::expandTree);
    connect(tree,      &QTreeWidget::itemClicked, this, &page_system_info::slot_itemClicked);
}


void  page_system_info::refreshPage()
{
}

void page_system_info::onEnter()
{
    if (!lockCheck->isChecked())
    {
        populateTree();
    }
}

void page_system_info::populateTree()
{
    qDebug() << "page_system_info::populateTree()";

    QApplication::setOverrideCursor(Qt::WaitCursor);

    tree->clear();

    QTreeWidgetItem * item;

    qDebug() << "page_system_info::populateTree() - mosaics start";

    // Decoraton maker
    int numStyles = 0;
    QString name = "No Mosaic";
    item = new QTreeWidgetItem();
    item->setText(0,"Decoration Maker");
    MosaicPtr  mosaic = decorationMaker->getMosaic();
    if (mosaic)
    {
        numStyles = mosaic->getStyleSet().size();
        name      = mosaic->getName();
    }
    item->setText(1,QString("Styles: %1").arg(numStyles));
    item->setText(2,QString("Mosaic: %1").arg(name));
    tree->addTopLevelItem(item);
    tree->expandItem(item);

    populateStyles(item,mosaic);

    qDebug() << "page_system_info::populateTree() - mosaics done";

    // Motif Maker
    const QVector<PrototypePtr> & prototypes = motifMaker->getPrototypes();
    // selected prototype
    item = new QTreeWidgetItem;
    item->setText(0,"Motif Maker");
    item->setText(1,QString("Protos: %1").arg(prototypes.size()));
    int dels = 0;
    for (auto prototype : prototypes)
    {
        dels += prototype->numDesignElements();
    }
    item->setText(2,QString("Design Elements: %1").arg(dels));
    tree->addTopLevelItem(item);

    PrototypePtr pp = motifMaker->getSelectedPrototype();
    populatePrototype(item,pp,"Selected Prototype");

    for (auto& proto : prototypes)
    {
        populatePrototype(item,proto,"Prototype");
    }

    // selected design element
    QTreeWidgetItem * item2 = new QTreeWidgetItem;
    item2->setText(0,"Selected Design Element");
    DesignElementPtr dep = motifMaker->getSelectedDesignElement();
    item2->setText(1,addr(dep.get()));
    item->addChild(item2);

    populateDEL(item2,dep);
    qDebug() << "page_system_info::populateTree() - motifs done";

    item2 = new QTreeWidgetItem;
    item2->setText(0,"Active Feature");
    FeaturePtr fp = motifMaker->getActiveFeature();
    item2->setText(1,addr(fp.get()));
    if (fp)
    {
        item->setText(2, QString("Points: %1 Rot: %2").arg(fp->numPoints()).arg(fp->getRotation()));
    }
    item->addChild(item2);

    // Tiling Maker
    const QVector<TilingPtr> & tilings = tilingMaker->getTilings();

    item = new QTreeWidgetItem;
    item->setText(0,"Tiling Maker");
    item->setText(1,QString("Tilings: %1").arg(tilings.size()));
    int features = 0;
    for (auto& tiling : tilings)
    {
        features += tiling->countPlacedFeatures();
    }
    item->setText(2,QString("Placed Features: %1").arg(features));
    tree->addTopLevelItem(item);

    TilingPtr tp = tilingMaker->getSelected();
    populateTiling(item,tp,"Selected Tiling");

    for (auto& tiling : tilings)
    {
        populateTiling(item,tiling,"Tiling");
    }

    qDebug() << "page_system_info::populateTree() - tilings done";

    // selected feature
    item = new QTreeWidgetItem;
    item->setText(0,"Selected Feature");
    fp = vcontrol->getSelectedFeature();
    item->setText(1,addr(fp.get()));
    if (fp)
    {
        item->setText(2, QString("Points: %1 Rot: %2").arg(fp->numPoints()).arg(fp->getRotation()));
    }
    tree->addTopLevelItem(item);

    // adjust
    tree->setColumnWidth(0,201);
    tree->resizeColumnToContents(2);

    QApplication::restoreOverrideCursor();
    qDebug() << "page_system_info::populateTree() - done";
}

void page_system_info::slot_itemClicked(QTreeWidgetItem * item, int col)
{
    Q_UNUSED(col)

    if (item->isExpanded())
    {
        tree->collapseItem(item);
    }
    else
    {
        tree->expandItem(item);
    }

    tree->resizeColumnToContents(2);
}

void page_system_info::populateStyles(QTreeWidgetItem * parent, MosaicPtr mosaic)
{
    if (!mosaic)
    {
        return;
    }

    QTreeWidgetItem * item;

    QVector<TilingPtr> tilings = mosaic->getTilings();
    for (auto& tp : tilings)
    {
        item = new QTreeWidgetItem();
        item->setText(0,"Tiling");
        item->setText(1,addr(tp.get()));
        item->setText(2,tp->getName());
        parent->addChild(item);
        populateTiling(item,tp,"Tiling");
    }

    const StyleSet & sset = mosaic->getStyleSet();
    for (auto style : sset)
    {
        item = new QTreeWidgetItem;
        item->setText(0,"Style");
        item->setText(1,addr(style.get()));
        item->setText(2,style->getStyleDesc());
        parent->addChild(item);
        tree->expandItem(item);

        if (style->getStyleType() == STYLE_FILLED)
        {
            FilledPtr fp = std::dynamic_pointer_cast<Filled>(style);
            Q_ASSERT(fp);
            QString astring = QString("Filled: algo=%1 cleanse=%2 blacks=%3 whites=%4")
                .arg(fp->getAlgorithm())
                .arg(fp->getCleanseLevel())
                .arg(fp->getDrawInsideBlacks())
                .arg(fp->getDrawOutsideWhites());
            item->setText(2,astring);  // overwrites
        }

        populateMap(item,style->getExistingMap());
        populatePrototype(item,style->getPrototype(),"Prototype");
    }
}

void page_system_info::populateMap(QTreeWidgetItem *parent, MapPtr mp)
{
    //mp->dump();

    QTreeWidgetItem * item;

    item = new QTreeWidgetItem;
    item->setText(0,"Map");
    item->setText(1,addr((mp.get())));
    parent->addChild(item);
    tree->expandItem(item);

    if (mp)
    {
        item->setText(2,mp->name());
    }
    else
    {
        item->setText(2,"EMPTY Map");
        return;
    }

    const QVector<VertexPtr> & vertices = mp->getVertices();
    const QVector<EdgePtr>   & edges    = mp->getEdges();

    QTreeWidgetItem * item2;
    item2 = new QTreeWidgetItem;
    item2->setText(0,"Vertices");
    item2->setText(1,QString("Count: %1").arg(mp->numVertices()));
    item->addChild(item2);
    //tree->expandItem(item2);

    for (int i=0; i< mp->numVertices(); i++)
    {
        VertexPtr v = vertices.at(i);
        QPointF pos = v->pt;
        QTreeWidgetItem * item3 = new QTreeWidgetItem;
        item3->setText(0,QString("(%1) %2").arg(i).arg(addr(v.get())));
        item3->setText(1,QString::number(pos.x()));
        item3->setText(2,QString::number(pos.y()));
        item2->addChild(item3);

        const NeighboursPtr n = mp->getRawNeighbours(v);
        std::vector<WeakEdgePtr> * wedges = dynamic_cast<std::vector<WeakEdgePtr>*>(n.get());
        for (auto pos = wedges->begin(); pos != wedges->end(); pos++)
        {
            WeakEdgePtr wedge = *pos;
            EdgePtr edge = wedge.lock();
            QTreeWidgetItem * item4 = new QTreeWidgetItem;
            item4->setText(0,"Edge");
            item4->setText(1,QString::number(edges.indexOf(edge)));
            item3->addChild(item4);
        }
    }

    item2 = new QTreeWidgetItem;
    item2->setText(0,"Edges");
    item2->setText(1,QString("Count: %1").arg(mp->numEdges()));
    item->addChild(item2);
    //tree->expandItem(item2);

    for (int i=0; i< mp->numEdges(); i++)
    {
        EdgePtr e = edges.at(i);
        QTreeWidgetItem * item3 = new QTreeWidgetItem;
        item3->setText(0,QString("(%1) %2").arg(i).arg(addr(e.get())));
        item3->setText(1, QString("from %1 to %2").arg(vertices.indexOf(e->v1)).arg(vertices.indexOf(e->v2)));
        item2->addChild(item3);
    }
}

void page_system_info::populatePrototype(QTreeWidgetItem *parent, PrototypePtr pp, QString name)
{
    if  (!pp) return;

    TilingPtr tiling                 = pp->getTiling();
    QVector<DesignElementPtr> & dels = pp->getDesignElements();
    QVector<QTransform> & tforms     = pp->getTranslations();

    // summary
    QTreeWidgetItem * pitem = new QTreeWidgetItem;
    pitem->setText(0,name);
    pitem->setText(1,addr(pp.get()));
    QString astring = "Translations: " + QString::number(pp->getTranslations().size()) + "  Design Elements: " + QString::number(dels.size());
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
        populateTiling(item,tiling,"Tiling");
    }

    // design elements
    QTreeWidgetItem * item = new QTreeWidgetItem();
    item->setText(0,"Design Elements");
    item->setText(1,QString("Count: %1").arg(dels.count()));
    pitem->addChild(item);
    tree->expandItem(item);
    for (auto& del : dels)
    {
        populateDEL(item,del);
    }
    tree->expandItem(pitem);

    // locations
    QTreeWidgetItem * item2 = new QTreeWidgetItem();
    item2->setText(0,"Locations");
    item2->setText(2,QString("Transforms: %1").arg(tforms.count()));
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

void page_system_info::populateDEL(QTreeWidgetItem * parent, DesignElementPtr de)
{
    if (!de) return;

    QTreeWidgetItem * item2 = new QTreeWidgetItem;
    item2->setText(0,"DEL");
    item2->setText(1,addr(de.get()));
    parent->addChild(item2);
    tree->expandItem(item2);

    FeaturePtr feature = de->getFeature();
    QTreeWidgetItem * item = new QTreeWidgetItem;
    item->setText(0,"Feature");
    item->setText(1, addr(feature.get()));
    item->setText(2, QString("Points: %1 Rot: %2 %3").arg(feature->numPoints())
                                                     .arg(feature->getRotation())
                                                     .arg((feature->isRegular()) ? "Regular" : "Irregular"));
    item2->addChild(item);
    tree->expandItem(item);

    FigurePtr figure = de->getFigure();
    item = new QTreeWidgetItem;
    item->setText(0,"Figure");
    item->setText(1,addr(figure.get()));
    QString astring = figure->getFigureDesc() + " " + figure->getFigTypeString();
    item->setText(2,astring);
    item2->addChild(item);
    tree->expandItem(item);
}

void page_system_info::populateTiling(QTreeWidgetItem * parent, TilingPtr tp, QString name)
{
    if (!tp) return;

    const QVector<PlacedFeaturePtr> & qlpf = tp->getPlacedFeatures();

    // summary
    QTreeWidgetItem * pitem = new QTreeWidgetItem;
    pitem->setText(0,name);
    pitem->setText(1,addr(tp.get()));
    QString astring = tp->getName() + " Features: " + QString::number(qlpf.size());
    astring += " T1" + Utils::str(tp->getTrans1()) + " T2" + Utils::str(tp->getTrans2());
    pitem->setText(2,astring);
    parent->addChild(pitem);

    for (auto pfp : qlpf)
    {
        QTransform tr  = pfp->getTransform();
        FeaturePtr fp  = pfp->getFeature();

        QTreeWidgetItem * item = new QTreeWidgetItem;
        item->setText(0,"Placed Feature");
        item->setText(1,addr(pfp.get()));
        item->setText(2,Transform::toInfoString(tr));
        pitem->addChild(item);

        QTreeWidgetItem * item2 = new QTreeWidgetItem;
        item2->setText(0,"Feature");
        item2->setText(1, addr(fp.get()));
        item->setText(2, QString("Points: %1 Rot: %2 %3").arg(fp->numPoints())
                                                         .arg(fp->getRotation())
                                                         .arg((fp->isRegular()) ? "Regular" : "Irregular"));
        item->addChild(item2);
    }
}

void page_system_info::expandTree()
{
    tree->expandAll();
}

void page_system_info::dumpTree()
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

void  page_system_info::dumpWalkTree(QTextStream & ts,  QTreeWidgetItem * item )
{
    ts << item->text(0) << "  " << item->text(1) << "  " << item->text(2) << endl;

    for( int i = 0; i < item->childCount(); ++i )
    {
        dumpWalkTree(ts, item->child(i));
    }
}
