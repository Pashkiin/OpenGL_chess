/*
Niniejszy program jest wolnym oprogramowaniem; możesz go
rozprowadzać dalej i / lub modyfikować na warunkach Powszechnej
Licencji Publicznej GNU, wydanej przez Fundację Wolnego
Oprogramowania - według wersji 2 tej Licencji lub(według twojego
wyboru) którejś z późniejszych wersji.

Niniejszy program rozpowszechniany jest z nadzieją, iż będzie on
użyteczny - jednak BEZ JAKIEJKOLWIEK GWARANCJI, nawet domyślnej
gwarancji PRZYDATNOŚCI HANDLOWEJ albo PRZYDATNOŚCI DO OKREŚLONYCH
ZASTOSOWAŃ.W celu uzyskania bliższych informacji sięgnij do
Powszechnej Licencji Publicznej GNU.

Z pewnością wraz z niniejszym programem otrzymałeś też egzemplarz
Powszechnej Licencji Publicznej GNU(GNU General Public License);
jeśli nie - napisz do Free Software Foundation, Inc., 59 Temple
Place, Fifth Floor, Boston, MA  02110 - 1301  USA
*/

#define GLM_FORCE_RADIANS
#define GLM_FORCE_SWIZZLE

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stdlib.h>
#include <stdio.h>
#include <Windows.h>
#include "constants.h"
#include "lodepng.h"
#include "shaderprogram.h"
#include "myCube.h"
#include "myTeapot.h"
#include "customModel.h"
#include "Piece.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <iostream>
#include <fstream>
#include <vector>

GLFWwindow* window;

float speed_x = 0;
float speed_y = 0;
float aspectRatio = 1;
float zoom = 30.0;
float angle_x = 0;
float angle_y = 0;


ShaderProgram* sp;
GLuint woodTex;
vector<ModelHandler> chessboardPieces;

fstream game;
bool gameOver = false;
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void keyCallbackAlt(GLFWwindow* window, int key, int scancode, int action, int mods);
void drawScene(GLFWwindow* window, float angle_x, float angle_y);

GLuint readTexture(const char* filename) {
	GLuint tex;
	glActiveTexture(GL_TEXTURE0);
	std::vector<unsigned char> image;
	unsigned width, height;
	unsigned error = lodepng::decode(image, width, height, filename);
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, (unsigned char*)image.data());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	return tex;
}

float z_function(float x) {
	float res;
	res = -pow((x - 1), 2) + 1;
	return res;
}

void error_callback(int error, const char* description) {
	fputs(description, stderr);
}

void windowResizeCallback(GLFWwindow* window, int width, int height) {
	if (height == 0) return;
	aspectRatio = (float)width / (float)height;
	glViewport(0, 0, width, height);
}

