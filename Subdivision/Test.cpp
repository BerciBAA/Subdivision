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

// Global variables for rotation angles
float angleX = 0.0f;
float angleY = 0.0f;
float angleZ = 0.0f;
float centerX = 0.0f, centerY = 0.0f, centerZ = 0.0f;
float cameraX = 0.0f, cameraY = 0.0f, cameraZ = 5.0f;

float mouseX = 0.0f, mouseY = 0.0f;
bool isFilled = true;

float minX = FLT_MAX, minY = FLT_MAX, minZ = FLT_MAX;
float maxX = FLT_MIN, maxY = FLT_MIN, maxZ = FLT_MIN;

float paddingFactor = 0.2f;

float width = 1.0f, height = 1.0f;
float paddingX = width * paddingFactor;
float paddingY = height * paddingFactor;

float left = 0.0f;
float right = 0.0f;
float bottom = 0.0f;
float top = 0.0f;
float nearPlane = 0.0f;
float farPlane = 0.0f;

float radius = 5.0f;

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

float buttonYMin = 0.92f;
float buttonYMax = 0.98f;

std::vector<Button> mainMenuButtons = {
    {"Wire/Fill", -0.95f, -0.5f, buttonYMin, buttonYMax, []() {
        isFilled = !isFilled;
        std::cout << "Fill clicked!" << std::endl;

    } },
    {"Subdivison", -0.45f, -0.0f, buttonYMin, buttonYMax, []() {setMenuState(SUBDIVISION_MENU); } } // Changes to SETTINGS_MENU
};

