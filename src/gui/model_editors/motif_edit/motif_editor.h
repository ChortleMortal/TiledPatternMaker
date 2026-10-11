#pragma once

#include <QComboBox>
#include "gui/widgets/panel_misc.h"
#include "model/motifs/extender.h"

class SliderSet;
class SpinSet;
class DoubleSliderSet;
class ExtenderEditor;
class MotifEditor;
class MotifEditorWidget;

typedef std::shared_ptr<class Motif>        MotifPtr;
typedef std::shared_ptr<class TileMotif>    TileMotifPtr;

typedef std::weak_ptr<class Motif>          WeakMotifPtr;
typedef std::weak_ptr<class TileMotif>      WeakTileMotifPtr;
typedef std::weak_ptr<class Extender>       WeakExtenderPtr;

class MotifEditor : public QWidget
{
    Q_OBJECT

public:
    MotifEditor(QString motifName);
    ~MotifEditor();

    void    setMotif(MotifPtr motif);
    void    motifModified(MotifPtr motif);

    void    addLayout(QBoxLayout * layout)      { vbox->addLayout(layout);}
    void    addWidget(QWidget    * widget)      { vbox->addWidget(widget);}

    virtual void editorToMotif(bool doEmit);

    virtual void refresh();

    QList<QWidget *> setupExtenders(MotifPtr motif);
    void      addExtender();
    void      deleteExtender();

    MotifPtr  getMotif() { return wMotif.lock(); }

signals:
    void    sig_updateView();

public slots:

protected:
    WeakMotifPtr    wMotif;
    QString          name;

    MotifEditorWidget * parent;

    AQVBoxLayout    * vbox;
    SliderSet       * motifSides;
    DoubleSliderSet	* motifScale;
    DoubleSliderSet * motifRotate;
    QLabel          * connectScale;

    QVector<ExtenderEditor *>  extenderEds;
};

