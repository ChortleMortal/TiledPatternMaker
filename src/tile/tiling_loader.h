#ifndef TILING_LOADER_H
#define TILING_LOADER_H

#include <QTextStream>
#include "geometry/xform.h"
#include "misc/pugixml.hpp"
#include "settings/filldata.h"

typedef std::shared_ptr<class Tiling>           TilingPtr;
typedef std::shared_ptr<class BackgroundImage>   BkgdImgPtr;

class TilingLoader
{
    friend class TilingManager;

public:
    TilingPtr   readTiling(QString file);
    TilingPtr   readTiling(QTextStream & st);
    TilingPtr   readTilingXML(QString file);
    TilingPtr   readTilingXML(pugi::xml_node & tiling_node);

    static void getBackgroundImage(pugi::xml_node & node);

protected:

    QString     readQuotedString(QTextStream & str);
    QPointF     getPoint(QString txt);
    QTransform  getAffineTransform(QString txt);
    QTransform  getAffineTransform(pugi::xml_node & node);
    static QTransform  getQTransform(QString txt);

    Xform       getXform(pugi::xml_node & node);
    FillData    getFill(QString txt, bool isSingle);

    void        getViewSettings(pugi::xml_node & node);

private:
    TilingLoader() {}
    TilingPtr      tiling;
};

#endif
