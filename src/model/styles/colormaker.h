#pragma once
#ifndef COLORMAKER_H
#define COLORMAKER_H

#include "sys/enums/efilltype.h"
#include "model/styles/colorset.h"
#include "sys/geometry/faces.h"
#include "sys/geometry/dcel.h"

typedef std::shared_ptr<class DCEL> DCELPtr;

class Filled;

// The ColorMaker is used t o create the data to be drawn
class ColorMaker
{
    friend class Filled;

public:
    ColorMaker(Filled * filled);
    ~ColorMaker();

    void clear();

    void buildFaceSets(eFillType algorithm, DCELPtr dcel);

    FaceGroups & getFaceGroups() { return faceGroups; }
    FaceSet    & getBlackFaces() { return blackFaces; }
    FaceSet    & getWhiteFaces() { return whiteFaces; }
    FaceSet    & getFacesToDo()  { return facesToDo; }

protected:
    void addFaceResults(FaceSet & fset);

    void buildFaceGroups(DCELPtr dcel);

    void createFacesToDo(DCELPtr dcel);
    void initDCELFaces(DCELPtr dcel);
    void removeOverlappingFaces();

    void assignColorsOriginal(DCELPtr dcel);
    void assignColorsNew1(DCELPtr dcel);
    void assignColorSets(ColorSet *fs);
    void assignColorGroups(ColorGroup *colorGroup);
    void assignColorsToFaces(FaceSet & fset);
    void assignPaletteColors(DCELPtr dcel);

private:
    FaceSet     facesToDo;
    FaceSet     whiteFaces;
    FaceSet     blackFaces;
    FaceGroups  faceGroups;

    Filled *     filled;
};

#endif // COLORMAKER_H
