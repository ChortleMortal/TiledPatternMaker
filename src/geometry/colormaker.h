#ifndef COLORMAKER_H
#define COLORMAKER_H

#include "geometry/dcel.h"
#include "base/shared.h"

class ColorMaker
{
public:
    ColorMaker(DCELPtr d);

    void createFacesToDo();

    void buildFaceGroups();

    void assignColorsOriginal();
    void assignColorsNew1();
    void assignColorSets(ColorSet &fs);
    void assignColorGroups(ColorGroup & colorGroup);

    void assignColorsToFaces(FaceSet & fset);
    void addFaceResults(FaceSet & fset);

    FaceGroup   & getFaceGroup()     { return faceGroup; }
    FaceSet     & getWhiteFaces()    { return whites; }
    FaceSet     & getBlackFaces()    { return blacks; }
    FaceSet     & getFacesToDo();

protected:
    DAC_DEPRECATED void removeDuplicateFaces();
    DAC_DEPRECATED void decomposeCrossoverFaces();
                   void removeOverlappingFaces();

    FaceSet     facesToDo;
    FaceSet     whites;
    FaceSet     blacks;

    FaceGroup   faceGroup;

private:
    DCELPtr     dcel;
};

#endif // COLORMAKER_H
