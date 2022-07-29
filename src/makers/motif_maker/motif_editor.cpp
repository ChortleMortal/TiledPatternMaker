#include <QDebug>

#include "figures/figure.h"
#include "makers/motif_maker/explicit_figure_editors.h"
#include "makers/motif_maker/motif_editor.h"
#include "makers/motif_maker/motif_maker.h"
#include "mosaic/design_element.h"
#include "settings/configuration.h"
#include "tiledpatternmaker.h"

Q_DECLARE_METATYPE(FigureEditor *)

////////////////////////////////////////////////////////////////////////////
//
// MasterFigureEditor.java
//
// The top-level figure editor that understands the complete range of
// figure editors available in the applet and branches out to the right
// kind of editor as the DesignElement being edited is changed.

MotifEditor::MotifEditor(page_motif_maker * menu)
{
    config = Configuration::getInstance();

    // Explicit figure editors.
    explicit_edit           = new ExplicitEditor(menu,"explcit_edit");
    explcit_infer_edit      = new ExplicitInferEditor(menu,"explcit_infer_edit");
    explict_star_edit       = new ExplicitStarEditor(menu,"explict_star_edit");
    explicit_rosette_edit   = new ExplicitRosetteEditor(menu,"explicit_rosette_edit");
    explicit_hourglass_edit = new ExplicitHourglassEditor(menu,"explicit_hourglass_edit");
    explicit_girih_edit     = new ExplicitGirihEditor(menu,"explicit_girih_edit");
    explicit_intersect_edit = new ExplicitIntersectEditor(menu,"explicit_intersect_edit");
    explicit_feature_edit   = new ExplicitFeatureEditor(menu,"explicit_feature_edit");

    // Radial figure editors.
    radial_star_edit       = new StarEditor(menu,"radial_star_edit");
    radial_rosette_edit    = new RosetteEditor(menu,"radial_rosette_edit");
    connect_rosette_edit   = new ConnectRosetteEditor(menu,"connect_rosette_edit");
    connect_star_edit      = new ConnectStarEditor(menu,"connect_star_edit");
    ex_star_edit           = new ExtendedStarEditor(menu,"ex_star_edit");
    ex_rosette_edit        = new ExtendedRosetteEditor(menu,"ex_rosette_edit");

    // Panel containing the editors.
    mfw = new MotifFigureWidget();

    choiceCombo = new FigTypeChoiceCombo(this);

    QHBoxLayout * hbox = new QHBoxLayout();
    hbox->addStretch();
    hbox->addWidget(choiceCombo);
    hbox->addStretch();

    QVBoxLayout * layout = new QVBoxLayout();
    layout->addWidget(mfw);
    layout->addLayout(hbox);
    setLayout(layout);
}

