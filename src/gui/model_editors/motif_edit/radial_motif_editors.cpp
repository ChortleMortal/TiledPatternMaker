#include <QCheckBox>

#include "gui/model_editors/motif_edit/radial_motif_editors.h"
#include "gui/widgets/dlg_rosette_tips.h"
#include "gui/widgets/layout_sliderset.h"
#include "model/motifs/rosette.h"
#include "model/motifs/rosette2.h"
#include "model/motifs/star.h"
#include "model/motifs/star2.h"

using std::dynamic_pointer_cast;
using std::make_shared;
using std::shared_ptr;

typedef shared_ptr<RadialMotif>    RadialPtr;

////////////////////////////////////////////////////////
//
// StarEditor
//
////////////////////////////////////////////////////////

RadialMotifEditor::RadialMotifEditor(QString name) : MotifEditor(name)
{
}

void RadialMotifEditor::addCommonRow()
{
    chk_on_point  = new QCheckBox("On Point");
    chk_inscribe = new QCheckBox("Inscribed");

    commonRadialRow = new QHBoxLayout;
    commonRadialRow->addStretch();
    commonRadialRow->addWidget(chk_inscribe);
    commonRadialRow->addWidget(chk_on_point);
    addLayout(commonRadialRow);

    connect(chk_inscribe,   &QCheckBox::clicked,            this, [this]() { editorToMotif(true);});
    connect(chk_on_point,   &QCheckBox::clicked,            this, [this]() { editorToMotif(true);});
}

void RadialMotifEditor::setMotif(shared_ptr<RadialMotif> motif)
{
    weakRadial = motif;
    MotifEditor::setMotif(motif);
}

void RadialMotifEditor::refresh()
{
    auto motif = weakRadial.lock();
    if (motif)
    {
        MotifEditor::refresh();

        chk_inscribe->setChecked(motif->getInscribe());
        chk_on_point->setChecked(motif->getOnPoint());
    }
}

void RadialMotifEditor::editorToMotif(bool doEmit)
{
    auto motif = weakRadial.lock();
    if (motif)
    {

        motif->setInscribe(chk_inscribe->isChecked());
        motif->setOnPoint(chk_on_point->isChecked());

        MotifEditor::editorToMotif(doEmit);
    }
}


////////////////////////////////////////////////////////
//
// StarEditor
//
////////////////////////////////////////////////////////

StarEditor::StarEditor(QString name, StarPtr star) : RadialMotifEditor(name)
{
    d_slider = new DoubleSliderSet("Star Editor Hops D", 3.0, 0.0, 10.0, 100 );
    s_slider = new SliderSet("Star Editor Intersects S", 2, 1, 5);

    addLayout(d_slider);
    addLayout(s_slider);

    addCommonRow();

    connect(d_slider,       &DoubleSliderSet::valueChanged, this, [this]() { editorToMotif(true);});
    connect(s_slider,       &SliderSet::valueChanged,       this, [this]() { editorToMotif(true);});

    Q_ASSERT(star);
    wstar = star;
    setMotif(star);
}

void StarEditor::refresh()
{
    auto star = wstar.lock();
    if (star)
    {
        RadialMotifEditor::refresh();

        qreal  dd = star->getD();
        int    ss = star->getS();

        blockSignals(true);
        d_slider->setValue( dd);
        s_slider->setValue( ss);
        blockSignals(false);
    }
}

void StarEditor::editorToMotif(bool doEmit)
{
    auto star = wstar.lock();
    if (star)
    {
        qreal dval = d_slider->value();
        int sval   = s_slider->value();

        star->setD(dval);
        star->setS(sval);
        star->resetMotifMap();

        RadialMotifEditor::editorToMotif(doEmit);
    }
}

////////////////////////////////////////////////////////
//
// Star2Editor
//
////////////////////////////////////////////////////////

Star2Editor::Star2Editor(QString name, Star2Ptr star) : RadialMotifEditor(name)
{
    theta_slider = new DoubleSliderSet("Star Editor Angle theta", 45, 0.0, 90.0, 10 );
    s_slider     = new SliderSet("Star Editor Intersects S", 2, 1, 5);

    addLayout(theta_slider);
    addLayout(s_slider);
    addCommonRow();

    connect(theta_slider, &DoubleSliderSet::valueChanged, this, [this]() { editorToMotif(true);});
    connect(s_slider,     &SliderSet::valueChanged,       this, [this]() { editorToMotif(true);});

    Q_ASSERT(star);
    wstar = star;
    setMotif(star);
}

void Star2Editor::refresh()
{

    auto star = wstar.lock();
    if (star)
    {
        RadialMotifEditor::refresh();

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
        qreal thetaval = theta_slider->value();
        int sval       = s_slider->value();

        star->setTheta(thetaval);
        star->setS(sval);
        star->resetMotifMap();

        RadialMotifEditor::editorToMotif(doEmit);
    }
}

////////////////////////////////////////////////////////
//
// ResetteEditor
//
////////////////////////////////////////////////////////

RosetteEditor::RosetteEditor(QString name, RosettePtr rose) : RadialMotifEditor(name)
{
    connectScale = nullptr;

    q_slider = new DoubleSliderSet("RosetteEditor Q (Tip Angle)", 0.0, -3.0, 3.0, 100 );
    s_slider = new SliderSet("RosetteEditor S (Sides Intersections)", 1, 1, 5);

    addLayout(q_slider);
    addLayout(s_slider);
    addCommonRow();

    connect(q_slider,     &DoubleSliderSet::valueChanged, this, [this]() { editorToMotif(true);});
    connect(s_slider,     &SliderSet::valueChanged,       this, [this]() { editorToMotif(true);});

    Q_ASSERT(rose);
    wrosette = rose;
    setMotif(rose);
}

