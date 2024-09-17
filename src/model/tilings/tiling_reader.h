#pragma once
#ifndef TILING_READER_H
#define TILING_READER_H

#include <QTextStream>
#include "sys/geometry/xform.h"
#include "sys/sys/pugixml.hpp"
#include "model/settings/filldata.h"
#include "model/styles/colorset.h"
#include "sys/sys/versioning.h"

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
    TilingPtr    readTilingXML(VersionedFile & xfile);
    TilingPtr    readTilingXML(VersionedFile & xfile, MRBasePtr base);

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
