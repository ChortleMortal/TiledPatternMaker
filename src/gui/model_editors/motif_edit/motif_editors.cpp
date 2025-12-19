#include <QDebug>
#include <QGroupBox>
#include "gui/model_editors/motif_edit/motif_maker_widget.h"
#include "gui/model_editors/motif_edit/motif_editor_widget.h"
#include "model/prototypes/prototype.h"
#include "model/makers/prototype_maker.h"
#include "model/prototypes/design_element.h"
#include "model/motifs/motif.h"
#include "model/motifs/radial_motif.h"
#include "gui/panels/page_motif_maker.h"
#include "model/settings/configuration.h"
#include "gui/widgets/layout_sliderset.h"
#include "gui/panels/panel_misc.h"
#include "gui/top/system_view.h"


Q_DECLARE_METATYPE(NamedMotifEditor *)

/////////////////////////////////////////////////////////
///
/// MotifEditorWidget
///
////////////////////////////////////////////////////////

SpecificEditorWidget::SpecificEditorWidget()
{
    setContentsMargins(0,0,0,0);
    current = nullptr;
}

void SpecificEditorWidget::setEditor(NamedMotifEditor * fe)
{
    // remove old layout
    QLayout * l = layout();
    if (l)
    {
        QLayoutItem * item;
        while ( (item = l->itemAt(0)) != nullptr)
        {
            QWidget * w = item->widget();
            if (w)
            {
                w->setParent(nullptr);
            }
        }
        delete l;
    }

    // set new layout
    AQVBoxLayout * aLayout = new AQVBoxLayout();
    aLayout->addWidget(fe);
    setLayout(aLayout);
    adjustSize();

    // delete old widget
    if (current)
        delete current;
    current = fe;
}

/////////////////////////////////////////////////////////
///
/// MotifTypeChoiceCombo
///
////////////////////////////////////////////////////////


// An abstract class for containing the controls related to the editing
// of one kind of figure.  A complex hierarchy of FigureEditors gets built
// up to become the changeable controls for editing figures in FigureMaker.

NamedMotifEditor::NamedMotifEditor(QString motifName)
{
    name  = motifName;

    setContentsMargins(0,0,0,0);

    conEd        = nullptr;
    connectScale = nullptr;

    motifSides     = new SliderSet("Sides", 6, 1, 64);
    motifScale     = new DoubleSliderSet("Scale", 1.0, 0.1, 9.0, 100 );
    motifRotate    = new DoubleSliderSet("Rotate",0.0, -360.0, 360.0, 1);

    vbox  = new AQVBoxLayout();
    vbox->addLayout(motifSides);
    vbox->addLayout(motifRotate);
    vbox->addLayout(motifScale);

    setLayout(vbox);

    connect(this,       &NamedMotifEditor::sig_motif_modified,  this,      &NamedMotifEditor::slot_motifModified); //, Qt::QueuedConnection);
    connect(this,       &NamedMotifEditor::sig_motif_modified,  Sys::viewController, &SystemViewController::slot_updateView);
    connect(motifSides, &SliderSet::valueChanged,               this, [this]() { editorToMotif(true);});
    connect(motifScale, &DoubleSliderSet::valueChanged,         this, [this]() { editorToMotif(true);});
    connect(motifRotate,&DoubleSliderSet::valueChanged,         this, [this]() { editorToMotif(true);});

    MotifMakerWidget * mmw = Sys::prototypeMaker->getWidget();
    MotifEditorWidget * ed = mmw->getMotifEditor();
    connect(this,       &NamedMotifEditor::sig_redisplay,       ed, &MotifEditorWidget::slot_motifTypeChanged, Qt::QueuedConnection);
}

void NamedMotifEditor::setMotif(MotifPtr motif, bool doEmit)
{
    Q_UNUSED(doEmit);

    ConnectPtr cp =  motif->getRadialConnector();
    if (cp)
    {
        conEd = new ConnectorEditor(cp);
        QHBoxLayout * hbox = conEd->createConnectorLayout(this);
        addLayout(hbox);
    }

    for (const ExtenderPtr &ep : motif->getExtenders())
    {
        ExtenderEditor * ed = new ExtenderEditor(ep);
        eds.push_back(ed);
        QVBoxLayout * vbox = ed->createExtenderLayout(this);
        addLayout(vbox);
    }

    wMotif = motif;
}

