#include <QCheckBox>

#include "motifs/extended_rosette.h"
#include "motifs/extended_star.h"
#include "motifs/rosette_connect.h"
#include "motifs/star.h"
#include "motifs/star_connect.h"
#include "makers/motif_maker/motif_button.h"
#include "makers/motif_maker/regular_motif_editors.h"
#include "makers/motif_maker/motif_maker.h"
#include "mosaic/design_element.h"
#include "panels/page_motif_maker.h"
#include "tiledpatternmaker.h"
#include "widgets/layout_sliderset.h"

using std::dynamic_pointer_cast;
using std::make_shared;

typedef std::shared_ptr<RadialMotif>    RadialPtr;

// An abstract class for containing the controls related to the editing
// of one kind of figure.  A complex hierarchy of FigureEditors gets built
// up to become the changeable controls for editing figures in FigureMaker.

NamedMotifEditor::NamedMotifEditor(page_motif_maker * fm, QString motifName)
{
    menu = fm;
    name     = motifName;

    vbox = new AQVBoxLayout();
    setLayout(vbox);

    boundarySides = new SliderSet("Boundary sides", 4, 1, 64);
    boundaryScale = new DoubleSliderSet("Boundary Scale", 1.0, 0.1, 4.0, 100 );
    motifScale   = new DoubleSliderSet("Motif Scale", 1.0, 0.1, 4.0, 100 );
    motifRotate  = new DoubleSliderSet("Motif Rotation",0.0, -360.0, 360.0, 1);

    boundaryScale->setPrecision(8);
    motifScale->setPrecision(8);

    addLayout(boundarySides);
    addLayout(boundaryScale);
    addLayout(motifScale);
    addLayout(motifRotate);

    connect(this,          &NamedMotifEditor::sig_motif_modified, menu, &page_motif_maker::slot_motifModified); //, Qt::QueuedConnection);

    connect(boundaryScale, &DoubleSliderSet::valueChanged, this, [this]() { editorToMotif(true);});
    connect(boundarySides, &SliderSet::valueChanged,       this, [this]() { editorToMotif(true);});
    connect(motifScale,    &DoubleSliderSet::valueChanged, this, [this]() { editorToMotif(true);});
    connect(motifRotate,   &DoubleSliderSet::valueChanged, this, [this]() { editorToMotif(true);});
}

void NamedMotifEditor::setMotif(MotifPtr fig, bool doEmit)
{
    Q_UNUSED(doEmit);
    wMotif = fig;
}

void NamedMotifEditor::motifToEditor()
{
    auto fig = wMotif.lock();
    if (!fig)
        return;

    const ExtendedBoundary & eb = fig->getExtendedBoundary();
    int    bs = eb.sides;
    qreal  sc = eb.scale;
    qreal  fs = fig->getMotifScale();
    qreal  rr = fig->getMotifRotate();

    blockSignals(true);
    boundarySides->setValues(bs, 1, 64);
    boundaryScale->setValues(sc, 0.1, 4.0);
    motifScale->setValues(fs, 0.1, 4.0);
    motifRotate->setValues(rr,-360.0,360.0);
    blockSignals(false);
}

void NamedMotifEditor::editorToMotif(bool doEmit)
{
    auto fig = wMotif.lock();
    if (!fig)
        return;

    int sides     = boundarySides->value();
    qreal bscale  = boundaryScale->value();
    qreal fscale  = motifScale->value();
    qreal  rot    = motifRotate->value();

    ExtendedBoundary & eb = fig->getRWExtendedBoundary();

    blockSignals(true);
    eb.sides = sides;
    eb.scale = bscale;
    fig->setMotifScale(fscale);
    fig->setMotifRotate(rot);
    blockSignals(false);

    if (doEmit)
        emit sig_motif_modified(fig);
}

StarEditor::StarEditor(page_motif_maker *fm, QString figname) : NamedMotifEditor(fm,figname)
{
    n_slider = new SliderSet("Radial Points N", 8, 3, 64);
    d_slider = new DoubleSliderSet("Star Editor Hops D", 3.0, 1.0, 10.0, 100 );
    s_slider = new SliderSet("Star Editor Intersects S", 2, 1, 5);

    addLayout(n_slider);
    addLayout(d_slider);
    addLayout(s_slider);

    connect(n_slider, &SliderSet::valueChanged,       this, [this]() { editorToMotif(true);});
    connect(d_slider, &DoubleSliderSet::valueChanged, this, [this]() { editorToMotif(true);});
    connect(s_slider, &SliderSet::valueChanged,       this, [this]() { editorToMotif(true);});
}

