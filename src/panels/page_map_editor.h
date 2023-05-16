#pragma once
#ifndef PAGE_FIGURE_EDITOR_H
#define PAGE_FIGURE_EDITOR_H

class QGroupBox;
class QTextEdit;
class QCheckBox;
class QToolButton;
class QRadioButton;
class QButtonGroup;

class MapEditor;
class CropMaker;

#include "widgets/panel_page.h"

typedef std::shared_ptr<class Map>       MapPtr;

class ClickableLabel;

class page_map_editor : public panel_page
{
    Q_OBJECT

public:
    page_map_editor(ControlPanel * cpanel);

    void onRefresh() override;
    void onEnter() override;
    void onExit() override;

public slots:
    void slot_mosaicLoaded(QString name);
    void slot_mosaicChanged();
    void slot_tilingLoaded (QString name);
    void slot_setMouseMode(int mode, bool checked);

private slots:
    void slot_loadMosaicPrototype();
    void slot_loadMotifPrototype();
    void slot_loadMotif();
    void slot_loadTileUnit();
    void slot_loadTileRep();
    void slot_loadMapFile();
    void slot_createMap();
    void slot_unloadMaps();

    void slot_pushMap();
    void slot_saveMapToFile();

    void slot_convertToExplicit();
    void slot_verify();
    void slot_divideIntersectingEdges();
    void slot_joinColinearEdges();
    void slot_cleanNeighbours();
    void slot_cleanVertices();
    void slot_rebuildNeighbours();
    void slot_removeUnconnectedVertices();
    void slot_removeSingleConnectVertices();
    void slot_removeZombieEdges();
    void slot_popstash();
    void slot_undoConstructionLines();
    void slot_redoConstructionLines();
    void slot_clearConstructionLines();
    void slot_dumpMap();
    void slot_editCrop();
    void slot_embedCrop();
    void slot_applyCrop();
    void slot_completeCrop();
    void slot_cleanseMap();
    void slot_saveTemplate();
    void slot_loadTemplate();

    void slot_showBounds(bool show);
    void slot_showCons(bool show);
    void slot_showMap(bool show);
    void slot_showPoints(bool show);
    void slot_showMidPoints(bool show);
    void slot_showDirPoints(bool show);
    void slot_showArcCentre(bool show);

    void slot_debugChk(bool on);
    void slot_chkViewMap(bool checked);
    void slot_chkViewDCEL(bool checked);

    void slot_radiusChanged(qreal r);
    void slot_createAngleChanged(qreal angle);
    void slot_createLenChanged(qreal len);
    void slot_lineWidthChanged(qreal r);
    void slot_consWidthChanged(qreal r);
    void slot_mergeSensitivityA(qreal r);
    void slot_mergeSensitivityB(int sens);

    void slot_viewLayer(int id, bool checked);
    void slot_editLayer(int id, bool checked);

    void slot_whiteClicked(bool state);


protected:
    QGroupBox   * createMapSelects();
    QGroupBox   * createStatusGroup();
    QGroupBox   * createSettingsGroup();
    QGroupBox   * createMapGroup();
    QGroupBox   * createEditGroup();
    QGroupBox   * createLoadGroup();
    QGroupBox   * createViewGroup();
    QGroupBox   * createPushGroup();
    QGroupBox   * createConstructionGroup();

    void    refreshStatusBox();
    void    tallySelects();
    void    tallyMapButtons();

private:
    MapEditor     * maped;
    CropMaker     * cropMaker;

    QGroupBox     * editorStatusBox;
    QTextEdit     * statusBox;
    QCheckBox     * animateChk;

    QRadioButton  * viewDCEL;

    QToolButton   * pbEmbedCrop;
    QToolButton   * pbApplyCrop;
    QToolButton   * pbCleanseVertices;

    QCheckBox     * compositeEChk;
    QCheckBox     * layer1EChk;
    QCheckBox     * layer2EChk;
    QCheckBox     * layer3EChk;

    QCheckBox     * compositeVChk;
    QCheckBox     * layer1VChk;
    QCheckBox     * layer2VChk;
    QCheckBox     * layer3VChk;
    QCheckBox     * chkWhiteBkgd;

    QCheckBox     * showConsChk;

    QButtonGroup   * editGroup;
    QButtonGroup   * viewGroup;
    QButtonGroup   * modeGroup;

    QString         lastNamedTemplate;
    QString         lastNamedMap;

    QString         defaultStyle;
};

#endif
