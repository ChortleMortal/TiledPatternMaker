#pragma once
#ifndef MOTIF_EDITORS_H
#define MOTIF_EDITORS_H

#include <QComboBox>
#include "gui/panels/panel_misc.h"
#include "sys/enums/emotiftype.h"
#include "model/motifs/extender.h"
#include "model/motifs/motif_connector.h"

class SliderSet;
class SpinSet;
class DoubleSliderSet;
class ExtenderEditor;
class ConnectorEditor;

typedef std::shared_ptr<class Motif>            MotifPtr;
typedef std::shared_ptr<class DesignElement>    DesignElementPtr;
typedef std::weak_ptr<class Motif>              WeakMotifPtr;
typedef std::weak_ptr<class DesignElement>      WeakDesignElementPtr;
typedef std::weak_ptr<class Extender>           WeakExtenderPtr;

// An abstract class for containing the controls related to the editing
// of one kind of motif.  A complex hierarchy of  Motif Editors gets built
// up to become the changeable controls for editing motifs in MotifMaker.

class NamedMotifEditor : public QWidget
{
    Q_OBJECT

public:
    NamedMotifEditor(QString motifName);

    void    setMotif(MotifPtr motif, bool doEmit);

    void    addLayout(QBoxLayout * layout) { vbox->addLayout(layout);}
    void    addWidget(QWidget    * widget) { vbox->addWidget(widget);}

    virtual void    editorToMotif(bool doEmit);
    virtual void    motifToEditor();

    void    addConnector();
    void    addExtender();
    void    deleteExtender();

signals:
    void    sig_motif_modified(MotifPtr motif);
    void    sig_redisplay(eMotifType type);
    void    sig_updateView();

public slots:
    void    slot_motifModified(MotifPtr motif);

    void    slot_displayScale();
    void    slot_deleteConnector();

protected:
    WeakDesignElementPtr wDel;
    WeakMotifPtr         wMotif;
    QString              name;

    AQVBoxLayout    * vbox;
    SliderSet       * motifSides;
    DoubleSliderSet	* motifScale;
    DoubleSliderSet * motifRotate;
    QLabel          * connectScale;

    QVector<ExtenderEditor *>  eds;
    ConnectorEditor        *   conEd;

};

class ExtenderEditor : QObject
{
    Q_OBJECT;

public:
    ExtenderEditor(ExtenderPtr extender)  { wextender = extender; }

    QVBoxLayout *   createExtenderLayout(NamedMotifEditor * parent);
    void            editorToExtender();
    void            extenderToEditor();

private:
    SpinSet         * boundarySides;
    DoubleSliderSet	* boundaryScale;
    DoubleSliderSet	* boundaryRotate;

    QCheckBox       * extendRaysChk;
    QCheckBox       * extendTipsToBoundChk;
    QCheckBox       * extendTipsToTileChk;
    QCheckBox       * embedBoundaryChk;
    QCheckBox       * embedTileChk;
    SpinSet         * connectRaysBox;

    NamedMotifEditor * editor;
    WeakExtenderPtr    wextender;
};

class ConnectorEditor : QObject
{
    Q_OBJECT;

public:
    ConnectorEditor(ConnectPtr connector)  { wconnector = connector; }

    QHBoxLayout *   createConnectorLayout(NamedMotifEditor * parent);
    void            editorToConnector();
    void            connectorToEditor();

private:
    QLabel           * connectScale;

    NamedMotifEditor * editor;
    wConnectPtr        wconnector;
};


class SpecificEditorWidget : public QWidget
{
public:
    SpecificEditorWidget();

    void setEditor(NamedMotifEditor * fe);
    NamedMotifEditor * getEditor()  { return current; }

private:
    NamedMotifEditor * current;
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
