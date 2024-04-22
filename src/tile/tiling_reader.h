#pragma once
#ifndef TILING_READER_H
#define TILING_READER_H

#include <QTextStream>
#include "geometry/xform.h"
#include "misc/pugixml.hpp"
#include "settings/filldata.h"
#include "misc/colorset.h"

typedef std::shared_ptr<class Tiling>           TilingPtr;
typedef std::shared_ptr<class MosaicReaderBase> MRBasePtr;
typedef std::shared_ptr<class BackgroundImage>  BkgdImagePtr;

class TilingReader
{
    friend class TilingManager;
    friend class MosaicReader;
    friend class LegacyLoader;
    friend class TilingBMPEngine;

public:
    TilingPtr    readTiling(QString file);
    TilingPtr    readTiling(QTextStream & st);
    TilingPtr    readTilingXML(QString file);
    TilingPtr    readTilingXML(QString file,MRBasePtr base);

    ColorGroup & getTileColors() { return tileColors; }

    static BkgdImagePtr getBackgroundImage(pugi::xml_node & node);

protected:
    TilingPtr   readTilingXML(pugi::xml_node & tiling_node, MRBasePtr base);

    QString     readQuotedString(QTextStream & str);
    QPointF     getPoint(QString txt);
    QTransform  getAffineTransform(QString txt);
    QTransform  getAffineTransform(pugi::xml_node & node);
    static QTransform  getQTransform(QString txt);

    Xform       getXform(pugi::xml_node & node);
    FillData    getFill(QString txt, bool isSingle);

    void        getViewSettings(pugi::xml_node & node);

private:
    TilingReader() {}
    TilingPtr      tiling;
    BkgdImagePtr   bip;
    ColorGroup     tileColors;
};

#endif
