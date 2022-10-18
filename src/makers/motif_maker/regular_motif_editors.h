#ifndef REGULAR_MOTIF_EDITORS_H
#define REGULAR_MOTIF_EDITORS_H

#include "widgets/panel_misc.h"

class QCheckBox;

class page_motif_maker;
class SliderSet;
class DoubleSliderSet;
class AQWidget;

typedef std::shared_ptr<class Motif>                MotifPtr;
typedef std::shared_ptr<class Star>                 StarPtr;
typedef std::shared_ptr<class Rosette>              RosettePtr;
typedef std::shared_ptr<class StarConnect>          StarConnectPtr;
typedef std::shared_ptr<class RosetteConnect>       RosetteConnectPtr;
typedef std::shared_ptr<class ExtendedStar>         ExtStarPtr;
typedef std::shared_ptr<class ExtendedRosette>      ExtRosettePtr;
typedef std::shared_ptr<class DesignElement>        DesignElementPtr;

typedef std::weak_ptr<class Motif>                  WeaskMotifPtr;
typedef std::weak_ptr<class Star>                   WeakStarPtr;
typedef std::weak_ptr<class Rosette>                WeakRosettePtr;
typedef std::weak_ptr<class StarConnect>            WeakStarConnectPtr;
typedef std::weak_ptr<class RosetteConnect>         WeakRosetteConnectPtr;
typedef std::weak_ptr<class ExtendedStar>           WeakExtStarPtr;
typedef std::weak_ptr<class ExtendedRosette>        WeakExtRosettePtr;

// An abstract class for containing the controls related to the editing
// of one kind of motif.  A complex hierarchy of  Motif Editors gets built
// up to become the changeable controls for editing motifs in MotifMaker.
class NamedMotifEditor : public AQWidget
{
    Q_OBJECT

public:
    NamedMotifEditor(page_motif_maker * fm, QString motifName);

    virtual void    setMotif(DesignElementPtr del, bool doEmit) = 0;
            void    setMotif(MotifPtr fig, bool doEmit);

    void            addLayout(QBoxLayout * layout) { vbox->addLayout(layout);}
    void            addWidget(QWidget    * widget) { vbox->addWidget(widget);}

signals:
    void            sig_motif_modified(MotifPtr fig);

protected:
    virtual void    editorToMotif(bool doEmit);
    virtual void    motifToEditor();

    WeaskMotifPtr   wMotif;
    QString         name;

    page_motif_maker * menu;
    AQVBoxLayout     * vbox;

    DoubleSliderSet	* motifScale;
    DoubleSliderSet	* boundaryScale;
    SliderSet       * boundarySides;
    DoubleSliderSet * motifRotate;
};

class StarEditor : public NamedMotifEditor
{
    Q_OBJECT

public:
    StarEditor(page_motif_maker * fm, QString figname);

    virtual void setMotif(DesignElementPtr del, bool doEmit) override;
            void setMotif(std::shared_ptr<Star>(star), bool doEmit);

protected:
    virtual void editorToMotif(bool doEmit) override;
    virtual void motifToEditor() override;

    SliderSet       *   n_slider;
    DoubleSliderSet	*	d_slider;
    SliderSet		*	s_slider;

private:
    WeakStarPtr			wstar;
};


class RosetteEditor : public NamedMotifEditor
{
    Q_OBJECT

public:
    RosetteEditor(page_motif_maker *fm, QString figname);

    virtual void setMotif(DesignElementPtr del, bool doEmit) override;
            void setMotif(std::shared_ptr<Rosette>(rosette), bool doEmit);

protected:
    virtual void editorToMotif(bool doEmit) override;
    virtual void motifToEditor() override;

    SliderSet       *   n_slider;
    DoubleSliderSet	*	q_slider;
    DoubleSliderSet	*	k_slider;
    SliderSet       *   s_slider;

private:
    WeakRosettePtr      wrosette;
};

class ConnectStarEditor : public StarEditor
{
    Q_OBJECT

public:
    ConnectStarEditor(page_motif_maker * fm, QString figname);

    virtual void setMotif(DesignElementPtr del, bool doEmit) override;
            void setMotif(std::shared_ptr<StarConnect>(starcon), bool doEmit);

public slots:

protected:
    void calcScale();

    QPushButton * defaultBtn;

private:
    WeakStarConnectPtr wstarConnect;
};

class ConnectRosetteEditor : public RosetteEditor
{
    Q_OBJECT

public:
    ConnectRosetteEditor(page_motif_maker *fm, QString figname);

    virtual void setMotif(DesignElementPtr del, bool doEmit) override;
            void setMotif(std::shared_ptr<RosetteConnect>(rosettecon), bool doEmit);

protected:
    void    calcScale();

    QPushButton * defaultBtn;

private:
    WeakRosetteConnectPtr wrosetteConnect;
};

class ExtendedStarEditor : public StarEditor
{
    Q_OBJECT

public:
    ExtendedStarEditor(page_motif_maker * fm, QString figname);

    virtual void setMotif(DesignElementPtr del, bool doEmit) override;
    void setMotif(std::shared_ptr<ExtendedStar>(extended), bool doEmit);

protected:
    virtual void editorToMotif(bool doEmit) override;
    virtual void motifToEditor() override;

private:
    WeakExtStarPtr    wextended;

    QCheckBox       * extendPeriphBox;
    QCheckBox       * extendFreeBox;
    QCheckBox       * connectBoundaryBox;
};

class ExtendedRosetteEditor : public RosetteEditor
{
    Q_OBJECT

public:
    ExtendedRosetteEditor(page_motif_maker *fm, QString figname);

    virtual void setMotif(DesignElementPtr del, bool doEmit) override;
            void setMotif(std::shared_ptr<ExtendedRosette>(extended), bool doEmit);

protected:
    virtual void editorToMotif(bool doEmit) override;
    virtual void motifToEditor() override;

private:
    WeakExtRosettePtr wextended;

    QCheckBox       * extendPeriphBox;
    QCheckBox       * extendFreeBox;
    QCheckBox       * connectBoundaryBox;
};

#endif