std::vector<Button> subdivisonMenuButtons = {
    {"Back", -0.95f, -0.65f, buttonYMin, buttonYMax, []() {setMenuState(MAIN_MENU); } },
    {"2", -0.63f, -0.33f, buttonYMin, buttonYMax, []() { std::cout << "2 clicked!" << std::endl; }},
    {"3", -0.31f, -0.01f, buttonYMin, buttonYMax, []() { std::cout << "3 clicked!" << std::endl; }}
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

Mesh* meshPtr = nullptr;

// Function to calculate the midpoint of the object
void calculateMinMaxMidPoints(const Mesh& mesh) {
    for (const auto* vertex : mesh.vertices) {
        minX = std::min(minX, vertex->x);
        minY = std::min(minY, vertex->y);
        minZ = std::min(minZ, vertex->z);

        maxX = std::max(maxX, vertex->x);
        maxY = std::max(maxY, vertex->y);
        maxZ = std::max(maxZ, vertex->z);
    }

    // Set the midpoint
    centerX = (minX + maxX) / 2.0f;
    centerY = (minY + maxY) / 2.0f;
    centerZ = (minZ + maxZ) / 2.0f;
}

void calculateOrthoSize(float paddingFactor) {
    width = maxX - minX;
    height = maxY - minY;

    paddingX = width * paddingFactor;
    paddingY = height * paddingFactor;

    left = minX - paddingX;
    right = maxX + paddingX;
    bottom = minY - paddingY;
    top = maxY + paddingY;
    nearPlane = minZ - (maxZ - minZ);
    farPlane = maxZ + (maxZ - minZ);

    cameraZ = farPlane;

    radius = std::max((maxX - minX), (maxY - minY));
    radius = std::max((maxZ - minZ), radius);
}

// Initialization routine.
void setup(void)
{
	glClearColor(1.0, 1.0, 1.0, 0.0);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    if (meshPtr != nullptr) {
        calculateMinMaxMidPoints(*meshPtr);
        calculateOrthoSize(paddingFactor);
    }
}

// OpenGL window reshape routine.
void resize(int w, int h)
{
    glViewport(0, 0, w, h);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(left, right, bottom, top, nearPlane, farPlane);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(centerX, centerY, cameraZ, centerX, centerY, centerZ, 0.0f, 1.0f, 0.0f);
}

void setPadding(float padding) {
    calculateOrthoSize(padding);
    resize(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
}

// Keyboard input processing routine.
void keyInput(unsigned char key, int x, int y)
{
    switch (key) {
    case 'x': angleX += 5.0f; break; // Rotate around X-axis
    case 'y': angleY += 5.0f; break; // Rotate around Y-axis
    case 'z': angleZ += 5.0f; break; // Rotate around Z-axis
    case 'X': angleX -= 5.0f; break; // Rotate around X-axis (reverse)
    case 'Y': angleY -= 5.0f; break; // Rotate around Y-axis (reverse)
    case 'Z': angleZ -= 5.0f; break; // Rotate around Z-axis (reverse)
    case 32: isFilled = !isFilled; break;
    case 27: exit(0); break;
    case '+': {
        paddingFactor += 0.1;
        setPadding(paddingFactor);
        ;
        break;
    }
    case '-': {
        paddingFactor -= 0.1;
        setPadding(paddingFactor);
        break;
    }
    default: break;
    }
    glutPostRedisplay();  // Request a redraw after movement
}

int extractFirstNumber(const std::string& coordinate) {
    size_t firstSlash = coordinate.find('/');
    if (firstSlash != std::string::npos) {
        // If it's in the "vertex/texture/normal" format, take the first index
        std::string firstPart = coordinate.substr(0, firstSlash);
        return std::stoi(firstPart);  // Convert to integer
    }
    else {
        return std::stoi(coordinate);
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
            std::string vertexData;
            while (ss >> vertexData) {
                int index = extractFirstNumber(vertexData);
                face.push_back(index);
            }
            facesIndices.push_back(face);
        }
    }
    std::cout << "Loaded OBJ with " << verticesPos.size() << " vertices and " << facesIndices.size() << " faces.\n";

    objFile.close();
}


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

            float red = (v->x + 1.0f) / 2.0f;   // Normalizálva 0 és 1 közé
            float green = (v->y + 1.0f) / 2.0f;
            float blue = (v->z + 1.0f) / 2.0f;
            glColor3f(red, green, blue);
            //std::cout << "Rendering Vertex: (" << v->x << ", " << v->y << ", " << v->z << ")\n";
            glVertex3f(v->x, v->y, v->z);  // A vertex koordináták megadása


            currentEdge = currentEdge->next;
        } while (currentEdge != startEdge);  // Visszatérünk a kezdõ élhez

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
        if (button.isHovered) {
            glColor3f(0.0f, 0.0f, 0.0f);
        }
        else {
            glColor3f(1.0f, 1.0f, 0.0f);
        }
        float textX = button.xStart + 0.05f;
        float textY = button.yStart + 0.001f;
        glRasterPos2f(textX, textY);
        for (const char& c : button.text) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
        }
    }

    glColor3f(1.0f, 1.0f, 0.0f);
    glRasterPos2f(0.5f, buttonYMin + 0.001f);

    std::ostringstream oss;
    oss << "Padding: " << paddingFactor * 100 << "%";

    for (const char& c : oss.str()) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
    }

    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_DEPTH_TEST);
}


void drawScene(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Szín- és mélységi buffer törlése   | GL_DEPTH_BUFFER_BIT

    glLoadIdentity();

    drawNavBar();

    // Translate to the midpoint, apply rotations, then translate back
    glTranslatef(centerX, centerY, centerZ);
    glRotatef(angleX, 1.0f, 0.0f, 0.0f);
    glRotatef(angleY, 0.0f, 1.0f, 0.0f);
    glRotatef(angleZ, 0.0f, 0.0f, 1.0f);
    glTranslatef(-centerX, -centerY, -centerZ);

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

void checkForErrors() {

    std::cout << "\n\nChecking for errors:\n" << std::endl;


    // Check for square faces
    int squareFaceCounter = 0;
    for (auto& face : meshPtr->faces) {
        int edgeCounter = 1;
        HalfEdge* start = face->edge;
        HalfEdge* next = start->next;
        while (next != start) {
            edgeCounter++;
            next = next->next;
        }
        if (edgeCounter == 4)
            squareFaceCounter++;
        else if (edgeCounter > 4)
            std::cout << "WARNING: Face with " << edgeCounter << " edges!" << std::endl;
    }
    if (squareFaceCounter > 0)
        std::cout << "WARNING: " << squareFaceCounter << " face with square edges found!" << std::endl;

}


// Main routine.
int main(int argc, char** argv)
{

    std::string objFile = "globe.obj";

    populateHalfEdgeStructure(objFile);

    checkForErrors();

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