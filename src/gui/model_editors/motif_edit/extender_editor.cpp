#include <QDebug>
#include <QGroupBox>

#include "gui/model_editors/motif_edit/extender_editor.h"
#include "gui/model_editors/motif_edit/motif_editor.h"
#include "gui/model_editors/motif_edit/radial_motif_editors.h"
#include "gui/widgets/layout_sliderset.h"

////////////////////////////////////////////////////////////////////////////
//
// Extender Editor
//
////////////////////////////////////////////////////////////////////////////

QVBoxLayout * ExtenderEditor::createExtenderLayout(MotifEditor *parent)
{
    editor = parent;

    // Extender settings
    QLabel * extLabel    = new QLabel("Extensions : ");
    extendTipsToBoundChk = new QCheckBox("Tips to Boundary");
    extendRaysChk        = new QCheckBox("Rays to Boundary");
    embedBoundaryChk     = new QCheckBox("Embed Boundary");
    extendTipsToTileChk  = new QCheckBox("Tips to Tile");
    embedTileChk         = new QCheckBox("Embed Tile");

    QHBoxLayout * hbox   = new QHBoxLayout;
    hbox->setSpacing(5);
    hbox->addWidget(extLabel);
    hbox->addWidget(extendTipsToBoundChk);
    hbox->addWidget(extendRaysChk);
    hbox->addWidget(extendTipsToTileChk);
    hbox->addWidget(embedBoundaryChk);
    hbox->addWidget(embedTileChk);

    // connect rays
    connectRaysBox       = new SpinSet(  "Ray-Ray",0,0,99,false,true,true);

    // stellation
    stellationBox        = new SpinSet(  "Stellation",0,0,6,false,true,true);
    fitChk               = new QCheckBox("Fit to tile");

    QHBoxLayout * hbox3  = new QHBoxLayout;
    hbox3->addLayout(connectRaysBox);
    hbox3->addStretch();
    hbox3->addLayout(stellationBox);
    hbox3->addWidget(fitChk);

    // Extended Boundary settings
    QLabel * lExt  = new QLabel("Extended Boundary : ");
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
    vbox->addLayout(hbox3);

    // connect
    connect(boundarySides,        &SpinSet::valueChanged,         this, [this]() { editor->editorToMotif(true);});
    connect(boundaryScale,        &DoubleSliderSet::valueChanged, this, [this]() { editor->editorToMotif(true);});
    connect(boundaryRotate,       &DoubleSliderSet::valueChanged, this, [this]() { editor->editorToMotif(true);});

    connect(extendRaysChk,        &QCheckBox::clicked,            this, [this]() { editor->editorToMotif(true);});
    connect(extendTipsToTileChk,  &QCheckBox::clicked,            this, [this]() { editor->editorToMotif(true);});
    connect(extendTipsToBoundChk, &QCheckBox::clicked,            this, [this]() { editor->editorToMotif(true);});
    connect(connectRaysBox,       &SpinSet::valueChanged,         this, [this]() { editor->editorToMotif(true);});
    connect(stellationBox,        &SpinSet::valueChanged,         this, [this]() { editor->editorToMotif(true);});
    connect(embedBoundaryChk,     &QCheckBox::clicked,            this, [this]() { editor->editorToMotif(true);});
    connect(embedTileChk,         &QCheckBox::clicked,            this, [this]() { editor->editorToMotif(true);});
    connect(fitChk,               &QCheckBox::clicked,            this, [this]() { editor->editorToMotif(true);});

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
    uint stellation            = stellationBox->value();
    bool fitToTile             = fitChk->isChecked();

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
    extender->setStellate(stellation);
    extender->setFitToTile(fitToTile);

    blockSignals(false);
}

void ExtenderEditor::refresh()
{
    ExtenderPtr extender = wextender.lock();
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
    uint stellation    = extender->getStellation();
    bool fitToTile     = extender->getFitToTile();

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
    stellationBox->setValue(stellation);
    fitChk->setChecked(fitToTile);

    blockSignals(false);
}

