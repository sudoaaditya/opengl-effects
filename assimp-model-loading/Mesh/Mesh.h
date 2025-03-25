#ifndef MESH_H
#define MESH_H

#include <vector>
#include <string>
#include "../vmath/vmath.h"
using namespace std;

#define MAX_BONE_INFLUENCE 4

struct Vertex {
    //
    vmath::vec3 Position;
    vmath::vec3 Normal;
    vmath::vec2 TexCoords;
    vmath::vec3 Tangent;
    vmath::vec3 Bitangent;

    int m_boneIDs[MAX_BONE_INFLUENCE];
    float m_weights[MAX_BONE_INFLUENCE];
};

struct Texture {
    unsigned int id;
    string type;
    string path;
};

class Mesh {
    public:
        vector<Vertex> vertices;
        vector<unsigned int> indices;
        vector<Texture> textures;
        unsigned int vao;

        Mesh(vector<Vertex>, vector<unsigned int>, vector<Texture>);
        void draw(unsigned int);

    private:
        unsigned int vbo, ebo;

        void setupMesh();
};

#endif