void StarEditor::setMotif(DesignElementPtr del, bool doEmit)
{
    if (!del || !del->getMotif())
    {
        wstar.reset();
        return;
    }

    MotifPtr oldfig = del->getMotif();
    auto oldstar = dynamic_pointer_cast<Star>(oldfig);
    if (!oldstar)
    {
        // create a star with some defaults
        int n =oldfig->getN();
        auto newstar = make_shared<Star>(*oldfig,n, n <=6 ? n / 3.0 : 3.0, 2);
        del->setMotif(newstar);
        setMotif(newstar,doEmit);
    }
    else
    {
        // always create because it could be a ConnectStar or an Extended star
        auto newstar = make_shared<Star>(*oldstar.get());
        del->setMotif(newstar);
        setMotif(newstar,doEmit);
    }
}

void StarEditor::setMotif(std::shared_ptr<Star>(star), bool doEmit)
{
    wstar = star;
    NamedMotifEditor::setMotif(star,false);

    motifToEditor();
    editorToMotif(doEmit);
}

void StarEditor::motifToEditor()
{
    auto starfig = wstar.lock();
    if (starfig)
    {
        NamedMotifEditor::motifToEditor();

        int    nn = starfig->getN();
        qreal  dd = starfig->getD();
        int    ss = starfig->getS();

        //double dmax = 0.5 * (double)nn;
        blockSignals(true);
        n_slider->setValues(nn,3,64);
        d_slider->setValues( dd, 1.0, static_cast<qreal>(nn) * 2.0);
        s_slider->setValues( ss, 1.0, nn * 2);
        blockSignals(false);
    }
}

void StarEditor::editorToMotif(bool doEmit)
{
    auto starfig = wstar.lock();
    if (starfig)
    {
        NamedMotifEditor::editorToMotif(false);

        int nval      = n_slider->value();
        qreal dval    = d_slider->value();
        int sval      = s_slider->value();

        blockSignals(true);
        starfig->setN(nval);
        starfig->setD(dval);
        starfig->setS(sval);
        starfig->resetMaps();
        blockSignals(false);

        if (doEmit)
            emit sig_motif_modified(starfig);
    }
}

// The controls for editing a Star.  Glue code, just like RosetteEditor.
// DAC - Actually the comment above is not true

RosetteEditor::RosetteEditor(page_motif_maker * fm, QString figname) : NamedMotifEditor(fm,figname)
{
    n_slider = new SliderSet("Radial Points N", 8, 3, 64);
    q_slider = new DoubleSliderSet("RosetteEditor Q (Tip Angle)", 0.0, -3.0, 3.0, 100 );
    k_slider = new DoubleSliderSet("RosetteEditor K (Neck Angle)", 0.0, -3.0, 3.0, 1000 );
    k_slider->setPrecision(4);
    s_slider = new SliderSet("RosetteEditor S (Sides Intersections)", 1, 1, 5);

    addLayout(n_slider);
    addLayout(q_slider);
    addLayout(k_slider);
    addLayout(s_slider);

    connect(n_slider, &SliderSet::valueChanged,       this, [this]() { editorToMotif(true);});
    connect(q_slider, &DoubleSliderSet::valueChanged, this, [this]() { editorToMotif(true);});
    connect(k_slider, &DoubleSliderSet::valueChanged, this, [this]() { editorToMotif(true);});
    connect(s_slider, &SliderSet::valueChanged,       this, [this]() { editorToMotif(true);});
}

void RosetteEditor::setMotif(DesignElementPtr del, bool doEmit)
{
    if (!del || !del->getMotif())
    {
        wrosette.reset();
        return;
    }

    MotifPtr oldfig = del->getMotif();
    auto oldrosette = dynamic_pointer_cast<Rosette>(oldfig);
    if (!oldrosette)
    {
        // create using defualts
        int n = oldfig->getN();
        auto rosette  = make_shared<Rosette>(*oldfig.get(), n, 0.0, 3, 0.0);
        del->setMotif(rosette);
        setMotif(rosette,doEmit);
    }
    else
    {
        // always create becuase it could be a ConnectRosette or an ExtendedRosette
        auto rosette = make_shared<Rosette>(*oldrosette.get());
        del->setMotif(rosette);
        setMotif(rosette,doEmit);
    }
}

