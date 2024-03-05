#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <iostream>
#include <cstdio>
#include <string>

#define STB_PERLIN_IMPLEMENTATION
#include "stb_noise.h"
#include "stb_easy_font.h"
#pragma comment(lib, "glew32.lib")

#include "Shader.h"
#include "Texture.h"
#include "Camera.h"
#include "ObjModel.h"
#include "FBO.h"


class Vertex
{
public:
	glm::vec3 position;
	glm::vec3 color;
	glm::vec2 texcoord;
	glm::vec3 normal;

	Vertex(const glm::vec3& position, const glm::vec3& color, const glm::vec2& texcoord, const glm::vec3& normal) :
		position(position), color(color), texcoord(texcoord), normal(normal)
	{
	}
};


FBO* fbo;

std::vector<Vertex> roomVertices;
std::vector<Shader*> shaders;
std::vector<Shader*> postProcessingShaders;
std::vector<ObjModel*> models;
int selectedModel = 0;
int postProcessingShader = 0;
Texture* gridTexture;

Shader* mainShader;

glm::ivec2 screenSize;
float rotation;
bool rotating = false;
int lastTime;
bool keys[255];
bool holdMouse = false;


std::vector<Vertex> buildCube(const glm::vec3& p, const glm::vec3& s)
{
	glm::vec4 color[8];
	for (int i = 0; i < 8; i++)
	{
		color[i].r = rand() / (float)RAND_MAX;
		color[i].g = rand() / (float)RAND_MAX;
		color[i].b = rand() / (float)RAND_MAX;
		color[i].a = 1;
	}


	std::vector<Vertex> verts;
	//bottom
	verts.push_back(Vertex(p + glm::vec3(-s.x, -s.y, -s.z), color[0], glm::vec2(0, 0), glm::vec3(0, -1, 0)));
	verts.push_back(Vertex(p + glm::vec3(s.x, -s.y, -s.z), color[4], glm::vec2(1, 0), glm::vec3(0, -1, 0)));
	verts.push_back(Vertex(p + glm::vec3(s.x, -s.y, s.z), color[5], glm::vec2(1, 1), glm::vec3(0, -1, 0)));
	verts.push_back(Vertex(p + glm::vec3(-s.x, -s.y, s.z), color[1], glm::vec2(0, 1), glm::vec3(0, -1, 0)));

	return verts;
}


#ifdef WIN32
void GLAPIENTRY onDebug(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message,
                        const void* userParam)
#else
void onDebug(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, GLvoid* userParam)
#endif
{
	std::cout << message << std::endl;
}

void init()
{
	glewInit();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glClearColor(35 / 255.0f, 145 / 255.0f, 247 / 255.0f, 1.0f);


	mainShader = new Shader("room");
	shaders.push_back(new Shader("multitex"));


	postProcessingShaders.push_back(new Shader("pp/simple"));


	//	models.push_back(new ObjModel("gamecube/model_n.obj"));
	models.push_back(new ObjModel("statue/statue.obj"));
	models.push_back(new ObjModel("cube/cube.obj"));

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);

	if (glDebugMessageCallback)
	{
		glDebugMessageCallback(&onDebug, NULL);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
		glEnable(GL_DEBUG_OUTPUT);
	}

	rotation = 0;
	lastTime = glutGet(GLUT_ELAPSED_TIME);

	roomVertices = buildCube(glm::vec3(0, 10, 0), glm::vec3(10, 10, 10));
	gridTexture = new Texture("grid.png");

	fbo = new FBO(2048, 2048, true, FBO::Type::Color, FBO::Type::Color);
}

void print_string(std::string text, const glm::mat4& mvp)
{
	static char buffer[99999]; // ~500 chars
	int num_quads;

	int width = stb_easy_font_width((char*)text.c_str());

	num_quads = stb_easy_font_print(-width / 2, 0, (char*)text.c_str(), NULL, buffer, sizeof(buffer));

	mainShader->use();
	mainShader->setUniform("modelViewProjectionMatrix", mvp);
	mainShader->setUniform("tex", 0);

	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(3);
	glVertexAttribPointer(0, 2, GL_FLOAT, false, 16, buffer);
	glDrawArrays(GL_QUADS, 0, num_quads * 4);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);

	mainShader->setUniform("tex", 1);
}

