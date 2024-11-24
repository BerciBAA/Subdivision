#include "ButterflySubdivision.h"


Vertex* ButterflySubdivision::createBoundaryVertex(HalfEdge* he, Mesh* mesh)
{
    // Midpoint of the edge
    Vertex* v0 = he->origin;
    Vertex* v1 = he->next->origin;

    float x = (v0->x + v1->x) / 2.0f;
    float y = (v0->y + v1->y) / 2.0f;
    float z = (v0->z + v1->z) / 2.0f;

    std::stringstream vertexNameStream;
    vertexNameStream << "v" << vertexNameIdx++;

    Vertex* newVertex = new Vertex(x, y, z, vertexNameStream.str());
    return newVertex;
}

Vertex* ButterflySubdivision::createInteriorVertex(HalfEdge* he, Mesh* mesh)
{
    Vertex* v0 = he->origin;
    Vertex* v1 = he->twin->origin;
    Vertex* v2 = he->next->twin->origin;
    Vertex* v3 = he->twin->next->twin->origin;

    // Opposite vertices
    Vertex* v4 = he->twin->next->twin->next->twin->origin; // left lower wing
    Vertex* v5 = he->twin->next->next->twin->next->twin->origin; // right lower wing
    Vertex* v6 = he->next->next->twin->next->twin->origin; // left upper wing
    Vertex* v7 = he->next->twin->next->twin->origin;

    // Butterfly formula
    float x = (v0->x + v1->x) * 0.5f + (v2->x + v3->x) * 0.125f
        - (v4->x + v5->x + v6->x + v7->x) * 0.0625f;
    float y = (v0->y + v1->y) * 0.5f + (v2->y + v3->y) * 0.125f
        - (v4->y + v5->y + v6->y + v7->y) * 0.0625f;
    float z = (v0->z + v1->z) * 0.5f + (v2->z + v3->z) * 0.125f
        - (v4->z + v5->z + v6->z + v7->z) * 0.0625f;

    std::stringstream vertexNameStream;
    vertexNameStream << "v" << vertexNameIdx++;

    Vertex* newVertex = new Vertex(x, y, z, vertexNameStream.str());
    return newVertex;
}

Vertex* ButterflySubdivision::moveVertex(Vertex* v, Mesh* mesh)
{
    return nullptr;
}
