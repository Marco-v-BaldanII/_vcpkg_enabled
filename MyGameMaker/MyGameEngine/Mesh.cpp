#include "Mesh.h"
#include "GameObject.h"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "assimp/cimport.h"
#include <assimp/Importer.hpp>
#include <vector>
#include <string>
#include <GL/glew.h>
#include <IL/il.h>
#include <IL/ilu.h>
#include <IL/ilut.h>
#include <iostream>
#include <string>
using namespace std;


#define CHECKERS_HEIGHT 32
#define CHECKERS_WIDTH 32

Mesh::Mesh() {
    
  
}
Mesh::~Mesh() {

}

void Mesh::Load(const glm::vec3* vertices, size_t num_verts, unsigned int* indices, size_t num_indexs) {
	vertices_buffer.LoadData(vertices, num_verts * sizeof(glm::vec3));
	indices_buffer.LoadIndices(indices, num_indexs);
    texCoords_buffer.UnLoad();
	normals_buffer.UnLoad();
	colors_buffer.UnLoad();

	_vertices.clear();
	_indices.clear();
	_vertices.assign(vertices, vertices + num_verts);
	_indices.assign(indices, indices + num_indexs);
}
void Mesh::loadTexCoords(const glm::vec2* texCoords, size_t num_texCoords) {
	texCoords_buffer.LoadData(texCoords, num_texCoords * sizeof(glm::vec2));
}
void Mesh::LoadNormals(const glm::vec3* normals, size_t num_normals) {
	normals_buffer.LoadData(normals, num_normals * sizeof(glm::vec3));
}
void Mesh::LoadColors(const glm::u8vec3* colors, size_t num_colors) {
	colors_buffer.LoadData(colors, num_colors * sizeof(glm::u8vec3));
}
void Mesh::Draw() const 
{
    if (textureID) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, textureID);
	}

    //if (texCoords_buffer.Id()) {
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        //for shaders enable veretx attribute something ? watafak
        texCoords_buffer.bind();
        //instead of pointer glvertex atribute, the position attribute
		glTexCoordPointer(2, GL_FLOAT, 0, nullptr);
    //}
	if (normals_buffer.Id()) {
		glEnableClientState(GL_NORMAL_ARRAY);
		normals_buffer.bind();
		glNormalPointer(GL_FLOAT, 0, nullptr);
	}
	if (colors_buffer.Id()) {
		glEnableClientState(GL_COLOR_ARRAY);
		colors_buffer.bind();
		glColorPointer(3, GL_UNSIGNED_BYTE, 0, nullptr);
	}
    
	glEnableClientState(GL_VERTEX_ARRAY);
    vertices_buffer.bind();
	glVertexPointer(3, GL_FLOAT, 0, nullptr);


	indices_buffer.bind();
	glDrawElements(GL_TRIANGLES, _indices.size(), GL_UNSIGNED_INT, 0);

	glDisableClientState(GL_VERTEX_ARRAY);
	if (texCoords_buffer.Id()) { glDisableClientState(GL_TEXTURE_COORD_ARRAY);}
	if (normals_buffer.Id()) { glDisableClientState(GL_NORMAL_ARRAY);}
	if (colors_buffer.Id()) { glDisableClientState(GL_COLOR_ARRAY);}

    if (textureID) {
        glBindTexture(GL_TEXTURE_2D, 0);
        glDisable(GL_TEXTURE_2D);
    }
}



void Mesh::LoadMesh(const char* file_path) {
    const aiScene* scene = aiImportFile(file_path, aiProcessPreset_TargetRealtime_MaxQuality);
  
    if (scene != nullptr && scene->HasMeshes()) {
        for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
            size_t num_vertices = scene->mMeshes[i]->mNumVertices;
            glm::vec3* vertex = new glm::vec3[num_vertices * 3];
            memcpy(vertex, scene->mMeshes[i]->mVertices, sizeof(float) * num_vertices * 3);

            if (scene->mMeshes[i]->HasTextureCoords(0)) {
                glm::vec2* texCoords = new glm::vec2[num_vertices];
                for (size_t j = 0; j < num_vertices; ++j) {
                    texCoords[j] = glm::vec2(
                        scene->mMeshes[i]->mTextureCoords[0][j].x,
                        scene->mMeshes[i]->mTextureCoords[0][j].y
                    );
                }
                loadTexCoords(texCoords, num_vertices);
                delete[] texCoords;
            }
          

            if (scene->mMeshes[i]->HasFaces()) {
                size_t num_index = scene->mMeshes[i]->mNumFaces * 3;
                unsigned int* index = new unsigned int[num_index]; // assume each face is a triangle
                for (unsigned int j = 0; j < scene->mMeshes[i]->mNumFaces; ++j) {
                    memcpy(&index[j * 3], scene->mMeshes[i]->mFaces[j].mIndices, 3 * sizeof(unsigned int));
                }
				 Load(vertex,num_vertices,index,num_index);
            }

           
        }
        aiReleaseImport(scene);
    }
    else {
        // Handle error
    }
}


void Mesh::LoadTexture(const std::string& file_path) {

    auto img = ilGenImage();
    ilBindImage(img);
    ilLoadImage((const wchar_t*)file_path.c_str());
    auto width = ilGetInteger(IL_IMAGE_WIDTH);

    auto height = ilGetInteger(IL_IMAGE_HEIGHT);

    auto channels = ilGetInteger(IL_IMAGE_CHANNELS);
    auto data = ilGetData();

    //load image as a texture in VRAM
	textureID = texCoords_buffer.Id();
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, channels, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    //now we can delete image from RAM
    ilDeleteImage(img);
}

void Mesh::LoadCheckerTexture() {
    GLubyte checkerImage[CHECKERS_HEIGHT][CHECKERS_WIDTH][4];
    for (int i = 0; i < CHECKERS_HEIGHT; i++) {
        for (int j = 0; j < CHECKERS_WIDTH; j++) {
            int c = ((((i & 0x8) == 0) ^ (((j & 0x8)) == 0))) * 255;
            checkerImage[i][j][0] = (GLubyte)c;
            checkerImage[i][j][1] = (GLubyte)c;
            checkerImage[i][j][2] = (GLubyte)c;
            checkerImage[i][j][3] = (GLubyte)255;
        }
    }
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glGenTextures(1, &textureID );
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, CHECKERS_WIDTH, CHECKERS_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, checkerImage);
}

