///////////////////////////////////
// square.cpp
//
// OpenGL program to draw a square.
// 
// Sumanta Guha.
///////////////////////////////////

#include <GL/glew.h>
#include <GL/freeglut.h> 
#include <iostream>
#include <vector>

class HalfEdge;
class Face;

// Based on this source: https://jerryyin.info/geometry-processing-algorithms/half-edge/
class Vertex {
public:
    float x, y, z;
    HalfEdge* incidentEdge;

    Vertex(float x, float y, float z) : x(x), y(y), z(z), incidentEdge(nullptr) {}
};

class HalfEdge {
public:
    Vertex* origin;
    HalfEdge* twin;
    HalfEdge* next;
    HalfEdge* prev;
    Face* incidentFace;

    HalfEdge() : origin(nullptr), twin(nullptr), next(nullptr), prev(nullptr), incidentFace(nullptr) {}
};

class Face {
public:
    HalfEdge* edge;

    Face() : edge(nullptr) {}
};

class Mesh {
public:
    std::vector<Vertex*> vertices;
    std::vector<HalfEdge*> halfEdges;
    std::vector<Face*> faces;


    Mesh(const std::vector<std::vector<int>>& facesIndices, const std::vector<std::vector<float>>& verticesPos) {
        // Create vertices
        for (const auto& pos : verticesPos) {
            vertices.push_back(new Vertex(pos[0], pos[1], pos[2]));
        }

        // Create half-edges and faces
        for (const auto& face : facesIndices) {
            Face* newFace = new Face();
            faces.push_back(newFace);

            HalfEdge* prevEdge = nullptr;
            HalfEdge* firstEdge = nullptr;

            // Loop over each vertex index in the face
            for (int i = 0; i < face.size(); ++i) {
                int currVertexIdx = face[i] - 1;
                int nextVertexIdx = face[(i + 1) % face.size()] - 1;

                HalfEdge* edge = new HalfEdge();
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
            }

            newFace->edge = firstEdge;
        }

        createTwins();
    }

    void createTwins() {
        for (auto* he1 : halfEdges) {
            if (!he1->twin) {
                for (auto* he2 : halfEdges) {
                    if (he1->origin == he2->next->origin && he1->next->origin == he2->origin) {
                        he1->twin = he2;
                        he2->twin = he1;
                        break;
                    }
                }
            }
        }
    }

    ~Mesh() {
        for (auto* v : vertices) delete v;
        for (auto* he : halfEdges) delete he;
        for (auto* f : faces) delete f;
    }
};

// Drawing routine.
void drawScene(void)
{
	glClear(GL_COLOR_BUFFER_BIT);

	glColor3f(0.0, 0.0, 0.0);

	// Draw a polygon with specified vertices.
	glBegin(GL_POLYGON);
	glVertex3f(20.0, 20.0, 0.0);
	glVertex3f(80.0, 20.0, 0.0);
	glVertex3f(80.0, 80.0, 0.0);
	glVertex3f(20.0, 80.0, 0.0);
	glEnd();

	glFlush();
}

// Initialization routine.
void setup(void)
{
	glClearColor(1.0, 1.0, 1.0, 0.0);
}

// OpenGL window reshape routine.
void resize(int w, int h)
{
	glViewport(0, 0, w, h);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, 100.0, 0.0, 100.0, -1.0, 1.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

// Keyboard input processing routine.
void keyInput(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 27:
		exit(0);
		break;
	default:
		break;
	}
}

void testHalfEdgeStructure() {
    // Vertex positions from the OBJ file
    std::vector<std::vector<float>> vertexPositions = {
        {1.0, 4.0, 0.0}, {3.0, 4.0, 0.0}, {0.0, 2.0, 0.0},
        {2.0, 2.0, 0.0}, {4.0, 2.0, 0.0}, {1.0, 0.0, 0.0},
        {3.0, 0.0, 0.0}
    };

    // Face vertex indices from the OBJ file (converted to 0-based indexing)
    std::vector<std::vector<int>> faceIndices = {
        {1, 3, 4}, {1, 4, 2}, {2, 4, 5},
        {3, 6, 4}, {4, 6, 7}, {4, 7, 5}
    };

    // Create mesh
    Mesh mesh(faceIndices, vertexPositions);

    std::cout << "Mesh created successfully!" << std::endl;
}

// Main routine.
int main(int argc, char** argv)
{
    testHalfEdgeStructure();

	glutInit(&argc, argv);

	glutInitContextVersion(4, 3);
	glutInitContextProfile(GLUT_COMPATIBILITY_PROFILE);

	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA);

	glutInitWindowSize(500, 500);
	glutInitWindowPosition(100, 100);

	glutCreateWindow("square.cpp");

	glutDisplayFunc(drawScene);
	glutReshapeFunc(resize);
	glutKeyboardFunc(keyInput);

	glewExperimental = GL_TRUE;
	glewInit();

	setup();

	glutMainLoop();
}