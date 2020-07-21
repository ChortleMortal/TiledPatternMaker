#ifndef FACECYCLES_H
#define FACECYCLES_H

#include <QPolygonF>
#include "geometry/map.h"
#include "geometry/faces.h"

using namespace std;

class FaceCycles
{
public:
    FaceCycles();

    FaceSet getFaceSet(MapPtr map);

protected:
    void findAllCycles();
    void findNewCycles(vector<int> sub_path);

    inline auto visisted( int v, const vector<int> & path );
    inline auto rotate_to_smallest(vector<int> path );
    inline auto invert(vector<int> path );
    inline auto isNew(const vector<int> & path );

private:
    vector< vector<int> > cycles;
    MapPtr map;
};

#endif // FACECYCLES_H
