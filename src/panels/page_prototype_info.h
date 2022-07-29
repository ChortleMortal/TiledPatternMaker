#ifndef PAGE_PROTOS_H
#define PAGE_PROTOS_H

#include <QGridLayout>
#include "widgets/panel_page.h"
#include "settings/configuration.h"
#include "mosaic/prototype.h"
#include "viewers/prototype_view.h"

typedef std::weak_ptr<DesignElement>    WeakDesignElementPtr;
typedef std::weak_ptr<Prototype>        WeakPrototypePtr;
typedef std::weak_ptr<Figure>           WeakFigurePtr;
typedef std::weak_ptr<Feature>          WeakFeaturePtr;

enum eProtoCol
{
    PROTO_ROW_PROTO,
    PROTO_ROW_TILING,
    PROTO_ROW_DEL,
    PROTO_ROW_FEATURE,
    PROTO_ROW_FIGURE,
    PROTO_ROW_SCALE,
    PROTO_ROW_ROT,
    PROTO_ROW_X,
    PROTO_ROW_Y
};

struct sColData
{
    WeakPrototypePtr     wpp;
    WeakDesignElementPtr wdel;
    WeakFeaturePtr       wfeatp;
    WeakFigurePtr        wfigp;
};

class page_prototype_info : public panel_page
{
    Q_OBJECT

public:

    page_prototype_info(ControlPanel * cpanel);

    void onRefresh() override;
    void onEnter() override;
    void onExit() override;

public slots:

private slots:
    void    slot_prototypeSelected(int row, int col);
    void    drawDELClicked(bool enb);
    void    drawMapClicked(bool enb);
    void    drawFigureClicked(bool enb);
    void    drawFeatureClicked(bool enb);
    void    hiliteFigureClicked(bool enb);
    void    hiliteFeatureClicked(bool enb);
    void    setDefaultColors();
    void    setup();

protected:
    void    buildColorGrid();
    void    setProtoViewMode(eProtoViewMode mode, bool enb);
    void    pickColor(QColor & color);

private:
    class AQTableWidget * protoTable;
    QGridLayout         * showSettings;
    PrototypeViewPtr      pview;

    QVector<sColData>     data;
};

#endif
