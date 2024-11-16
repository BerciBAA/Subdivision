#pragma once
#include <unordered_map>
#include <iostream>

#include "HalfEdge.h"
#include "globals.h"

class ButterflySubdivision {
public:
    void subdivide(Mesh* mesh);
    ButterflySubdivision() = default;

private:
    Vertex* createBoundaryVertex(HalfEdge* he, Mesh* mesh);
    Vertex* createInteriorVertex(HalfEdge* he, Mesh* mesh);
    void rebuildFace(Face* face, Mesh* mesh, std::vector<HalfEdge*>& newHalfEdges, std::vector<Face*>& newFaces, std::unordered_map<HalfEdge*, Vertex*>& edgeVertexMap);
};

