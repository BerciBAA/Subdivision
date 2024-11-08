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
#include <cmath>
#include <functional>

// Camera movement variables
float cameraX = 1.0f, cameraY = 1.0f, cameraZ = 1.0f;  // Camera position
float cameraSpeed = 0.1f;  // Camera movement speed

float lookAtX = 0.0f, lookAtY = 0.0f, lookAtZ = 0.0f;  // Look-at target
float cameraAngle = 0.0f;  // Angle for rotating the camera around the Y-axis

float mouseX = 0.0f, mouseY = 0.0f;
bool isFilled = true;

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
    float scale = 1.0f;


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
        scaleToFit(10.0f);

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

    void scaleToFit(float targetSize) {
        float minX = FLT_MAX, minY = FLT_MAX, minZ = FLT_MAX;
        float maxX = -FLT_MAX, maxY = -FLT_MAX, maxZ = -FLT_MAX;

        for (const auto& vertex : vertices) {
            minX = std::min(minX, vertex->x);
            minY = std::min(minY, vertex->y);
            minZ = std::min(minZ, vertex->z);
            maxX = std::max(maxX, vertex->x);
            maxY = std::max(maxY, vertex->y);
            maxZ = std::max(maxZ, vertex->z);
        }
        std::cout << "Before Scaling: Min(" << minX << ", " << minY << ", " << minZ
            << "), Max(" << maxX << ", " << maxY << ", " << maxZ << ")\n";

        float midX = (minX + maxX) / 2.0f;
        float midY = (minY + maxY) / 2.0f;
        float midZ = (minZ + maxZ) / 2.0f;
        float maxDimension = std::max({ maxX - minX, maxY - minY, maxZ - minZ });

        scale = targetSize / maxDimension;

        for (auto& vertex : vertices) {
            vertex->x = (vertex->x - midX) * scale;
            vertex->y = (vertex->y - midY) * scale;
            vertex->z = (vertex->z - midZ) * scale;
        }

        for (const auto& vertex : vertices) {
            if (vertex->x < -10.0f || vertex->x > 10.0f ||
                vertex->y < -10.0f || vertex->y > 10.0f ||
                vertex->z < -10.0f || vertex->z > 10.0f) {
                std::cout << "Warning: Vertex out of bounds after scaling: ("
                    << vertex->x << ", " << vertex->y << ", " << vertex->z << ")"
                    << std::endl;
            }
        }
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
// END OF DATA STRUCTURE

// CUSTOM UI COMPONENTS
struct Button {
    std::string text;
    float xStart, xEnd, yStart, yEnd;
    bool isHovered;
    std::function<void()> onClick;

    Button(const std::string& text, float xStart, float xEnd, float yStart, float yEnd,
        std::function<void()> onClick)
        : text(text), xStart(xStart), xEnd(xEnd), yStart(yStart), yEnd(yEnd), isHovered(false), onClick(onClick) {}

    Button() {}
};
std::vector<Button> navButtons;


enum MenuState { MAIN_MENU, SUBDIVISION_MENU };
MenuState menuState = MAIN_MENU;
void setMenuState(MenuState newState);

std::vector<Button> mainMenuButtons = {
    {"Wire/Fill", -0.95f, -0.5f, 0.92f, 1.0f, []() { 
        isFilled = !isFilled;
        std::cout << "Fill clicked!" << std::endl;

    } },
    {"Subdivison", -0.45f, -0.0f, 0.92f, 1.0f, []() {setMenuState(SUBDIVISION_MENU); } } // Changes to SETTINGS_MENU
};

std::vector<Button> subdivisonMenuButtons = {
    {"Back", -0.95f, -0.65f, 0.92f, 1.0f, []() {setMenuState(MAIN_MENU); } },
    {"2", -0.63f, -0.33f, 0.92f, 1.0f, []() { std::cout << "2 clicked!" << std::endl; }},
    {"3", -0.31f, -0.01f, 0.92f, 1.0f, []() { std::cout << "3 clicked!" << std::endl; }}
};


void updateNavBar() {
    switch (menuState) {
    case MAIN_MENU:
        navButtons = mainMenuButtons;
        break;
    case SUBDIVISION_MENU:
        navButtons = subdivisonMenuButtons;
        break;
    }
}

void setMenuState(MenuState newState) {
    menuState = newState;

    updateNavBar();
}
// END OF CUSTOM UI COMPONENTS

  
// Initialization routine.
void setup(void)
{
	glClearColor(1.0, 1.0, 1.0, 0.0);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
}

// OpenGL window reshape routine.
void resize(int w, int h)
{
    glViewport(0, 0, w, h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    float viewSize = 10.0f;
    glOrtho(-viewSize, viewSize, -viewSize, viewSize, -viewSize, viewSize);

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

    case 'w':  // Move forward
        cameraX += cameraSpeed * sin(cameraAngle);
        cameraZ -= cameraSpeed * cos(cameraAngle);
        break;

    case 's':  // Move backward
        cameraX -= cameraSpeed * sin(cameraAngle);
        cameraZ += cameraSpeed * cos(cameraAngle);
        break;

    case 'a':  // Move left
        cameraX -= cameraSpeed * cos(cameraAngle);
        cameraZ -= cameraSpeed * sin(cameraAngle);
        break;

    case 'd':  // Move right
        cameraX += cameraSpeed * cos(cameraAngle);
        cameraZ += cameraSpeed * sin(cameraAngle);
        break;

    case 'q':  // Rotate left
        cameraAngle -= 0.1f;
        break;

    case 'e':  // Rotate right
        cameraAngle += 0.1f;
        break;
    case 32:
        isFilled = !isFilled;
        break;

    default:
        break;
    }

    glutPostRedisplay();  // Request a redraw after movement
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
    std::cout << "Loaded OBJ with " << verticesPos.size() << " vertices and " << facesIndices.size() << " faces.\n";

    objFile.close();
}

Mesh* meshPtr = nullptr;


void renderMesh(const Mesh& mesh) {

    //srand(static_cast<unsigned>(time(0)));

    for(int i = 0; i < mesh.faces.size(); i++) //mesh.faces.size()
     {
        /*if (i % 2 == 0) {
            float red = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
            float green = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
            float blue = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
            glColor3f(red, green, blue);
        }*/


        HalfEdge* startEdge = mesh.faces[i]->edge;
        HalfEdge* currentEdge = startEdge;
        if (isFilled)
        {
            glBegin(GL_TRIANGLES);
        }
        else {
            glBegin(GL_LINE_LOOP);
        }

        do {

            Vertex* v = currentEdge->origin;

            float red = (v->x + 1.0f) / 2.0f;   // Normaliz�lva 0 �s 1 k�z�
            float green = (v->y + 1.0f) / 2.0f;
            float blue = (v->z + 1.0f) / 2.0f;
            glColor3f(red, green, blue);
            //std::cout << "Rendering Vertex: (" << v->x << ", " << v->y << ", " << v->z << ")\n";
            glVertex3f(v->x, v->y, v->z);  // A vertex koordin�t�k megad�sa


            currentEdge = currentEdge->next;
        } while (currentEdge != startEdge);  // Visszat�r�nk a kezd� �lhez

        glEnd();
    }
}

void updateHoverState(float x, float y) {
    for (auto& button : navButtons) {
        button.isHovered = (x > button.xStart && x < button.xEnd && y > button.yStart && y < button.yEnd);
    }
}

void mouseMove(int x, int y) {
    mouseX = (float)x / (float)glutGet(GLUT_WINDOW_WIDTH) * 2.0f - 1.0f;
    mouseY = 1.0f - (float)y / (float)glutGet(GLUT_WINDOW_HEIGHT) * 2.0f;

    updateHoverState(mouseX, mouseY);
    glutPostRedisplay();
}

void mouseClick(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        for (auto& btn : navButtons) {
            if (btn.isHovered) {
                btn.onClick();
            }
        }
    }
}

