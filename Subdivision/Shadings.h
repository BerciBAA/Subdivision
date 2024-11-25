#pragma once
#include <GL/glew.h>
#include <GL/freeglut.h> 

#include "HalfEdge.h"

enum ShadingTypes { FLAT, GOURAUD, PHONG, NONE };

class Shadings
{
public:
	void static setupLighting(void);
	void static disableLighting(void);
	void static flatShading(HalfEdge* startEdge);
};