void animate(int to_move,int x_start, int x_end, int y_start, int y_end) {
	glfwSetKeyCallback(window, keyCallbackAlt);
	float z_x = 0.0;
	float z_ch = 2.0 / 30.0;
	if (x_start == x_end) {
		float ch = abs(y_end - y_start) / 30.0;
		if (y_end > y_start) {
			for (float y = y_start; y < y_end; y += ch) {
				chessboardPieces[to_move].yPosition = y;
				chessboardPieces[to_move].zPosition = z_function(z_x);
				z_x += z_ch;
				angle_x += speed_x * glfwGetTime();
				angle_y += speed_y * glfwGetTime();
				glfwSetTime(0);
				drawScene(window, angle_x, angle_y);
				glfwPollEvents();
			}
		}
		else if (y_start > y_end)
		{
			for (float y = y_start; y > y_end; y -= ch) {
				chessboardPieces[to_move].yPosition = y;
				chessboardPieces[to_move].zPosition = z_function(z_x);
				z_x += z_ch;
				angle_x += speed_x * glfwGetTime();
				angle_y += speed_y * glfwGetTime();
				glfwSetTime(0);
				drawScene(window, angle_x, angle_y);
				glfwPollEvents();
			}
		}
		chessboardPieces[to_move].xPosition = x_end;
		chessboardPieces[to_move].yPosition = y_end;
		chessboardPieces[to_move].zPosition = 0.0;
	}
	else if (y_start == y_end) {
		float ch = abs(x_end - x_start) / 30.0;
		if (x_end > x_start) {
			for (float x = x_start; x < x_end; x += ch) {
				chessboardPieces[to_move].xPosition = x;
				chessboardPieces[to_move].zPosition = z_function(z_x);
				z_x += z_ch;
				angle_x += speed_x * glfwGetTime();
				angle_y += speed_y * glfwGetTime();
				glfwSetTime(0);
				drawScene(window, angle_x, angle_y);
				glfwPollEvents();
			}
		}
		else if (x_start > x_end)
		{
			for (float x = x_start; x > x_end; x -= ch) {
				chessboardPieces[to_move].xPosition = x;
				chessboardPieces[to_move].zPosition = z_function(z_x);
				z_x += z_ch;
				angle_x += speed_x * glfwGetTime();
				angle_y += speed_y * glfwGetTime();
				glfwSetTime(0);
				drawScene(window, angle_x, angle_y);
				glfwPollEvents();
			}
		}
		chessboardPieces[to_move].xPosition = x_end;
		chessboardPieces[to_move].yPosition = y_end;
		chessboardPieces[to_move].zPosition = 0.0;
	}
	else {
		float ch_x = abs(x_end - x_start) / 30.0;
		float ch_y = abs(y_end - y_start) / 30.0;
		float x = x_start, y = y_start;
		if ((x_end > x_start) && (y_end > y_start)) {
			while ((x_end > x) && (y_end > y)) {
				x += ch_x;
				y += ch_y;
				chessboardPieces[to_move].xPosition = x;
				chessboardPieces[to_move].yPosition = y;
				chessboardPieces[to_move].zPosition = z_function(z_x);
				z_x += z_ch;
				angle_x += speed_x * glfwGetTime();
				angle_y += speed_y * glfwGetTime();
				glfwSetTime(0);
				drawScene(window, angle_x, angle_y);
				glfwPollEvents();
			}
		}
		else if ((x_end < x_start) && (y_end > y_start)) {
			while ((x_end < x) && (y_end > y)) {
				x -= ch_x;
				y += ch_y;
				chessboardPieces[to_move].xPosition = x;
				chessboardPieces[to_move].yPosition = y;
				chessboardPieces[to_move].zPosition = z_function(z_x);
				z_x += z_ch;
				angle_x += speed_x * glfwGetTime();
				angle_y += speed_y * glfwGetTime();
				glfwSetTime(0);
				drawScene(window, angle_x, angle_y);
				glfwPollEvents();
			}
		}
		else if ((x_end < x_start) && (y_end < y_start)) {
			while ((x_end < x) && (y_end < y)) {
				x -= ch_x;
				y -= ch_y;
				chessboardPieces[to_move].xPosition = x;
				chessboardPieces[to_move].yPosition = y;
				chessboardPieces[to_move].zPosition = z_function(z_x);
				z_x += z_ch;
				angle_x += speed_x * glfwGetTime();
				angle_y += speed_y * glfwGetTime();
				glfwSetTime(0);
				drawScene(window, angle_x, angle_y);
				glfwPollEvents();
			}
		}
		else if ((x_end > x_start) && (y_end < y_start)) {
			while ((x_end > x) && (y_end < y)) {
				x += ch_x;
				y -= ch_y;
				chessboardPieces[to_move].xPosition = x;
				chessboardPieces[to_move].yPosition = y;
				chessboardPieces[to_move].zPosition = z_function(z_x);
				z_x += z_ch;
				angle_x += speed_x * glfwGetTime();
				angle_y += speed_y * glfwGetTime();
				glfwSetTime(0);
				drawScene(window, angle_x, angle_y);
				glfwPollEvents();
			}
		}
		chessboardPieces[to_move].xPosition = x_end;
		chessboardPieces[to_move].yPosition = y_end;
		chessboardPieces[to_move].zPosition = 0.0;
	}
	glfwSetKeyCallback(window, keyCallback);
}

