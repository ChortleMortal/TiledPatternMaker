#include <QDebug>
#include <QStack>

#include "colormaker.h"
#include "geometry/loose.h"
#include "geometry/dcel.h"
#include "geometry/edge.h"

ColorMaker::ColorMaker()
{
}

ColorMaker::~ColorMaker()
{
#ifdef EXPLICIT_DESTRUCTOR
   qDebug() << "deleting ColorMaker";
    dcel.reset();
    facesToDo.clear();
    whiteFaces.clear();
    blackFaces.clear();
    faceGroups.clear();
#endif
}
void ColorMaker::createFacesToDo()
{
    whiteFaces.clear();
    blackFaces.clear();
    facesToDo.clear();

    // remove outer
    for (auto & face : qAsConst(dcel->faces))
    {
        if (!face->outer)
        {
            facesToDo.push_back(face);
        }
    }
}

void ColorMaker::assignColorsOriginal()
{
    createFacesToDo();

    // recurse through faces to assign colours
    // this now deals with non-contiguous figures
    int facesToProcess;
    int facesLeft;
    do
    {
        facesToProcess = facesToDo.size();
        //qDebug() << "facesToProcess=" << facesToProcess;
        assignColorsToFaces(facesToDo); // Propagate colours using a DFS (Depth First Search).
        addFaceResults(facesToDo);
        facesLeft = facesToDo.size();
        //qDebug() << "facesLeft=" << facesLeft;
    } while ((facesLeft != 0) &&  (facesLeft < facesToProcess));

    qDebug() << "done: white =" << whiteFaces.size() << "black =" << blackFaces.size();
}

// Propagate colours using a DFS (Depth First Search).
void ColorMaker::assignColorsToFaces(FaceSet &fset)
{
    if (fset.size() == 0)
    {
        qWarning() << "no faces to assign";
        return;
    }

    QStack<FacePtr> st;
    FacePtr aface = fset.at(0);
    aface->state = FACE_PROCESSING;
    st.push(aface);

    eColor color = C_WHITE;     // seed

    while (!st.empty())
    {
        aface = st.pop();

        //eColor color = C_WHITE;     // seed
        EdgePtr head = aface->incident_edge.lock();
        if (head)
        {
            EdgePtr edge = head;
            do
            {
                auto twin = edge->twin.lock();
                FacePtr nfi;
                if (twin)
                {
                    nfi = twin->incident_face.lock();
                }
                if (nfi)
                {
                    if (!nfi->outer)
                    {
                        switch( nfi->state )
                        {
                        case FACE_UNDONE:
                            nfi->state = FACE_PROCESSING;
                            st.push(nfi);
                            break;

                        case FACE_PROCESSING:
                        case FACE_DONE:
                        case FACE_REMOVE:
                            break;

                        case FACE_BLACK:
                            color = C_BLACK;
                            break;

                        case FACE_WHITE:
                            color = C_WHITE;
                            break;
                        }
                    }
                }
                edge = edge->next.lock();
            } while (edge != head);
        }

        if (color == C_BLACK)
        {
            aface->state = FACE_WHITE;
        }
        else
        {
            Q_ASSERT(color == C_WHITE);
            aface->state = FACE_BLACK;
        }
    }
}

void ColorMaker::addFaceResults(FaceSet & fset)
{
    FaceSet faces2do;
    for (auto & face : fset)
    {
        if( face->state == FACE_WHITE )
        {
            whiteFaces.push_back(face);
        }
        else if(face->state == FACE_BLACK )
        {
            blackFaces.push_back(face);
        }
        else
        {
            face->state = FACE_UNDONE;
            faces2do.push_back(face);
        }
    }
    fset = faces2do;
}

// DAC Notes
// An edge can only be in two faces
// A vertex can be in multiple faces
void ColorMaker::assignColorsNew1()
{
    createFacesToDo();

    eFaceState newState = FACE_WHITE;     // seed
    for (auto & fp : qAsConst(facesToDo))
    {
        if (fp->state != FACE_UNDONE)
        {
            continue;
        }
        fp->state = newState;
        qreal size = fp->area;
        for (auto & fp2 : qAsConst(facesToDo))
        {
            if (fp2->state == FACE_UNDONE)
            {
                if (Loose::equals(size,fp2->area))
                {
                    fp2->state = newState;
                }
            }
        }
        newState = (newState == FACE_WHITE) ? FACE_BLACK : FACE_WHITE;
    }

    for (auto & fi : qAsConst(facesToDo))
    {
        if( fi->state == FACE_WHITE )
        {
            whiteFaces.push_back(fi);
        }
        else if( fi->state == FACE_BLACK )
        {
            blackFaces.push_back(fi);
        }
        else
        {
            Q_ASSERT(false);
        }
    }

    //whiteFaces.sortByPositon();
    //blackFaces.sortByPositon();
}

