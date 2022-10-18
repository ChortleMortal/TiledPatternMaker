#ifndef MOTIF_EDITOR_H
#define MOTIF_EDITOR_H

#include <QComboBox>
#include "widgets/panel_misc.h"
#include "enums/emotiftype.h"

class Configuration;

typedef std::shared_ptr<class Motif>           MotifPtr;
typedef std::shared_ptr<class DesignElement>   DesignElementPtr;
typedef std::weak_ptr<class DesignElement>     WeakDesignElementPtr;

class MotifWidget : public AQWidget
{
public:
    MotifWidget();
    MotifWidget(class NamedMotifEditor * fe);

    void setEditor(class NamedMotifEditor * fe);
};


class MotifTypeChoiceCombo : public QComboBox
{
    Q_OBJECT

public:
    MotifTypeChoiceCombo(class MotifEditor * editor);

    void updateChoices(MotifPtr motif);
    void addChoice(eMotifType type, QString name);
    int  getChoiceIndex(eMotifType type);


signals:
    void sig_motifTypeChanged(eMotifType);

private slots:
    void slot_motifTypeSelected(int index);
};

////////////////////////////////////////////////////////////////////////////
//
// MasterFigureEditor.java
//
// The top-level motif editor that understands the complete range of
// motif editors available in the applet and branches out to the right
// kind of editor as the DesignElement being edited is changed.

class MotifEditor : public QWidget
{
    Q_OBJECT

public:
    MotifEditor(class page_motif_maker * menu);

    void  selectMotif(DesignElementPtr del);

public slots:
    void slot_motifTypeChanged(eMotifType type);

protected:
    void  selectCurrentEditor(NamedMotifEditor* fe);
    NamedMotifEditor * getEditor(eMotifType type);

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
    class ExplicitTileEditor      * explicit_tile_edit;

    // Radial figure editors.
    class StarEditor	          * radial_star_edit;
    class RosetteEditor           * radial_rosette_edit;
    class ConnectRosetteEditor    * connect_rosette_edit;
    class ConnectStarEditor       * connect_star_edit;
    class ExtendedStarEditor      * ex_star_edit;
    class ExtendedRosetteEditor   * ex_rosette_edit;

    QHBoxLayout                   * comboLayout;
    MotifTypeChoiceCombo          * choiceCombo;
    MotifWidget                   * mfw;
};

#endif