void drawNavBar(void) {
    glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);

    glColor3f(0.2f, 0.2f, 0.2f); // navbar
    glBegin(GL_QUADS);
        glVertex2f(-1.0f, 0.9f);
        glVertex2f(1.0f, 0.9f);
        glVertex2f(1.0f, 1.0f);
        glVertex2f(-1.0f, 1.0f);
    glEnd();

    for (const auto& button : navButtons) {
        if (button.isHovered) {
            glColor3f(1.0f, 1.0f, 0.0f);
        }
        else {
            glColor3f(0.3f, 0.3f, 0.3f);
        }

        // Draw button rectangle
        glBegin(GL_QUADS);
            glVertex2f(button.xStart, button.yStart);
            glVertex2f(button.xEnd, button.yStart);
            glVertex2f(button.xEnd, button.yEnd);
            glVertex2f(button.xStart, button.yEnd);
        glEnd();

        // Render button text
        glColor3f(1.0f, 1.0f, 1.0f);
        float textX = button.xStart + 0.05f;
        float textY = button.yStart + 0.02f;
        glRasterPos2f(textX, textY);
        for (const char& c : button.text) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
        }
    }

    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_DEPTH_TEST);
}


void drawScene(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Sz�n- �s m�lys�gi buffer t�rl�se   | GL_DEPTH_BUFFER_BIT

    glLoadIdentity();

    drawNavBar();

    //gluLookAt(2.0, 2.0, 1.0,  // Kamera poz�ci�ja kicsit fel�lr�l
    //          0.0, 0.0, 0.0,  // K�z�ppont
    //          0.0, 1.0, 0.0); // Felfel� ir�ny (Y tengely)

    gluLookAt(cameraX, cameraY, cameraZ,
        cameraX + sin(cameraAngle), lookAtY, cameraZ - cos(cameraAngle),
        0.0, 1.0, 0.0);

    if (meshPtr != nullptr) {
        renderMesh(*meshPtr);
    }

    glFlush();

}


void populateHalfEdgeStructure(const std::string& objFile) {

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

    

    //std::cout << meshPtr->toString();
}



// Main routine.
int main(int argc, char** argv)
{

    std::string objFile = "suitcase.obj";

    populateHalfEdgeStructure(objFile);

    setMenuState(MAIN_MENU);

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

    glutPassiveMotionFunc(mouseMove); // mouse movement
    glutMouseFunc(mouseClick);        // mouse clicks

	glewExperimental = GL_TRUE;
	glewInit();

	setup();

	glutMainLoop();
}