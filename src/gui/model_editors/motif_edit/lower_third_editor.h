#pragma once

#include <QComboBox>
#include "model/motifs/extender.h"

class MotifEditor;

typedef std::shared_ptr<class Motif>        MotifPtr;
typedef std::weak_ptr<class Motif>          WeakMotifPtr;

class LowerThirdWidget : public QTabWidget
{
public:
    LowerThirdWidget();

    void          set(MotifEditor * editor, MotifPtr motif);
    MotifEditor * getCurrentEditor()  { return motifEditor; }

    void          refresh();

private:
    QList<QWidget *> tabs;
    MotifEditor    * motifEditor;    // same as tab1
    WeakMotifPtr     weakMotif;
};

