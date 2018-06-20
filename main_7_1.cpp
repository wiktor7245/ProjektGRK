#include "glew.h"
#include "freeglut.h"
#include "glm.hpp"
#include "ext.hpp"
#include <iostream>
#include <cmath>
#include <vector>
#include <time.h>
#include <stdlib.h>
#include "Shader_Loader.h"
#include "Render_Utils.h"
#include "Camera.h"
#include "Texture.h"

GLuint programColor;
GLuint programTexture;
GLuint programTexture2;

Core::Shader_Loader shaderLoader;

obj::Model shipModel;
obj::Model sphereModel;
obj::Model grassModel;
obj::Model groundModel;
obj::Model rockModel;

glm::vec3 cameraPos = glm::vec3(0, 0, 5);
glm::vec3 cameraDir; // Wektor "do przodu" kamery
glm::vec3 cameraSide; // Wektor "w bok" kamery
float cameraAngle = 0;

glm::mat4 cameraMatrix, perspectiveMatrix;

glm::vec3 lightDir = glm::normalize(glm::vec3(1.0f, -0.9f, -1.0f));

glm::quat rotation = glm::quat(1, 0, 0, 0);

GLuint textureAsteroid;
GLuint textureFish;
GLuint textureGround;
GLuint textureGrass;
GLuint textureRock;
const int asteroidsTransSize = 20;
glm::vec3 asteroidsTrans [asteroidsTransSize];

glm::vec2 mouseOldCords;
glm::vec2 mouseDiff;

float mouseSensitivity=0.01f;


//----------
bool tryb=true;
glm::vec3 cameraPos2;
float i=0.0;
glm::quat rotation2;
float ac = 0.0;


//----------

void keyboard(unsigned char key, int x, int y)
{
	
	float angleSpeed = 10.0f;
	float moveSpeed =0.1f;
	glm::quat quatZ;
	switch(key)
	{
	case 'z': tryb = true;
		quatZ = glm::angleAxis(angleSpeed * mouseSensitivity, glm::vec3(0.0f, 0.0f, -1.0f));
		rotation = glm::normalize(quatZ * rotation);
		cameraDir = glm::inverse(rotation) * glm::vec3(0.0f, 0.0f, -1.0f);
		cameraSide = glm::inverse(rotation) * glm::vec3(1.0f, 0.0f, 0.0f);
		break;
	case 'x': tryb = true;
		quatZ = glm::angleAxis(angleSpeed * mouseSensitivity, glm::vec3(0.0f, 0.0f, 1.0f));
		rotation = glm::normalize(quatZ * rotation);
		cameraDir = glm::inverse(rotation) * glm::vec3(0.0f, 0.0f, -1.0f);
		cameraSide = glm::inverse(rotation) * glm::vec3(1.0f, 0.0f, 0.0f);
		break;
	case 'w':tryb = true; cameraPos += cameraDir * moveSpeed; break;
	case 's':tryb = true; cameraPos -= cameraDir * moveSpeed; break;
	case 'd':tryb = true; cameraPos += cameraSide * moveSpeed; break;
	case 'a':tryb = true; cameraPos -= cameraSide * moveSpeed; break;
	}
}

void mouse(int x, int y)
{
	mouseDiff.x += mouseOldCords.x - x;
	mouseDiff.y += mouseOldCords.y - y;
	mouseOldCords.x = x;
	mouseOldCords.y = y;
}

