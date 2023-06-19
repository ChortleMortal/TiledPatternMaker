#pragma once
#ifndef MOTIFMAKERWIDGET_H
#define MOTIFMAKERWIDGET_H

#include <QWidget>

typedef std::shared_ptr<class DesignElement>       DesignElementPtr;
typedef std::shared_ptr<class DesignElementButton> DELBtnPtr;
typedef std::shared_ptr<class Motif>               MotifPtr;
typedef std::shared_ptr<class Prototype>           ProtoPtr;

// This widget is housed by page_motif_maker.
// It provides selection and display of DesignElements (DELs)
// and its maker (controller) functions both select motifs
// and modify them
// However DELs (particularly their tiles) can
// be changed externally, so the particular DEL must be
// refreshed here and the only way to do this is to select
// the DEL and rebuild the motif

class MotifMakerWidget : public QWidget
{
    Q_OBJECT

public:
    MotifMakerWidget();

   void selectPrototype(ProtoPtr proto);
   void selectDEL(DesignElementPtr designElement);

public slots:
    void slot_selectMotifButton(DELBtnPtr btn);
    void setCurrentButtonViewTransform();

private:
    class Configuration       * config;
    class DELSelectorWidget   * delSelector;
    class MotifEditorWidget   * motifEditor;
    class DesignElementButton * viewerBtn;
    class PrototypeData       * motifViewData;
};

#endif // MOTIFMAKERWIDGET_H