void move() {
	if (gameOver) return;

	int x_start, y_start, x_end, y_end, toRemove = -1, to_move = -1;

	game >> x_start >> y_start >> x_end >> y_end;

	for (int i = 0; i != chessboardPieces.size(); i++) {
		if (chessboardPieces[i].xPosition == x_start && chessboardPieces[i].yPosition == y_start) {
			to_move = i;
		}
		else if (chessboardPieces[i].xPosition == x_end && chessboardPieces[i].yPosition == y_end) {
			toRemove = i;
		}
	}

	if (to_move != -1) {
		animate(to_move, x_start, x_end, y_start, y_end);
	}

	if (toRemove != -1) {
		chessboardPieces.erase(chessboardPieces.begin() + toRemove);
	}

	//chessboardPieces.push_back(CustomModel("rook", "white", 1, 1));

	if (game.eof()) {
		std::cout << "GAME OVER!" << endl;
		gameOver = true;
	}
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {

	if (action == GLFW_PRESS) {
		if (key == GLFW_KEY_A) speed_x = -PI;
		if (key == GLFW_KEY_D) speed_x = PI;
		if (key == GLFW_KEY_S) speed_y = PI;
		if (key == GLFW_KEY_W) speed_y = -PI;

		if (key == GLFW_KEY_SPACE) {
			move();
		}
	}
	if (action == GLFW_RELEASE) {
		if (key == GLFW_KEY_A) speed_x = 0;
		if (key == GLFW_KEY_D) speed_x = 0;
		if (key == GLFW_KEY_S) speed_y = 0;
		if (key == GLFW_KEY_W) speed_y = 0;

	}
}

void keyCallbackAlt(GLFWwindow* window, int key, int scancode, int action, int mods) {

	if (action == GLFW_PRESS) {
		if (key == GLFW_KEY_A) speed_x = -PI;
		if (key == GLFW_KEY_D) speed_x = PI;
		if (key == GLFW_KEY_S) speed_y = PI;
		if (key == GLFW_KEY_W) speed_y = -PI;

	}
	if (action == GLFW_RELEASE) {
		if (key == GLFW_KEY_A) speed_x = 0;
		if (key == GLFW_KEY_D) speed_x = 0;
		if (key == GLFW_KEY_S) speed_y = 0;
		if (key == GLFW_KEY_W) speed_y = 0;

	}
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {

	if (yoffset < 0) {
		zoom *= 1.1f;  // Zoom in
	}
	else {
		zoom /= 1.1f;  // Zoom out
	}

	if (zoom > 30.0) zoom = 30.0;
	else if (zoom < 1.0) zoom = 1.0;
}

void initChessPieces() {

	//generate starting white pieces
	chessboardPieces.push_back(ModelHandler("rook", "white", 1, 1));
	chessboardPieces.push_back(ModelHandler("knight", "white", 2, 1));
	chessboardPieces.push_back(ModelHandler("bishop", "white", 3, 1));
	chessboardPieces.push_back(ModelHandler("queen", "white", 4, 1));
	chessboardPieces.push_back(ModelHandler("king", "white", 5, 1));
	chessboardPieces.push_back(ModelHandler("bishop", "white", 6, 1));
	chessboardPieces.push_back(ModelHandler("knight", "white", 7, 1));
	chessboardPieces.push_back(ModelHandler("rook", "white", 8, 1));

	for (int i = 1; i < 9; i++)
		chessboardPieces.push_back(ModelHandler("pawn", "white", i, 2));

	chessboardPieces.push_back(ModelHandler("rook", "black", 1, 8));
	chessboardPieces.push_back(ModelHandler("knight", "black", 2, 8));
	chessboardPieces.push_back(ModelHandler("bishop", "black", 3, 8));
	chessboardPieces.push_back(ModelHandler("queen", "black", 4, 8));
	chessboardPieces.push_back(ModelHandler("king", "black", 5, 8));
	chessboardPieces.push_back(ModelHandler("bishop", "black", 6, 8));
	chessboardPieces.push_back(ModelHandler("knight", "black", 7, 8));
	chessboardPieces.push_back(ModelHandler("rook", "black", 8, 8));

	for (int i = 1; i < 9; i++)
		chessboardPieces.push_back(ModelHandler("pawn", "black", i, 7));

	//load piece models and textures matricies
	for (int i = 0; i < chessboardPieces.size(); i++) {
		chessboardPieces[i].loadModel();
	}
}

void initOpenGLProgram(GLFWwindow* window) {
	initShaders();
	glClearColor(0.8, 0.8, 0.8, 1);
	glEnable(GL_DEPTH_TEST);
	glfwSetWindowSizeCallback(window, windowResizeCallback);
	glfwSetKeyCallback(window, keyCallback);
	glfwSetScrollCallback(window, scrollCallback);

	sp = new ShaderProgram("v_simplest.glsl", NULL, "f_simplest.glsl");

	woodTex = readTexture("wood.png");

	initChessPieces();
}

void freeOpenGLProgram(GLFWwindow* window) {
	delete sp;
}

void drawBoard(float angle_x, float angle_y) {

	glm::mat4 Mbase = glm::mat4(1.0f), M;
	Mbase = glm::rotate(Mbase, angle_y, glm::vec3(1.0f, 0.0f, 0.0f));
	Mbase = glm::rotate(Mbase, PI + angle_x, glm::vec3(0.0f, 1.0f, 0.0f));
	Mbase = glm::scale(Mbase, glm::vec3(1.0f, 0.1f, 1.0f));


	// send parameters for frawing cubes
	glUniform1f(sp->u("mixParam"), 0.3);

	glEnableVertexAttribArray(sp->a("vertex"));
	glVertexAttribPointer(sp->a("vertex"), 4, GL_FLOAT, false, 0, myCubeVertices);

	glEnableVertexAttribArray(sp->a("normal"));
	glVertexAttribPointer(sp->a("normal"), 4, GL_FLOAT, false, 0, myCubeNormals);

	glEnableVertexAttribArray(sp->a("texCoord"));
	glVertexAttribPointer(sp->a("texCoord"), 2, GL_FLOAT, false, 0, myCubeTexCoords);

	glUniform1i(sp->u("textureMap"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, woodTex);


	//draw black squares
	glEnableVertexAttribArray(sp->a("color"));
	glVertexAttribPointer(sp->a("color"), 4, GL_FLOAT, false, 0, myCubeBlack);

	float offset = 2.0;
	for (float x = -7.0; x <= 8.0; x += 2.0) {
		for (int y = -7.0 + offset; y <= 8.0; y += 4.0) {
			M = glm::translate(Mbase, glm::vec3(x, 0.0f, y));
			glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(M));
			glDrawArrays(GL_TRIANGLES, 0, myCubeVertexCount);
		}

		if (offset == 0.0)	offset = 2.0;
		else offset = 0.0;
	}


	//draw white squares
	glEnableVertexAttribArray(sp->a("color"));
	glVertexAttribPointer(sp->a("color"), 4, GL_FLOAT, false, 0, myCubeWhite);

	offset = 0.0;
	for (float x = -7.0; x <= 8.0; x += 2.0) {
		for (int y = -7.0 + offset; y <= 8.0; y += 4.0) {
			M = glm::translate(Mbase, glm::vec3(x, 0.0f, y));
			glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(M));
			glDrawArrays(GL_TRIANGLES, 0, myCubeVertexCount);
		}

		if (offset == 0.0)	offset = 2.0;
		else offset = 0.0;
	}


	// disable colour - no longer needed to draw pieces
	glDisableVertexAttribArray(sp->a("color"));

	// set mix parameter to 1 -> use only texture
	glUniform1f(sp->u("mixParam"), 1.0f);

	// draw all figures in correct places
	for (int i = 0; i < chessboardPieces.size(); i++) {
		M = glm::scale(Mbase, glm::vec3(0.1f, 1.0f, 0.1f));
		M = glm::translate(M, glm::vec3(-70.0f, 0.0f, 70.0f));
		float new_pos_y = (chessboardPieces[i].yPosition - 1.0);
		float new_pos_x = (chessboardPieces[i].yPosition - 1.0);
		float new_pos_z = chessboardPieces[i].zPosition * 20;
		M = glm::translate(M, glm::vec3((20.0f * (chessboardPieces[i].xPosition - 1.0)), 0.00f+new_pos_z, (-20.0f * new_pos_y)));

		chessboardPieces[i].draw(M, sp);
	}

	//disable rest of parameters
	glDisableVertexAttribArray(sp->a("vertex"));
	glDisableVertexAttribArray(sp->a("normal"));
	glDisableVertexAttribArray(sp->a("texCoord"));

}

void drawScene(GLFWwindow* window, float angle_x, float angle_y) {

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 P = glm::perspective(50.0f * PI / 180.0f, aspectRatio, 0.01f, 50.0f);
	glm::mat4 V = glm::lookAt(glm::vec3(0, 0, -1 * zoom), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	sp->use();
	glUniformMatrix4fv(sp->u("P"), 1, false, glm::value_ptr(P));
	glUniformMatrix4fv(sp->u("V"), 1, false, glm::value_ptr(V));

	drawBoard(angle_x, angle_y);

	glfwSwapBuffers(window);
}


int main(void)
{
	

	glfwSetErrorCallback(error_callback);

	if (!glfwInit()) {
		fprintf(stderr, "Nie można zainicjować GLFW.\n");
		exit(EXIT_FAILURE);
	}

	window = glfwCreateWindow(1000, 1000, "ChessMediaPlayer", NULL, NULL);

	if (!window)
	{
		fprintf(stderr, "Nie można utworzyć okna.\n");
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Nie można zainicjować GLEW.\n");
		exit(EXIT_FAILURE);
	}

	initOpenGLProgram(window);
	game.open("game_course.txt", ios::in);

	std::cout << "GAME START!" << std::endl;

	
	glfwSetTime(0);
	while (!glfwWindowShouldClose(window))
	{
		angle_x += speed_x * glfwGetTime();
		angle_y += speed_y * glfwGetTime();
		glfwSetTime(0);
		drawScene(window, angle_x, angle_y);
		glfwPollEvents();
	}

	freeOpenGLProgram(window);

	game.close();

	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}
