#include <QCheckBox>

#include "figures/extended_rosette.h"
#include "figures/extended_star.h"
#include "figures/rosette_connect_figure.h"
#include "figures/star.h"
#include "figures/star_connect_figure.h"
#include "makers/motif_maker/feature_button.h"
#include "makers/motif_maker/figure_editors.h"
#include "makers/motif_maker/motif_maker.h"
#include "mosaic/design_element.h"
#include "panels/page_motif_maker.h"
#include "tiledpatternmaker.h"
#include "widgets/layout_sliderset.h"

using std::dynamic_pointer_cast;
using std::make_shared;

typedef std::shared_ptr<RadialFigure>    RadialPtr;

// An abstract class for containing the controls related to the editing
// of one kind of figure.  A complex hierarchy of FigureEditors gets built
// up to become the changeable controls for editing figures in FigureMaker.

FigureEditor::FigureEditor(page_motif_maker * fm, QString figname)
{
    menu = fm;
    name     = figname;

    vbox = new AQVBoxLayout();
    setLayout(vbox);

    boundarySides = new SliderSet("Boundary sides", 4, 1, 64);
    boundaryScale = new DoubleSliderSet("Boundary Scale", 1.0, 0.1, 4.0, 100 );
    figureScale   = new DoubleSliderSet("Figure Scale", 1.0, 0.1, 4.0, 100 );
    figureRotate  = new DoubleSliderSet("Figure Rotation",0.0, -360.0, 360.0, 1);

    boundaryScale->setPrecision(8);
    figureScale->setPrecision(8);

    addLayout(boundarySides);
    addLayout(boundaryScale);
    addLayout(figureScale);
    addLayout(figureRotate);

    connect(this,          &FigureEditor::sig_figure_modified, menu, &page_motif_maker::slot_figureModified); //, Qt::QueuedConnection);

    connect(boundaryScale, &DoubleSliderSet::valueChanged, this, [this]() { editorToFigure(true);});
    connect(boundarySides, &SliderSet::valueChanged,       this, [this]() { editorToFigure(true);});
    connect(figureScale,   &DoubleSliderSet::valueChanged, this, [this]() { editorToFigure(true);});
    connect(figureRotate,  &DoubleSliderSet::valueChanged, this, [this]() { editorToFigure(true);});
}

void FigureEditor::setFigure(FigurePtr fig, bool doEmit)
{
    Q_UNUSED(doEmit);
    wfigure = fig;
}

void FigureEditor::figureToEditor()
{
    auto fig = wfigure.lock();
    if (!fig)
        return;

    int    bs = fig->getExtBoundarySides();
    qreal  sc = fig->getExtBoundaryScale();
    qreal  fs = fig->getFigureScale();
    qreal  rr = fig->getFigureRotate();

    blockSignals(true);
    boundarySides->setValues(bs, 1, 64);
    boundaryScale->setValues(sc, 0.1, 4.0);
    figureScale->setValues(fs, 0.1, 4.0);
    figureRotate->setValues(rr,-360.0,360.0);
    blockSignals(false);
}

void FigureEditor::editorToFigure(bool doEmit)
{
    auto fig = wfigure.lock();
    if (!fig)
        return;

    int sides     = boundarySides->value();
    qreal bscale  = boundaryScale->value();
    qreal fscale  = figureScale->value();
    qreal  rot    = figureRotate->value();

    blockSignals(true);
    fig->setExtBoundarySides(sides);
    fig->setExtBoundaryScale(bscale);
    fig->setFigureScale(fscale);
    fig->setFigureRotate(rot);
    blockSignals(false);

    if (doEmit)
        emit sig_figure_modified(fig);
}

