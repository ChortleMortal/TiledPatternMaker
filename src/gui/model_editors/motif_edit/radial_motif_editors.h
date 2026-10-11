#pragma once

#include "gui/model_editors/motif_edit/motif_editor.h"
#include "model/motifs/radial_motif.h"

class QCheckBox;
class QRadioButton;

class page_motif_maker;
class SliderSet;
class DoubleSliderSet;

using std::shared_ptr;
using std::weak_ptr;

typedef shared_ptr<class Motif>                MotifPtr;
typedef shared_ptr<class Star>                 StarPtr;
typedef shared_ptr<class Star2>                Star2Ptr;
typedef shared_ptr<class Rosette>              RosettePtr;
typedef shared_ptr<class Rosette2>             Rosette2Ptr;
typedef shared_ptr<class TileMotif>            TileMotifPtr;
typedef shared_ptr<class JeweledStar>          JeweledStarPtr;

typedef weak_ptr<class Motif>                  WeakMotifPtr;
typedef weak_ptr<class Star>                   WeakStarPtr;
typedef weak_ptr<class Star2>                  WeakStar2Ptr;
typedef weak_ptr<class Rosette>                WeakRosettePtr;
typedef weak_ptr<class Rosette2>               WeakRosette2Ptr;
typedef weak_ptr<class JeweledStar>            WeakJeweledStarPtr;

class RadialMotifEditor : public MotifEditor
{
public:
    RadialMotifEditor(QString name);

    void addCommonRow();

protected:
    void editorToMotif(bool doEmit) override;
    void refresh() override;

    void setMotif(shared_ptr<RadialMotif> motif);

    QHBoxLayout     * commonRadialRow;
    QCheckBox       * chk_on_point;
    QCheckBox       * chk_inscribe;

    weak_ptr<RadialMotif> weakRadial;
};

class StarEditor : public RadialMotifEditor
{
    Q_OBJECT

public:
    StarEditor(QString name, StarPtr star);

protected:
    void editorToMotif(bool doEmit) override;
    void refresh() override;

    DoubleSliderSet	* d_slider;
    SliderSet		* s_slider;

private:
    WeakStarPtr		  wstar;
};

class Star2Editor : public RadialMotifEditor
{
    Q_OBJECT

public:
    Star2Editor(QString name, Star2Ptr star);

protected:
    void editorToMotif(bool doEmit) override;
    void refresh() override;

    DoubleSliderSet	* theta_slider;
    SliderSet		* s_slider;

private:
    WeakStar2Ptr      wstar;
};

class RosetteEditor : public RadialMotifEditor
{
    Q_OBJECT

public:
    RosetteEditor(QString name, RosettePtr rose);

protected:
    void editorToMotif(bool doEmit) override;
    void refresh() override;

    DoubleSliderSet	* q_slider;
    SliderSet       * s_slider;

private:
    WeakRosettePtr    wrosette;
};

class Rosette2Editor : public RadialMotifEditor
{
    Q_OBJECT

public:
    Rosette2Editor(QString name, Rosette2Ptr rose);

protected:
    void editorToMotif(bool doEmit) override;
    void refresh() override;

    DoubleSliderSet	* kx_slider;
    DoubleSliderSet	* ky_slider;
    SliderSet       * s_slider;
    DoubleSliderSet	* k_slider;
    QCheckBox       * kaplanize;
    QPushButton     * convert;
    QLineEdit       * tipInfo;

private slots:
    void    convertConstrained();
    void    editTips();

private:
    WeakRosette2Ptr wrosette;
};