glm::mat4 createCameraMatrix()
{
	if (tryb) {
		/* klawisze */
		cameraDir = glm::vec3(cosf(cameraAngle - glm::radians(90.0f)), 0.0f, sinf(cameraAngle - glm::radians(90.0f)));
		glm::vec3 up = glm::vec3(0, 1, 0);
		cameraSide = glm::cross(cameraDir, up);
		/* --- */

		//return Core::createViewMatrix(cameraPos, cameraDir, up);
		glm::quat quatX = glm::angleAxis(mouseDiff.y * mouseSensitivity, glm::vec3(-1.0f, 0.0f, 0.0f));
		glm::quat quatY = glm::angleAxis(mouseDiff.x * mouseSensitivity, glm::vec3(0.0f, -1.0f, 0.0f));
		glm::quat rotationChange = quatY*quatX;
		mouseDiff.x = 0.0f;
		mouseDiff.y = 0.0f;
		rotation = glm::normalize(rotationChange * rotation);
		cameraDir = glm::inverse(rotation) * glm::vec3(0.0f, 0.0f, -1.0f);
		cameraSide = glm::inverse(rotation) * glm::vec3(1.0f, 0.0f, 0.0f);
		return Core::createViewMatrixQuat(cameraPos, rotation);
	}
	else
	{
		glm::quat quatY = glm::angleAxis(ac * -0.09f, glm::vec3(0.0f, -1.0f, 0.0f));
		glm::quat rotationChange2 = quatY * 1.0f;
		ac = 0.0f;
		glm::vec3 up = glm::vec3(0, 0, 0);
		rotation2 = glm::normalize(rotationChange2 * rotation2);
		//return Core::createViewMatrix(cameraPos2, cameraDir, up);
		return Core::createViewMatrixQuat(cameraPos2, rotation2);
	}
}

