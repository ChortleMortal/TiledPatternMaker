#include <QCheckBox>

#include "model/motifs/rosette.h"
#include "model/motifs/rosette2.h"
#include "model/motifs/star.h"
#include "model/motifs/star2.h"
#include "gui/model_editors/motif_edit/regular_motif_editors.h"
#include "gui/model_editors/motif_edit/motif_maker_widget.h"
#include "gui/model_editors/motif_edit/motif_editor_widget.h"
#include "model/prototypes/design_element.h"
#include "gui/panels/page_motif_maker.h"
#include "gui/widgets/layout_sliderset.h"

using std::dynamic_pointer_cast;
using std::make_shared;
using std::shared_ptr;

typedef shared_ptr<RadialMotif>    RadialPtr;

////////////////////////////////////////////////////////
//
// StarEditor
//
////////////////////////////////////////////////////////
StarEditor::StarEditor(QString name, DELPtr del, bool doEmit) : NamedMotifEditor(name)
{
    d_slider = new DoubleSliderSet("Star Editor Hops D", 3.0, 0.0, 10.0, 100 );
    s_slider = new SliderSet("Star Editor Intersects S", 2, 1, 5);
    version_combo = new QComboBox();
    version_combo->addItem("Version 1",1);
    version_combo->addItem("Version 2",2);
    version_combo->setFixedWidth(91);

    chk_inscribe = new QCheckBox("Inscribed");
    chk_on_point  = new QCheckBox("On Point");

    addLayout(d_slider);
    addLayout(s_slider);

    QHBoxLayout * hbox = new QHBoxLayout;
    hbox->addStretch();
    hbox->addWidget(version_combo);
    hbox->addStretch();
    hbox->addWidget(chk_inscribe);
    hbox->addWidget(chk_on_point);
    addLayout(hbox);

    connect(d_slider,       &DoubleSliderSet::valueChanged, this, [this]() { editorToMotif(true);});
    connect(s_slider,       &SliderSet::valueChanged,       this, [this]() { editorToMotif(true);});
    connect(chk_inscribe,   &QCheckBox::clicked,            this, [this]() { editorToMotif(true);});
    connect(chk_on_point,   &QCheckBox::clicked,            this, [this]() { editorToMotif(true);});
    connect(version_combo,  &QComboBox::currentIndexChanged,this,[this]()  { editorToMotif(true);});

    wDel = del;
    if (!del || !del->getMotif())
    {
        wstar.reset();
        return;
    }

    MotifPtr motif = del->getMotif();
    StarPtr star = dynamic_pointer_cast<Star>(motif);
    if (!star)
    {
        // create a star with some defaults
        int n =motif->getN();
        star = make_shared<Star>(*motif,n, n <=6 ? n / 3.0 : 3.0, 2);
        del->setMotif(star);
    }

    setMotif(star,doEmit);
}

void StarEditor::setMotif(StarPtr star, bool doEmit)
{
    Q_ASSERT(star);
    wstar = star;

    NamedMotifEditor::setMotif(star,false);

    StarEditor::motifToEditor();
    StarEditor::editorToMotif(doEmit);
}

void StarEditor::motifToEditor()
{
    auto star = wstar.lock();
    if (star)
    {
        NamedMotifEditor::motifToEditor();

        qreal  dd = star->getD();
        int    ss = star->getS();

        blockSignals(true);
        //double dmax = 0.5 * (double)nn;
        //d_slider->setValues( dd, 1.0, static_cast<qreal>(star->getN()) * 2.0);
        //s_slider->setValues( ss, 1.0, star->getN() * 2);
        d_slider->setValue( dd);
        s_slider->setValue( ss);
        chk_inscribe->setChecked(star->getInscribe());
        chk_on_point->setChecked(star->getOnPoint());
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
        star->setInscribe(chk_inscribe->isChecked());
        star->setOnPoint(chk_on_point->isChecked());
        star->resetMotifMap();

        if (doEmit)
            emit sig_motif_modified(star);
    }
}

