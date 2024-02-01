#pragma once
#ifndef MOTIF_EDITOR_WIDGET_H
#define MOTIF_EDITOR_WIDGET_H

#include <QComboBox>
#include "enums/emotiftype.h"
#include "makers/motif_maker/motif_maker_widgets.h"

class Configuration;
class NamedMotifEditor;
class PrototypeData;

typedef std::shared_ptr<class Motif>           MotifPtr;
typedef std::shared_ptr<class DesignElement>   DesignElementPtr;
typedef std::weak_ptr<class DesignElement>     WeakDesignElementPtr;

////////////////////////////////////////////////////////////////////////////
//
// MasterFigureEditor.java
//
// The top-level motif editor that understands the complete range of
// motif editors available in the applet and branches out to the right
// kind of editor as the DesignElement being edited is changed.

class MotifEditorWidget : public QWidget
{
    Q_OBJECT

public:
    MotifEditorWidget();
    
    void  delegate(DesignElementPtr del);

public slots:
    void slot_motifTypeChanged(eMotifType type);

protected:

private:
    void  delgate(NamedMotifEditor* fe);
    NamedMotifEditor * getEditor(eMotifType type);

    WeakDesignElementPtr            delegatedDesignElement;

    // Explicit figure editors.
    class ExplicitMapEditor         * explicit_map_edit;
    class IrregularNoMapEditor      * irregular_nomap_edit;
    class InferEditor               * infer_edit;
    class IrregularStarEditor       * irregular_star_edit;
    class IrregularRosetteEditor    * irregular_rosette_edit;
    class HourglassEditor           * hourglass_edit;
    class GirihEditor               * girih_edit;
    class IntersectEditor           * intersect_edit;
    class ExplicitTileEditor        * explicit_tile_edit;

    // Radial figure editors.
    class StarEditor                * radial_star_edit;
    class Star2Editor               * radial_star2_edit;
    class RosetteEditor             * radial_rosette_edit;
    class Rosette2Editor            * radial_rosette2_edit;
    class ConnectRosetteEditor      * connect_rosette_edit;
    class ConnectStarEditor         * connect_star_edit;
    class ExtendedStarEditor        * ex_star_edit;
    class ExtendedStar2Editor       * ex_star2_edit;
    class ExtendedRosetteEditor     * ex_rosette_edit;

    Configuration                   * config;
    MotifTypeCombo                  * typeCombo;
    SpecificEditorWidget            * specificEditorWidget;
    PrototypeData                   * protoMakerData;
};

#endif