void RosetteEditor::setMotif(std::shared_ptr<Rosette>(rosette), bool doEmit)
{
    wrosette = rosette;

    NamedMotifEditor::setMotif(rosette,false);

    motifToEditor();
    editorToMotif(doEmit);
}

void RosetteEditor::motifToEditor()
{
    auto rose = wrosette.lock();
    if (rose)
    {
        NamedMotifEditor::motifToEditor();

        int    nn = rose->getN();
        double qq = rose->getQ();
        double kk = rose->getK();
        int    ss = rose->getS();

        blockSignals(true);
        n_slider->setValues(nn,3,64);
        q_slider->setValues(qq, -3.0, 3.0);       // DAC was -1.0, 1.0
        s_slider->setValues(ss, 1.0, 5);
        k_slider->setValues(kk,-3.0, 3.0);
        blockSignals(false);
    }
}

void RosetteEditor::editorToMotif(bool doEmit)
{
    auto rose = wrosette.lock();
    if (rose)
    {
        NamedMotifEditor::editorToMotif(false);

        qreal  qval = q_slider->value();
        qreal  kval = k_slider->value();
        int    sval = s_slider->value();
        int    nval = n_slider->value();

        blockSignals(true);
        rose->setN(nval);
        rose->setQ(qval);
        rose->setS(sval);
        rose->setK(kval);
        rose->resetMaps();
        blockSignals(false);

        if (doEmit)
            emit sig_motif_modified(rose);
    }
}

// ConnectStarEditor
ConnectStarEditor::ConnectStarEditor(page_motif_maker *fm, QString figname) : StarEditor(fm,figname)
{
    defaultBtn = new QPushButton("Calc Scale");
    defaultBtn->setFixedWidth(131);

    addWidget(defaultBtn);

    QObject::connect(defaultBtn, &QPushButton::clicked,  this, &ConnectStarEditor::calcScale);
}

void ConnectStarEditor::setMotif(DesignElementPtr del, bool doEmit)
{
    if (!del || !del->getMotif())
    {
        wstarConnect.reset();
        return;
    }

    MotifPtr oldfig = del->getMotif();
    auto starc = dynamic_pointer_cast<StarConnect>(oldfig);
    if (starc)
    {
        setMotif(starc,doEmit);
    }
    else
    {
        StarPtr sp = dynamic_pointer_cast<Star>(oldfig);
        if (sp)
        {
            auto starConnect = make_shared<StarConnect>(*oldfig.get(),sp->getN(),sp->getD(),sp->getS());
            del->setMotif(starConnect);
            setMotif(starConnect,doEmit);
        }
        else
        {
            int n = oldfig->getN();
            auto starConnect = make_shared<StarConnect>(*oldfig.get(),n, n <= 6 ? n / 3.0 : 3.0, 2);
            del->setMotif(starConnect);
            setMotif(starConnect,doEmit);
        }
    }
}

void ConnectStarEditor::setMotif(std::shared_ptr<StarConnect>(starcon), bool doEmit)
{
    wstarConnect = starcon;

    StarEditor::setMotif(starcon,false);

    starcon->setMotifScale(starcon->computeConnectScale());

    motifToEditor();
    editorToMotif(doEmit);
}


void ConnectStarEditor::calcScale()
{
    auto starcon = wstarConnect.lock();
    if (starcon)
    {
        starcon->setMotifScale(starcon->computeConnectScale());
        editorToMotif(true);
    }
}

// ConnectRosetteEditor

ConnectRosetteEditor::ConnectRosetteEditor(page_motif_maker * fm, QString figname) : RosetteEditor(fm,figname)
{
    defaultBtn = new QPushButton("Calc Scale");
    defaultBtn->setFixedWidth(131);

    addWidget(defaultBtn);

    QObject::connect(defaultBtn, &QPushButton::clicked,  this, &ConnectRosetteEditor::calcScale);
}

