#pragma once
#ifndef COLORMAKER_H
#define COLORMAKER_H

#include "enums/efilltype.h"
#include "misc/colorset.h"
#include "geometry/faces.h"
#include "geometry/dcel.h"

typedef std::shared_ptr<class DCEL> DCELPtr;

class ColorMaker
{
    friend class Filled;

public:
    ColorMaker();
    ~ColorMaker();

    void clear();

    void buildFaceSets(eFillType algorithm, DCELPtr dcel);

    FaceGroups & getFaceGroups() { return faceGroups; }
    FaceSet    & getBlackFaces() { return blackFaces; }
    FaceSet    & getWhiteFaces() { return whiteFaces; }
    FaceSet    & getFacesToDo()  { return facesToDo; }
    DCELPtr      getDCEL()       { return wDcel.lock();}

    int          getColorIndex(int faceIndex);
    void         setColorIndex(int faceIndex,int colorIndex);

    void          setPaletteIndices(QVector<int> & indices);
    QVector<int>& getPaletteIndices();
    QColor        getColorFromPalette(int index) { return whiteColorSet.getQColor(index); }

protected:
    void addFaceResults(FaceSet & fset);

    void buildFaceGroups(DCELPtr dcel);

    void createFacesToDo(DCELPtr dcel);
    void initDCELFaces(DCELPtr dcel);
    void removeOverlappingFaces();

    void assignColorsOriginal(DCELPtr dcel);
    void assignColorsNew1(DCELPtr dcel);
    void assignColorSets(ColorSet &fs);
    void assignColorGroups(ColorGroup & colorGroup);
    void assignColorsToFaces(FaceSet & fset);
    void assignPaletteColors(DCELPtr dcel);

private:
    FaceSet     facesToDo;
    FaceSet     whiteFaces;
    FaceSet     blackFaces;
    FaceGroups  faceGroups;

    ColorSet    whiteColorSet;
    ColorSet    blackColorSet;
    ColorGroup  colorGroup;

    QVector<int> faceColorIndices;
    WeakDCELPtr  wDcel;
};

#endif // COLORMAKER_H
