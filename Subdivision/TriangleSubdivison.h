#pragma once
#include "HalfEdge.h"

#include <unordered_map>
#include <iostream>


class TriangleSubdivison
{

public:
    virtual void subdivide(Mesh* mesh) = 0;
    TriangleSubdivison() = default;

private:
    virtual Vertex* createBoundaryVertex(HalfEdge* he, Mesh* mesh) = 0;
    virtual Vertex* createInteriorVertex(HalfEdge* he, Mesh* mesh) = 0;

protected:
	void rebuildFace(Face* face, Mesh* mesh, std::vector<HalfEdge*>& newHalfEdges, std::vector<Face*>& newFaces, std::unordered_map<HalfEdge*, Vertex*>& edgeVertexMap);
};