void RosetteEditor::refresh()
{
    auto rose = wrosette.lock();
    if (rose)
    {
        RadialMotifEditor::refresh();

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
        qreal  qval = q_slider->value();
        int    sval = s_slider->value();

        blockSignals(true);
        rose->setQ(qval);
        rose->setS(sval);
        rose->resetMotifMap();
        blockSignals(false);

        RadialMotifEditor::editorToMotif(doEmit);
    }
}

////////////////////////////////////////////////////////
//
// Rosette2Editor
//
////////////////////////////////////////////////////////

Rosette2Editor::Rosette2Editor(QString name, Rosette2Ptr rose) : RadialMotifEditor(name)
{
    connectScale = nullptr;

    kx_slider = new DoubleSliderSet("Rosette2 KneeX Height", 0.25, 0, 1.0, 100 );
    ky_slider = new DoubleSliderSet("Rosette2 KneeY Width ", 0.25, 0, 1.0, 100 );
    k_slider  = new DoubleSliderSet("Rosette2 K (Knee Angle)", 0.0, -90.0, 90.0, 10);
    s_slider  = new SliderSet(      "Rosette2 S Intersections", 1, 1, 5);

    kaplanize     = new QCheckBox("Kaplan's constraint ");
    convert       = new QPushButton("Convert");

    QLabel * label            = new QLabel("Tip Types ");
    QPushButton * editTipsBtn = new QPushButton("Edit");
    tipInfo                   = new QLineEdit();
    tipInfo->setReadOnly(true);

    QHBoxLayout * hbox = new QHBoxLayout;
    hbox->addWidget(label);
    hbox->addSpacing(7);
    hbox->addWidget(tipInfo);
    hbox->addSpacing(11);
    hbox->addWidget(editTipsBtn);

    addLayout(kx_slider);
    addLayout(ky_slider);
    addLayout(k_slider);
    addLayout(s_slider);
    addLayout(hbox);
    addCommonRow();
    commonRadialRow->insertWidget(1,kaplanize);
    commonRadialRow->insertWidget(2,convert);

    connect(kx_slider,    &DoubleSliderSet::valueChanged, this, [this]() { editorToMotif(true);});
    connect(ky_slider,    &DoubleSliderSet::valueChanged, this, [this]() { editorToMotif(true);});
    connect(k_slider,     &DoubleSliderSet::valueChanged, this, [this]() { editorToMotif(true);});
    connect(s_slider,     &SliderSet::valueChanged,       this, [this]() { editorToMotif(true);});
    connect(kaplanize,    &QCheckBox::clicked,            this, [this]() { editorToMotif(true);});
    connect(convert,      &QPushButton::clicked,          this, &Rosette2Editor::convertConstrained);
    connect(editTipsBtn,  &QPushButton::clicked,          this, &Rosette2Editor::editTips);

    Q_ASSERT(rose);
    wrosette = rose;
    setMotif(rose);
}

void Rosette2Editor::refresh()
{
    auto rose = wrosette.lock();
    if (rose)
    {
        RadialMotifEditor::refresh();

        qreal  x  = rose->getKneeX();
        qreal  y  = rose->getKneeY();
        qreal  k  = rose->getK();
        int    ss = rose->getS();

        bool constrain = rose->getConstrain();
        convert->setVisible(constrain);

        QString info;
        QList<TipType> & tips = rose->getTipTypes();
        for (int i=0; i < tips.size(); i++)
        {
            if (i > 0)
            {
                info += " then ";
            }
            TipType tip = tips[i];
            info += sTipType[static_cast<int>(tip)];
        }
        if (tips.size() == 1)
        {
            info += " only";
        }
        if (rose->hasTipsToMove() && rose->numExtenders() == 0)
        {
            info += " (to Boundary needs an extender)";
        }
        tipInfo->setText(info);

        blockSignals(true);
        kx_slider->setValue(x);
        ky_slider->setValue(y);
        k_slider->setValue(k);
        s_slider->setValue(ss);
        kaplanize->setChecked(constrain);
        blockSignals(false);
    }
}

void Rosette2Editor::editorToMotif(bool doEmit)
{
    auto rose = wrosette.lock();
    if (rose)
    {
        qreal  x = kx_slider->value();
        qreal  y = ky_slider->value();
        qreal  k = k_slider->value();
        int    s = s_slider->value();
        bool   c = kaplanize->isChecked();

        convert->setVisible(c);

        rose->setKneeX(x);
        rose->setKneeY(y);
        rose->setK(k);
        rose->setS(s);
        rose->setConstrain(c);
        rose->resetMotifMap();

        RadialMotifEditor::editorToMotif(doEmit);
    }
}

void Rosette2Editor::editTips()
{
    auto rose = wrosette.lock();
    if (!rose) return;

    DlgRosetteTips dlg(rose, this);
    dlg.exec();
}

void Rosette2Editor::convertConstrained()
{
    auto rose = wrosette.lock();
    if (rose)
    {
        if (rose->convertConstrained())
        {
            refresh();
            editorToMotif(true);
        }
    }
}
