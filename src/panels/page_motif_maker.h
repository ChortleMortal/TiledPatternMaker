#ifndef PAGE_MOTIF_MAKER_H
#define PAGE_MOTIF_MAKER_H

#include <QScrollArea>

#include "widgets/panel_page.h"
#include "enums/efigtype.h"
#include "widgets/panel_misc.h"

class QComboBox;
class QCheckBox;

typedef std::shared_ptr<class Prototype>        PrototypePtr;
typedef std::shared_ptr<class Feature>          FeaturePtr;
typedef std::shared_ptr<class Figure>           FigurePtr;
typedef std::shared_ptr<class FeatureButton>    FeatureBtnPtr;

class page_motif_maker : public panel_page
{
    Q_OBJECT

public:
    page_motif_maker(ControlPanel * cpanel);

    void	onRefresh(void) override;
    void    onEnter() override;
    void    onExit() override;

    void     select(PrototypePtr prototype);
    FeaturePtr getActiveFeature();

public slots:
    void    slot_figureModified(FigurePtr fig);
    void    slot_figureTypeChanged(eFigType type);
    void    slot_selectFeatureButton(FeatureBtnPtr fb);
    void    slot_tilingChoicesChanged();
    void    slot_featureChanged();
    void    slot_tilingChanged();

private slots:
    void    whiteClicked(bool state);
    void    replicateRadialClicked(bool state);
    void    multiClicked(bool state);
    void    slot_combine();
    void    slot_duplicateCurrent();
    void    slot_deleteCurrent();
    void    slot_editCurrent();
    void    slot_prototypeSelected(int);

protected:
    AQWidget * createMotifWidget();

    void setPrototype(PrototypePtr proto);
    void setButtonTransforms();
    void figureModified(FigurePtr fp);

private:
    QCheckBox   * whiteBackground;

    QComboBox   * tilingListBox;

    class FeatureSelector    * featureSelector;
    class MotifEditor        * motifEditor;
    FeatureBtnPtr              viewerBtn;

    class MotifMaker         * motifMaker;
};

#endif
