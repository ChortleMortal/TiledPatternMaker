#ifndef MOTIF_EDITOR_H
#define MOTIF_EDITOR_H

#include <QComboBox>
#include "widgets/panel_misc.h"
#include "enums/efigtype.h"

class FigureEditor;
class Configuration;

typedef std::shared_ptr<class Figure>           FigurePtr;
typedef std::shared_ptr<class DesignElement>    DesignElementPtr;
typedef std::weak_ptr<class DesignElement>      WeakDesignElementPtr;

class MotifFigureWidget : public AQWidget
{
public:
    MotifFigureWidget();
    MotifFigureWidget(FigureEditor * fe);

    void setEditor(FigureEditor * fe);
};


class FigTypeChoiceCombo : public QComboBox
{
    Q_OBJECT

public:
    FigTypeChoiceCombo(class MotifEditor * editor);

    void updateChoices(FigurePtr figure);
    void addChoice(eFigType type, QString name);
    int  getChoiceIndex(eFigType type);


signals:
    void sig_figureTypeChanged(eFigType);

private slots:
    void slot_figureTypeSelected(int index);
};

////////////////////////////////////////////////////////////////////////////
//
// MasterFigureEditor.java
//
// The top-level figure editor that understands the complete range of
// figure editors available in the applet and branches out to the right
// kind of editor as the DesignElement being edited is changed.

class MotifEditor : public QWidget
{
    Q_OBJECT

public:
    MotifEditor(class page_motif_maker * menu);

    void  selectFigure(DesignElementPtr del);

public slots:
    void slot_figureTypeChanged(eFigType type);

protected:
    void  selectCurrentEditor(FigureEditor* fe);
    FigureEditor * getEditor(eFigType type);

private:
    WeakDesignElementPtr            currentDesignElement;

    Configuration                 * config;

    // Explicit figure editors.
    class ExplicitEditor          * explicit_edit;
    class ExplicitInferEditor     * explcit_infer_edit;
    class ExplicitStarEditor      * explict_star_edit;
    class ExplicitRosetteEditor   * explicit_rosette_edit;
    class ExplicitHourglassEditor * explicit_hourglass_edit;
    class ExplicitGirihEditor     * explicit_girih_edit;
    class ExplicitIntersectEditor * explicit_intersect_edit;
    class ExplicitFeatureEditor   * explicit_feature_edit;

    // Radial figure editors.
    class StarEditor	          * radial_star_edit;
    class RosetteEditor           * radial_rosette_edit;
    class ConnectRosetteEditor    * connect_rosette_edit;
    class ConnectStarEditor       * connect_star_edit;
    class ExtendedStarEditor      * ex_star_edit;
    class ExtendedRosetteEditor   * ex_rosette_edit;

    QHBoxLayout             * comboLayout;
    FigTypeChoiceCombo      * choiceCombo;
    MotifFigureWidget       * mfw;
};

#endif