void ConnectRosetteEditor::setMotif(DesignElementPtr del, bool doEmit)
{
    if (!del || !del->getMotif())
    {
        wrosetteConnect.reset();
        return;
    }

    MotifPtr oldfig = del->getMotif();
    auto rosettec = dynamic_pointer_cast<RosetteConnect>(oldfig);
    if (rosettec)
    {
        setMotif(rosettec,doEmit);
    }
    else
    {
        RosettePtr rsp = dynamic_pointer_cast<Rosette>(oldfig);
        if (rsp)
        {
            auto rosetteConnect = make_shared<RosetteConnect>(*oldfig.get(),rsp->getN(),rsp->getQ(),
                                                    rsp->getS(),rsp->getK());
            del->setMotif(rosetteConnect);
            setMotif(rosetteConnect,doEmit);
        }
        else
        {
            int n = oldfig->getN();
            auto rosetteConnect = make_shared<RosetteConnect>(*oldfig.get(), n, 0.0, 3, 0.0);
            del->setMotif(rosetteConnect);
            setMotif(rosetteConnect,doEmit);
        }
    }
}

void ConnectRosetteEditor::setMotif(std::shared_ptr<RosetteConnect>(rosettecon), bool doEmit)
{
    wrosetteConnect = rosettecon;

    RosetteEditor::setMotif(rosettecon,false);

    rosettecon->setMotifScale(rosettecon->computeConnectScale());

    motifToEditor();
    editorToMotif(doEmit);
}



void ConnectRosetteEditor::calcScale()
{
    auto rosecon = wrosetteConnect.lock();
    if (rosecon)
    {
        qreal scale = rosecon->computeConnectScale();
        motifScale->setValue(scale);
        editorToMotif(true);
    }
}

// ExtendedStarEditor
ExtendedStarEditor::ExtendedStarEditor(page_motif_maker *fm, QString figname) : StarEditor(fm,figname)
{
    extendPeriphBox    = new QCheckBox("Extend Peripheral Vertices");
    extendFreeBox      = new QCheckBox("Extend Free Vertices");
    connectBoundaryBox = new QCheckBox("Connect Boundary Vertices");

    addWidget(extendPeriphBox);
    addWidget(extendFreeBox);
    addWidget(connectBoundaryBox);

    connect(extendPeriphBox,    &QCheckBox::clicked,  this, [this]() { editorToMotif(true);});
    connect(extendFreeBox,      &QCheckBox::clicked,  this, [this]() { editorToMotif(true);});
    connect(connectBoundaryBox, &QCheckBox::clicked,  this, [this]() { editorToMotif(true);});

}

void ExtendedStarEditor::setMotif(DesignElementPtr del, bool doEmit)
{
    if (!del || !del->getMotif())
    {
        wextended.reset();
        return;
    }

    MotifPtr oldfig = del->getMotif();
    auto extended = dynamic_pointer_cast<ExtendedStar>(oldfig);
    if (extended)
    {
        del->setMotif(extended);
        setMotif(extended,doEmit);
    }
    else
    {
        StarPtr sp = dynamic_pointer_cast<Star>(oldfig);
        if (sp)
        {
            auto extended = make_shared<ExtendedStar>(*oldfig.get(),sp->getN(),sp->getD(),sp->getS());
            del->setMotif(extended);
            setMotif(extended,doEmit);
        }
        else
        {
            int  n = oldfig->getN();
            auto extended = make_shared<ExtendedStar>(*oldfig.get(), n, n <= 6 ? n / 3.0 : 3.0, 2);
            del->setMotif(extended);
            setMotif(extended,doEmit);
        }
    }
}

void ExtendedStarEditor::setMotif(std::shared_ptr<ExtendedStar>(extended), bool doEmit)
{
    wextended = extended;

    StarEditor::setMotif(extended,false);

    motifToEditor();
    editorToMotif(doEmit);
}


void ExtendedStarEditor::motifToEditor()
{
    auto estar = wextended.lock();
    if (estar)
    {
        auto & extender = estar->getExtender();
        bool ext_t      = extender.getExtendPeripheralVertices();
        bool ext_nt     = extender.getExtendFreeVertices();
        bool con_bd     = extender.getConnectBoundaryVertices();

        blockSignals(true);
        extendPeriphBox->setChecked(ext_t);
        extendFreeBox->setChecked(ext_nt);
        connectBoundaryBox->setChecked(con_bd);
        blockSignals(false);

        StarEditor::motifToEditor();
    }
}

