#ifndef COLORMAKER_H
#define COLORMAKER_H

#include "misc/colorset.h"
#include "geometry/faces.h"

typedef std::shared_ptr<class DCEL> DCELPtr;

class ColorMaker
{
public:
    ColorMaker();
    ~ColorMaker();

    void buildFaceGroups();
    void createFacesToDo();

    void assignColorsOriginal();
    void assignColorsNew1();
    void assignColorSets(ColorSet &fs);
    void assignColorGroups(ColorGroup & colorGroup);

    void assignColorsToFaces(FaceSet & fset);
    void addFaceResults(FaceSet & fset);

    DCELPtr getDCEL() { return dcel; }
    FaceGroups & getFaceGroups() { return faceGroups; }

protected:
    void removeOverlappingFaces();

    FaceSet     facesToDo;
    FaceSet     whiteFaces;
    FaceSet     blackFaces;

    FaceGroups  faceGroups;

    DCELPtr     dcel;

private:
};

#endif // COLORMAKER_H
