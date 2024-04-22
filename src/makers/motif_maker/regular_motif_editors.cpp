#include <QCheckBox>

#include "motifs/rosette.h"
#include "motifs/rosette2.h"
#include "motifs/extended_rosette.h"
#include "motifs/extended_star.h"
#include "motifs/rosette_connect.h"
#include "motifs/star.h"
#include "motifs/star2.h"
#include "motifs/star_connect.h"
#include "makers/motif_maker/design_element_button.h"
#include "makers/motif_maker/regular_motif_editors.h"
#include "mosaic/design_element.h"
#include "panels/page_motif_maker.h"
#include "tiledpatternmaker.h"
#include "widgets/layout_sliderset.h"

using std::dynamic_pointer_cast;
using std::make_shared;

typedef std::shared_ptr<RadialMotif>    RadialPtr;

////////////////////////////////////////////////////////
//
// StarEditor
//
////////////////////////////////////////////////////////
StarEditor::StarEditor(QString name) : NamedMotifEditor(name)
{
    d_slider = new DoubleSliderSet("Star Editor Hops D", 3.0, 0.0, 10.0, 100 );
    s_slider = new SliderSet("Star Editor Intersects S", 2, 1, 5);
    version_combo = new QComboBox();
    version_combo->addItem("Version 1",1);
    version_combo->addItem("Version 2",2);
    version_combo->setFixedWidth(91);

    addLayout(d_slider);
    addLayout(s_slider);

    QHBoxLayout * hbox = new QHBoxLayout;
    hbox->addStretch();
    hbox->addWidget(version_combo);
    hbox->addStretch();
    addLayout(hbox);

    connect(d_slider, &DoubleSliderSet::valueChanged, this, [this]() { editorToMotif(true);});
    connect(s_slider, &SliderSet::valueChanged,       this, [this]() { editorToMotif(true);});
    connect(version_combo,  QOverload<int>::of(&QComboBox::currentIndexChanged), this,[this]() { editorToMotif(true);});
}