void drawObjectColor(obj::Model * model, glm::mat4 modelMatrix, glm::vec3 color)
{
	GLuint program = programColor;

	glUseProgram(program);

	glUniform3f(glGetUniformLocation(program, "objectColor"), color.x, color.y, color.z);
	glUniform3f(glGetUniformLocation(program, "lightDir"), lightDir.x, lightDir.y, lightDir.z);

	glm::mat4 transformation = perspectiveMatrix * cameraMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(program, "modelViewProjectionMatrix"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(program, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);

	Core::DrawModel(model);

	glUseProgram(0);
}

void drawObjectTexture(obj::Model * model, glm::mat4 modelMatrix, GLuint textureId)
{
	GLuint program = programTexture;

	glUseProgram(program);

	glUniform3f(glGetUniformLocation(program, "lightDir"), lightDir.x, lightDir.y, lightDir.z);
	Core::SetActiveTexture(textureId, "textureSampler", program, 0);

	glm::mat4 transformation = perspectiveMatrix * cameraMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(program, "modelViewProjectionMatrix"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(program, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);

	Core::DrawModel(model);

	glUseProgram(0);
}


void drawObjectTexture2(obj::Model *model, glm::mat4 modelMatrix, GLuint textureId) {
	GLuint program = programTexture2;

	glUseProgram(program);
	glUniform3f(glGetUniformLocation(program, "lightDir"), lightDir.x, lightDir.y,lightDir.z);
	Core::SetActiveTexture(textureId, "textureSampler", program, 0);

	glm::mat4 transformation = perspectiveMatrix * cameraMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(program, "modelViewProjectionMatrix"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(program,"modelMatrix"),1,GL_FALSE,(float*)&modelMatrix);

	Core::DrawModel(model);
	glUseProgram(0);
}

void renderScene()
{
	float time2 = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;

	srand(time(NULL));

	// Aktualizacja macierzy widoku i rzutowania
	cameraMatrix = createCameraMatrix();
	perspectiveMatrix = Core::createPerspectiveMatrix();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0f, 0.2f, 1.2f, 1.0f);


	//ship
	glm::mat4 shipInitialTransformation = glm::translate(glm::vec3(0,-0.25f,0)) * glm::rotate(glm::radians(180.0f), glm::vec3(0,1,0)) * glm::scale(glm::vec3(0.25f));
	glm::mat4 shipModelMatrix = glm::translate(cameraPos + cameraDir * 0.5f) * glm::mat4_cast(glm::inverse(rotation)) * shipInitialTransformation;


	//polozenie gleby
	//glm::mat4 groundModelMatrix = glm::translate(glm::mat4(), glm::vec3(0, -2.25f, 0)) * /*glm::mat4_cast(glm::inverse(rotation))**/ glm::scale(glm::vec3(20, 0.05, 20));


	drawObjectTexture(&shipModel, shipModelMatrix, textureFish);
	//drawObjectTexture(&groundModel, groundModelMatrix, textureGround);
	drawObjectTexture(&groundModel, glm::translate(glm::vec3(0, -10, 0)) * glm::scale(glm::vec3(20,0.05,20)), textureGround);

	
	//grass1
	for (int i = 0; i <= 100; i++)
	{
		drawObjectTexture(&grassModel, glm::translate(glm::vec3(rand() % 100 + 1, -10, rand() % 100 + 1))* glm::scale(glm::vec3(3, 3, 3)), textureGrass); //ew scale 1,10,1 i nie ma cieni
		drawObjectTexture(&grassModel, glm::translate(glm::vec3(-(rand() % 100 + 1), -10, -(rand() % 100 + 1)))* glm::scale(glm::vec3(3, 3, 3)), textureGrass); //minus
		drawObjectTexture(&grassModel, glm::translate(glm::vec3((rand() % 100 + 1), -10, -(rand() % 100 + 1)))* glm::scale(glm::vec3(3, 3, 3)), textureGrass); //minus
		drawObjectTexture(&grassModel, glm::translate(glm::vec3(-(rand() % 100 + 1), -10, (rand() % 100 + 1)))* glm::scale(glm::vec3(3, 3, 3)), textureGrass); //minus
	}

	//rock
	for (int i = 0; i <= 100; i++)
	{
		drawObjectTexture(&rockModel, glm::translate(glm::vec3(rand() % 100 + 1, -10, rand() % 100 + 1))* glm::scale(glm::vec3(3, 3, 3)), textureRock); //ew scale 1,10,1 i nie ma cieni
		drawObjectTexture(&rockModel, glm::translate(glm::vec3(-(rand() % 100 + 1), -10, -(rand() % 100 + 1)))* glm::scale(glm::vec3(3, 3, 3)), textureRock); //minus
		drawObjectTexture(&rockModel, glm::translate(glm::vec3(-(rand() % 100 + 1), -10, (rand() % 100 + 1)))* glm::scale(glm::vec3(3, 3, 3)), textureRock); //minus
		drawObjectTexture(&rockModel, glm::translate(glm::vec3((rand() % 100 + 1), -10, -(rand() % 100 + 1)))* glm::scale(glm::vec3(3, 3, 3)), textureRock); //minus
	}

	//fish in the water
	for (int i = 0; i < asteroidsTransSize; i++) {

		//asteroidsTrans[i] = glm::vec3(0.0, time, 0.0);
		asteroidsTrans[i][2] += time2/100000;
		drawObjectTexture2(&sphereModel, glm::translate(asteroidsTrans[i]), textureAsteroid);
	}

	glutSwapBuffers();
}

void init()
{
	srand(time(0));
	glEnable(GL_DEPTH_TEST);
	programColor = shaderLoader.CreateProgram("shaders/shader_color.vert", "shaders/shader_color.frag");
	programTexture = shaderLoader.CreateProgram("shaders/shader_tex.vert", "shaders/shader_tex.frag");
	programTexture2 = shaderLoader.CreateProgram("shaders/shader_tex.vert", "shaders/shader_tex2.frag");
	
	//objects
	sphereModel = obj::loadModelFromFile("models/SeaHorse.obj");

	//model of fish
	shipModel = obj::loadModelFromFile("models/spaceship.obj");

	//model of ground
	groundModel = obj::loadModelFromFile("models/sphere.obj");

	//model of rock
	grassModel = obj::loadModelFromFile("models/grass.obj");

	//model of rock
	rockModel = obj::loadModelFromFile("models/rock.obj");

	//textures
	textureAsteroid = Core::LoadTexture("textures/fish2.png");
	textureFish = Core::LoadTexture("textures/fish.png");
	textureGround = Core::LoadTexture("textures/dirt.png");
	textureGrass = Core::LoadTexture("textures/grass.png");
	textureRock = Core::LoadTexture("textures/fish.png");
	for (int i = 0; i < asteroidsTransSize; i++) {
		asteroidsTrans[i] = glm::ballRand(30.0f);
	}
}

void shutdown()
{
	shaderLoader.DeleteProgram(programColor);
	shaderLoader.DeleteProgram(programTexture);
}

void idle()
{
	glutPostRedisplay();
}

int main(int argc, char ** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(200, 200);
	glutInitWindowSize(600, 600);
	glutCreateWindow("OpenGL Pierwszy Program");
	glewInit();

	init();
	glutKeyboardFunc(keyboard);
	glutPassiveMotionFunc(mouse);
	glutDisplayFunc(renderScene);
	glutIdleFunc(idle);

	glutMainLoop();

	shutdown();

	return 0;
}