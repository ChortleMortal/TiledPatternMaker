#ifndef COLORMAKER_H
#define COLORMAKER_H

#include "geometry/dcel.h"
#include "base/shared.h"

class ColorMaker
{
public:
    ColorMaker();

    void createFacesToDo();

    void buildFaceGroups();

    void assignColorsOriginal();
    void assignColorsNew1();
    void assignColorSets(ColorSet &fs);
    void assignColorGroups(ColorGroup & colorGroup);

    void assignColorsToFaces(FaceSet & fset);
    void addFaceResults(FaceSet & fset);

    FaceGroup  * getFaceGroup() { return &faceGroup; }

    DCELPtr getDCEL() { return dcel; }

protected:
    DAC_DEPRECATED void removeDuplicateFaces();
    DAC_DEPRECATED void decomposeCrossoverFaces();
                   void removeOverlappingFaces();

    FaceSet     facesToDo;
    FaceSet     whiteFaces;
    FaceSet     blackFaces;

    FaceGroup   faceGroup;

    DCELPtr     dcel;

private:
};

#endif // COLORMAKER_H
