#ifndef MOTIFDISPLAYWIDGET_H
#define MOTIFDISPLAYWIDGET_H

#include "panels/panel_misc.h"
#include "base/shared.h"

class MotifDisplayWidget : public AQWidget
{
    Q_OBJECT

public:
    MotifDisplayWidget(class page_motif_maker *menu);

    void prototypeChanged();
    void figureChanged();

    void setActiveFeature(FeatureBtnPtr fb);

public slots:
    void slot_launcherButton();

private:
    class FeatureLauncher    * launcher;
    class MasterFigureEditor * masterEdit;
    FeatureBtnPtr              viewerBtn;

    class MotifMaker         * motifMaker;
    class page_motif_maker   * menu;
};

#endif // MOTIFDISPLAYWIDGET_H
