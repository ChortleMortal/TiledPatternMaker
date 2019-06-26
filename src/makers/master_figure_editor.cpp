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

#include "master_figure_editor.h"

#include "figure_maker.h"
#include "base/tiledpatternmaker.h"
#include "tapp/Star.h"
#include "tapp/ExtendedStar.h"
#include "tapp/ExtendedRosette.h"

Q_DECLARE_METATYPE(FigureEditor *)

////////////////////////////////////////////////////////////////////////////
//
// MasterFigureEditor.java
//
// The top-level figure editor that understands the complete range of
// figure editors available in the applet and branches out to the right
// kind of editor as the DesignElement being edited is changed.

MasterFigureEditor::MasterFigureEditor(FigureMaker * ed)
{
    figureMaker    = ed;

    // Explicit figure editors.
    explicit_edit            = new ExplicitEditor(ed,"explcit_edit");
    explcit_infer_edit      = new ExplicitInferEditor(ed,"explcit_infer_edit");
    explict_star_edit       = new ExplicitStarEditor(ed,"explict_star_edit");
    explicit_rosette_edit   = new ExplicitRosetteEditor(ed,"explicit_rosette_edit");
    explicit_hourglass_edit = new ExplicitHourglassEditor(ed,"explicit_hourglass_edit");
    explicit_girih_edit     = new ExplicitGirihEditor(ed,"explicit_girih_edit");
    explicit_intersect_edit = new ExplicitIntersectEditor(ed,"explicit_intersect_edit");
    explicit_feature_edit   = new ExplicitFeatureEditor(ed,"explicit_feature_edit");

    // Radial figure editors.
    radial_star_edit       = new StarEditor(ed,"radial_star_edit");
    radial_rosette_edit    = new RosetteEditor(ed,"radial_rosette_edit");
    connect_rosette_edit   = new ConnectRosetteEditor(ed,"connect_rosette_edit");
    connect_star_edit      = new ConnectStarEditor(ed,"connect_star_edit");
    ex_star_edit           = new ExtendedStarEditor(ed,"ex_star_edit");
    ex_rosette_edit        = new ExtendedRosetteEditor(ed,"ex_rosette_edit");

    // Panel containing the editors.
    mfw                   = new MasterFigureWidget();

    QHBoxLayout * hbox = new QHBoxLayout();
    choiceCombo2       = new QComboBox();
    choiceCombo2->setFixedWidth(221);

    addChoice( FIG_TYPE_STAR, "Star");
    addChoice( FIG_TYPE_CONNECT_STAR, "Star Connect");
    addChoice( FIG_TYPE_EXTENDED_STAR, "Star Extended");

    addChoice( FIG_TYPE_ROSETTE,"Rosette");
    addChoice( FIG_TYPE_CONNECT_ROSETTE, "Rosette Connect");
    addChoice( FIG_TYPE_EXTENDED_ROSETTE, "Rosette Extended");


    addChoice( FIG_TYPE_EXPLICIT, "Explicit");
    addChoice( FIG_TYPE_INFER, "Infer Explicit");
    addChoice( FIG_TYPE_EXPLICIT_STAR, "Star Explicit");
    addChoice( FIG_TYPE_EXPLICIT_ROSETTE, "Rosette Explicit");
    addChoice( FIG_TYPE_HOURGLASS,"Hourglass Explicit");
    addChoice( FIG_TYPE_GIRIH, "Girih Tiles Explicit");
    addChoice( FIG_TYPE_INTERSECT, "Intersect Explicit");
    addChoice( FIG_TYPE_FEATURE, "Feature Explicit");

    hbox->addStretch();
    hbox->addWidget(choiceCombo2);
    hbox->addStretch();

    QVBoxLayout * layout = new QVBoxLayout();
    layout->addWidget(mfw);
    layout->addLayout(hbox);
    setLayout(layout);

    connect(choiceCombo2, SIGNAL(currentIndexChanged(int)),  this, SLOT(slot_figureChoiceSelected(int)));
    connect(this, &MasterFigureEditor::sig_figure_changed,   ed,   &FigureMaker::slot_figureChanged);
}