void NamedMotifEditor::motifToEditor()
{
    auto motif = wMotif.lock();
    if (!motif)
        return;

    qreal  msc = motif->getMotifScale();
    qreal  mro = motif->getMotifRotate();
    int    ms  = motif->getN();

    blockSignals(true);

    motifSides->setValue(ms);
    motifScale->setValue(msc);
    motifRotate->setValue(mro);

    blockSignals(false);

    if (conEd)
    {
        conEd->connectorToEditor();
    }

    for (auto ed : eds)
    {
        ed->extenderToEditor();
    }
}

void NamedMotifEditor::editorToMotif(bool doEmit)
{
    auto motif = wMotif.lock();
    if (!motif)
        return;

    int   msides = motifSides->value();
    qreal mscale = motifScale->value();
    qreal mrot   = motifRotate->value();

    motif->setN(msides);
    motif->setMotifScale(mscale);
    motif->setMotifRotate(mrot);

    if (conEd)
    {
        conEd->editorToConnector();

    }
    for (auto ed : eds)
    {
        ed->editorToExtender();
    }

    if (doEmit)
        emit sig_motif_modified(motif);
}

void NamedMotifEditor::slot_motifModified(MotifPtr motif)
{
    motif->resetMotifMap();
    motif->buildMotifMap();

    auto del = Sys::prototypeMaker->getSelectedDEL();
    if (!del)
    {
        return;
    }

    del->setMotif(motif);

    // notify motif maker
    bool multi = (Sys::config->motifMakerView == MOTIF_VIEW_SELECTED);
    Sys::prototypeMaker->select(MVD_DELEM,del,multi);     // if this is the samne design element this does nothing

    auto tiling = Sys::prototypeMaker->getSelectedPrototype()->getTiling();
    ProtoEvent pevent;
    pevent.event = PROM_MOTIF_CHANGED;
    pevent.tiling = tiling;
    Sys::prototypeMaker->sm_takeUp(pevent);
    
    //data->select(MVD_DELEM,del,multi);
    Sys::prototypeMaker->getWidget()->update();
    update();
    emit sig_updateView();
}

void NamedMotifEditor::addConnector()
{
    auto motif = wMotif.lock();
    if (motif)
    {
        if (!motif->getRadialConnector())
        {
            motif->addConnector();
            emit sig_redisplay(motif->getMotifType());
        }
    }
}

void NamedMotifEditor::addExtender()
{
    auto motif = wMotif.lock();
    if (motif)
    {
        motif->addExtender();
    }
}

void NamedMotifEditor::deleteExtender()
{
    // deletes the last extender
    auto motif = wMotif.lock();
    if (motif)
    {
        auto extenders = motif->getExtenders();
        if (extenders.count())
        {
            ExtenderPtr ep = extenders.last();
            if (ep)
            {
                motif->deleteExtender(ep);
            }
        }
    }
}

void NamedMotifEditor::slot_displayScale()
{
    auto motif = wMotif.lock();
    if (motif)
    {
        auto connector = motif->getRadialConnector();
        if (connector && connectScale)
        {
            qreal scale    = connector->getScale();
            connectScale->setText(QString("Connector scale = %1").arg(QString::number(scale)));
        }
    }
}

void NamedMotifEditor::slot_deleteConnector()
{
    auto motif = wMotif.lock();
    if (motif)
    {
        auto connector = motif->getRadialConnector();
        if (connector)
        {
            motif->deleteRadialConnector();
            emit sig_redisplay(motif->getMotifType());
        }
    }
}

////////////////////////////////////////////////////////////////////////////
//
// Extender Editor
//
////////////////////////////////////////////////////////////////////////////

