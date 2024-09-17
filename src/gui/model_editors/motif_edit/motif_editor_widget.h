#pragma once
#ifndef MOTIF_EDITOR_WIDGET_H
#define MOTIF_EDITOR_WIDGET_H

#include <QComboBox>
#include "sys/enums/emotiftype.h"
#include "gui/model_editors/motif_edit/motif_editors.h"

class Configuration;
class NamedMotifEditor;
class PrototypeMaker;

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

    void  onRefresh();

public slots:
    void slot_motifTypeChanged(eMotifType type);

protected slots:
    void slot_addExtender();
    void slot_deleteExtender();
    void slot_addConnector();
    void slot_cleanse();

protected:

private:
    void delegate(DesignElementPtr del, eMotifType type, bool doEmit);

    WeakDesignElementPtr    delegatedDesignElement;

    MotifTypeCombo        * typeCombo;
    SpecificEditorWidget  * specificEditorWidget;

    QPushButton           * cleanBtn;
    QLabel                * cleanStatus;
};

#endif

