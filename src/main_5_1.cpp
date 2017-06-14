#include "glew.h"
#include "freeglut.h"
#include "glm.hpp"
#include "ext.hpp"
#include <iostream>
#include <cmath>

#include "Shader_Loader.h"
#include "Render_Utils.h"
#include "Camera.h"
#include "Texture.h"

GLuint programColor;
GLuint programTexture;
GLuint earthTexture;
GLuint marsTexture;
GLuint spaceTexture;
GLuint sunTexture;


Core::Shader_Loader shaderLoader;

obj::Model shipModel;
obj::Model sphereModel;

float cameraAngle = 0;
glm::vec3 cameraPos = glm::vec3(-5, 0, 0);
glm::vec3 cameraDir;

glm::mat4 cameraMatrix, perspectiveMatrix;

glm::vec3 lightDir = glm::normalize(glm::vec3(1.0f, -0.9f, -1.0f));

void keyboard(unsigned char key, int x, int y)
{
	float angleSpeed = 0.1f;
	float moveSpeed = 0.1f;
	switch(key)
	{
	case 'z': cameraAngle -= angleSpeed; break;
	case 'x': cameraAngle += angleSpeed; break;
	case 'w': cameraPos += cameraDir * moveSpeed; break;
	case 's': cameraPos -= cameraDir * moveSpeed; break;
	case 'd': cameraPos += glm::cross(cameraDir, glm::vec3(0,1,0)) * moveSpeed; break;
	case 'a': cameraPos -= glm::cross(cameraDir, glm::vec3(0,1,0)) * moveSpeed; break;
	case 'q': cameraPos.y += moveSpeed; break;
	case 'e': cameraPos.y -= moveSpeed; break;
	}
}

glm::mat4 createCameraMatrix()
{
	// Obliczanie kierunku patrzenia kamery (w plaszczyznie x-z) przy uzyciu zmiennej cameraAngle kontrolowanej przez klawisze.
	cameraDir = glm::vec3(cosf(cameraAngle), 0.0f, sinf(cameraAngle));
	glm::vec3 up = glm::vec3(0,1,0);

	return Core::createViewMatrix(cameraPos, cameraDir, up);
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

void drawObjectTexture(obj::Model * model, glm::mat4 modelMatrix, GLuint textureIdent)
{
	GLuint program = programTexture;
	//GLuint textureIdent = newTexture;

	glUseProgram(program);

	Core::SetActiveTexture(textureIdent, "activeTexture", program, 0);

	
	glUniform3f(glGetUniformLocation(program, "lightDir"), lightDir.x, lightDir.y, lightDir.z);

	glm::mat4 transformation = perspectiveMatrix * cameraMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(program, "modelViewProjectionMatrix"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(program, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);

	Core::DrawModel(model);

	glUseProgram(0);
}


void renderScene()
{
	// Aktualizacja macierzy widoku i rzutowania. Macierze sa przechowywane w zmiennych globalnych, bo uzywa ich funkcja drawObject.
	// (Bardziej elegancko byloby przekazac je jako argumenty do funkcji, ale robimy tak dla uproszczenia kodu.
	//  Jest to mozliwe dzieki temu, ze macierze widoku i rzutowania sa takie same dla wszystkich obiektow!)
	float time = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
	cameraMatrix = createCameraMatrix();
	perspectiveMatrix = Core::createPerspectiveMatrix();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0f, 0.3f, 0.3f, 1.0f);

	// Macierz statku "przyczepia" go do kamery. Warto przeanalizowac te linijke i zrozumiec jak to dziala.
	glm::mat4 shipModelMatrix = glm::translate(cameraPos + cameraDir * 0.5f + glm::vec3(0,-0.25f,0)) * glm::rotate(-cameraAngle + glm::radians(90.0f), glm::vec3(0,1,0)) * glm::scale(glm::vec3(0.1f));
	drawObjectColor(&shipModel, shipModelMatrix, glm::vec3(0.6f));

	glm::mat4 earthMatrix = glm::translate(glm::vec3(0, 0, 0)) * glm::rotate(glm::radians(24.0f * time), glm::vec3(0, 12, 0));
	glm::mat4 sunMatrix = glm::translate(glm::vec3(0, 0, 0)) * glm::rotate(glm::radians(2.0f * time), glm::vec3(0, 12, 0));
	glm::mat4 marsMatrix = glm::translate(glm::vec3(0, 0, 0)) * glm::rotate(glm::radians(23.0f * time), glm::vec3(0, 12, 0));
	glm::mat4 spaceMatrix = glm::translate(glm::vec3(0, 0, 0)) * glm::scale(glm::vec3(50.0f));


	earthMatrix = earthMatrix * glm::translate(glm::vec3(10, 0, 0)) * glm::rotate(glm::radians(45.0f * time), glm::vec3(0, 12, -1)) * glm::scale(glm::vec3(1.0f)); //rotacja wokol slonca
	marsMatrix = marsMatrix * glm::translate(glm::vec3(15, 0, 0)) * glm::rotate(glm::radians(22.0f * time), glm::vec3(0, 12, -1)) * glm::scale(glm::vec3(1.0f));  // rotacja wokol slonca
	
	drawObjectTexture(&sphereModel, earthMatrix, earthTexture); //ziemia
	drawObjectTexture(&sphereModel, spaceMatrix, spaceTexture);  //'t³o'
	drawObjectTexture(&sphereModel, sunMatrix * glm::scale(glm::vec3(3.0f)), sunTexture); //slonce
	drawObjectTexture(&sphereModel, marsMatrix * glm::scale(glm::vec3(0.5f)), marsTexture); //mars

	glutSwapBuffers();
}

void init()
{
	glEnable(GL_DEPTH_TEST);
	programColor = shaderLoader.CreateProgram("shaders/shader_color.vert", "shaders/shader_color.frag");
	programTexture = shaderLoader.CreateProgram("shaders/shader_tex.vert", "shaders/shader_tex.frag");
	sphereModel = obj::loadModelFromFile("models/sphere.obj");
	shipModel = obj::loadModelFromFile("models/spaceship.obj");
	earthTexture = Core::LoadTexture("textures/earth2.png");
	marsTexture = Core::LoadTexture("textures/mars.png");
	spaceTexture = Core::LoadTexture("textures/kosmos.png");
	sunTexture = Core::LoadTexture("textures/sun.png");
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
	glutCreateWindow("Projekt GRK Jakub Niepsuj");
	glewInit();

	init();
	glutKeyboardFunc(keyboard);
	glutDisplayFunc(renderScene);
	glutIdleFunc(idle);

	glutMainLoop();

	shutdown();

	return 0;
}
