#ifndef MOSAIC_XML_REGENERATOR_H
#define MOSAIC_XML_REGENERATOR_H

#include "sys/sys/versioning.h"
#include <QImage>
#include <QList>

typedef std::shared_ptr<class Tiling>        TilingPtr;
typedef std::shared_ptr<class Mosaic>        MosaicPtr;

class SystemViewController;

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
    class SystemViewController * bmpViewController;
};

#endif
