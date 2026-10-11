#pragma once

#include "sys/sys/versioning.h"
#include <QImage>
#include <QList>

typedef std::shared_ptr<class Tiling>        TilingPtr;
typedef std::shared_ptr<class Mosaic>        MosaicPtr;

class ViewController;

class MosaicXMLRegenerator
{
public:
    MosaicXMLRegenerator();
    ~MosaicXMLRegenerator();

    bool        regemerateXML(VersionedName vname);

protected:
    MosaicPtr   loadMosaic(VersionedName vname);
    bool        saveMosaic(MosaicPtr mosaic, bool forceOverwrite);

private:
    class ViewController * bmpViewController;
};
