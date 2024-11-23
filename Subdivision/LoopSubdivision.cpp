#include "LoopSubdivision.h"

Vertex* LoopSubdivision::createBoundaryVertex(HalfEdge* he, Mesh* mesh)
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
    mesh->vertices.push_back(newVertex);
    return newVertex;
}

Vertex* LoopSubdivision::createInteriorVertex(HalfEdge* he, Mesh* mesh)
{
    Vertex* v0 = he->origin;
    Vertex* v1 = he->twin->origin;
    Vertex* v2 = he->next->twin->origin;
    Vertex* v3 = he->twin->next->twin->origin;

    // Loop formula
    float x = (v0->x + v1->x) * 0.375f + (v2->x + v3->x) * 0.125f;
    float y = (v0->y + v1->y) * 0.375f + (v2->y + v3->y) * 0.125f;
    float z = (v0->z + v1->z) * 0.375f + (v2->z + v3->z) * 0.125f;

    std::stringstream vertexNameStream;
    vertexNameStream << "v" << vertexNameIdx++;

    Vertex* newVertex = new Vertex(x, y, z, vertexNameStream.str());
    mesh->vertices.push_back(newVertex);
    return newVertex;
}

Vertex* LoopSubdivision::moveVertex(Vertex* v, Mesh* mesh)
{
    //TODO: create new vertex with modified coordinates

    // check how many neighbor vertices
    std::vector<Vertex*> neighborVertices;
    for (const auto& hfe : mesh->halfEdges) {
        if (hfe->twin->origin == v) {
            neighborVertices.push_back(hfe->twin->origin);
        }
    }
    std::cout << "Vertex " << v->name << " has " << neighborVertices.size() << " neighbors!" << std::endl;
     
     
    // move newly created vertex
    // n = 2 -> boundary 
    // n = 3 -> format 1
    // n > 3 -> format 2

    return v;
}
