#include "Shadings.h"

void Shadings::setupLighting(void) {
    // Enable the lighting system
    glEnable(GL_LIGHTING);

    // Light property vectors.
    float lightAmb[] = { 0.0, 0.0, 0.0, 1.0 };
    float lightDifAndSpec[] = { 1.0, 1.0, 1.0, 1.0 };
    float globAmb[] = { 0.05, 0.05, 0.05, 1.0 };

    // Light properties.
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmb);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDifAndSpec);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightDifAndSpec);

    glEnable(GL_LIGHT0); // Enable particular light source.
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globAmb); // Global ambient light.
    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE); // Enable local viewpoint.

    // Material property vectors.
    float matSpec[] = { 1.0, 1.0, 1.0, 1.0 };
    float matShine[] = { 50.0f };

    // Material properties shared by all the spheres.
    glMaterialfv(GL_FRONT, GL_SPECULAR, matSpec);
    glMaterialfv(GL_FRONT, GL_SHININESS, matShine);

    // Cull back faces.
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    // Enable color material mode:
    // The ambient and diffuse color of the front faces will track the color set by glColor().
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
}

void Shadings::disableLighting(void) {
    //// Disable the lighting system
    //glDisable(GL_LIGHTING);
    //glDisable(GL_LIGHT0);

    //// Reset global ambient lighting to avoid influencing rendering
    //float noAmbientLight[] = { 0.0, 0.0, 0.0, 1.0 };
    //glLightModelfv(GL_LIGHT_MODEL_AMBIENT, noAmbientLight);

    //// Reset material properties to avoid leftover effects
    //float defaultMaterial[] = { 1.0, 1.0, 1.0, 1.0 }; // White default
    //glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, defaultMaterial);
    //glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, defaultMaterial);
    //glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, defaultMaterial);
    //glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 0.0f); // No shininess

    //// Ensure color material tracking is enabled so glColor works
    //glEnable(GL_COLOR_MATERIAL);
    //glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    //// Disable face culling (optional, depending on your needs)
    //glDisable(GL_CULL_FACE);

    float lightAmb[] = { 1.0, 1.0, 1.0, 1.0 };
    float lightDifAndSpec[] = { 0.0, 0.0, 0.0, 0.0 };
    float globAmb[] = { 1.0, 1.0, 1.0, 1.0 };

    // Light properties.
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmb);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDifAndSpec);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightDifAndSpec);

    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globAmb);
}

void Shadings::flatShading(Face* f) {
    std::array<float, 3> norms = calculateFaceNormal(f);

    // Set the normal for the triangle
    glNormal3f(norms[0], norms[1], norms[2]);
}

void Shadings::gouraudShading(Vertex* v, Mesh* mesh) {
    float* vertexNorms = vertexNormals[v];

    // Set the normal for the vertex
    glNormal3f(vertexNorms[0], vertexNorms[1], vertexNorms[2]);
}

void Shadings::calculateNormals(Mesh* mesh)
{
    // TODO: delete not in use key value pairs
     
    // calculate every face normal
    for (Face* face : mesh->faces) {
        std::array<float, 3> fnorms = calculateFaceNormal(face);
        faceNormals[face][0] = fnorms[0];
        faceNormals[face][1] = fnorms[1];
        faceNormals[face][2] = fnorms[2];
    }

    // calculate every vertex normal for Gouraud shading
    for (Vertex* vertex : mesh->vertices) {
        std::array<float, 3> vertexNorms = calculateVertexNormal(vertex, mesh);

        vertexNormals[vertex][0] = vertexNorms[0];
        vertexNormals[vertex][1] = vertexNorms[1];
        vertexNormals[vertex][2] = vertexNorms[2];
    }

    std::cout << "recalculated normals" << std::endl;
}

std::array<float, 3> Shadings::calculateFaceNormal(Face* f) {
    HalfEdge* startEdge = f->edge;

    Vertex* v1 = startEdge->origin;
    Vertex* v2 = startEdge->next->origin;
    Vertex* v3 = startEdge->next->next->origin;

    // Calculate vectors for the triangle edges
    float ux = v2->x - v1->x;
    float uy = v2->y - v1->y;
    float uz = v2->z - v1->z;

    float vx = v3->x - v1->x;
    float vy = v3->y - v1->y;
    float vz = v3->z - v1->z;

    // Compute the cross product (u × v) to get the normal
    float nx = uy * vz - uz * vy;
    float ny = uz * vx - ux * vz;
    float nz = ux * vy - uy * vx;

    // Normalize the normal vector
    float length = sqrt(nx * nx + ny * ny + nz * nz);
    if (length > 0.0f) {
        nx /= length;
        ny /= length;
        nz /= length;
    }

    return {nx, ny, nz};
}

std::array<float, 3> Shadings::calculateVertexNormal(Vertex* v, Mesh* mesh)
{
    std::array<float, 3> vertexNorms = { 0.0f, 0.0f, 0.0f };

    std::vector<Face*> neighborFaces;
    for (const auto& hfe : mesh->halfEdges) {
        if (hfe->origin == v &&
            (std::find(neighborFaces.begin(), neighborFaces.end(), hfe->incidentFace) == neighborFaces.end())) {
            neighborFaces.push_back(hfe->incidentFace);
        }
    }

    for (const auto& neighborFace : neighborFaces) {
        float* faceNorms = faceNormals[neighborFace];
        vertexNorms[0] += faceNorms[0];
        vertexNorms[1] += faceNorms[1];
        vertexNorms[2] += faceNorms[2];
    }

    float length = sqrt(vertexNorms[0] * vertexNorms[0] + vertexNorms[1] * vertexNorms[1] + vertexNorms[2] * vertexNorms[2]);
    if (length > 0.0f) {
        vertexNorms[0] /= length;
        vertexNorms[1] /= length;
        vertexNorms[2] /= length;
    }

    return vertexNorms;
}
