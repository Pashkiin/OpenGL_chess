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

class Piece {
	private:
		std::string type;
		std::string color;
		std::string texPath;
		
	public:
		Piece(std::string pieceType, std::string pieceColor) {
			type = pieceType;
			color = pieceColor;
			if (color == "white") texPath = "white.png";
			else if (color == "black") {
				cout << "Im black!"<<endl;
				texPath = "black.png";
			}
		}

		float xPosition;
		float yPosition;

		std::string texturePath() {
			return texPath;
		}
};