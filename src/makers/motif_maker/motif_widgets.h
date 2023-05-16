#pragma once
#ifndef MOTIFWIDGETS_H
#define MOTIFWIDGETS_H

#include <QComboBox>
#include "widgets/panel_misc.h"
#include "enums/emotiftype.h"

class SliderSet;
class DoubleSliderSet;

typedef std::shared_ptr<class Motif>            MotifPtr;
typedef std::shared_ptr<class DesignElement>    DesignElementPtr;
typedef std::weak_ptr<class Motif>              WeakMotifPtr;
typedef std::weak_ptr<class DesignElement>      WeakDesignElementPtr;

// An abstract class for containing the controls related to the editing
// of one kind of motif.  A complex hierarchy of  Motif Editors gets built
// up to become the changeable controls for editing motifs in MotifMaker.

class NamedMotifEditor : public AQWidget
{
    Q_OBJECT

public:
    NamedMotifEditor(QString motifName);

    virtual void    setMotif(DesignElementPtr del, bool doEmit) = 0;
            void    setMotif(MotifPtr motif, bool doEmit);

    void            addLayout(QBoxLayout * layout) { vbox->addLayout(layout);}
    void            addWidget(QWidget    * widget) { vbox->addWidget(widget);}

signals:
    void            sig_motif_modified(MotifPtr motif);

public slots:
    void            slot_motifModified(MotifPtr motif);
protected:
    virtual void    editorToMotif(bool doEmit);
    virtual void    motifToEditor();

    WeakDesignElementPtr wDel;
    WeakMotifPtr         wMotif;
    QString              name;

    AQVBoxLayout    * vbox;

    DoubleSliderSet	* boundaryScale;
    SliderSet       * boundarySides;
    DoubleSliderSet	* motifScale;
    DoubleSliderSet * motifRotate;
    SliderSet       * motifSides;
};

class SpecificEditorWidget : public AQWidget
{
public:
    SpecificEditorWidget();
    SpecificEditorWidget(NamedMotifEditor * fe);

    void setEditor(NamedMotifEditor * fe);
};

class MotifTypeCombo : public QComboBox
{
    Q_OBJECT

public:
    MotifTypeCombo();

    void updateChoices(MotifPtr motif);
    void add(eMotifType type, QString name);
    int  getIndex(eMotifType type);

signals:
    void sig_motifTypeChanged(eMotifType);

private slots:
    void slot_motifTypeSelected(int index);
};

#endif // MOTIFWIDGETS_H
