#ifndef FIGURE_EDITORS_H
#define FIGURE_EDITORS_H

#include "widgets/panel_misc.h"

class QCheckBox;

class page_motif_maker;
class SliderSet;
class DoubleSliderSet;
class AQWidget;

typedef std::shared_ptr<class Figure>               FigurePtr;
typedef std::shared_ptr<class Star>                 StarPtr;
typedef std::shared_ptr<class Rosette>              RosettePtr;
typedef std::shared_ptr<class StarConnectFigure>    StarConnectPtr;
typedef std::shared_ptr<class RosetteConnectFigure> RosetteConnectPtr;
typedef std::shared_ptr<class ExtendedStar>         ExtStarPtr;
typedef std::shared_ptr<class ExtendedRosette>      ExtRosettePtr;
typedef std::shared_ptr<class DesignElement>        DesignElementPtr;

typedef std::weak_ptr<class Figure>                 WeakFigurePtr;
typedef std::weak_ptr<class Star>                   WeakStarPtr;
typedef std::weak_ptr<class Rosette>                WeakRosettePtr;
typedef std::weak_ptr<class StarConnectFigure>      WeakStarConnectPtr;
typedef std::weak_ptr<class RosetteConnectFigure>   WeakRosetteConnectPtr;
typedef std::weak_ptr<class ExtendedStar>           WeakExtStarPtr;
typedef std::weak_ptr<class ExtendedRosette>        WeakExtRosettePtr;

// An abstract class for containing the controls related to the editing
// of one kind of figure.  A complex hierarchy of FigureEditors gets built
// up to become the changeable controls for editing figures in FigureMaker.
class FigureEditor : public AQWidget
{
    Q_OBJECT

public:
    FigureEditor(page_motif_maker * fm, QString figname);

    void    setFigure(FigurePtr fig, bool doEmit);
    virtual void  setFigure(DesignElementPtr del, bool doEmit) = 0;

    void    addLayout(QBoxLayout * layout) { vbox->addLayout(layout);}
    void    addWidget(QWidget    * widget) { vbox->addWidget(widget);}

signals:
    void sig_figure_modified(FigurePtr fig);

protected:
    virtual void editorToFigure(bool doEmit);
    virtual void figureToEditor();

    WeakFigurePtr   wfigure;
    QString         name;

    page_motif_maker * menu;
    AQVBoxLayout    * vbox;

    DoubleSliderSet	* figureScale;
    DoubleSliderSet	* boundaryScale;
    SliderSet       * boundarySides;
    DoubleSliderSet * figureRotate;
};

class StarEditor : public FigureEditor
{
    Q_OBJECT

public:
    StarEditor(page_motif_maker * fm, QString figname);

    virtual void setFigure(DesignElementPtr del, bool doEmit) override;

protected:
    virtual void editorToFigure(bool doEmit) override;
    virtual void figureToEditor() override;

    SliderSet       *   n_slider;
    DoubleSliderSet	*	d_slider;
    SliderSet		*	s_slider;

private:
    WeakStarPtr			wstar;
};


class RosetteEditor : public FigureEditor
{
    Q_OBJECT

public:
    RosetteEditor(page_motif_maker *fm, QString figname);

    virtual void setFigure(DesignElementPtr del, bool doEmit) override;

protected:
    virtual void editorToFigure(bool doEmit) override;
    virtual void figureToEditor() override;

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

    virtual void setFigure(DesignElementPtr del, bool doEmit) override;

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

    virtual void setFigure(DesignElementPtr del, bool doEmit) override;

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

    virtual void setFigure(DesignElementPtr del, bool doEmit) override;

protected:
    virtual void editorToFigure(bool doEmit) override;
    virtual void figureToEditor() override;

private:
    WeakExtStarPtr    wextended;

    QCheckBox       * extendBox1;
    QCheckBox       * extendBox2;

};

class ExtendedRosetteEditor : public RosetteEditor
{
    Q_OBJECT

public:
    ExtendedRosetteEditor(page_motif_maker *fm, QString figname);

    virtual void setFigure(DesignElementPtr del, bool doEmit) override;

protected:
    virtual void editorToFigure(bool doEmit) override;
    virtual void figureToEditor() override;

private:
    WeakExtRosettePtr wextended;

    QCheckBox       * extendPeriphBox;
    QCheckBox       * extendFreeBox;
    QCheckBox       * connectBoundaryBox;
};

#endif

