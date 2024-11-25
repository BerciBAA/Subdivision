#pragma once
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <array>

#include "HalfEdge.h"

enum ShadingTypes { FLAT, GOURAUD, PHONG, NONE };

class Shadings
{
public:
	void static setupLighting(void);
	void static disableLighting(void);
	void static flatShading(Face* f);
	void static gouraudShading(Vertex* v, Mesh* mesh);
private:
	std::array<float, 3> static calculateNormForFace(Face* f);
};