StarEditor::StarEditor(page_motif_maker *fm, QString figname) : FigureEditor(fm,figname)
{
    n_slider = new SliderSet("Radial Points N", 8, 3, 64);
    d_slider = new DoubleSliderSet("Star Editor Hops D", 3.0, 1.0, 10.0, 100 );
    s_slider = new SliderSet("Star Editor Intersects S", 2, 1, 5);

    addLayout(n_slider);
    addLayout(d_slider);
    addLayout(s_slider);

    connect(n_slider, &SliderSet::valueChanged,       this, [this]() { editorToFigure(true);});
    connect(d_slider, &DoubleSliderSet::valueChanged, this, [this]() { editorToFigure(true);});
    connect(s_slider, &SliderSet::valueChanged,       this, [this]() { editorToFigure(true);});
}

void StarEditor::setFigure(DesignElementPtr del, bool doEmit)
{
    if (!del || !del->getFigure())
    {
        wstar.reset();
        return;
    }

    FigurePtr fig = del->getFigure();
    wstar = dynamic_pointer_cast<Star>(fig);
    if (!wstar.lock())
    {
        int n = fig->getN();
        auto star = make_shared<Star>(*fig.get(), n, n <=6 ? n / 3.0 : 3.0, 2);
        del->setFigure(star);
        wstar = star;
    }

    Q_ASSERT(wstar.lock());
    FigureEditor::setFigure(wstar.lock(),false);

    figureToEditor();
    editorToFigure(doEmit);
}

