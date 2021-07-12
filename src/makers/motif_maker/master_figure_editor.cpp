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

#include "makers/motif_maker/master_figure_editor.h"
#include "makers/motif_maker/motif_maker.h"
#include "base/tiledpatternmaker.h"
#include "tapp/star.h"
#include "tapp/extended_star.h"
#include "tapp/extended_rosette.h"
#include "makers/motif_maker/explicit_figure_editors.h"
#include "settings/configuration.h"

Q_DECLARE_METATYPE(FigureEditor *)

////////////////////////////////////////////////////////////////////////////
//
// MasterFigureEditor.java
//
// The top-level figure editor that understands the complete range of
// figure editors available in the applet and branches out to the right
// kind of editor as the DesignElement being edited is changed.

MasterFigureEditor::MasterFigureEditor(page_motif_maker * menu)
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
    mfw = new MasterFigureWidget();

    choiceCombo2 = new FigTypeChoiceCombo(this);
    connect(choiceCombo2, &FigTypeChoiceCombo::sig_figureTypeChanged,  this, &MasterFigureEditor::slot_figureTypeChanged);

    QHBoxLayout * hbox = new QHBoxLayout();
    hbox->addStretch();
    hbox->addWidget(choiceCombo2);
    hbox->addStretch();

    QVBoxLayout * layout = new QVBoxLayout();
    layout->addWidget(mfw);
    layout->addLayout(hbox);
    setLayout(layout);
}

void MasterFigureEditor::masterResetWithFigure(FigurePtr figure)
{
    masterFigure = figure;
    if (!masterFigure)
    {
        explicit_edit->resetWithFigure(figure,false);
        explcit_infer_edit->resetWithFigure(figure,false);
        explict_star_edit->resetWithFigure(figure,false);
        explicit_rosette_edit->resetWithFigure(figure,false);
        explicit_hourglass_edit->resetWithFigure(figure,false);
        explicit_girih_edit->resetWithFigure(figure,false);
        explicit_intersect_edit->resetWithFigure(figure,false);
        explicit_feature_edit->resetWithFigure(figure,false);

        radial_star_edit->resetWithFigure(figure,false);
        radial_rosette_edit->resetWithFigure(figure,false);
        connect_rosette_edit->resetWithFigure(figure,false);
        connect_star_edit->resetWithFigure(figure,false);
        ex_star_edit->resetWithFigure(figure,false);
        ex_rosette_edit->resetWithFigure(figure,false);
        return;
    }

    // DAC note:  When a design element is creeated from a feature in the tiling it defaults to a rosette
    // So everything starts as a rosette (except for explicit)

    qDebug() << "masterResetWithFigure fig:" << figure->getFigureDesc();

    choiceCombo2->updateChoices(figure);

    eFigType figType = figure->getFigType();

    switch (figType)
    {
    case FIG_TYPE_UNDEFINED:
    case FIG_TYPE_RADIAL:
        qCritical("unexpected figure type");
        break;
    case FIG_TYPE_EXTENDED_STAR:
        selectCurrentEditor(ex_star_edit);
        ex_star_edit->resetWithFigure(figure,false);
        break;
    case FIG_TYPE_EXTENDED_ROSETTE:
        selectCurrentEditor(ex_rosette_edit);
        ex_rosette_edit->resetWithFigure(figure,false);
        break;
    case FIG_TYPE_STAR:
        selectCurrentEditor(radial_star_edit);
        radial_star_edit->resetWithFigure(figure,false);
        break;
    case FIG_TYPE_ROSETTE:
        selectCurrentEditor(radial_rosette_edit);
        radial_rosette_edit->resetWithFigure(figure,false);
        break;
    case FIG_TYPE_CONNECT_ROSETTE:
        selectCurrentEditor(connect_rosette_edit);
        connect_rosette_edit->resetWithFigure(figure,false);
        break;
    case FIG_TYPE_CONNECT_STAR:
        selectCurrentEditor(connect_star_edit);
        connect_star_edit->resetWithFigure(figure,false);
        break;
    case FIG_TYPE_EXPLICIT:
        selectCurrentEditor(explicit_edit);
        explicit_edit->resetWithFigure(figure,false);
        break;
    case FIG_TYPE_EXPLICIT_INFER:
        selectCurrentEditor(explcit_infer_edit);
        explcit_infer_edit->resetWithFigure(figure,false);
        break;
    case FIG_TYPE_EXPLICIT_ROSETTE:
        selectCurrentEditor(explicit_rosette_edit);
        explicit_rosette_edit->resetWithFigure(figure,false);
        break;
    case FIG_TYPE_EXPLICIT_HOURGLASS:
        selectCurrentEditor(explicit_hourglass_edit);
        explicit_hourglass_edit->resetWithFigure(figure,false);
        break;
    case FIG_TYPE_EXPLICIT_INTERSECT:
        selectCurrentEditor(explicit_intersect_edit);
        explicit_intersect_edit->resetWithFigure(figure,false);
        break;
    case FIG_TYPE_EXPLICIT_GIRIH:
        selectCurrentEditor(explicit_girih_edit);
        explicit_girih_edit->resetWithFigure(figure,false);
        break;
    case FIG_TYPE_EXPLICIT_STAR:
        selectCurrentEditor(explict_star_edit);
        explict_star_edit->resetWithFigure(figure,false);
        break;
    case FIG_TYPE_EXPLICIT_FEATURE:
        selectCurrentEditor(explicit_feature_edit);
        explicit_feature_edit->resetWithFigure(figure,false);
        break;
    }

    choiceCombo2->select(figType);

    figureTypeChanged(figType, false);
}