void MotifEditor::selectFigure(DesignElementPtr del)
{
    currentDesignElement = del;
    FigurePtr figure = del->getFigure();
    if (!figure)
    {
        explicit_edit->setFigure(del,false);
        explcit_infer_edit->setFigure(del,false);
        explict_star_edit->setFigure(del,false);
        explicit_rosette_edit->setFigure(del,false);
        explicit_hourglass_edit->setFigure(del,false);
        explicit_girih_edit->setFigure(del,false);
        explicit_intersect_edit->setFigure(del,false);
        explicit_feature_edit->setFigure(del,false);

        radial_star_edit->setFigure(del,false);
        radial_rosette_edit->setFigure(del,false);
        connect_rosette_edit->setFigure(del,false);
        connect_star_edit->setFigure(del,false);
        ex_star_edit->setFigure(del,false);
        ex_rosette_edit->setFigure(del,false);
        return;
    }

    // DAC note:  When a design element is creeated from a feature in the tiling it defaults to a rosette
    // So everything starts as a rosette (except for explicit)

    qDebug() << "MotifEditor::selectFigure :" << figure->getFigureDesc();

    choiceCombo->updateChoices(figure);

    eFigType figType = figure->getFigType();
    switch (figType)
    {
    case FIG_TYPE_UNDEFINED:
    case FIG_TYPE_RADIAL:
        qCritical("unexpected figure type");
        break;
    case FIG_TYPE_EXTENDED_STAR:
        selectCurrentEditor(ex_star_edit);
        ex_star_edit->setFigure(del,false);
        break;
    case FIG_TYPE_EXTENDED_ROSETTE:
        selectCurrentEditor(ex_rosette_edit);
        ex_rosette_edit->setFigure(del,false);
        break;
    case FIG_TYPE_STAR:
        selectCurrentEditor(radial_star_edit);
        radial_star_edit->setFigure(del,false);
        break;
    case FIG_TYPE_ROSETTE:
        selectCurrentEditor(radial_rosette_edit);
        radial_rosette_edit->setFigure(del,false);
        break;
    case FIG_TYPE_CONNECT_ROSETTE:
        selectCurrentEditor(connect_rosette_edit);
        connect_rosette_edit->setFigure(del,false);
        break;
    case FIG_TYPE_CONNECT_STAR:
        selectCurrentEditor(connect_star_edit);
        connect_star_edit->setFigure(del,false);
        break;
    case FIG_TYPE_EXPLICIT:
        selectCurrentEditor(explicit_edit);
        explicit_edit->setFigure(del,false);
        break;
    case FIG_TYPE_EXPLICIT_INFER:
        selectCurrentEditor(explcit_infer_edit);
        explcit_infer_edit->setFigure(del,false);
        break;
    case FIG_TYPE_EXPLICIT_ROSETTE:
        selectCurrentEditor(explicit_rosette_edit);
        explicit_rosette_edit->setFigure(del,false);
        break;
    case FIG_TYPE_EXPLICIT_HOURGLASS:
        selectCurrentEditor(explicit_hourglass_edit);
        explicit_hourglass_edit->setFigure(del,false);
        break;
    case FIG_TYPE_EXPLICIT_INTERSECT:
        selectCurrentEditor(explicit_intersect_edit);
        explicit_intersect_edit->setFigure(del,false);
        break;
    case FIG_TYPE_EXPLICIT_GIRIH:
        selectCurrentEditor(explicit_girih_edit);
        explicit_girih_edit->setFigure(del,false);
        break;
    case FIG_TYPE_EXPLICIT_STAR:
        selectCurrentEditor(explict_star_edit);
        explict_star_edit->setFigure(del,false);
        break;
    case FIG_TYPE_EXPLICIT_FEATURE:
        selectCurrentEditor(explicit_feature_edit);
        explicit_feature_edit->setFigure(del,false);
        break;
    }
}

void MotifEditor::selectCurrentEditor(FigureEditor* fe)
{
    mfw->setEditor(fe);
    adjustSize();
}

void MotifEditor::slot_figureTypeChanged(eFigType type)
{
    if (!currentDesignElement.lock())
    {
        qWarning("MotifEditor::figureChoiceSelected - no design element");
        return;
    }

    FigureEditor * editor = getEditor(type);
    if (editor)
    {
        selectCurrentEditor(editor);
        editor->setFigure(currentDesignElement.lock(),true);
    }
}

FigureEditor * MotifEditor::getEditor(eFigType type)
{
    switch (type)
    {
    case FIG_TYPE_RADIAL:
        break;
    case FIG_TYPE_UNDEFINED:
    case FIG_TYPE_ROSETTE:
        return radial_rosette_edit;
    case FIG_TYPE_STAR:
        return radial_star_edit;
    case FIG_TYPE_CONNECT_STAR:
        return connect_star_edit;
    case FIG_TYPE_CONNECT_ROSETTE:
        return connect_rosette_edit;
    case FIG_TYPE_EXTENDED_ROSETTE:
        return ex_rosette_edit;
    case FIG_TYPE_EXTENDED_STAR:
        return ex_star_edit;
    case FIG_TYPE_EXPLICIT:
        return explicit_edit;
    case FIG_TYPE_EXPLICIT_INFER:
        return explcit_infer_edit;
    case FIG_TYPE_EXPLICIT_ROSETTE:
        return explicit_rosette_edit;
    case FIG_TYPE_EXPLICIT_HOURGLASS:
        return explicit_hourglass_edit;
    case FIG_TYPE_EXPLICIT_INTERSECT:
        return explicit_intersect_edit;
    case FIG_TYPE_EXPLICIT_GIRIH:
        return  explicit_girih_edit;
    case FIG_TYPE_EXPLICIT_STAR:
        return explict_star_edit;
    case FIG_TYPE_EXPLICIT_FEATURE:
        return explicit_feature_edit;
    }
    qCritical("Unexpected figure type (2");
    return nullptr;
}