////////////////////////////////////////////////////////
//
// Star2Editor
//
////////////////////////////////////////////////////////
Star2Editor::Star2Editor(QString name, DELPtr del, bool doEmit) : NamedMotifEditor(name)
{
    theta_slider = new DoubleSliderSet("Star Editor Angle theta", 45, 0.0, 90.0, 10 );
    s_slider     = new SliderSet("Star Editor Intersects S", 2, 1, 5);

    chk_inscribe = new QCheckBox("Inscribed");
    chk_on_point  = new QCheckBox("On Point");

    addLayout(theta_slider);
    addLayout(s_slider);

    QHBoxLayout * hbox = new QHBoxLayout;
    hbox->addStretch();
    hbox->addWidget(chk_inscribe);
    hbox->addWidget(chk_on_point);
    addLayout(hbox);

    connect(theta_slider, &DoubleSliderSet::valueChanged, this, [this]() { editorToMotif(true);});
    connect(s_slider,     &SliderSet::valueChanged,       this, [this]() { editorToMotif(true);});
    connect(chk_inscribe, &QCheckBox::clicked,            this, [this]() { editorToMotif(true);});
    connect(chk_on_point, &QCheckBox::clicked,            this, [this]() { editorToMotif(true);});

    wDel = del;
    if (!del || !del->getMotif())
    {
        wstar.reset();
        return;
    }

    MotifPtr motif = del->getMotif();
    Star2Ptr star = dynamic_pointer_cast<Star2>(motif);
    if (!star)
    {
        // create a star with some defaults
        int n =motif->getN();
        star = make_shared<Star2>(*motif,n, 60.0, 1);
        del->setMotif(star);
    }

    setMotif(star,doEmit);
}

void Star2Editor::setMotif(Star2Ptr star, bool doEmit)
{
    Q_ASSERT(star);
    wstar = star;

    NamedMotifEditor::setMotif(star,false);

    Star2Editor::motifToEditor();
    Star2Editor::editorToMotif(doEmit);
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
        chk_inscribe->setChecked(star->getInscribe());
        chk_on_point->setChecked(star->getOnPoint());
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
        star->setInscribe(chk_inscribe->isChecked());
        star->setOnPoint(chk_on_point->isChecked());
        star->resetMotifMap();

        if (doEmit)
            emit sig_motif_modified(star);
    }
}

////////////////////////////////////////////////////////
//
// ResetteEditor
//
////////////////////////////////////////////////////////
RosetteEditor::RosetteEditor(QString name, DELPtr del, bool doEmit) : NamedMotifEditor(name)
{
    connectScale = nullptr;

    q_slider = new DoubleSliderSet("RosetteEditor Q (Tip Angle)", 0.0, -3.0, 3.0, 100 );
    s_slider = new SliderSet("RosetteEditor S (Sides Intersections)", 1, 1, 5);

    chk_inscribe = new QCheckBox("Inscribed");
    chk_on_point  = new QCheckBox("On Point");

    addLayout(q_slider);
    addLayout(s_slider);

    QHBoxLayout * hbox = new QHBoxLayout;
    hbox->addStretch();
    hbox->addWidget(chk_inscribe);
    hbox->addWidget(chk_on_point);
    addLayout(hbox);

    connect(q_slider,     &DoubleSliderSet::valueChanged, this, [this]() { editorToMotif(true);});
    connect(s_slider,     &SliderSet::valueChanged,       this, [this]() { editorToMotif(true);});
    connect(chk_inscribe, &QCheckBox::clicked,            this, [this]() { editorToMotif(true);});
    connect(chk_on_point, &QCheckBox::clicked,            this, [this]() { editorToMotif(true);});

    wDel = del;
    if (!del || !del->getMotif())
    {
        wrosette.reset();
        return;
    }

    MotifPtr motif     = del->getMotif();
    RosettePtr rosette = dynamic_pointer_cast<Rosette>(motif);
    if (!rosette)
    {
        // create using defaults
        int n = motif->getN();
        rosette  = make_shared<Rosette>(*motif.get(), n, 0.0, 3);
        del->setMotif(rosette);
    }

    setMotif(rosette,doEmit);
}

void RosetteEditor::setMotif(RosettePtr rosette, bool doEmit)
{
    Q_ASSERT(rosette);
    wrosette = rosette;

    NamedMotifEditor::setMotif(rosette,false);

    RosetteEditor::motifToEditor();
    RosetteEditor::editorToMotif(doEmit);
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
        chk_inscribe->setChecked(rose->getInscribe());
        chk_on_point->setChecked(rose->getOnPoint());
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
        rose->setInscribe(chk_inscribe->isChecked());
        rose->setOnPoint(chk_on_point->isChecked());
        rose->resetMotifMap();
        blockSignals(false);

        if (doEmit)
            emit sig_motif_modified(rose);
    }
}



