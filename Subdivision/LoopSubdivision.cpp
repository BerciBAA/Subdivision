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
    return newVertex;
}

Vertex* LoopSubdivision::moveVertex(Vertex* v, Mesh* mesh)
{
    // create new vertex to be modified
    Vertex* newVertex = new Vertex(v);

    // check how many neighbor vertices
    std::vector<Vertex*> neighborVertices;
    for (const auto& hfe : mesh->halfEdges) {
        if (hfe->origin == v) {
            neighborVertices.push_back(hfe->twin->origin);
            hfe->origin = newVertex;
        }
    }     
    int n = neighborVertices.size();
    float beta = 0;

    // move newly created vertex
    // n = 2 -> boundary
    if (n == 2) {
        newVertex->x = v->x * 0.75f + neighborVertices[0]->x * 0.125f + neighborVertices[1]->x * 0.125f;
        newVertex->y = v->y * 0.75f + neighborVertices[0]->y * 0.125f + neighborVertices[1]->y * 0.125f;
        newVertex->z = v->z * 0.75f + neighborVertices[0]->z * 0.125f + neighborVertices[1]->z * 0.125f;
    }
    // n = 3 -> format 1
    else if (n == 3)
    {
        beta = 0.1875f;
        float origVertexPart = 1.0f - n * beta;

        newVertex->x = v->x * origVertexPart;
        newVertex->y = v->y * origVertexPart;
        newVertex->z = v->z * origVertexPart;

        for (const auto& neighborVertex : neighborVertices)
        {
            newVertex->x += neighborVertex->x * beta;
            newVertex->y += neighborVertex->y * beta;
            newVertex->z += neighborVertex->z * beta;
        }
    }
    // n > 3 -> format 2
    else if (n > 3) {
        beta = 3.0f / (8.0f * n);
        float origVertexPart = 1.0f - n * beta;

        newVertex->x = v->x * origVertexPart;
        newVertex->y = v->y * origVertexPart;
        newVertex->z = v->z * origVertexPart;

        for (const auto& neighborVertex : neighborVertices)
        {
            newVertex->x += neighborVertex->x * beta;
            newVertex->y += neighborVertex->y * beta;
            newVertex->z += neighborVertex->z * beta;
        }
    }
    else {
        std::cout << "Error: Vertex " << v->name << " has " << n << " neighbor vertices!" << std::endl;
    }

    return newVertex;
}