/////////////////////////////////////////////////////////
/// MotifFigureWidget
////////////////////////////////////////////////////////

MotifFigureWidget::MotifFigureWidget()
{
}

MotifFigureWidget::MotifFigureWidget(FigureEditor * fe)
{
    setFixedWidth(600);
    AQVBoxLayout * aLayout = new AQVBoxLayout();
    aLayout->addWidget(fe);
    setLayout(aLayout);
}

void MotifFigureWidget::setEditor(FigureEditor * fe)
{
    AQVBoxLayout * aLayout = new AQVBoxLayout();
    aLayout->addWidget(fe);

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
    setLayout(aLayout);
    adjustSize();
}


/////////////////////////////////////////////////////////
/// FigTypeChoiceCombo
////////////////////////////////////////////////////////

FigTypeChoiceCombo:: FigTypeChoiceCombo(MotifEditor *editor)
{
    setFixedWidth(221);

    connect(this, QOverload<int>::of(&QComboBox::currentIndexChanged), this,  &FigTypeChoiceCombo::slot_figureTypeSelected);
    connect(this, &FigTypeChoiceCombo::sig_figureTypeChanged,          editor, &MotifEditor::slot_figureTypeChanged);
}

void FigTypeChoiceCombo::updateChoices(FigurePtr figure)
{
    blockSignals(true);

    clear();

    if (figure->isRadial())
    {
        addChoice(FIG_TYPE_STAR,                "Star");
        addChoice(FIG_TYPE_CONNECT_STAR,        "Star Connect");
        addChoice(FIG_TYPE_EXTENDED_STAR,       "Star Extended");
        addChoice(FIG_TYPE_ROSETTE,             "Rosette");
        addChoice(FIG_TYPE_CONNECT_ROSETTE,     "Rosette Connect");
        addChoice(FIG_TYPE_EXTENDED_ROSETTE,    "Rosette Extended");
        addChoice(FIG_TYPE_EXPLICIT_FEATURE,    "Explicit Feature");
    }
    else
    {
        addChoice(FIG_TYPE_EXPLICIT,            "Explicit");
        addChoice(FIG_TYPE_EXPLICIT_INFER,      "Infer Explicit");
        addChoice(FIG_TYPE_EXPLICIT_STAR,       "Star Explicit");
        addChoice(FIG_TYPE_EXPLICIT_ROSETTE,    "Rosette Explicit");
        addChoice(FIG_TYPE_EXPLICIT_HOURGLASS,  "Hourglass Explicit");
        addChoice(FIG_TYPE_EXPLICIT_GIRIH,      "Girih Tiles Explicit");
        addChoice(FIG_TYPE_EXPLICIT_INTERSECT,  "Intersect Explicit");
        addChoice(FIG_TYPE_EXPLICIT_FEATURE,    "Explicit Feature");
    }

    int index = getChoiceIndex(figure->getFigType());
    if (index >= 0)
    {
        setCurrentIndex(index);
        qDebug() << "MotifEditor - set choice index=" << index;
    }
    else
    {
        setCurrentIndex(0);
        qDebug() << "MotifEditor - forcing choice index=0";
    }

    blockSignals(false);
}

void FigTypeChoiceCombo::addChoice(eFigType type, QString name)
{
    addItem(name,QVariant(type));
}

int FigTypeChoiceCombo::getChoiceIndex(eFigType type)
{
    qDebug().noquote()  << "type is" << Figure::getTypeString(type);

    return findData(QVariant(type));
}

void FigTypeChoiceCombo::slot_figureTypeSelected(int index)
{
    Q_UNUSED(index);

    QVariant qv = currentData();
    eFigType type = static_cast<eFigType>(qv.toInt());
    qDebug() << "MotifEditor type="  << sFigType[type];

    emit sig_figureTypeChanged(type);
}