FigurePtr MasterFigureEditor::getFigureFromEditor()
{
    FigurePtr fp;
    FigureEditor * fed = getCurrentEditor();
    if (fed)
    {
        fp = fed->getFigure();
    }
    return fp;
}


void MasterFigureEditor::MasterResetWithFigure(FigurePtr figure, FeatureBtnPtr btn)
{
    masterFigure = figure;
    if (!masterFigure)
    {
        explicit_edit->resetWithFigure(figure);
        explcit_infer_edit->resetWithFigure(figure);
        explict_star_edit->resetWithFigure(figure);
        explicit_rosette_edit->resetWithFigure(figure);
        explicit_hourglass_edit->resetWithFigure(figure);
        explicit_girih_edit->resetWithFigure(figure);
        explicit_intersect_edit->resetWithFigure(figure);
        explicit_feature_edit->resetWithFigure(figure);

        radial_star_edit->resetWithFigure(figure);
        radial_rosette_edit->resetWithFigure(figure);
        connect_rosette_edit->resetWithFigure(figure);
        connect_star_edit->resetWithFigure(figure);
        ex_star_edit->resetWithFigure(figure);
        ex_rosette_edit->resetWithFigure(figure);
        return;
    }

    // DAC note:  When a design element is creeated from a feature in the tiling it defaults to a rosette
    // So everything starts as a rosette (except for explicit)

    currentButton = btn;
    updateChoices(figure);

    eFigType figType = figure->getFigType();

    switch (figType)
    {
    case FIG_TYPE_UNDEFINED:
    case FIG_TYPE_RADIAL:
        qCritical("unexpected figure type");
        break;
    case FIG_TYPE_EXTENDED_STAR:
        selectCurrentEditor(ex_star_edit);
        ex_star_edit->resetWithFigure(figure);
        break;
    case FIG_TYPE_EXTENDED_ROSETTE:
        selectCurrentEditor(ex_rosette_edit);
        ex_rosette_edit->resetWithFigure(figure);
        break;
    case FIG_TYPE_STAR:
        selectCurrentEditor(radial_star_edit);
        radial_star_edit->resetWithFigure(figure);
        break;
    case FIG_TYPE_ROSETTE:
        selectCurrentEditor(radial_rosette_edit);
        radial_rosette_edit->resetWithFigure( figure );
        break;
    case FIG_TYPE_CONNECT_ROSETTE:
        selectCurrentEditor(connect_rosette_edit);
        connect_rosette_edit->resetWithFigure(figure);
        break;
    case FIG_TYPE_CONNECT_STAR:
        selectCurrentEditor(connect_star_edit);
        connect_star_edit->resetWithFigure(figure);
        break;
    case FIG_TYPE_EXPLICIT:
        selectCurrentEditor(explicit_edit);
        explicit_edit->resetWithFigure(figure);
        break;
    case FIG_TYPE_INFER:
        selectCurrentEditor(explcit_infer_edit);
        explcit_infer_edit->resetWithFigure(figure);
        break;
    case FIG_TYPE_EXPLICIT_ROSETTE:
        selectCurrentEditor(explicit_rosette_edit);
        explicit_rosette_edit->resetWithFigure(figure);
        break;
    case FIG_TYPE_HOURGLASS:
        selectCurrentEditor(explicit_hourglass_edit);
        explicit_hourglass_edit->resetWithFigure(figure);
        break;
    case FIG_TYPE_INTERSECT_PROGRESSIVE:
    case FIG_TYPE_INTERSECT:
        selectCurrentEditor(explicit_intersect_edit);
        explicit_intersect_edit->resetWithFigure(figure);
        break;
    case FIG_TYPE_GIRIH:
        selectCurrentEditor(explicit_girih_edit);
        explicit_girih_edit->resetWithFigure(figure);
        break;
    case FIG_TYPE_EXPLICIT_STAR:
        selectCurrentEditor(explict_star_edit);
        explict_star_edit->resetWithFigure(figure);
        break;
    case FIG_TYPE_FEATURE:
        selectCurrentEditor(explicit_feature_edit);
        explicit_feature_edit->resetWithFigure(figure);
        break;
    }

    int index = getChoiceIndex(figType);
    choiceCombo2->blockSignals(true);
    choiceCombo2->setCurrentIndex(index);
    choiceCombo2->blockSignals(false);

    figureChoiceSelected(index);    // not the slot
}

