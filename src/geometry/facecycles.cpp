#include "facecycles.h"
#include "geometry/Map.h"

FaceCycles::FaceCycles()
{

}

FaceSet FaceCycles::getFaceSet(MapPtr map)
{
    this->map = map;

    FaceSet set;

    int sz = map->getVertices()->size();
    for( int idx = 0; idx < sz; ++idx )
    {
        VertexPtr v = map->getVertices()->at(idx);
        v->setTmpIndex(idx);
    }

    findAllCycles();

    for (auto it = cycles.begin(); it != cycles.end(); it++)
    {
        FacePtr face = make_shared<Face>();
        vector<int> pts = *it;
        for (auto it2 = pts.begin(); it2 != pts.end(); it2++)
        {
            int index = *it2;
            VertexPtr v = map->getVertex(index);
            face->push_back(v);
        }
        set.push_back(face);
    }

    return set;
}

void FaceCycles::findAllCycles()
{
    qDebug() << "FaceCycles::findAllCycles()";
    int count = 1;
    for(auto it = map->getEdges()->begin(); it != map->getEdges()->end(); it++)
    {
        EdgePtr edge = *it;
        findNewCycles( vector<int>(1,edge->getV1()->getTmpIndex()));
        qDebug() << "processed edgeV1=" << count;
        findNewCycles( vector<int>(1,edge->getV2()->getTmpIndex()));
        qDebug() << "processed edgeV2=" << count;
    }
    qDebug() << "FaceCycles::findAllCycles() - COMPLETE";
}

auto FaceCycles::visisted( int v, const vector<int> & path )
{
    return find(path.begin(),path.end(),v) != path.end();
}

auto FaceCycles::rotate_to_smallest( vector<int> path )
{
    rotate(path.begin(), min_element(path.begin(), path.end()), path.end());
    return path;
}

auto FaceCycles::invert(vector<int> path)
{
    reverse(path.begin(),path.end());
    return rotate_to_smallest(path);
}

auto FaceCycles::isNew(const vector<int> & path )
{
    return find(cycles.begin(), cycles.end(), path) == cycles.end();
};

void FaceCycles::findNewCycles(vector<int> sub_path)
{
    int start_node = sub_path[0];
    int next_node;

    // visit each edge and each node of each edge
    for(auto it = map->getEdges()->begin(); it != map->getEdges()->end(); it++)
    {
        EdgePtr edge = *it;
        int node1 = edge->getV1()->getTmpIndex();
        int node2 = edge->getV2()->getTmpIndex();
        if( node1 == start_node || node2 == start_node)
        {
            if(node1 == start_node)
                next_node = node2;
            else
                next_node = node1;

            if( !visisted(next_node, sub_path) )
            {
                // neighbor node not on path yet
                vector<int> sub;
                sub.push_back(next_node);
                sub.insert(sub.end(), sub_path.begin(), sub_path.end());
                findNewCycles( sub );
            }
            else if( sub_path.size() > 2 && next_node == sub_path.back() )
            {
                // cycle found
                auto p = rotate_to_smallest(sub_path);
                auto inv = invert(p);

                if( isNew(p) && isNew(inv) )
                    cycles.push_back( p );
            }
        }
    }
}
