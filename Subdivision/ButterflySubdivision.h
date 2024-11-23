#pragma once

#include "TriangleSubdivison.h"

class ButterflySubdivision : public TriangleSubdivison {
public:
    ButterflySubdivision() = default;

private:
    Vertex* createBoundaryVertex(HalfEdge* he, Mesh* mesh);
    Vertex* createInteriorVertex(HalfEdge* he, Mesh* mesh);
    Vertex* moveVertex(Vertex* v, Mesh* mesh);
};