void MasterFigureEditor::selectCurrentEditor(FigureEditor* fe)
{
    mfw->setEditor(fe);
    adjustSize();
}

void MasterFigureEditor::slot_figureTypeChanged(eFigType type)
{
    if (!masterFigure)
    {
        qWarning("MasterFigureEditor::figureChoiceSelected - no figure");
        return;
    }

    figureTypeChanged(type,true);

}

void MasterFigureEditor::figureTypeChanged(eFigType type, bool doEmit)
{
    FigureEditor * editor = getEditor(type);
    if (editor)
    {
        selectCurrentEditor(editor);
        editor->resetWithFigure(masterFigure,doEmit);
    }
}

FigureEditor * MasterFigureEditor::getEditor(eFigType type)
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
/// MasterFigureWidget
////////////////////////////////////////////////////////

MasterFigureWidget::MasterFigureWidget()
{
}

MasterFigureWidget::MasterFigureWidget(FigureEditor * fe)
{
    setFixedWidth(600);
    AQVBoxLayout * aLayout = new AQVBoxLayout();
    aLayout->addWidget(fe);
    setLayout(aLayout);
}

void MasterFigureWidget::setEditor(FigureEditor * fe)
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

FigTypeChoiceCombo:: FigTypeChoiceCombo(MasterFigureEditor *editor)
{
    setFixedWidth(221);

    connect(this, QOverload<int>::of(&QComboBox::currentIndexChanged), this,  &FigTypeChoiceCombo::slot_figureTypeSelected);
    connect(this, &FigTypeChoiceCombo::sig_figureTypeChanged,          editor, &MasterFigureEditor::slot_figureTypeChanged);
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
        qDebug() << "MasterFigureEditor - set choice index=" << index;
    }
    else
    {
        setCurrentIndex(0);
        qDebug() << "MasterFigureEditor - forcing choice index=0";
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

void FigTypeChoiceCombo::select(eFigType figType)
{
    int index = getChoiceIndex(figType);

    blockSignals(true);
    setCurrentIndex(index);
    blockSignals(false);
}

void FigTypeChoiceCombo::slot_figureTypeSelected(int index)
{
    Q_UNUSED(index);

    QVariant qv = currentData();
    eFigType type = static_cast<eFigType>(qv.toInt());
    qDebug() << "MasterFigureEditor type="  << sFigType[type];

    emit sig_figureTypeChanged(type);
}
