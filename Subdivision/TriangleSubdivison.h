#pragma once
#include "HalfEdge.h"

#include <unordered_map>
#include <iostream>


class TriangleSubdivison
{

protected:
	void rebuildFace(Face* face, Mesh* mesh, std::vector<HalfEdge*>& newHalfEdges, std::vector<Face*>& newFaces, std::unordered_map<HalfEdge*, Vertex*>& edgeVertexMap);
};

