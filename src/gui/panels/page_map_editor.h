#pragma once

#include <QActionGroup>
#include <QButtonGroup>
#include <QComboBox>
#include <QRadioButton>
#include <QGroupBox>
#include <QTextEdit>
#include "gui/map_editor/map_editor.h"
#include "gui/map_editor/map_editor_layer.h"
#include "gui/panels/panel_page.h"
#include "gui/viewers/layer.h"
#include "sys/sys/versioning.h"

class ClickableLabel;
class CropDlg;
class CropMaker;
class CropMakerView;
class MapEditorDb;
class MapEditorView;
class ResizingTextEdit;
class SMXWidget;

typedef std::shared_ptr<class Map> MapPtr;

class page_map_editor : public panel_page
{
    Q_OBJECT

    using Type    = Layer::Type;
    using MapType = MapEditorMap::MapType;
    using LayerId = MapEditorLayer::LayerId;

public:
    page_map_editor(ControlPanel * cpanel);

    void onRefresh() override;
    void onEnter() override;
    void onExit() override;

public slots:
    void slot_mosaicLoaded(VersionedFile vfile);
    void slot_mosaicChanged();
    void slot_tilingLoaded (VersionedFile vfile);
    void slot_setMouseMode(int mode, bool checked);
    void slot_cleansed();

private slots:
    void slot_import(int mapTypeId);

    void slot_pushMap();
    void slot_convertToFixedMap();
    void slot_saveMapToFile();
    void slot_loadCopies(bool show);

    void slot_convertToExplicit();
    void slot_verify();
    void slot_clearConstructionLines();
    void slot_dumpMap();
    void slot_cleanseMap();

    void slot_editCrop(bool checked);
    void slot_embedCrop();
    void slot_applyCrop();
    void slot_completeCrop();

    void slot_saveTemplate();
    void slot_loadTemplate();

    void slot_showBounds(bool show);
    void slot_showCons(bool show);
    void slot_showMap(bool show);
    void slot_showPoints(bool show);
    void slot_showMidPoints(bool show);
    void slot_showDirPoints(bool show);
    void slot_showArcCentre(bool show);
    void slot_showTileCentre(bool show);

    void slot_debugChk(bool on);
    void slot_chkViewMap(bool checked);
    void slot_chkViewDCEL(bool checked);

    void slot_radiusChanged(qreal r);
    void slot_createAngleChanged(qreal angle);
    void slot_createLenChanged(qreal len);
    void slot_lineWidthChanged(uint r);
    void slot_consWidthChanged(uint r);
    void slot_cleanseSettings();
    void slot_viewLayer(int id, bool checked);
    void slot_editLayer(int id, bool checked);
    void slot_unloadLayer(int id);

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

    void updateStyleSelections();
    void refreshStatusBox();
    void tallySelects();
    void tallyCropButtons();
    void initPageStatusString();

    void importMapFile();

private:
    MapEditor     * maped;
    MapEditorDb   * db;
    CropDlg       * cropDlg;

    QGroupBox     * constructionBox;
    QGroupBox     * editorStatusBox;
    ResizingTextEdit * statusTxt;

    QRadioButton  * viewDCEL;

    SMXWidget     * smxWidget;

    QCheckBox     * chkEditCrop;
    QPushButton   * pbEmbedCrop;
    QPushButton   * pbApplyCrop;

    QCheckBox     * compositeEChk;
    QCheckBox     * layer1EChk;
    QCheckBox     * layer2EChk;
    QCheckBox     * layer3EChk;

    QCheckBox     * compositeVChk;
    QCheckBox     * layer1VChk;
    QCheckBox     * layer2VChk;
    QCheckBox     * layer3VChk;

    QCheckBox     * showConsChk;

    QButtonGroup  * unloadGroup;
    QButtonGroup  * editGroup;
    QButtonGroup  * viewGroup;
    QButtonGroup  * modeGroup;
    QButtonGroup  * importGroup;

    QComboBox     * validStyles;    // styles which have style maps

    VersionedFile   lastNamedTemplate;
    VersionedName   lastNamedMap;

    QString         defaultStyle;
};

class ResizingTextEdit : public QTextEdit
{
    Q_OBJECT
public:
    using QTextEdit::QTextEdit;

    QSize sizeHint() const override
    {
        // Compute document height
        int docHeight = int(document()->size().height());
        int margins = contentsMargins().top() + contentsMargins().bottom();

        // Width comes from the layout, so keep default width
        return QSize(QTextEdit::sizeHint().width(), docHeight + margins);
    }
};