////////////////////////////////////////////////////////
//
// Rosette2Editor
//
////////////////////////////////////////////////////////
Rosette2Editor::Rosette2Editor(QString name, DELPtr del, bool doEmit) : NamedMotifEditor(name)
{
    connectScale = nullptr;

    kx_slider = new DoubleSliderSet("Rosette2 KneeX Height", 0.25, 0, 1.0, 100 );
    ky_slider = new DoubleSliderSet("Rosette2 KneeY Width ", 0.25, 0, 1.0, 100 );
    k_slider  = new DoubleSliderSet("Rosette2 K (Knee Angle)", 0.0, -90.0, 90.0, 10);
    s_slider  = new SliderSet(      "Rosette2 S Intersections", 1, 1, 5);

    chkOuter   = new QCheckBox("Outward");
    chkInner   = new QCheckBox("Inward");
    chkFlipped = new QCheckBox("Flipped");

    rSingle = new QRadioButton("Single");
    rAlter  = new QRadioButton("Alternating");

    QLabel  * label  = new QLabel("Tip Direction : ");
    QLabel  * label2 = new QLabel("Tip Mode : ");

    kaplanize     = new QCheckBox("Kaplan's constraint ");
    chk_inscribe  = new QCheckBox("Inscribed ");
    chk_on_point  = new QCheckBox("On Point ");
    convert       = new QPushButton("Convert");

    QHBoxLayout * hbox = new QHBoxLayout;
    hbox->addWidget(label);
    hbox->addWidget(chkOuter);
    hbox->addWidget(chkInner);
    hbox->addWidget(chkFlipped);
    hbox->addStretch(1);
    hbox->addWidget(label2);
    hbox->addWidget(rSingle);
    hbox->addWidget(rAlter);
    hbox->addStretch(4);

    QHBoxLayout * hbox2 = new QHBoxLayout;
    hbox2->addWidget(chk_inscribe);
    hbox2->addWidget(chk_on_point);
    hbox2->addWidget(kaplanize);
    hbox2->addWidget(convert);
    hbox2->addStretch(10);

    addLayout(kx_slider);
    addLayout(ky_slider);
    addLayout(k_slider);
    addLayout(s_slider);
    addLayout(hbox);
    addLayout(hbox2);

    connect(kx_slider,    &DoubleSliderSet::valueChanged, this, [this]() { editorToMotif(true);});
    connect(ky_slider,    &DoubleSliderSet::valueChanged, this, [this]() { editorToMotif(true);});
    connect(k_slider,     &DoubleSliderSet::valueChanged, this, [this]() { editorToMotif(true);});
    connect(s_slider,     &SliderSet::valueChanged,       this, [this]() { editorToMotif(true);});
    connect(kaplanize,    &QCheckBox::clicked,            this, [this]() { editorToMotif(true);});
    connect(chk_inscribe, &QCheckBox::clicked,            this, [this]() { editorToMotif(true);});
    connect(chk_on_point, &QCheckBox::clicked,            this, [this]() { editorToMotif(true);});

    connect(convert,      &QPushButton::clicked,          this, &Rosette2Editor::convertConstrained);
   \
    connect(rSingle,      &QRadioButton::clicked,         this, &Rosette2Editor::rSingleClicked);
    connect(rAlter,       &QRadioButton::clicked,         this, &Rosette2Editor::rAlternateClicked);

    connect(chkOuter,     &QCheckBox::clicked,            this, &Rosette2Editor::chkOuterClicked);
    connect(chkInner,     &QCheckBox::clicked,            this, &Rosette2Editor::chkInnerClicked);
    connect(chkFlipped,   &QCheckBox::clicked,            this, &Rosette2Editor::chkFlippedClicked);

    wDel = del;
    if (!del || !del->getMotif())
    {
        wrosette.reset();
        return;
    }

    MotifPtr motif = del->getMotif();
    Rosette2Ptr rosette = dynamic_pointer_cast<Rosette2>(motif);
    if (!rosette)
    {
        // create using defaults
        int n = motif->getN();
        rosette  = make_shared<Rosette2>(*motif.get(), n, 0.25,0.25,2,0.0, false);
        del->setMotif(rosette);
        setMotif(rosette,doEmit);
    }
    setMotif(rosette,doEmit);
}

