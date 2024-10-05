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
#include <sstream>
#include <fstream>
#include <string>
#include <cstdlib> 
#include <ctime>  

class HalfEdge;
class Face;
class Vertex;



// Based on this source: https://jerryyin.info/geometry-processing-algorithms/half-edge/
class Vertex {
public:
    float x, y, z;
    HalfEdge* incidentEdge;
    std::string name;

    Vertex(float x, float y, float z, const std::string& name = "") 
        : x(x), y(y), z(z), incidentEdge(nullptr), name(name) {}

    std::string toString() const {
        std::stringstream ss;
        ss << "Vertex " << name << " (" << x << ", " << y << ", " << z << ")";
        return ss.str();
    }
};

class HalfEdge {
public:
    Vertex* origin;
    HalfEdge* twin;
    HalfEdge* next;
    HalfEdge* prev;
    Face* incidentFace;
    std::string name;

    HalfEdge(const std::string& name = "") 
        : origin(nullptr), twin(nullptr), next(nullptr), prev(nullptr), incidentFace(nullptr), name(name) {}

    std::string toString() const;
};

class Face {
public:
    HalfEdge* edge;
    std::string name;

    Face(const std::string& name = "") : edge(nullptr), name(name) {}

    std::string toString() const {
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
};

std::string HalfEdge::toString() const {
    std::stringstream ss;
    ss << "HalfEdge " << name << " from " << origin->name << std::endl;
    ss << "twin: " << (twin ? twin->name : "-") << std::endl;
    ss << "next: " << (next ? next->name : "-") << std::endl;
    ss << "prev: " << (prev ? prev->name : "-") << std::endl;
    ss << "face: " << (incidentFace ? incidentFace->name : "-") << std::endl;
    return ss.str();
}

class Mesh {
public:
    std::vector<Vertex*> vertices;
    std::vector<HalfEdge*> halfEdges;
    std::vector<Face*> faces;


    Mesh(const std::vector<std::vector<int>>& facesIndices, const std::vector<std::vector<float>>& verticesPos) {
        // Create vertices
        int vertexNameIdx = 1;
        int faceNameIdx = 0;
        int halfEdgeNameIdx = 0;

        for (const auto& pos : verticesPos) {
            std::stringstream vertexNameStream;
            vertexNameStream << "v" << vertexNameIdx;

            vertices.push_back(new Vertex(pos[0], pos[1], pos[2], vertexNameStream.str() ));

            vertexNameIdx++;
        }

        // Create half-edges and faces
        for (const auto& face : facesIndices) {
            std::stringstream faceNameStream;
            faceNameStream << "f" << faceNameIdx;

            Face* newFace = new Face(faceNameStream.str());
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

        createTwinEdges(&halfEdgeNameIdx);
    }

    void createTwinEdges(int* halfEdgeNameIdx) {
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
                    boundaryHalfEdgeNameStream << "e" << *halfEdgeNameIdx;

                    HalfEdge* boundaryEdge = new HalfEdge(boundaryHalfEdgeNameStream.str());

                    boundaryEdge->origin = he1->next->origin;
                    boundaryEdge->twin = he1;
                    he1->twin = boundaryEdge;

                    boundaryEdge->incidentFace = nullptr;

                    boundaryHalfEdges.push_back(boundaryEdge);
                    (*halfEdgeNameIdx)++;
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

    ~Mesh() {
        for (auto* v : vertices) delete v;
        for (auto* he : halfEdges) delete he;
        for (auto* f : faces) delete f;
    }

    std::string toString() const {
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
};
  
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

    //MEGTALÁN EZT IS KELL MÓKOLNI DÁVID
    glOrtho(-10.0, 10.0, -10.0, 10.0, -10.0, 10.0);

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

void loadOBJ(const std::string& filename, std::vector<std::vector<float>>& verticesPos, std::vector<std::vector<int>>& facesIndices) {
    std::ifstream objFile(filename);
    if (!objFile.is_open()) {
        std::cerr << "Could not open the file: " << filename << std::endl;
        return;
    }

    std::string line;
    while (std::getline(objFile, line)) {
        std::istringstream ss(line);
        std::string token;
        ss >> token;

        // Process vertex position
        if (token == "v") {
            std::vector<float> vertex(3);
            ss >> vertex[0] >> vertex[1] >> vertex[2];
            verticesPos.push_back(vertex);
        }

        // Process face indices
        else if (token == "f") {
            std::vector<int> face;
            int index;
            while (ss >> index) {
                face.push_back(index);
            }
            facesIndices.push_back(face);
        }
    }

    objFile.close();
}

Mesh* meshPtr = nullptr;


void renderMesh(const Mesh& mesh) {

    srand(static_cast<unsigned>(time(0)));

    for(int i = 0; i < mesh.faces.size(); i++)
     {
        if (i % 2 == 0) {
            float red = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
            float green = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
            float blue = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
            glColor3f(red, green, blue);
        }


        HalfEdge* startEdge = mesh.faces[i]->edge;
        HalfEdge* currentEdge = startEdge;

        glBegin(GL_LINE_LOOP);
        
        do {
            Vertex* v = currentEdge->origin;
            glVertex3f(v->x, v->y, v->z);  // A vertex koordináták megadása
            currentEdge = currentEdge->next;
        } while (currentEdge != startEdge);  // Visszatérünk a kezdõ élhez

        glEnd();
    }
}


void drawScene(void) {
 glClear(GL_COLOR_BUFFER_BIT);

    glLoadIdentity();
    //EZT KELL ITT MÓKOLNI DÁVID EZT
    gluLookAt(2.0, 2.0, 1.0,  // Kamera pozíciója kicsit felülrõl
              0.0, 0.0, 0.0,  // Középpont
              0.0, 1.0, 0.0); // Felfelé irány (Y tengely)

    if (meshPtr != nullptr) {
        renderMesh(*meshPtr);
    }

    glFlush();

}


void testHalfEdgeStructure(const std::string& objFile) {

    /*// Vertex positions from the OBJ file
    std::vector<std::vector<float>> vertexPositions = {
        {1.0, 4.0, 0.0}, {3.0, 4.0, 0.0}, {0.0, 2.0, 0.0},
        {2.0, 2.0, 0.0}, {4.0, 2.0, 0.0}, {1.0, 0.0, 0.0},
        {3.0, 0.0, 0.0}
    };

    // Face vertex indices from the OBJ file (converted to 0-based indexing)
    std::vector<std::vector<int>> faceIndices = {
        {1, 3, 4}, {1, 4, 2}, {2, 4, 5},
        {3, 6, 4}, {4, 6, 7}, {4, 7, 5}
    };*/

    std::vector<std::vector<float>> vertexPositions;
    std::vector<std::vector<int>> faceIndices;


    loadOBJ(objFile, vertexPositions, faceIndices);

    // Create mesh
    meshPtr = new Mesh(faceIndices, vertexPositions);

    std::cout << "Mesh created successfully:" << std::endl;

    std::cout << meshPtr->toString();
}



// Main routine.
int main(int argc, char** argv)
{

    std::string objFile = "house_with_roof.obj";

    testHalfEdgeStructure(objFile);

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