#pragma once
// based on source: https://graphics.stanford.edu/~mdfisher/subdivision.html

#include "TriangleSubdivison.h"

class LoopSubdivision : public TriangleSubdivison
{
public:
    LoopSubdivision() = default;

private:
    virtual Vertex* createBoundaryVertex(HalfEdge* he, Mesh* mesh);
    virtual Vertex* createInteriorVertex(HalfEdge* he, Mesh* mesh);
    Vertex* moveVertex(Vertex* v, Mesh* mesh);
};