QVBoxLayout * ExtenderEditor::createExtenderLayout(NamedMotifEditor *parent)
{
    editor = parent;

    // Extender settings
    QLabel * extLabel  = new QLabel("Extend:");
    extendTipsToBoundChk = new QCheckBox("Tips to Boundary");
    extendRaysChk        = new QCheckBox("Rays to Boundary");
    embedBoundaryChk     = new QCheckBox("Embed Boundary");
    connectRaysBox       = new SpinSet(  "Connect Rays ",0,0,4);
    extendTipsToTileChk  = new QCheckBox("Tips to Tile");
    embedTileChk         = new QCheckBox("Embed Tile");

    QHBoxLayout * hbox = new QHBoxLayout;
    hbox->addWidget(extLabel);
    hbox->addWidget(extendTipsToBoundChk);
    hbox->addWidget(extendRaysChk);
    hbox->addWidget(embedBoundaryChk);
    hbox->addWidget(embedTileChk);
    hbox->addWidget(extendTipsToTileChk);
    hbox->addLayout(connectRaysBox);

    // Extended Boundary settings
    QLabel * lExt = new QLabel("Boundary : ");
    boundarySides  = new SpinSet("Sides ", 4, 1, 64);
    boundaryScale  = new DoubleSliderSet("Scale", 1.0, 0.1, 9.0, 100 );
    boundaryRotate = new DoubleSliderSet("Rotate",0.0, -360.0, 360.0, 1);

    QHBoxLayout * hbox2 = new QHBoxLayout;
    hbox2->addWidget(lExt);
    hbox2->addStretch();
    hbox2->addLayout(boundarySides);
    hbox2->addStretch();
    hbox2->addLayout(boundaryRotate);
    hbox2->addStretch();
    hbox2->addLayout(boundaryScale);

    // assemble
    QVBoxLayout * vbox  = new QVBoxLayout();
    vbox->addLayout(hbox2);
    vbox->addLayout(hbox);

    // connect
    connect(boundarySides,        &SpinSet::valueChanged,         this, [this]() { editor->editorToMotif(true);});
    connect(boundaryScale,        &DoubleSliderSet::valueChanged, this, [this]() { editor->editorToMotif(true);});
    connect(boundaryRotate,       &DoubleSliderSet::valueChanged, this, [this]() { editor->editorToMotif(true);});

    connect(extendRaysChk,        &QCheckBox::clicked,            this, [this]() { editor->editorToMotif(true);});
    connect(extendTipsToTileChk,  &QCheckBox::clicked,            this, [this]() { editor->editorToMotif(true);});
    connect(extendTipsToBoundChk, &QCheckBox::clicked,            this, [this]() { editor->editorToMotif(true);});
    connect(connectRaysBox,       &SpinSet::valueChanged,         this, [this]() { editor->editorToMotif(true);});
    connect(embedBoundaryChk,     &QCheckBox::clicked,            this, [this]() { editor->editorToMotif(true);});
    connect(embedTileChk,         &QCheckBox::clicked,            this, [this]() { editor->editorToMotif(true);});

    return vbox;
}

void ExtenderEditor::editorToExtender()
{
    auto extender = wextender.lock();
    if (!extender) return;

    int   bsides = boundarySides->value();
    qreal bscale = boundaryScale->value();
    qreal brot   = boundaryRotate->value();

    bool extendFreeVertices    = extendTipsToBoundChk->isChecked();
    bool extendMotifEdges      = extendRaysChk->isChecked();
    bool extendBoundaryToTile  = extendTipsToTileChk->isChecked();
    bool embedBoundary         = embedBoundaryChk->isChecked();
    bool embedTile             = embedTileChk->isChecked();
    uint connectBoundary       = connectRaysBox->value();

    blockSignals(true);

    ExtendedBoundary & eb = extender->getRWExtendedBoundary();
    eb.setSides(bsides);
    eb.setScale(bscale);
    eb.setRotate(brot);

    extender->setExtendTipsToBound(extendFreeVertices);
    extender->setExtendRays(extendMotifEdges);
    extender->setExtendTipsToTile(extendBoundaryToTile);
    extender->setConnectRays(connectBoundary);
    extender->setEmbedBoundary(embedBoundary);
    extender->setEmbedTile(embedTile);

    blockSignals(false);
}

void ExtenderEditor::extenderToEditor()
{
    auto extender = wextender.lock();
    if (!extender) return;

    const ExtendedBoundary & eb = extender->getExtendedBoundary();
    int    bs  = eb.getSides();
    qreal  bsc = eb.getScale();
    qreal  bro = eb.getRotate();

    bool ext_free      = extender->getExtendTipsToBound();
    bool ext_motif     = extender->getExtendRays();
    bool ext_bound     = extender->getExtendBoundaryToTile();
    uint con_bd        = extender->getConnectRays();
    bool embedBoundary = extender->getEmbedBoundary();
    bool embedTile     = extender->getEmbedTile();

    blockSignals(true);

    boundarySides->setValue(bs);
    boundaryScale->setValue(bsc);
    boundaryRotate->setValue(bro);

    extendTipsToBoundChk->setChecked(ext_free);
    extendRaysChk->setChecked(ext_motif);
    extendTipsToTileChk->setChecked(ext_bound);
    connectRaysBox->setValue(con_bd);
    embedBoundaryChk->setChecked(embedBoundary);
    embedTileChk->setChecked(embedTile);

    blockSignals(false);
}

