#include "HalfEdge.h"
#include "globals.h"

extern int vertexNameIdx = 1;
extern int faceNameIdx = 0;
extern int halfEdgeNameIdx = 0;

// Based on this source: https://jerryyin.info/geometry-processing-algorithms/half-edge/
Vertex::Vertex(float x, float y, float z, const std::string& name)
    : x(x), y(y), z(z), incidentEdge(nullptr), name(name) {}

Vertex::Vertex(Vertex* v) : x(0), y(0), z(0), incidentEdge(nullptr), name("error")
{
    x = v->x;
    y = v->y;
    z = v->z;
    incidentEdge = v->incidentEdge;
    name = v->name;
}

std::string Vertex::toString() const {
    std::stringstream ss;
    ss << "Vertex " << name << " (" << x << ", " << y << ", " << z << ")";
    return ss.str();
}

HalfEdge::HalfEdge(const std::string& name)
    : origin(nullptr), twin(nullptr), next(nullptr), prev(nullptr), incidentFace(nullptr), name(name) {}

HalfEdge::HalfEdge()
    : origin(nullptr), twin(nullptr), next(nullptr), prev(nullptr), incidentFace(nullptr)
{
    name = getNextName();
}

Face::Face(std::string& name) : edge(nullptr), name(name) {}

Face::Face() : edge(nullptr)
{
    std::stringstream faceNameStream;
    faceNameStream << "f" << faceNameIdx++;
    name = faceNameStream.str();
}

std::string Face::toString() const {
    std::stringstream ss;
    ss << "Face " << name << " with vertices: ";

    HalfEdge* startEdge = edge;
    HalfEdge* currEdge = startEdge;

    do {
        ss << currEdge->origin->name;
        if (currEdge->next != startEdge) {
            ss << " -> ";
        }
        currEdge = currEdge->next;
    } while (currEdge != startEdge);

    return ss.str();
}

std::string HalfEdge::toString() const {
    std::stringstream ss;
    ss << "HalfEdge " << name << " from " << origin->name << std::endl;
    ss << "twin: " << (twin ? twin->name : "-") << std::endl;
    ss << "next: " << (next ? next->name : "-") << std::endl;
    ss << "prev: " << (prev ? prev->name : "-") << std::endl;
    ss << "face: " << (incidentFace ? incidentFace->name : "-") << std::endl;
    return ss.str();
}

bool HalfEdge::isBoundaryEdge()
{
    return incidentFace == nullptr;
}

Mesh::Mesh(const std::vector<std::vector<int>>& facesIndices, const std::vector<std::vector<float>>& verticesPos) {
    // Create vertices
    for (const auto& pos : verticesPos) {
        std::stringstream vertexNameStream;
        vertexNameStream << "v" << vertexNameIdx;

        vertices.push_back(new Vertex(pos[0], pos[1], pos[2], vertexNameStream.str()));

        vertexNameIdx++;
    }

    // Create half-edges and faces
    for (const auto& face : facesIndices) {
        Face* newFace = new Face();
        faces.push_back(newFace);

        HalfEdge* prevEdge = nullptr;
        HalfEdge* firstEdge = nullptr;

        for (int i = 0; i < face.size(); ++i) {
            std::stringstream halfEdgeNameStream;
            halfEdgeNameStream << "e" << halfEdgeNameIdx;

            int currVertexIdx = face[i] - 1;
            int nextVertexIdx = face[(i + 1) % face.size()] - 1;

            HalfEdge* edge = new HalfEdge(halfEdgeNameStream.str());
            edge->origin = vertices[currVertexIdx];
            edge->incidentFace = newFace;

            halfEdges.push_back(edge);

            if (prevEdge) {
                prevEdge->next = edge;
                edge->prev = prevEdge;
            }
            else {
                firstEdge = edge;
            }

            prevEdge = edge;

            if (i == face.size() - 1) {
                edge->next = firstEdge;
                firstEdge->prev = edge;
            }

            if (!vertices[currVertexIdx]->incidentEdge) {
                vertices[currVertexIdx]->incidentEdge = edge;
            }

            halfEdgeNameIdx++;
        }

        newFace->edge = firstEdge;
        faceNameIdx++;
    }

    createTwinEdges();
}

void Mesh::createTwinEdges() {
    bool foundTwin = false;
    std::vector<HalfEdge*> boundaryHalfEdges;

    for (auto* he1 : halfEdges) {
        if (!he1->twin) {
            foundTwin = false;
            for (auto* he2 : halfEdges) {
                if (he1->origin == he2->next->origin && he1->next->origin == he2->origin) {
                    he1->twin = he2;
                    he2->twin = he1;
                    foundTwin = true;
                    break;
                }
            }

            if (!foundTwin) {
                std::stringstream boundaryHalfEdgeNameStream;
                boundaryHalfEdgeNameStream << "e" << halfEdgeNameIdx;

                HalfEdge* boundaryEdge = new HalfEdge(boundaryHalfEdgeNameStream.str());

                boundaryEdge->origin = he1->next->origin;
                boundaryEdge->twin = he1;
                he1->twin = boundaryEdge;

                boundaryEdge->incidentFace = nullptr;

                boundaryHalfEdges.push_back(boundaryEdge);
                (halfEdgeNameIdx)++;
            }
        }
    }

    // Set prev and next for boundary edges
    for (auto* boundaryHalfEdge : boundaryHalfEdges) {
        Vertex* origin = boundaryHalfEdge->origin;
        Vertex* destination = boundaryHalfEdge->twin->origin;

        for (auto* otherBoundaryHalfEdge : boundaryHalfEdges) {

            if (otherBoundaryHalfEdge->origin == destination)
                boundaryHalfEdge->next = otherBoundaryHalfEdge;

            if (otherBoundaryHalfEdge->twin->origin == origin)
                boundaryHalfEdge->prev = otherBoundaryHalfEdge;
        }
    }

    for (auto* boundaryHalfEdge : boundaryHalfEdges) {
        halfEdges.push_back(boundaryHalfEdge);
    }
}

Mesh::~Mesh() {
    for (auto* v : vertices) delete v;
    for (auto* he : halfEdges) delete he;
    for (auto* f : faces) delete f;
}

std::string Mesh::toString() const {
    std::stringstream ss;

    ss << "Vertices:\n";
    for (int i = 0; i < vertices.size(); ++i) {
        ss << vertices[i]->toString() << std::endl;
    }

    ss << "\nHalfEdges:\n";
    for (int i = 0; i < halfEdges.size(); ++i) {
        ss << halfEdges[i]->toString() << std::endl;
    }

    ss << "\nFaces:\n";
    for (int i = 0; i < faces.size(); ++i) {
        ss << faces[i]->toString() << std::endl;
    }

    return ss.str();
}

std::string HalfEdge::getNextName()
{
    std::stringstream halfEdgeNameStream;
    halfEdgeNameStream << "e" << halfEdgeNameIdx++;
    return halfEdgeNameStream.str();
}