void ExtendedStarEditor::editorToMotif(bool doEmit)
{
    auto estar = wextended.lock();
    if (estar)
    {
        bool extendPeripheralVertices = extendPeriphBox->isChecked();
        bool extendFreeVertices       = extendFreeBox->isChecked();
        bool connectBoundary          = connectBoundaryBox->isChecked();

        auto & extender = estar->getExtender();

        blockSignals(true);
        extender.setExtendPeripheralVertices(extendPeripheralVertices);
        extender.setExtendFreeVertices(extendFreeVertices);
        extender.setConnectBoundaryVertices(connectBoundary);
        blockSignals(false);

        StarEditor::editorToMotif(false);

        if (doEmit)
            emit sig_motif_modified(estar);
    }
}

// ExtendedRosetteEditor

ExtendedRosetteEditor::ExtendedRosetteEditor(page_motif_maker * fm, QString figname) : RosetteEditor(fm,figname)
{
    extendPeriphBox    = new QCheckBox("Extend PeripheralVertices");
    extendFreeBox      = new QCheckBox("Extend Free Vertices");
    connectBoundaryBox = new QCheckBox("Connect Boundary Vertices");

    addWidget(extendPeriphBox);
    addWidget(extendFreeBox);
    addWidget(connectBoundaryBox);

    connect(extendPeriphBox,    &QCheckBox::clicked,  this, [this]() { editorToMotif(true);});
    connect(extendFreeBox,      &QCheckBox::clicked,  this, [this]() { editorToMotif(true);});
    connect(connectBoundaryBox, &QCheckBox::clicked,  this, [this]() { editorToMotif(true);});
}

void ExtendedRosetteEditor::setMotif(DesignElementPtr del, bool doEmit)
{
    if (!del || !del->getMotif())
    {
        wextended.reset();
        return;
    }

    MotifPtr oldfig = del->getMotif();
    auto extended = dynamic_pointer_cast<ExtendedRosette>(oldfig);
    if (extended)
    {
        setMotif(extended, doEmit);
    }
    else
    {
        RosettePtr rsp = dynamic_pointer_cast<Rosette>(oldfig);
        if (rsp)
        {
            auto extended = make_shared<ExtendedRosette>(*oldfig.get(),rsp->getN(),rsp->getQ(),
                                                    rsp->getS(),rsp->getK());
            del->setMotif(extended);
            setMotif(extended, doEmit);
        }
        else
        {
            int n = oldfig->getN();
            auto extended = make_shared<ExtendedRosette>(*oldfig.get(), n, 0.0, 3, 0);
            del->setMotif(extended);
            setMotif(extended, doEmit);
        }
    }
}

void ExtendedRosetteEditor::setMotif(std::shared_ptr<ExtendedRosette>(extended), bool doEmit)
{
    wextended = extended;

    RosetteEditor::setMotif(extended,false);

    motifToEditor();
    editorToMotif(doEmit);
}


void ExtendedRosetteEditor::motifToEditor()
{
    auto erose = wextended.lock();
    if (erose)
    {
        auto & extender = erose->getExtender();
        bool ext_t      = extender.getExtendPeripheralVertices();
        bool ext_nt     = extender.getExtendFreeVertices();
        bool con_bd     = extender.getConnectBoundaryVertices();

        blockSignals(true);
        extendPeriphBox->setChecked(ext_t);
        extendFreeBox->setChecked(ext_nt);
        connectBoundaryBox->setChecked(con_bd);
        blockSignals(false);

        RosetteEditor::motifToEditor();
    }
}

void ExtendedRosetteEditor::editorToMotif(bool doEmit)
{
    auto erose = wextended.lock();
    if (erose)
    {
        bool extendPeripherals  = extendPeriphBox->isChecked();
        bool extendFreeVertices = extendFreeBox->isChecked();
        bool connectBoundary    = connectBoundaryBox->isChecked();

        blockSignals(true);
        auto & extender = erose->getExtender();
        extender.setExtendPeripheralVertices(extendPeripherals);
        extender.setExtendFreeVertices(extendFreeVertices);
        extender.setConnectBoundaryVertices(connectBoundary);
        blockSignals(false);

        RosetteEditor::editorToMotif(false);

        if (doEmit)
            emit sig_motif_modified(erose);
    }
}
