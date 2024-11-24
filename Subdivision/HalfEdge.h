#pragma once
#ifndef HALF_EDGE_H
#define HALF_EDGE_H

#include <vector>
#include <string>
#include <sstream>

// Forward declarations for pointers
class HalfEdge;
class Face;

class Vertex {
public:
    float x, y, z;
    HalfEdge* incidentEdge;
    std::string name;

    Vertex(float x, float y, float z, const std::string& name = "");
    Vertex(Vertex* v);
    std::string toString() const;
};

class HalfEdge {
public:
    Vertex* origin;
    HalfEdge* twin;
    HalfEdge* next;
    HalfEdge* prev;
    Face* incidentFace;
    std::string name;

    HalfEdge(const std::string& name);
    HalfEdge();

    std::string toString() const;
    bool isBoundaryEdge();
    std::string getNextName();
};

class Face {
public:
    HalfEdge* edge;
    std::string name;

    Face(std::string& name);
    Face();
    std::string toString() const;
};

class Mesh {
public:
    std::vector<Vertex*> vertices;
    std::vector<HalfEdge*> halfEdges;
    std::vector<Face*> faces;

    Mesh(const std::vector<std::vector<int>>& facesIndices, const std::vector<std::vector<float>>& verticesPos);
    ~Mesh();

    std::string toString() const;
    void createTwinEdges();
};

#endif // HALF_EDGE_H