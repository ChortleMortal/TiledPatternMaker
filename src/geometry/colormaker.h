#pragma once
#ifndef COLORMAKER_H
#define COLORMAKER_H

#include "misc/colorset.h"
#include "geometry/faces.h"

typedef std::shared_ptr<class DCEL> DCELPtr;

class ColorMaker
{
    friend class Filled;

public:
    ColorMaker();
    ~ColorMaker();

    void clear();

    void buildFaceSets(int algorithm, DCEL * dcel);
    void assignColors( int algorithm, DCEL * dcel);

    FaceGroups & getFaceGroups() { return faceGroups; }
    FaceSet    & getBlackFaces() { return blackFaces; }
    FaceSet    & getWhiteFaces() { return whiteFaces; }

protected:
    void assignColorsToFaces(FaceSet & fset);
    void addFaceResults(FaceSet & fset);

    void buildFaceGroups(DCEL * dcel);

    void createFacesToDo(DCEL * dcel);
    void removeOverlappingFaces();

    void assignColorsOriginal(DCEL * dcel);
    void assignColorsNew1(DCEL * dcel);
    void assignColorSets(ColorSet &fs);
    void assignColorGroups(ColorGroup & colorGroup);

    FaceSet    facesToDo;
    FaceSet     whiteFaces;
    FaceSet     blackFaces;
    FaceGroups  faceGroups;

    ColorSet    whiteColorSet;
    ColorSet    blackColorSet;
    ColorGroup  colorGroup;

private:
};

#endif // COLORMAKER_H
