#pragma once
#include "HalfEdge.h"
#include "globals.h"

#include <unordered_map>
#include <iostream>
#include <algorithm>


class TriangleSubdivison
{
public:
    void subdivide(Mesh* mesh, bool moveVertices);
    TriangleSubdivison() = default;

private:
    virtual Vertex* createBoundaryVertex(HalfEdge* he, Mesh* mesh) = 0;
    virtual Vertex* createInteriorVertex(HalfEdge* he, Mesh* mesh) = 0;
    virtual Vertex* moveVertex(Vertex* v, Mesh* mesh) = 0;

protected:
	void rebuildFace(Face* face, Mesh* mesh, std::vector<HalfEdge*>& newHalfEdges, std::vector<Face*>& newFaces, std::unordered_map<HalfEdge*, Vertex*>& edgeVertexMap);
};