void Rosette2Editor::setMotif(Rosette2Ptr rosette, bool doEmit)
{
    Q_ASSERT(rosette);
    wrosette = rosette;

    NamedMotifEditor::setMotif(rosette,false);

    Rosette2Editor::motifToEditor();
    Rosette2Editor::editorToMotif(doEmit);
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
        eTipMode tt    = rose->getTipMode();
        uint tipTypes  = rose->getTipTypes();
        bool constrain = rose->getConstrain();

        convert->setVisible(constrain);

        blockSignals(true);
        kx_slider->setValue(x);
        ky_slider->setValue(y);
        k_slider->setValue(k);
        s_slider->setValue(ss);
        kaplanize->setChecked(constrain);
        chk_inscribe->setChecked(rose->getInscribe());
        chk_on_point->setChecked(rose->getOnPoint());
        if (tt == TIP_MODE_REGULAR)
            rSingle->setChecked(true);
        else
            rAlter->setChecked(true);
        chkOuter->setChecked(tipTypes & TIP_TYPE2_OUTER);
        chkInner->setChecked(tipTypes & TIP_TYPE2_INNER);
        chkFlipped->setChecked(tipTypes & TIP_TYPE2_FLIPPED);

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
        bool   c = kaplanize->isChecked();
        eTipMode tt = (rSingle->isChecked()) ? TIP_MODE_REGULAR : TIP_MODE_ALTERNATE;

        convert->setVisible(c);

        rose->setKneeX(x);
        rose->setKneeY(y);
        rose->setK(k);
        rose->setS(s);
        rose->setTipMode(tt);
        rose->setConstrain(c);
        rose->setInscribe(chk_inscribe->isChecked());
        rose->setOnPoint(chk_on_point->isChecked());
        rose->resetMotifMap();

        if (chkOuter->isChecked())
            rose->addTipType(TIP_TYPE2_OUTER);
        else
            rose->removeTipType(TIP_TYPE2_OUTER);

        if (chkInner->isChecked())
            rose->addTipType(TIP_TYPE2_INNER);
        else
            rose->removeTipType(TIP_TYPE2_INNER);

        if (chkFlipped->isChecked())
            rose->addTipType(TIP_TYPE2_FLIPPED);
        else
            rose->removeTipType(TIP_TYPE2_FLIPPED);

        if (doEmit)
            emit sig_motif_modified(rose);
    }
}

void Rosette2Editor::chkOuterClicked(bool checked)
{
    auto rose = wrosette.lock();
    if (!rose) return;

    if (checked)
    {
        if (rose->getTipMode() == TIP_MODE_REGULAR)
        {
            // single turn off others
            chkInner->setChecked(false);
            chkFlipped->setChecked(false);
        }
        else
        {
            // only allow turn off which will turn on the one that is not selected
            chkOuter->setChecked(false);
            return;
        }
    }
    else
    {
        if (rose->getTipMode() == TIP_MODE_REGULAR)
        {
            if (!chkInner->isChecked() && !chkFlipped->isChecked())
            {
                // something must be checked
                chkInner->setChecked(true);
            }
        }
        else
        {
            // turn on the non selected
            if (!chkInner->isChecked())
                chkInner->setChecked(true);
            else
                chkFlipped->setChecked(true);
        }
    }

    editorToMotif(true);
}

void Rosette2Editor::chkInnerClicked(bool checked)
{
    auto rose = wrosette.lock();
    if (!rose) return;

    if (checked)
    {
        if (rose->getTipMode() == TIP_MODE_REGULAR)
        {
            // single turn off others
            chkOuter->setChecked(false);
            chkFlipped->setChecked(false);
        }
        else
        {
            // only allow turn off which will turn on the one that is not selected
            chkInner->setChecked(false);
            return;
        }
    }
    else
    {
        if (rose->getTipMode() == TIP_MODE_REGULAR)
        {
            if (!chkOuter->isChecked() && !chkFlipped->isChecked())
            {
                // something must be checked
                chkOuter->setChecked(true);
            }
        }
        else
        {
            // turn on the non selected
            if (!chkOuter->isChecked())
                chkOuter->setChecked(true);
            else
                chkFlipped->setChecked(true);
        }
    }

    editorToMotif(true);
}

void Rosette2Editor::chkFlippedClicked(bool checked)
{
    auto rose = wrosette.lock();
    if (!rose) return;

    if (checked)
    {
        if (rose->getTipMode() == TIP_MODE_REGULAR)
        {
            // single turn off others
            chkOuter->setChecked(false);
            chkInner->setChecked(false);
        }
        else
        {
            // only allow turn off which will turn on the one that is not selected
            chkFlipped->setChecked(false);
            return;
        }
    }
    else
    {
        if (rose->getTipMode() == TIP_MODE_REGULAR)
        {
            if (!chkOuter->isChecked() && !chkInner->isChecked())
            {
                // something must be checked
                chkOuter->setChecked(true);
            }
        }
        else
        {
            // turn on the non selected
            if (!chkOuter->isChecked())
                chkOuter->setChecked(true);
            else
                chkInner->setChecked(true);
        }
    }

    editorToMotif(true);
}

void Rosette2Editor::rSingleClicked(bool checked)
{
    if (checked)
    {
        // assumes 2 are selected so turn one off
        if (chkFlipped->isChecked())
            chkFlipped->setChecked(false);
        else if (chkInner->isChecked())
            chkInner->setChecked(false);

        editorToMotif(true);
    }
}

void Rosette2Editor::rAlternateClicked(bool checked)
{
    if (checked)
    {
        // assumes ones is one, so turn on another
        if (chkOuter->isChecked())
            chkInner->setChecked(true);
        else if (chkInner->isChecked())
            chkFlipped->setChecked(true);
        else
            chkOuter->setChecked(true);
        editorToMotif(true);
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