////////////////////////////////////////////////////////////////////////////
//
// Connector Editor
//
////////////////////////////////////////////////////////////////////////////

QHBoxLayout * ConnectorEditor::createConnectorLayout(NamedMotifEditor *parent)
{
    editor = parent;

    QPushButton * deleteBtn = new QPushButton("Delete Connector");
    connectScale = new QLabel("Connect scale");

    QHBoxLayout * hbox = new QHBoxLayout();
    hbox->addWidget(connectScale);
    hbox->addStretch();
    hbox->addWidget(deleteBtn);

    QObject::connect(deleteBtn,       &QPushButton::clicked,             parent, &NamedMotifEditor::slot_deleteConnector);
    ConnectPtr con = wconnector.lock();
    if (con)
        QObject::connect(con.get(),  &MotifConnector::sig_scaleChanged,  parent, &NamedMotifEditor::slot_displayScale);

    return hbox;
}

void ConnectorEditor::editorToConnector()
{
}

void ConnectorEditor::connectorToEditor()
{
    ConnectPtr connector = wconnector.lock();
    if (connector)
    {
        qreal scale    = connector->getScale();
        connectScale->setText(QString("Connector scale = %1").arg(QString::number(scale)));
    }
}


/////////////////////////////////////////////////////////
///
/// MotifTypeChoiceCombo
///
////////////////////////////////////////////////////////

MotifTypeCombo:: MotifTypeCombo()
{
    setFixedWidth(131);

    connect(this, &QComboBox::currentIndexChanged, this,   &MotifTypeCombo::slot_motifTypeSelected);
}

void MotifTypeCombo::updateChoices(MotifPtr motif)
{
    blockSignals(true);

    clear();

    if (motif->isRadial())
    {
        add(MOTIF_TYPE_ROSETTE2,            "Rosette2 (New)");
        add(MOTIF_TYPE_ROSETTE,             "Rosette");
        add(MOTIF_TYPE_STAR2,               "Star2 (New)");
        add(MOTIF_TYPE_STAR,                "Star");
        add(MOTIF_TYPE_EXPLCIT_TILE,        "Explicit Tile");
    }
    else
    {
        add(MOTIF_TYPE_EXPLICIT_MAP,        "Explicit Map");
        add(MOTIF_TYPE_INFERRED,            "Inferred");
        add(MOTIF_TYPE_IRREGULAR_STAR,      "Irregular Star");
        add(MOTIF_TYPE_IRREGULAR_ROSETTE,   "Irregular Rosette");
        add(MOTIF_TYPE_HOURGLASS,           "Hourglass");
        add(MOTIF_TYPE_GIRIH,               "Girih");
        add(MOTIF_TYPE_INTERSECT,           "Intersect");
        add(MOTIF_TYPE_EXPLCIT_TILE,        "Explicit Tile");
        add(MOTIF_TYPE_IRREGULAR_NO_MAP,    "No Motif (Empty)");
    }

    int index = getIndex(motif->getMotifType());
    if (index >= 0)
    {
        setCurrentIndex(index);
        qDebug() << "MotifTypeCombo - set index=" << index;
    }
    else
    {
        setCurrentIndex(0);
        qDebug() << "MotifTypeCombo - forcing cindex=0";
    }

    blockSignals(false);
}

void MotifTypeCombo::add(eMotifType type, QString name)
{
    addItem(name,QVariant(type));
}

int MotifTypeCombo::getIndex(eMotifType type)
{
    qDebug().noquote()  << "type is" << Motif::getTypeString(type);

    return findData(QVariant(type));
}

void MotifTypeCombo::slot_motifTypeSelected(int index)
{
    Q_UNUSED(index);

    QVariant qv = currentData();
    eMotifType type = static_cast<eMotifType>(qv.toInt());
    qDebug() << "MotifTypeCombo type="  << sMotifType[type];

    emit sig_motifTypeChanged(type);
}

