#pragma once
#ifndef TILING_READER_H
#define TILING_READER_H

#include <QTextStream>
#include "sys/geometry/xform.h"
#include "sys/sys/pugixml.hpp"
#include "model/settings/filldata.h"
#include "model/styles/colorset.h"
#include "sys/sys/versioning.h"
#include "sys/engine/tiling_bmp_generator.h"

typedef std::shared_ptr<class Tiling>           TilingPtr;
typedef std::shared_ptr<class BackgroundImage>  BkgdImagePtr;

class SystemViewController;
class ReaderBase;
class Layer;

class TilingReader
{

public:
    TilingReader(SystemViewController * vc) { _vc = vc; legacyCenterConverted = false; }

    TilingPtr    readTiling(QString file);
    TilingPtr    readTiling(QTextStream & st);
    TilingPtr    readTilingXML(VersionedFile & xfile, ReaderBase * mrbase);

    ColorGroup & getTileColors();

    BkgdImagePtr getBackgroundImage(pugi::xml_node & node);

protected:
    TilingPtr   readTilingXML(pugi::xml_node & tiling_node, ReaderBase * mrbase);

    QString     readQuotedString(QTextStream & str);
    QPointF     getPoint(QString txt);
    QTransform  getAffineTransform(QString txt);
    QTransform  getAffineTransform(pugi::xml_node & node);
    static QTransform  getQTransform(QString txt);

    Xform       getOldView(pugi::xml_node & node);
    FillData    getFill(QString txt, bool isSingle);

    void        getViewSettings(pugi::xml_node & node);

    bool        legacyCenterConverted;

private:
    SystemViewController * _vc;

    VersionedFile  _xfile;
    TilingPtr      tiling;
    BkgdImagePtr   bip;

    static bool    debug;
    uint           _tilingVersion;
};

#endif