void StarEditor::setMotif(DesignElementPtr del, bool doEmit)
{
    wDel = del;
    if (!del || !del->getMotif())
    {
        wstar.reset();
        return;
    }

    MotifPtr oldMotif = del->getMotif();
    auto oldstar = dynamic_pointer_cast<Star>(oldMotif);
    if (!oldstar)
    {
        // create a star with some defaults
        int n =oldMotif->getN();
        auto newstar = make_shared<Star>(*oldMotif,n, n <=6 ? n / 3.0 : 3.0, 2);
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
    auto star = wstar.lock();
    if (star)
    {
        NamedMotifEditor::motifToEditor();

        qreal  dd = star->getD();
        int    ss = star->getS();

        //double dmax = 0.5 * (double)nn;
        blockSignals(true);
        //d_slider->setValues( dd, 1.0, static_cast<qreal>(star->getN()) * 2.0);
        //s_slider->setValues( ss, 1.0, star->getN() * 2);
        d_slider->setValue( dd);
        s_slider->setValue( ss);
        blockSignals(false);

        int ver = star->getVersion();
        int index = version_combo->findData(ver);
        version_combo->blockSignals(true);
        version_combo->setCurrentIndex(index);
        version_combo->blockSignals(false);
    }
}

void StarEditor::editorToMotif(bool doEmit)
{
    auto star = wstar.lock();
    if (star)
    {
        NamedMotifEditor::editorToMotif(false);

        qreal dval = d_slider->value();
        int sval   = s_slider->value();
        int ver    = version_combo->currentData().toInt();

        star->setD(dval);
        star->setS(sval);
        star->setVersion(ver);
        star->resetMotifMaps();

        if (doEmit)
            emit sig_motif_modified(star);
    }
}

////////////////////////////////////////////////////////
//
// Star2Editor
//
////////////////////////////////////////////////////////
Star2Editor::Star2Editor(QString name) : NamedMotifEditor(name)
{
    theta_slider = new DoubleSliderSet("Star Editor Angle theta", 45, 0.0, 90.0, 10 );
    s_slider     = new SliderSet("Star Editor Intersects S", 2, 1, 5);

    addLayout(theta_slider);
    addLayout(s_slider);

    QHBoxLayout * hbox = new QHBoxLayout;
    hbox->addStretch();
    hbox->addStretch();
    addLayout(hbox);

    connect(theta_slider, &DoubleSliderSet::valueChanged, this, [this]() { editorToMotif(true);});
    connect(s_slider,     &SliderSet::valueChanged,       this, [this]() { editorToMotif(true);});
}

void Star2Editor::setMotif(DesignElementPtr del, bool doEmit)
{
    wDel = del;
    if (!del || !del->getMotif())
    {
        wstar.reset();
        return;
    }

    MotifPtr oldMotif = del->getMotif();
    auto oldstar = dynamic_pointer_cast<Star2>(oldMotif);
    if (!oldstar)
    {
        // create a star with some defaults
        int n =oldMotif->getN();
        auto newstar = make_shared<Star2>(*oldMotif,n, 45.0, 1);
        del->setMotif(newstar);
        setMotif(newstar,doEmit);
    }
    else
    {
        // always create because it could be a ConnectStar or an Extended star
        auto newstar = make_shared<Star2>(*oldstar.get());
        del->setMotif(newstar);
        setMotif(newstar,doEmit);
    }
}

void Star2Editor::setMotif(std::shared_ptr<Star2>(star), bool doEmit)
{
    wstar = star;
    NamedMotifEditor::setMotif(star,false);

    motifToEditor();
    editorToMotif(doEmit);
}

void Star2Editor::motifToEditor()
{
    auto star = wstar.lock();
    if (star)
    {
        NamedMotifEditor::motifToEditor();

        qreal theta = star->getTheta();
        int   ss    = star->getS();

        blockSignals(true);
        theta_slider->setValue(theta);
        s_slider->setValue(ss);
        blockSignals(false);
    }
}

void Star2Editor::editorToMotif(bool doEmit)
{
    auto star = wstar.lock();
    if (star)
    {
        NamedMotifEditor::editorToMotif(false);

        qreal thetaval = theta_slider->value();
        int sval       = s_slider->value();

        star->setTheta(thetaval);
        star->setS(sval);
        star->resetMotifMaps();

        if (doEmit)
            emit sig_motif_modified(star);
    }
}

////////////////////////////////////////////////////////
//
// ResetteEditor
//
// The controls for editing a Star.  Glue code, just like RosetteEditor.
// DAC - Actually the comment above is not true
//
////////////////////////////////////////////////////////
RosetteEditor::RosetteEditor(QString name) : NamedMotifEditor(name)
{
    q_slider = new DoubleSliderSet("RosetteEditor Q (Tip Angle)", 0.0, -3.0, 3.0, 100 );
    s_slider = new SliderSet("RosetteEditor S (Sides Intersections)", 1, 1, 5);

    addLayout(q_slider);
    addLayout(s_slider);

    connect(q_slider, &DoubleSliderSet::valueChanged, this, [this]() { editorToMotif(true);});
    connect(s_slider, &SliderSet::valueChanged,       this, [this]() { editorToMotif(true);});
}

void RosetteEditor::setMotif(DesignElementPtr del, bool doEmit)
{
    wDel = del;
    if (!del || !del->getMotif())
    {
        wrosette.reset();
        return;
    }

    MotifPtr oldMotif = del->getMotif();
    auto oldrosette = dynamic_pointer_cast<Rosette>(oldMotif);
    if (!oldrosette)
    {
        // create using defualts
        int n = oldMotif->getN();
        auto rosette  = make_shared<Rosette>(*oldMotif.get(), n, 0.0, 3);
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

void RosetteEditor::setMotif(RosettePtr rosette, bool doEmit)
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

        double qq = rose->getQ();
        int    ss = rose->getS();

        blockSignals(true);
        q_slider->setValues(qq, -3.0, 3.0);       // DAC was -1.0, 1.0
        s_slider->setValues(ss, 1.0, 5);
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
        int    sval = s_slider->value();

        blockSignals(true);
        rose->setQ(qval);
        rose->setS(sval);
        rose->resetMotifMaps();
        blockSignals(false);

        if (doEmit)
            emit sig_motif_modified(rose);
    }
}

////////////////////////////////////////////////////////
//
// Resette2Editor
//
////////////////////////////////////////////////////////
Rosette2Editor::Rosette2Editor(QString name) : NamedMotifEditor(name)
{
    kx_slider = new DoubleSliderSet("Rosette2 KneeX height", 0.25, 0, 1.0, 100 );
    ky_slider = new DoubleSliderSet("Rosette2 KneeY width ", 0.25, 0, 1.0, 100 );
    k_slider  = new DoubleSliderSet("Rosette2 K (Knee Angle)", 0.0, -90.0, 90.0, 10);
    s_slider  = new SliderSet(      "Rosette2 S Intersections", 1, 1, 5);

    QRadioButton * rOuter = new QRadioButton("Outwards");
    QRadioButton * rInner = new QRadioButton("Inwards");
    QRadioButton * rAlter = new QRadioButton("Alternating");
    QLabel       * label  = new QLabel("Tip Direction :");
                kaplanize = new QCheckBox("Kaplan's constraint");
                  convert = new QPushButton("Convert");

    QHBoxLayout * hbox = new QHBoxLayout;
    hbox->addStretch();
    hbox->addWidget(label);
    hbox->addSpacing(11);
    hbox->addWidget(rOuter);
    hbox->addWidget(rInner);
    hbox->addWidget(rAlter);
    hbox->addSpacing(31);
    hbox->addWidget(kaplanize);
    hbox->addSpacing(5);
    hbox->addWidget(convert);
    hbox->addStretch();

    addLayout(kx_slider);
    addLayout(ky_slider);
    addLayout(k_slider);
    addLayout(s_slider);
    addLayout(hbox);

    tipGroup = new QButtonGroup;
    tipGroup->addButton(rOuter,TIP_TYPE_OUTER);
    tipGroup->addButton(rInner,TIP_TYPE_INNER);
    tipGroup->addButton(rAlter,TIP_TYPE_ALTERNATE);

    connect(kx_slider, &DoubleSliderSet::valueChanged, this, [this]() { editorToMotif(true);});
    connect(ky_slider, &DoubleSliderSet::valueChanged, this, [this]() { editorToMotif(true);});
    connect(k_slider,  &DoubleSliderSet::valueChanged, this, [this]() { editorToMotif(true);});
    connect(s_slider,  &SliderSet::valueChanged,       this, [this]() { editorToMotif(true);});
    connect(tipGroup,  &QButtonGroup::idClicked,       this, [this]() { editorToMotif(true);});
    connect(kaplanize, &QCheckBox::clicked,            this, [this]() { editorToMotif(true);});
    connect(convert,   &QPushButton::clicked,          this, &Rosette2Editor::convertConstrained);
}

void Rosette2Editor::setMotif(DesignElementPtr del, bool doEmit)
{
    wDel = del;
    if (!del || !del->getMotif())
    {
        wrosette.reset();
        return;
    }

    MotifPtr oldMotif = del->getMotif();
    auto oldrosette = dynamic_pointer_cast<Rosette2>(oldMotif);
    if (!oldrosette)
    {
        // create using defualts
        int n = oldMotif->getN();
        auto rosette  = make_shared<Rosette2>(*oldMotif.get(), n, 0.25,0.25,2,0.0, false);
        del->setMotif(rosette);
        setMotif(rosette,doEmit);
    }
    else
    {
        // always create becuase it could be a ConnectRosette or an ExtendedRosette
        auto rosette = make_shared<Rosette2>(*oldrosette.get());
        del->setMotif(rosette);
        setMotif(rosette,doEmit);
    }
}

void Rosette2Editor::setMotif(Rosette2Ptr rosette, bool doEmit)
{
    wrosette = rosette;

    NamedMotifEditor::setMotif(rosette,false);

    motifToEditor();
    editorToMotif(doEmit);
}

void Rosette2Editor::motifToEditor()
{
    auto rose = wrosette.lock();
    if (rose)
    {
        NamedMotifEditor::motifToEditor();

        qreal  x  = rose->getKneeX();
        qreal  y  = rose->getKneeY();
        qreal  k  = rose->getK();
        int    ss = rose->getS();
        eTipType tt    = rose->getTipType();
        bool constrain = rose->getConstrain();

        convert->setVisible(constrain);

        blockSignals(true);
        kx_slider->setValue(x);
        ky_slider->setValue(y);
        k_slider->setValue(k);
        s_slider->setValue(ss);
        tipGroup->button(tt)->setChecked(true);
        kaplanize->setChecked(constrain);
        blockSignals(false);
    }
}

void Rosette2Editor::editorToMotif(bool doEmit)
{
    auto rose = wrosette.lock();
    if (rose)
    {
        NamedMotifEditor::editorToMotif(false);

        qreal  x = kx_slider->value();
        qreal  y = ky_slider->value();
        qreal  k = k_slider->value();
        int    s = s_slider->value();
        eTipType tt = static_cast<eTipType>(tipGroup->checkedId());
        bool   c = kaplanize->isChecked();

        convert->setVisible(c);

        rose->setKneeX(x);
        rose->setKneeY(y);
        rose->setK(k);
        rose->setS(s);
        rose->setTipType(tt);
        rose->setConstrain(c);
        rose->resetMotifMaps();

        if (doEmit)
            emit sig_motif_modified(rose);
    }
}

void Rosette2Editor::convertConstrained()
{
    auto rose = wrosette.lock();
    if (rose)
    {
        if (rose->convertConstrained())
        {
            motifToEditor();
            editorToMotif(true);
        }
    }
}

////////////////////////////////////////////////////////
//
// ConnectStarEditor
//
////////////////////////////////////////////////////////
ConnectStarEditor::ConnectStarEditor(QString figname) : StarEditor(figname)
{
    defaultBtn = new QPushButton("Calc Scale");
    defaultBtn->setFixedWidth(131);

    addWidget(defaultBtn);

    QObject::connect(defaultBtn, &QPushButton::clicked,  this, &ConnectStarEditor::calcScale);
}

void ConnectStarEditor::setMotif(DesignElementPtr del, bool doEmit)
{
    wDel = del;
    if (!del || !del->getMotif())
    {
        wstarConnect.reset();
        return;
    }

    MotifPtr oldMotif = del->getMotif();
    auto starc = dynamic_pointer_cast<StarConnect>(oldMotif);
    if (starc)
    {
        setMotif(starc,doEmit);
    }
    else
    {
        StarPtr sp = dynamic_pointer_cast<Star>(oldMotif);
        if (sp)
        {
            auto starConnect = make_shared<StarConnect>(*oldMotif.get(),sp->getN(),sp->getD(),sp->getS());
            del->setMotif(starConnect);
            setMotif(starConnect,doEmit);
        }
        else
        {
            int n = oldMotif->getN();
            auto starConnect = make_shared<StarConnect>(*oldMotif.get(),n, n <= 6 ? n / 3.0 : 3.0, 2);
            del->setMotif(starConnect);
            setMotif(starConnect,doEmit);
        }
    }
}

void ConnectStarEditor::setMotif(std::shared_ptr<StarConnect>(starcon), bool doEmit)
{
    wstarConnect = starcon;

    StarEditor::setMotif(starcon,false);

    Q_ASSERT(starcon->getTile());
    starcon->setMotifScale(starcon->computeConnectScale());

    motifToEditor();
    editorToMotif(doEmit);
}

void ConnectStarEditor::calcScale()
{
    auto starcon = wstarConnect.lock();
    if (starcon)
    {
        Q_ASSERT(starcon->getTile());
        starcon->setMotifScale(starcon->computeConnectScale());
        editorToMotif(true);
    }
}

////////////////////////////////////////////////////////
//
// ConnectRosetteEditor
//
////////////////////////////////////////////////////////
ConnectRosetteEditor::ConnectRosetteEditor(QString name) : RosetteEditor(name)
{
    defaultBtn = new QPushButton("Calc Scale");
    defaultBtn->setFixedWidth(131);

    addWidget(defaultBtn);

    QObject::connect(defaultBtn, &QPushButton::clicked,  this, &ConnectRosetteEditor::calcScale);
}

void ConnectRosetteEditor::setMotif(DesignElementPtr del, bool doEmit)
{
    wDel = del;
    if (!del || !del->getMotif())
    {
        wrosetteConnect.reset();
        return;
    }

    MotifPtr oldMotif = del->getMotif();
    Q_ASSERT(oldMotif->getTile());
    auto rosettec = dynamic_pointer_cast<RosetteConnect>(oldMotif);
    if (rosettec)
    {
        setMotif(rosettec,doEmit);
    }
    else
    {
        RosettePtr rsp = dynamic_pointer_cast<Rosette>(oldMotif);
        if (rsp)
        {
            auto rosetteConnect = make_shared<RosetteConnect>(*oldMotif.get(),rsp->getN(),rsp->getQ(), rsp->getS());
            del->setMotif(rosetteConnect);
            setMotif(rosetteConnect,doEmit);
        }
        else
        {
            int n = oldMotif->getN();
            auto rosetteConnect = make_shared<RosetteConnect>(*oldMotif.get(), n, 0.0, 3);
            del->setMotif(rosetteConnect);
            setMotif(rosetteConnect,doEmit);
        }
    }
}

void ConnectRosetteEditor::setMotif(std::shared_ptr<RosetteConnect>(rosettecon), bool doEmit)
{
    Q_ASSERT(rosettecon->getTile());

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

////////////////////////////////////////////////////////
//
// ExtendedStarEditor
//
////////////////////////////////////////////////////////
ExtendedStarEditor::ExtendedStarEditor(QString name) : StarEditor(name)
{
    extendPeriphBox    = new QCheckBox("Extend Peripheral Vertices");
    extendFreeBox      = new QCheckBox("Extend Free Vertices");
    connectBoundaryBox = new QCheckBox("Connect Boundary Vertices");

    QHBoxLayout * hbox = new QHBoxLayout;
    hbox->addStretch();
    hbox->addWidget(extendPeriphBox);
    hbox->addWidget(extendFreeBox);
    hbox->addWidget(connectBoundaryBox);
    hbox->addStretch();
    addLayout(hbox);

    connect(extendPeriphBox,    &QCheckBox::clicked,  this, [this]() { editorToMotif(true);});
    connect(extendFreeBox,      &QCheckBox::clicked,  this, [this]() { editorToMotif(true);});
    connect(connectBoundaryBox, &QCheckBox::clicked,  this, [this]() { editorToMotif(true);});
}

void ExtendedStarEditor::setMotif(DesignElementPtr del, bool doEmit)
{
    wDel = del;
    if (!del || !del->getMotif())
    {
        wextended.reset();
        return;
    }

    MotifPtr oldMotif = del->getMotif();
    auto extended = dynamic_pointer_cast<ExtendedStar>(oldMotif);
    if (extended)
    {
        del->setMotif(extended);
        setMotif(extended,doEmit);
    }
    else
    {
        StarPtr sp = dynamic_pointer_cast<Star>(oldMotif);
        if (sp)
        {
            auto extended = make_shared<ExtendedStar>(*oldMotif.get(),sp->getN(),sp->getD(),sp->getS());
            del->setMotif(extended);
            setMotif(extended,doEmit);
        }
        else
        {
            int  n = oldMotif->getN();
            auto extended = make_shared<ExtendedStar>(*oldMotif.get(), n, n <= 6 ? n / 3.0 : 3.0, 2);
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
        StarEditor::motifToEditor();

        auto & extender = estar->getExtender();
        bool ext_t      = extender.getExtendPeripheralVertices();
        bool ext_nt     = extender.getExtendFreeVertices();
        bool con_bd     = extender.getConnectBoundaryVertices();

        blockSignals(true);
        extendPeriphBox->setChecked(ext_t);
        extendFreeBox->setChecked(ext_nt);
        connectBoundaryBox->setChecked(con_bd);
        blockSignals(false);
    }
}

void ExtendedStarEditor::editorToMotif(bool doEmit)
{
    auto estar = wextended.lock();
    if (estar)
    {
        StarEditor::editorToMotif(false);

        bool extendPeripheralVertices = extendPeriphBox->isChecked();
        bool extendFreeVertices       = extendFreeBox->isChecked();
        bool connectBoundary          = connectBoundaryBox->isChecked();

        auto & extender = estar->getExtender();
        blockSignals(true);
        extender.setExtendPeripheralVertices(extendPeripheralVertices);
        extender.setExtendFreeVertices(extendFreeVertices);
        extender.setConnectBoundaryVertices(connectBoundary);
        blockSignals(false);

        if (doEmit)
            emit sig_motif_modified(estar);
    }
}

////////////////////////////////////////////////////////
//
// ExtendedStar2Editor
//
////////////////////////////////////////////////////////
ExtendedStar2Editor::ExtendedStar2Editor(QString name) : Star2Editor(name)
{
    extendPeriphBox    = new QCheckBox("Extend Peripheral Vertices");
    extendFreeBox      = new QCheckBox("Extend Free Vertices");
    connectBoundaryBox = new QCheckBox("Connect Boundary Vertices");

    QHBoxLayout * hbox = new QHBoxLayout;
    hbox->addStretch();
    hbox->addWidget(extendPeriphBox);
    hbox->addWidget(extendFreeBox);
    hbox->addWidget(connectBoundaryBox);
    hbox->addStretch();
    addLayout(hbox);

    connect(extendPeriphBox,    &QCheckBox::clicked,  this, [this]() { editorToMotif(true);});
    connect(extendFreeBox,      &QCheckBox::clicked,  this, [this]() { editorToMotif(true);});
    connect(connectBoundaryBox, &QCheckBox::clicked,  this, [this]() { editorToMotif(true);});
}

void ExtendedStar2Editor::setMotif(DesignElementPtr del, bool doEmit)
{
    wDel = del;
    if (!del || !del->getMotif())
    {
        wextended2.reset();
        return;
    }

    MotifPtr oldMotif = del->getMotif();
    auto extended = dynamic_pointer_cast<ExtendedStar2>(oldMotif);
    if (extended)
    {
        del->setMotif(extended);
        setMotif(extended,doEmit);
    }
    else
    {
        Star2Ptr sp = dynamic_pointer_cast<Star2>(oldMotif);
        if (sp)
        {
            auto extended = make_shared<ExtendedStar2>(*oldMotif.get(),sp->getN(),sp->getTheta(),sp->getS());
            del->setMotif(extended);
            setMotif(extended,doEmit);
        }
        else
        {
            int  n = oldMotif->getN();
            auto extended = make_shared<ExtendedStar2>(*oldMotif.get(), n, 45.0, 1);
            del->setMotif(extended);
            setMotif(extended,doEmit);
        }
    }
}

void ExtendedStar2Editor::setMotif(std::shared_ptr<ExtendedStar2>(extended), bool doEmit)
{
    wextended2 = extended;

    Star2Editor::setMotif(extended,false);

    motifToEditor();
    editorToMotif(doEmit);
}

void ExtendedStar2Editor::motifToEditor()
{
    auto estar = wextended2.lock();
    if (estar)
    {
        Star2Editor::motifToEditor();

        auto & extender = estar->getExtender();
        bool ext_t      = extender.getExtendPeripheralVertices();
        bool ext_nt     = extender.getExtendFreeVertices();
        bool con_bd     = extender.getConnectBoundaryVertices();

        blockSignals(true);
        extendPeriphBox->setChecked(ext_t);
        extendFreeBox->setChecked(ext_nt);
        connectBoundaryBox->setChecked(con_bd);
        blockSignals(false);
    }
}

void ExtendedStar2Editor::editorToMotif(bool doEmit)
{
    auto estar = wextended2.lock();
    if (estar)
    {
        Star2Editor::editorToMotif(false);

        bool extendPeripheralVertices = extendPeriphBox->isChecked();
        bool extendFreeVertices       = extendFreeBox->isChecked();
        bool connectBoundary          = connectBoundaryBox->isChecked();

        auto & extender = estar->getExtender();
        blockSignals(true);
        extender.setExtendPeripheralVertices(extendPeripheralVertices);
        extender.setExtendFreeVertices(extendFreeVertices);
        extender.setConnectBoundaryVertices(connectBoundary);
        blockSignals(false);

        if (doEmit)
            emit sig_motif_modified(estar);
    }
}

////////////////////////////////////////////////////////
//
// ExtendedRosetteEditor
//
////////////////////////////////////////////////////////
ExtendedRosetteEditor::ExtendedRosetteEditor(QString name) : RosetteEditor(name)
{
    extendPeriphBox    = new QCheckBox("Extend PeripheralVertices");
    extendFreeBox      = new QCheckBox("Extend Free Vertices");
    connectBoundaryBox = new QCheckBox("Connect Boundary Vertices");

    QHBoxLayout * hbox = new QHBoxLayout;
    hbox->addStretch();
    hbox->addWidget(extendPeriphBox);
    hbox->addWidget(extendFreeBox);
    hbox->addWidget(connectBoundaryBox);
    hbox->addStretch();
    addLayout(hbox);

    connect(extendPeriphBox,    &QCheckBox::clicked,  this, [this]() { editorToMotif(true);});
    connect(extendFreeBox,      &QCheckBox::clicked,  this, [this]() { editorToMotif(true);});
    connect(connectBoundaryBox, &QCheckBox::clicked,  this, [this]() { editorToMotif(true);});
}

void ExtendedRosetteEditor::setMotif(DesignElementPtr del, bool doEmit)
{
    wDel = del;
    if (!del || !del->getMotif())
    {
        wextended.reset();
        return;
    }

    MotifPtr oldMotif = del->getMotif();
    auto extended = dynamic_pointer_cast<ExtendedRosette>(oldMotif);
    if (extended)
    {
        setMotif(extended, doEmit);
    }
    else
    {
        RosettePtr rsp = dynamic_pointer_cast<Rosette>(oldMotif);
        if (rsp)
        {
            auto extended = make_shared<ExtendedRosette>(*oldMotif.get(),rsp->getN(),rsp->getQ(),rsp->getS());
            del->setMotif(extended);
            setMotif(extended, doEmit);
        }
        else
        {
            int n = oldMotif->getN();
            auto extended = make_shared<ExtendedRosette>(*oldMotif.get(), n, 0.0, 3);
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
        RosetteEditor::motifToEditor();

        auto & extender = erose->getExtender();
        bool ext_t      = extender.getExtendPeripheralVertices();
        bool ext_nt     = extender.getExtendFreeVertices();
        bool con_bd     = extender.getConnectBoundaryVertices();

        blockSignals(true);
        extendPeriphBox->setChecked(ext_t);
        extendFreeBox->setChecked(ext_nt);
        connectBoundaryBox->setChecked(con_bd);
        blockSignals(false);
    }
}

void ExtendedRosetteEditor::editorToMotif(bool doEmit)
{
    auto erose = wextended.lock();
    if (erose)
    {
        RosetteEditor::editorToMotif(false);

        bool extendPeripherals  = extendPeriphBox->isChecked();
        bool extendFreeVertices = extendFreeBox->isChecked();
        bool connectBoundary    = connectBoundaryBox->isChecked();

        blockSignals(true);
        auto & extender = erose->getExtender();
        extender.setExtendPeripheralVertices(extendPeripherals);
        extender.setExtendFreeVertices(extendFreeVertices);
        extender.setConnectBoundaryVertices(connectBoundary);
        blockSignals(false);

        if (doEmit)
            emit sig_motif_modified(erose);
    }
}

////////////////////////////////////////////////////////
//
// ExtendedRosette2Editor
//
////////////////////////////////////////////////////////
ExtendedRosette2Editor::ExtendedRosette2Editor(QString name) : Rosette2Editor(name)
{
    extendPeriphBox    = new QCheckBox("Extend PeripheralVertices");
    extendFreeBox      = new QCheckBox("Extend Free Vertices");
    connectBoundaryBox = new QCheckBox("Connect Boundary Vertices");

    QHBoxLayout * hbox = new QHBoxLayout;
    hbox->addStretch();
    hbox->addWidget(extendPeriphBox);
    hbox->addWidget(extendFreeBox);
    hbox->addWidget(connectBoundaryBox);
    hbox->addStretch();
    addLayout(hbox);

    connect(extendPeriphBox,    &QCheckBox::clicked,  this, [this]() { editorToMotif(true);});
    connect(extendFreeBox,      &QCheckBox::clicked,  this, [this]() { editorToMotif(true);});
    connect(connectBoundaryBox, &QCheckBox::clicked,  this, [this]() { editorToMotif(true);});
}

void ExtendedRosette2Editor::setMotif(DesignElementPtr del, bool doEmit)
{
    wDel = del;
    if (!del || !del->getMotif())
    {
        wextended.reset();
        return;
    }

    MotifPtr oldMotif = del->getMotif();
    auto extended = dynamic_pointer_cast<ExtendedRosette2>(oldMotif);
    if (extended)
    {
        setMotif(extended, doEmit);
    }
    else
    {
        Rosette2Ptr rsp = dynamic_pointer_cast<Rosette2>(oldMotif);
        if (rsp)
        {
            auto extended = make_shared<ExtendedRosette2>(*oldMotif.get(),rsp->getN(),rsp->getKneeX(),rsp->getKneeY(),rsp->getS(),rsp->getK(),rsp->getConstrain());
            del->setMotif(extended);
            setMotif(extended, doEmit);
        }
        else
        {
            int n = oldMotif->getN();
            auto extended = make_shared<ExtendedRosette2>(*oldMotif.get(), n, 0.25,0.25, 3, 0.0, false);
            del->setMotif(extended);
            setMotif(extended, doEmit);
        }
    }
}

void ExtendedRosette2Editor::setMotif(std::shared_ptr<ExtendedRosette2>(extended), bool doEmit)
{
    wextended = extended;

    Rosette2Editor::setMotif(extended,false);

    motifToEditor();
    editorToMotif(doEmit);
}

void ExtendedRosette2Editor::motifToEditor()
{
    auto erose = wextended.lock();
    if (erose)
    {
        Rosette2Editor::motifToEditor();

        auto & extender = erose->getExtender();
        bool ext_t      = extender.getExtendPeripheralVertices();
        bool ext_nt     = extender.getExtendFreeVertices();
        bool con_bd     = extender.getConnectBoundaryVertices();

        blockSignals(true);
        extendPeriphBox->setChecked(ext_t);
        extendFreeBox->setChecked(ext_nt);
        connectBoundaryBox->setChecked(con_bd);
        blockSignals(false);
    }
}

void ExtendedRosette2Editor::editorToMotif(bool doEmit)
{
    auto erose = wextended.lock();
    if (erose)
    {
        Rosette2Editor::editorToMotif(false);

        bool extendPeripherals  = extendPeriphBox->isChecked();
        bool extendFreeVertices = extendFreeBox->isChecked();
        bool connectBoundary    = connectBoundaryBox->isChecked();

        blockSignals(true);
        auto & extender = erose->getExtender();
        extender.setExtendPeripheralVertices(extendPeripherals);
        extender.setExtendFreeVertices(extendFreeVertices);
        extender.setConnectBoundaryVertices(connectBoundary);
        blockSignals(false);

        if (doEmit)
            emit sig_motif_modified(erose);
    }
}
