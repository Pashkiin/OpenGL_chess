#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "shaderprogram.h"
#include "lodepng.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

using namespace std;

GLuint readTextureASSIMP(const char* filename) {
	GLuint tex;
	glActiveTexture(GL_TEXTURE0);

	//Wczytanie do pami?ci komputera
	std::vector<unsigned char> image;   //Alokuj wektor do wczytania obrazka
	unsigned width, height;   //Zmienne do których wczytamy wymiary obrazka
	//Wczytaj obrazek
	unsigned error = lodepng::decode(image, width, height, filename);

	//Import do pami?ci karty graficznej
	glGenTextures(1, &tex); //Zainicjuj jeden uchwyt
	glBindTexture(GL_TEXTURE_2D, tex); //Uaktywnij uchwyt
	//Wczytaj obrazek do pami?ci KG skojarzonej z uchwytem
	glTexImage2D(GL_TEXTURE_2D, 0, 4, width, height, 0,
		GL_RGBA, GL_UNSIGNED_BYTE, (unsigned char*)image.data());

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	return tex;
}

struct BasicMesh {
	int NumVertices = 0;
	vector<glm::vec4> vertices = vector<glm::vec4>();
	vector<glm::vec4> normals = vector<glm::vec4>();
	vector<glm::vec2> texCoords = vector<glm::vec2>();
	vector<unsigned int> indices = vector<unsigned int>();
	GLuint texture;
};

class ModelHandler {
private:
	std::string color;
	std::string modelPath;
	std::string type;

public:

	float xPosition;
	float yPosition;
	float zPosition;
	
	vector<BasicMesh> Meshes = vector<BasicMesh>();
	ModelHandler(std::string type, std::string color, float xPosition, float yPosition, float zPosition) {
		this->color = color;
		this->type = type;
		this->xPosition = xPosition;
		this->yPosition = yPosition;
		this->zPosition = zPosition;

		if (type == "pawn") modelPath = "./chess_models/Pawn.obj";
		else if (type == "king") modelPath = "./chess_models/King.obj";
		else if (type == "queen") modelPath = "./chess_models/Queen.obj";
		else if (type == "rook") modelPath = "./chess_models/Rook.obj";
		else if (type == "bishop") modelPath = "./chess_models/Bishop.obj";
		else if (type == "knight") modelPath = "./chess_models/Knight.obj";
		else modelPath = "./chess_models/Piece.obj";
	}
	ModelHandler(std::string type, std::string color, float xPosition, float yPosition) {
		this->color = color;
		this->type = type;
		this->xPosition = xPosition;
		this->yPosition = yPosition;
		this->zPosition = 0.0;

		if (type == "pawn") modelPath = "./chess_models/Pawn.obj";
		else if (type == "king") modelPath = "./chess_models/King.obj";
		else if (type == "queen") modelPath = "./chess_models/Queen.obj";
		else if (type == "rook") modelPath = "./chess_models/Rook.obj";
		else if (type == "bishop") modelPath = "./chess_models/Bishop.obj";
		else if (type == "knight") modelPath = "./chess_models/Knight.obj";
	}

	void loadModel() {
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(
			modelPath,
			aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			std::cout << "Failed to load model: " << importer.GetErrorString() << std::endl;
			return;
		}
		if (scene->HasMeshes()) {
			for (int numOfMesh = 0; numOfMesh < scene->mNumMeshes; numOfMesh++) {
				aiMesh* mesh = scene->mMeshes[numOfMesh];
				BasicMesh meshStruct;
				meshStruct.NumVertices = mesh->mNumVertices;
				// Wczytanie wszystkich wierzcholkow, wektorow normalnych i wspolrzednych teksturowania
				for (int i = 0; i < mesh->mNumVertices; i++) {

					aiVector3D vertex = mesh->mVertices[i];
					meshStruct.vertices.push_back(glm::vec4(vertex.x, vertex.y, vertex.z, 1));
					//cout << vertex.x << vertex.y << endl;
					aiVector3D normal = mesh->mNormals[i];
					meshStruct.normals.push_back(glm::vec4(normal.x, normal.y, normal.z, 0));
					//cout << normal.x << normal.y << normal.z << endl;
					aiVector3D texCoord = mesh->mTextureCoords[0][i];
					meshStruct.texCoords.push_back(glm::vec2(texCoord.x, texCoord.y));
				}
				// Wczytanie faces
				for (int i = 0; i < mesh->mNumFaces; i++) {
					aiFace& face = mesh->mFaces[i];
					for (int j = 0; j < face.mNumIndices; j++) {
						meshStruct.indices.push_back(face.mIndices[j]);
					}
				}

				aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

				aiString str;
				material->GetTexture(aiTextureType_DIFFUSE, 1, &str);

				if (color == "black") meshStruct.texture = readTextureASSIMP("black.png");
				else if (color == "white") meshStruct.texture = readTextureASSIMP("white.png");
				Meshes.push_back(meshStruct);
			}
		}
	}

	void draw(glm::mat4 M, ShaderProgram *sp) {

		for (int i = 0; i < Meshes.size(); i++) {

			glEnableVertexAttribArray(sp->a("vertex"));
			glVertexAttribPointer(sp->a("vertex"), 4, GL_FLOAT, false, 0, Meshes[i].vertices.data());
			glEnableVertexAttribArray(sp->a("normal"));
			glVertexAttribPointer(sp->a("normal"), 4, GL_FLOAT, false, 0, Meshes[i].normals.data());
			glEnableVertexAttribArray(sp->a("texCoord"));
			glVertexAttribPointer(sp->a("texCoord"), 2, GL_FLOAT, false, 0, Meshes[i].texCoords.data());

			glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(M));

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, Meshes[i].texture);
			glUniform1i(sp->u("textureMap"), 0);

			glDrawElements(GL_TRIANGLES, Meshes[i].indices.size(), GL_UNSIGNED_INT, Meshes[i].indices.data());

		}
	}
};
