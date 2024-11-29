#pragma once
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <array>
#include <iostream>

#include "HalfEdge.h"
#include "globals.h"

enum ShadingTypes { FLAT, GOURAUD, PHONG, NONE };

class Shadings
{
public:
	void static setupLighting(void);
	void static disableLighting(void);
	void static flatShading(Face* f);
	void static gouraudShading(Vertex* v, Mesh* mesh);
	void static calculateNormals(Mesh* mesh);
private:
	std::array<float, 3> static calculateFaceNormal(Face* f);
	std::array<float, 3> static calculateVertexNormal(Vertex* v, Mesh* mesh);
};

