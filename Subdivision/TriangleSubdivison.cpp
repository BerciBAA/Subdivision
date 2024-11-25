#include "TriangleSubdivison.h"

void TriangleSubdivison::subdivide(Mesh* mesh, bool moveVertices)
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

    // move old vertices
    if (moveVertices){
        std::vector<Vertex*> newVertices;
        for (const auto& vertex : mesh->vertices) {
            Vertex* movedVertex = moveVertex(vertex, mesh);
            newVertices.push_back(movedVertex);
        }
        // TODO: delete from memory to prevent memory leak
        mesh->vertices = newVertices;
        std::cout << "Moved vertices!" << std::endl;
    }


    // add new vertices
    for (const auto& pair : edgeVertexMap) {
        if (std::find(mesh->vertices.begin(), mesh->vertices.end(), pair.second) == mesh->vertices.end())
            mesh->vertices.push_back(pair.second);
    }

    std::vector<HalfEdge*> newHalfEdges;
    std::vector<Face*> newFaces;

    // build small triangles
    for (Face* f : mesh->faces) {
        rebuildFace(f, mesh, newHalfEdges, newFaces, edgeVertexMap);
    }

    // TODO: delete from memory to prevent memory leak
    mesh->halfEdges = newHalfEdges;
    mesh->faces = newFaces;

    mesh->createTwinEdges();
}

void TriangleSubdivison::rebuildFace(Face* face, Mesh* mesh, std::vector<HalfEdge*>& newHalfEdges, std::vector<Face*>& newFaces, std::unordered_map<HalfEdge*, Vertex*>& edgeVertexMap)
{
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
