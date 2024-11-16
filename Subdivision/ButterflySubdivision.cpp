#include "ButterflySubdivision.h"

void ButterflySubdivision::subdivide(Mesh* mesh)
{
    std::unordered_map<HalfEdge*, Vertex*> edgeVertexMap;

    for (HalfEdge* he : mesh->halfEdges) {
        if (!edgeVertexMap[he] && !edgeVertexMap[he->twin]) {
            edgeVertexMap[he] = he->isBoundaryEdge()
                ? createBoundaryVertex(he, mesh)
                : createInteriorVertex(he, mesh);
            edgeVertexMap[he->twin] = edgeVertexMap[he]; // Share new vertex
        }
    }

    for (const auto& pair : edgeVertexMap) {
        mesh->vertices.push_back(pair.second);
    }

    std::vector<HalfEdge*> newHalfEdges;
    std::vector<Face*> newFaces;

    for (Face* f : mesh->faces) {
        rebuildFace(f, mesh, newHalfEdges, newFaces, edgeVertexMap);
    }

    mesh->halfEdges = newHalfEdges;
    mesh->faces = newFaces;

    mesh->createTwinEdges();
}

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
    mesh->vertices.push_back(newVertex);
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
    mesh->vertices.push_back(newVertex);
    return newVertex;
}

void ButterflySubdivision::rebuildFace(Face* face, Mesh* mesh, std::vector<HalfEdge*>& newHalfEdges, std::vector<Face*>& newFaces, std::unordered_map<HalfEdge*, Vertex*>& edgeVertexMap)
{
    //TODO: get every vertex in triangle then connect them!
    
    HalfEdge* he1 = face->edge;
    HalfEdge* he2 = he1->next;
    HalfEdge* he3 = he2->next;

    Vertex* ov1 = he1->origin;
    Vertex* ov2 = he2->origin;
    Vertex* ov3 = he3->origin;

    if (!edgeVertexMap[he1] || !edgeVertexMap[he2] || !edgeVertexMap[he3]) {
        std::cout << "Error: No new vertex found for face: " << face->toString() << std::endl;
        return;
    }

    Vertex* nv1 = edgeVertexMap[he1];
    Vertex* nv2 = edgeVertexMap[he2];
    Vertex* nv3 = edgeVertexMap[he3];


    // new face 1
    Face* nf1 = new Face();

    HalfEdge* nhe1 = new HalfEdge();
    HalfEdge* nhe2 = new HalfEdge();
    HalfEdge* nhe3 = new HalfEdge();
    
    nhe1->origin = ov1; nhe1->next = nhe2; nhe1->prev = nhe3; nhe1->incidentFace = nf1;
    nhe2->origin = nv1; nhe2->next = nhe3; nhe2->prev = nhe1; nhe2->incidentFace = nf1;
    nhe3->origin = nv3; nhe3->next = nhe1; nhe3->prev = nhe2; nhe3->incidentFace = nf1;
    nf1->edge = nhe1;

    // new face 2
    Face* nf2 = new Face();

    HalfEdge* nhe4 = new HalfEdge();
    HalfEdge* nhe5 = new HalfEdge();
    HalfEdge* nhe6 = new HalfEdge();

    nhe4->origin = nv1; nhe4->next = nhe5; nhe4->prev = nhe6; nhe4->incidentFace = nf2;
    nhe5->origin = nv2; nhe5->next = nhe6; nhe5->prev = nhe4; nhe5->incidentFace = nf2;
    nhe6->origin = nv3; nhe6->next = nhe4; nhe6->prev = nhe5; nhe6->incidentFace = nf2;
    nf2->edge = nhe4;

    // new face 3
    Face* nf3 = new Face();

    HalfEdge* nhe7 = new HalfEdge();
    HalfEdge* nhe8 = new HalfEdge();
    HalfEdge* nhe9 = new HalfEdge();

    nhe7->origin = nv1; nhe7->next = nhe8; nhe7->prev = nhe9; nhe7->incidentFace = nf3;
    nhe8->origin = ov2; nhe8->next = nhe9; nhe8->prev = nhe7; nhe8->incidentFace = nf3;
    nhe9->origin = nv2; nhe9->next = nhe7; nhe9->prev = nhe8; nhe9->incidentFace = nf3;
    nf3->edge = nhe7;

    // new face 4
    Face* nf4 = new Face();

    HalfEdge* nhe10 = new HalfEdge();
    HalfEdge* nhe11 = new HalfEdge();
    HalfEdge* nhe12 = new HalfEdge();

    nhe10->origin = nv3; nhe10->next = nhe11; nhe10->prev = nhe12; nhe10->incidentFace = nf4;
    nhe11->origin = nv2; nhe11->next = nhe12; nhe11->prev = nhe10; nhe11->incidentFace = nf4;
    nhe12->origin = ov3; nhe12->next = nhe10; nhe12->prev = nhe11; nhe12->incidentFace = nf4;
    nf4->edge = nhe10;

    // add newly created parts to the mesh

    newFaces.push_back(nf1);
    newFaces.push_back(nf2);
    newFaces.push_back(nf3);
    newFaces.push_back(nf4);

    newHalfEdges.push_back(nhe1); newHalfEdges.push_back(nhe2); newHalfEdges.push_back(nhe3);
    newHalfEdges.push_back(nhe4); newHalfEdges.push_back(nhe5); newHalfEdges.push_back(nhe6);
    newHalfEdges.push_back(nhe7); newHalfEdges.push_back(nhe8); newHalfEdges.push_back(nhe9);
    newHalfEdges.push_back(nhe10); newHalfEdges.push_back(nhe11); newHalfEdges.push_back(nhe12);
}