void display()
{
	fbo->bind();
	glViewport(0, 0, fbo->getWidth(), fbo->getHeight());
	glEnable(GL_DEPTH_TEST);


	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glm::mat4 projection = glm::perspective(glm::radians(70.0f), screenSize.x / (float)screenSize.y, 0.01f, 300.0f);
	//begin met een perspective matrix
	glm::mat4 view = camera.getMat();


	mainShader->use();
	mainShader->setUniform("tex", 1);
	mainShader->setUniform("modelViewProjectionMatrix", projection * view);
	glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(Vertex), &roomVertices[0].position);
	//geef aan dat de posities op deze locatie zitten
	glVertexAttribPointer(1, 4, GL_FLOAT, false, sizeof(Vertex), &roomVertices[0].color);
	//geef aan dat de kleuren op deze locatie zitten
	glVertexAttribPointer(2, 2, GL_FLOAT, false, sizeof(Vertex), &roomVertices[0].texcoord);
	//geef aan dat de texcoords op deze locatie zitten
	glVertexAttribPointer(3, 3, GL_FLOAT, false, sizeof(Vertex), &roomVertices[0].normal);
	//geef aan dat de normalen op deze locatie zitten
	gridTexture->bind();
	glDrawArrays(GL_QUADS, 0, roomVertices.size()); //en tekenen :)


	glm::mat4 modelMatrix = glm::mat4(1);
	modelMatrix = glm::translate(modelMatrix, glm::vec3(0, 0, 0));
	modelMatrix = glm::rotate(modelMatrix, rotation, glm::vec3(0, 1, 0));
	if (selectedModel == 5)
		modelMatrix = glm::scale(modelMatrix, glm::vec3(0.01f, 0.01f, 0.01f));

	glm::mat4 mvp = projection * view * modelMatrix;

	glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(modelMatrix)));

	models[selectedModel]->draw();

	fbo->unbind();

	glViewport(0, 0, screenSize.x, screenSize.y);

	fbo->use();
	glActiveTexture(GL_TEXTURE3);
	glActiveTexture(GL_TEXTURE0);

	std::vector<glm::vec2> verts;
	verts.push_back(glm::vec2(-1, -1));
	verts.push_back(glm::vec2(1, -1));
	verts.push_back(glm::vec2(1, 1));
	verts.push_back(glm::vec2(-1, 1));


	glDisable(GL_DEPTH_TEST);
	Shader* shader = postProcessingShaders[postProcessingShader]->isValid()
		                 ? postProcessingShaders[postProcessingShader]
		                 : postProcessingShaders[0];
	shader->use();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	glUniform1f(shader->getUniform("time"), glutGet(GLUT_ELAPSED_TIME) / 1000.0f);
	glUniform1i(shader->getUniform("s_texture"), 0);
	glUniform1i(shader->getUniform("s_texture2"), 1);
	glUniform1i(shader->getUniform("s_depth"), 2);
	glUniform1i(shader->getUniform("s_distort"), 3);
	glVertexAttribPointer(0, 2, GL_FLOAT, false, 2 * 4, &verts[0]); //geef aan dat de posities op deze locatie zitten
	glDrawArrays(GL_QUADS, 0, verts.size()); //en tekenen :)


	glutSwapBuffers();
}

void reshape(int newWidth, int newHeight)
{
	screenSize.x = newWidth;
	screenSize.y = newHeight;
	glViewport(0, 0, newWidth, newHeight);
	glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y)
{
	if (key == VK_ESCAPE)
		glutLeaveMainLoop();
	if (key == 'r')
		rotating = !rotating;
	if (key == ']' || key == '[')
		selectedModel = (selectedModel + (int)models.size() + (key == ']' ? 1 : -1)) % models.size();
	if (key == ',' || key == '.')
		postProcessingShader = (postProcessingShader + (int)postProcessingShaders.size() + (key == '.' ? 1 : -1)) %
			postProcessingShaders.size();
	if (key == ' ')
		holdMouse = !holdMouse;

	keys[key] = true;
}

void keyboardUp(unsigned char key, int, int)
{
	keys[key] = false;
}


float updateTime = 1;

void update()
{
	float time = glutGet(GLUT_ELAPSED_TIME);
	float deltaTime = (time - lastTime) / 1000.0f;

	updateTime -= deltaTime;
	if (updateTime < 0)
	{
		for (auto s : shaders)
			s->checkForUpdate();
		for (auto s : postProcessingShaders)
			s->checkForUpdate();
		updateTime = 1;
	}

	if (rotating)
		rotation += deltaTime * 1.0f;

	float speed = 15;

	if (keys['d']) camera.move(180, deltaTime * speed);
	if (keys['a']) camera.move(0, deltaTime * speed);
	if (keys['w']) camera.move(90, deltaTime * speed);
	if (keys['s']) camera.move(270, deltaTime * speed);
	if (keys['q']) camera.position.y += deltaTime * speed;
	if (keys['z']) camera.position.y -= deltaTime * speed;
	//camera.position = glm::clamp(camera.position, glm::vec3(-9.5, 0.5, -9.5), glm::vec3(9.5, 19.5, 9.5));


	glutPostRedisplay();
	lastTime = time;
}

extern "C" {
_declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}


bool justMovedMouse = false;

void mousePassiveMotion(int x, int y)
{
	if (!holdMouse)
		return;
	int dx = x - screenSize.x / 2;
	int dy = y - screenSize.y / 2;
	if ((dx != 0 || dy != 0) && abs(dx) < 400 && abs(dy) < 400 && !justMovedMouse)
	{
		camera.rotation.x += dx / 10.0f;
		camera.rotation.y += dy / 10.0f;
	}
	if (!justMovedMouse)
	{
		glutWarpPointer(screenSize.x / 2, screenSize.y / 2);
		justMovedMouse = true;
	}
	else
		justMovedMouse = false;
}

int main(int argc, char* argv[])
{
	glutInit(&argc, argv);
	glutInitWindowSize(1900, 1000);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutCreateWindow("Realtime Shadowmaps");

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutKeyboardUpFunc(keyboardUp);
	glutPassiveMotionFunc(mousePassiveMotion);
	glutIdleFunc(update);

	init();


	glutMainLoop();
}
