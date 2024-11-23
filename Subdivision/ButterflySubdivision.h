#pragma once
#include <iostream>

#include "HalfEdge.h"
#include "TriangleSubdivison.h"
#include "globals.h"

class ButterflySubdivision : TriangleSubdivison {
public:
    void subdivide(Mesh* mesh);
    ButterflySubdivision() = default;

private:
    Vertex* createBoundaryVertex(HalfEdge* he, Mesh* mesh);
    Vertex* createInteriorVertex(HalfEdge* he, Mesh* mesh);
};