void ColorMaker::buildFaceGroups()
{
    qDebug() << "ColorMaker::buildFaceGroups ........";

    createFacesToDo();
    faceGroups.clear();

#if 0
    for (auto face : qAsConst(facesToDo))
    {
        face->sortForComparison();
    }

    removeOverlappingFaces();	// fix 1
#endif

    for (auto & face : qAsConst(facesToDo))
    {
        face->state = FACE_UNDONE;
    }

    // create face groups
    for (auto it = facesToDo.begin(); it != facesToDo.end(); it++)
    {
        FacePtr fp = *it;
        if (fp->state != FACE_UNDONE)
        {
            continue;
        }

        fp->state      = FACE_DONE;

        FaceSetPtr fsp = std::make_shared<FaceSet>();
        fsp->area      = fp->area;
        fsp->sides     = fp->getNumSides();
        fsp->push_back(fp);
        faceGroups.push_back(fsp);

        for (auto it2 = (it + 1); it2 != facesToDo.end(); it2++)
        {
            FacePtr fp2 = *it2;
            if (fp2->state == FACE_UNDONE)
            {
                if (Loose::equals(fsp->area,fp2->area) && (fsp->sides == fp2->getNumSides()))
                {
                    fp2->state = FACE_DONE;
                    fsp->push_back(fp2);
                }
            }
        }
    }

    //dumpFaceGroup("algorithm 23");
    std::sort(faceGroups.begin(), faceGroups.end(), FaceSet::sort);    // largest first
    //dumpFaceGroup("algorithm 23 post sort");
    qDebug() << "Num face groups=" << faceGroups.size();
}

void ColorMaker::assignColorSets(ColorSet & colorSet)
{
    qDebug() << "ColorMaker::assignColorsNew2" << faceGroups.size() << faceGroups.totalSize();;

    // first make the color set size the same as the face group size
    if (colorSet.size() < faceGroups.size())
    {
        qWarning() <<  "less colors than faces: faces=" << faceGroups.size() << "colors=" << colorSet.size();
        int diff = faceGroups.size() - colorSet.size();
        for (int i = 0; i < diff; i++)
        {
            TPColor tpc(Qt::yellow);
            tpc.hidden = true;
            colorSet.addColor(tpc);
        }
    }
    else if (colorSet.size() > faceGroups.size())
    {
        qWarning() <<  "more colors than faces: faces=" << faceGroups.size() << "colors=" << colorSet.size();
        colorSet.resize(faceGroups.size());
    }
    Q_ASSERT(colorSet.size() == faceGroups.size());

    // assign the color set to the sorted group
    colorSet.resetIndex();
    for (FaceSetPtr & face : faceGroups)
    {
        face->tpcolor = colorSet.getNextColor();
    }
}

void  ColorMaker::assignColorGroups(ColorGroup & colorGroup)
{
    qDebug() << "ColorMaker::assignColorGroups" << faceGroups.size() << faceGroups.totalSize();

    // first make the color set size the same as the face group size
    if (colorGroup.size() < faceGroups.size())
    {
        qWarning() <<  "less color groups than face groups: facegroup=" << faceGroups.size() << "colorgroup=" << colorGroup.size();
        int diff = faceGroups.size() - colorGroup.size();
        for (int i = 0; i < diff; i++)
        {
            ColorSet cset;
            TPColor tpc(Qt::yellow);
            cset.addColor(tpc);
            cset.hide(true);
            colorGroup.addColorSet(cset);
        }
    }
    else if (colorGroup.size() > faceGroups.size())
    {
        qWarning() <<  "more  color groups than face  groups: facegroup" << faceGroups.size() << "colorgroup=" << colorGroup.size();
        colorGroup.resize(faceGroups.size());
    }

    Q_ASSERT(colorGroup.size() == faceGroups.size());
#if 0
    // this is  helpful for consistency
    qDebug() << "sorting...";
    for (FaceSetPtr fsp  : faceGroup)
    {
        fsp->sortByPositon();
    }

    // assign the color group to the sorted group
    qDebug() << "assigning...";
    colorGroup.resetIndex();
    for (FaceSetPtr fsp  : faceGroup)
    {
        fsp->colorSet  = colorGroup.getNextColorSet();
    }

#else
    colorGroup.resetIndex();
    int i=0;
    for (FaceSetPtr fsp  : faceGroups)
    {
        //qDebug() << "sorting..." << i;
        //fsp->sortByPositon();
        qDebug() << "assigning..." << i;
        fsp->pColorSet  = colorGroup.getNextColorSet();
        i++;
    }
#endif
    qDebug() << "ColorMaker::assignColorGroups - done";
}


void ColorMaker::removeOverlappingFaces()
{
    int start = facesToDo.size();
    qDebug() << "ColorMaker::removeOverlappingFaces START faces:" << start;

    for (auto & face : qAsConst(facesToDo))
    {
        face->state = FACE_UNDONE;
    }

    UniqueQVector<FacePtr> overlaps;

    for (auto it = facesToDo.begin(); it != facesToDo.end(); it++)
    {
        FacePtr f1 = *it;

        if (f1->state == FACE_UNDONE)
            f1->state = FACE_DONE;

        for (auto it2 = it+1; it2 != facesToDo.end(); it2++)
        {
            FacePtr f2 = *it2;

            if (f2->state != FACE_UNDONE)
                continue;

            //qDebug() << "overlap" << dcel->faceIndex(f1) << dcel->faceIndex(f2);

            if (f1->overlaps(f2))
            {
                Q_ASSERT(f1 != f2);
                // which one is the larger overlapper?
                if (f2->area >= f1->area)
                {
                    f2->state = FACE_REMOVE;
                    overlaps.push_back(f2);
                }
                else
                {
                    f1->state = FACE_REMOVE;
                    overlaps.push_back(f1);
                }
            }
        }
    }

    if (overlaps.size())
        qWarning()  << "removing" << overlaps.size() << "faces";

    for (auto & face : overlaps)
    {
        facesToDo.removeAll(face);
    }

    int end = facesToDo.size();
    qDebug() << "ColorMaker::removeOverlappingFaces - END faces:" << end;
    if (start != end)
    {
        qWarning() << "ColorMaker::removeOverlappingFaces start =" << start << "end =" << end;
    }
}