void MasterFigureEditor::selectCurrentEditor(FigureEditor* fe)
{
    mfw->setEditor(fe);

    adjustSize();
}

void MasterFigureEditor::slot_figureChoiceSelected(int index)
{
    figureChoiceSelected(index);
    emit sig_figure_changed();
}

void MasterFigureEditor::figureChoiceSelected(int index)
{
    qDebug().noquote() << "MasterFigureEditor::figureChoiceSelected" << "unused index=" << index;
    qDebug().noquote() << "MasterFigureEditor::figureChoiceSelected" << choiceCombo2->currentText() << choiceCombo2->currentIndex();
    Q_ASSERT(index == choiceCombo2->currentIndex());

    QVariant qv = choiceCombo2->currentData();
    eFigType type = static_cast<eFigType>(qv.toInt());
    qDebug() << "MasterFigureEditor type="  << sFigType[type];

    FigurePtr fp = masterFigure;
    if (!fp)
    {
        qWarning("MasterFigureEditor::figureChoiceSelected - no figure");
        return;
    }

    FigureEditor * fe  =  getEditor(type);
    selectCurrentEditor(fe);
    editorsPerButton[currentButton.lock()] = fe;

    fe->resetWithFigure(fp);        // DAC

    PrototypePtr pp = figureMaker->getPrototype();
    //qDebug() << "pp="  << pp.get();
    Workspace::getInstance()->setWSPrototype(pp);
}

void MasterFigureEditor::updateChoices(FigurePtr figure)
{
    choiceCombo2->blockSignals(true);

    int index = getChoiceIndex(figure->getFigType());
    if (index >= 0)
    {
        choiceCombo2->setCurrentIndex(index);
        qDebug() << "MasterFigureEditor - set choice index=" << index;
    }
    else
    {
        choiceCombo2->setCurrentIndex(0);
        qDebug() << "MasterFigureEditor - forcing choice index=0";
    }

    choiceCombo2->blockSignals(false);
}

void MasterFigureEditor::addChoice(eFigType type, QString name)
{
    choiceCombo2->addItem(name,QVariant(type));
}

int MasterFigureEditor::getChoiceIndex(eFigType type)
{
    qDebug().noquote()  << "type is" << Figure::getTypeString(type);

    return choiceCombo2->findData(QVariant(type));
}

FigureEditor * MasterFigureEditor::getCurrentEditor()
{
    return editorsPerButton.value(currentButton.lock());
}

void MasterFigureEditor::reset()
{
    editorsPerButton.clear();
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
    case FIG_TYPE_INFER:
        return explcit_infer_edit;
    case FIG_TYPE_EXPLICIT_ROSETTE:
        return explicit_rosette_edit;
    case FIG_TYPE_HOURGLASS:
        return explicit_hourglass_edit;
    case FIG_TYPE_INTERSECT_PROGRESSIVE:
    case FIG_TYPE_INTERSECT:
        return explicit_intersect_edit;
    case FIG_TYPE_GIRIH:
        return  explicit_girih_edit;
    case FIG_TYPE_EXPLICIT_STAR:
        return explict_star_edit;
    case FIG_TYPE_FEATURE:
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