void StarEditor::figureToEditor()
{
    auto starfig = wstar.lock();
    if (starfig)
    {
        FigureEditor::figureToEditor();

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

void StarEditor::editorToFigure(bool doEmit)
{
    auto starfig = wstar.lock();
    if (starfig)
    {
        FigureEditor::editorToFigure(false);

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
            emit sig_figure_modified(starfig);
    }
}

// The controls for editing a Star.  Glue code, just like RosetteEditor.
// DAC - Actually the comment above is not true

RosetteEditor::RosetteEditor(page_motif_maker * fm, QString figname) : FigureEditor(fm,figname)
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

    connect(n_slider, &SliderSet::valueChanged,       this, [this]() { editorToFigure(true);});
    connect(q_slider, &DoubleSliderSet::valueChanged, this, [this]() { editorToFigure(true);});
    connect(k_slider, &DoubleSliderSet::valueChanged, this, [this]() { editorToFigure(true);});
    connect(s_slider, &SliderSet::valueChanged,       this, [this]() { editorToFigure(true);});
}

void RosetteEditor::setFigure(DesignElementPtr del, bool doEmit)
{
    if (!del || !del->getFigure())
    {
        wrosette.reset();
        return;
    }

    FigurePtr fig = del->getFigure();
    wrosette = dynamic_pointer_cast<Rosette>(fig);
    if (!wrosette.lock())
    {
        int n = fig->getN();
        auto rosette  = make_shared<Rosette>(*fig.get(), n, 0.0, 3, 0.0);
        del->setFigure(rosette);
        wrosette = rosette;
    }

    Q_ASSERT(wrosette.lock());
    FigureEditor::setFigure(wrosette.lock(),false);
    figureToEditor();
    editorToFigure(doEmit);
}

void RosetteEditor::figureToEditor()
{
    auto rose = wrosette.lock();
    if (rose)
    {
        FigureEditor::figureToEditor();

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

void RosetteEditor::editorToFigure(bool doEmit)
{
    auto rose = wrosette.lock();
    if (rose)
    {
        FigureEditor::editorToFigure(false);

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
            emit sig_figure_modified(rose);
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

void ConnectStarEditor::setFigure(DesignElementPtr del, bool doEmit)
{
    if (!del || !del->getFigure())
    {
        wstarConnect.reset();
        return;
    }

    FigurePtr fig = del->getFigure();
    wstarConnect = dynamic_pointer_cast<StarConnectFigure>(fig);
    if (!wstarConnect.lock())
    {
        StarPtr sp = dynamic_pointer_cast<Star>(fig);
        if (sp)
        {
            auto starConnect = make_shared<StarConnectFigure>(*fig.get(),sp->getN(),sp->getD(),sp->getS());
            del->setFigure(starConnect);
            wstarConnect = starConnect;
        }
        else
        {
            int n = fig->getN();
            auto starConnect = make_shared<StarConnectFigure>(*fig.get(),n, n <= 6 ? n / 3.0 : 3.0, 2);
            del->setFigure(starConnect);
            wstarConnect = starConnect;
        }
    }

    Q_ASSERT(wstarConnect.lock());
    StarEditor::setFigure(del,false);

    wstarConnect.lock()->setFigureScale(wstarConnect.lock()->computeConnectScale());

    figureToEditor();
    editorToFigure(doEmit);
}

void ConnectStarEditor::calcScale()
{
    auto starcon = wstarConnect.lock();
    if (starcon)
    {
        qreal scale = starcon->computeConnectScale();
        figureScale->setValue(scale);
        editorToFigure(true);
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

void ConnectRosetteEditor::setFigure(DesignElementPtr del, bool doEmit)
{
    if (!del || !del->getFigure())
    {
        wrosetteConnect.reset();
        return;
    }

    FigurePtr fig = del->getFigure();
    wrosetteConnect = dynamic_pointer_cast<RosetteConnectFigure>(fig);
    if (!wrosetteConnect.lock())
    {

        RosettePtr rsp = dynamic_pointer_cast<Rosette>(fig);
        if (rsp)
        {
            auto rosetteConnect = make_shared<RosetteConnectFigure>(*fig.get(),rsp->getN(),rsp->getQ(),
                                                    rsp->getS(),rsp->getK());
            del->setFigure(rosetteConnect);
            wrosetteConnect = rosetteConnect;
        }
        else
        {
            int n = fig->getN();
            auto rosetteConnect = make_shared<RosetteConnectFigure>(* fig.get(), n, 0.0, 3, 0.0);
            del->setFigure(rosetteConnect);
            wrosetteConnect = rosetteConnect;
        }
    }

    Q_ASSERT(wrosetteConnect.lock());
    RosetteEditor::setFigure(del,false);

    wrosetteConnect.lock()->setFigureScale(wrosetteConnect.lock()->computeConnectScale());

    figureToEditor();
    editorToFigure(doEmit);
}

void ConnectRosetteEditor::calcScale()
{
    auto rosecon = wrosetteConnect.lock();
    if (rosecon)
    {
        qreal scale = rosecon->computeConnectScale();
        figureScale->setValue(scale);
        editorToFigure(true);
    }
}

// ExtendedStarEditor
ExtendedStarEditor::ExtendedStarEditor(page_motif_maker *fm, QString figname) : StarEditor(fm,figname)
{
    extendBox1 = new QCheckBox("Extend Peripheral Vertices");
    extendBox2 = new QCheckBox("Extend Free Vertices");

    addWidget(extendBox1);
    addWidget(extendBox2);

    connect(extendBox1,    &QCheckBox::clicked,  this, [this]() { editorToFigure(true);});
    connect(extendBox2,    &QCheckBox::clicked,  this, [this]() { editorToFigure(true);});
}

void ExtendedStarEditor::setFigure(DesignElementPtr del, bool doEmit)
{
    if (!del || !del->getFigure())
    {
        wextended.reset();
        return;
    }

    FigurePtr fig = del->getFigure();
    wextended = dynamic_pointer_cast<ExtendedStar>(fig);
    if (!wextended.lock())
    {
        StarPtr sp = dynamic_pointer_cast<Star>(fig);
        if (sp)
        {
            auto extended = make_shared<ExtendedStar>(*fig.get(),sp->getN(),sp->getD(),sp->getS());
            del->setFigure(extended);
            wextended = extended;
        }
        else
        {
            int  n = fig->getN();
            auto extended = make_shared<ExtendedStar>(*fig.get(), n, n <= 6 ? n / 3.0 : 3.0, 2);
            del->setFigure(extended);
            wextended = extended;
        }
    }

    Q_ASSERT(wextended.lock());
    StarEditor::setFigure(del,false);

    figureToEditor();
    editorToFigure(doEmit);
}

void ExtendedStarEditor::figureToEditor()
{
    auto estar = wextended.lock();
    if (estar)
    {
        bool ext_t   = estar->getExtendPeripheralVertices();
        bool ext_nt  = estar->getExtendFreeVertices();

        blockSignals(true);
        extendBox1->setChecked(ext_t);
        extendBox2->setChecked(ext_nt);
        blockSignals(false);

        StarEditor::figureToEditor();
    }
}

void ExtendedStarEditor::editorToFigure(bool doEmit)
{
    auto estar = wextended.lock();
    if (estar)
    {
        bool extendPeripheralVertices = extendBox1->isChecked();
        bool extendFreeVertices       = extendBox2->isChecked();

        blockSignals(true);
        estar->setExtendPeripheralVertices(extendPeripheralVertices);
        estar->setExtendFreeVertices(extendFreeVertices);
        blockSignals(false);

        StarEditor::editorToFigure(false);

        if (doEmit)
            emit sig_figure_modified(estar);
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

    connect(extendPeriphBox,    &QCheckBox::clicked,       this, [this]() { editorToFigure(true);});
    connect(extendFreeBox,      &QCheckBox::clicked,       this, [this]() { editorToFigure(true);});
    connect(connectBoundaryBox, &QCheckBox::clicked,       this, [this]() { editorToFigure(true);});
}

void ExtendedRosetteEditor::setFigure(DesignElementPtr del, bool doEmit)
{
    if (!del || !del->getFigure())
    {
        wextended.reset();
        return;
    }

    FigurePtr fig = del->getFigure();
    wextended = dynamic_pointer_cast<ExtendedRosette>(fig);
    if (!wextended.lock())
    {
        RosettePtr rsp = dynamic_pointer_cast<Rosette>(fig);
        if (rsp)
        {
            auto extended = make_shared<ExtendedRosette>(*fig.get(),rsp->getN(),rsp->getQ(),
                                                    rsp->getS(),rsp->getK());
            del->setFigure(extended);
            wextended = extended;
        }
        else
        {
            int n = fig->getN();
            auto extended = make_shared<ExtendedRosette>(*fig.get(), n, 0.0, 3, 0);
            del->setFigure(extended);
            wextended = extended;
        }
    }

    Q_ASSERT(wextended.lock());
    RosetteEditor::setFigure(del,false);

    figureToEditor();
    editorToFigure(doEmit);
}


void ExtendedRosetteEditor::figureToEditor()
{
    auto erose = wextended.lock();
    if (erose)
    {
        bool ext_t   = erose->getExtendPeripheralVertices();
        bool ext_nt  = erose->getExtendFreeVertices();
        bool con_bd  = erose->getConnectBoundaryVertices();

        blockSignals(true);
        extendPeriphBox->setChecked(ext_t);
        extendFreeBox->setChecked(ext_nt);
        connectBoundaryBox->setChecked(con_bd);
        blockSignals(false);

        RosetteEditor::figureToEditor();
    }
}

void ExtendedRosetteEditor::editorToFigure(bool doEmit)
{
    auto erose = wextended.lock();
    if (erose)
    {
        bool extendPeripherals  = extendPeriphBox->isChecked();
        bool extendFreeVertices = extendFreeBox->isChecked();
        bool connectBoundary    = connectBoundaryBox->isChecked();

        blockSignals(true);
        erose->setExtendPeripheralVertices(extendPeripherals);
        erose->setExtendFreeVertices(extendFreeVertices);
        erose->setConnectBoundaryVertices(connectBoundary);
        blockSignals(false);

        RosetteEditor::editorToFigure(false);

        if (doEmit)
            emit sig_figure_modified(erose);
    }
}
