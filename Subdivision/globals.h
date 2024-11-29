#pragma once
#include <unordered_map>
#include "HalfEdge.h"

extern int vertexNameIdx;
extern int faceNameIdx;
extern int halfEdgeNameIdx;

extern std::unordered_map<Vertex*, float[3]> vertexNormals;
extern std::unordered_map<Face*, float[3]> faceNormals;