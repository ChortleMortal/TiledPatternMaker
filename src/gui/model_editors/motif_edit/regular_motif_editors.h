#pragma once
#ifndef REGULAR_MOTIF_EDITORS_H
#define REGULAR_MOTIF_EDITORS_H

#include "gui/model_editors/motif_edit/motif_editors.h"

class QCheckBox;

class page_motif_maker;
class SliderSet;
class DoubleSliderSet;

typedef std::shared_ptr<class Motif>                MotifPtr;
typedef std::shared_ptr<class Star>                 StarPtr;
typedef std::shared_ptr<class Star2>                Star2Ptr;
typedef std::shared_ptr<class Rosette>              RosettePtr;
typedef std::shared_ptr<class Rosette2>             Rosette2Ptr;
typedef std::shared_ptr<class DesignElement>        DesignElementPtr;

typedef std::weak_ptr<class Motif>                  WeakMotifPtr;
typedef std::weak_ptr<class Star>                   WeakStarPtr;
typedef std::weak_ptr<class Star2>                  WeakStar2Ptr;
typedef std::weak_ptr<class Rosette>                WeakRosettePtr;
typedef std::weak_ptr<class Rosette2>               WeakRosette2Ptr;

class StarEditor : public NamedMotifEditor
{
    Q_OBJECT

public:
    StarEditor(QString name, DesignElementPtr del, bool doEmit);

    void setMotif(StarPtr star, bool doEmit);

protected:
    void editorToMotif(bool doEmit) override;
    void motifToEditor() override;

    DoubleSliderSet	* d_slider;
    SliderSet		* s_slider;
    QComboBox       * version_combo;
    QCheckBox       * chk_inscribe;
    QCheckBox       * chk_on_point;

private:
    WeakStarPtr		  wstar;
};

class Star2Editor : public NamedMotifEditor
{
    Q_OBJECT

public:
    Star2Editor(QString name, DesignElementPtr del, bool doEmit);

    void setMotif(Star2Ptr star, bool doEmit);

protected:
    void editorToMotif(bool doEmit) override;
    void motifToEditor() override;

    DoubleSliderSet	* theta_slider;
    SliderSet		* s_slider;
    QCheckBox       * chk_inscribe;
    QCheckBox       * chk_on_point;

private:
    WeakStar2Ptr      wstar;
};

class RosetteEditor : public NamedMotifEditor
{
    Q_OBJECT

public:
    RosetteEditor(QString name, DesignElementPtr del, bool doEmit);

protected:
    void setMotif(RosettePtr rosette, bool doEmit);

    void editorToMotif(bool doEmit) override;
    void motifToEditor() override;

    DoubleSliderSet	* q_slider;
    SliderSet       * s_slider;
    QCheckBox       * chk_inscribe;
    QCheckBox       * chk_on_point;

private:
    WeakRosettePtr    wrosette;
};

class Rosette2Editor : public NamedMotifEditor
{
    Q_OBJECT

public:
    Rosette2Editor(QString name, DesignElementPtr del, bool doEmit);

protected:
    void setMotif(Rosette2Ptr rosette, bool doEmit);

    void editorToMotif(bool doEmit) override;
    void motifToEditor() override;

    DoubleSliderSet	* kx_slider;
    DoubleSliderSet	* ky_slider;
    SliderSet       * s_slider;
    DoubleSliderSet	* k_slider;
    QButtonGroup    * tipGroup;
    QCheckBox       * kaplanize;
    QPushButton     * convert;
    QCheckBox       * chk_inscribe;
    QCheckBox       * chk_on_point;

private slots:
    void    convertConstrained();

private:
    WeakRosette2Ptr             wrosette;
};

#endif

