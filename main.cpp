#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <iostream>
#include <fstream>
#include <vector>
#include <iostream>
#include <string>

#include "stb_image_write.h"
#include <thread>

#define STB_PERLIN_IMPLEMENTATION
#include "stb_easy_font.h"
#pragma comment(lib, "glew32.lib")

#include "Shader.h"
#include "Texture.h"
#include "Camera.h"
#include "ObjModel.h"
#include "FBO.h"

#define SHADOW_MAP_WIDTH 2048
#define SHADOW_MAP_HEIGHT 2048


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



std::vector<Vertex> roomVertices;
std::vector<Shader*> shaders;
std::vector<ObjModel*> models;
int selectedModel = 0;
Texture* gridTexture;

Shader* mainShader;

Shader* shadowMappingShader;
Shader* shadowMappingDepthShader;

glm::ivec2 screenSize;
float rotation;
bool rotating = false;
int lastTime;
bool keys[255];
bool holdMouse = false;

// unsigned int depthMapFBO;
// unsigned int depthMap;

FBO* depthMapFBO;

void initShadowMapping()
{
	depthMapFBO = new FBO(SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT, true, FBO::Type::Depth);
	// glGenFramebuffers(1, &depthMapFBO);
	// glGenTextures(1, &depthMap);
	// glBindTexture(GL_TEXTURE_2D, depthMap);
	// glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// // attach depth texture as FBO's depth buffer
	// glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	// glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	// glDrawBuffer(GL_NONE);
	// glReadBuffer(GL_NONE);
	// glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

glm::vec3 lightPos;
void initLight()
{
	lightPos = glm::vec3(-2.0f, 5.0f, 2.0f);
}


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
	verts.reserve(4);

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

	shadowMappingShader = new Shader("shadow_mapping");
	shadowMappingDepthShader = new Shader("shadow_mapping_depth");

	shadowMappingShader->use();
	shadowMappingShader->setUniform("diffuseTexture", 0);
	shadowMappingShader->setUniform("shadowMap", 1);



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

	shadowMappingDepthShader->getUniform("lightSpaceMatrix");

	initShadowMapping();
	initLight();

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

unsigned int cubeVAO = 0;
unsigned int cubeVBO = 0;

// Functie om een kubus te initialiseren
void initCube() {
	if (cubeVAO == 0) {
		float vertices[] = {
			// Positie              // Kleur (rood)
			// // Achterkant
			// -0.5f, -0.5f, -0.5f,   1.0f, 0.0f, 0.0f,
			//  0.5f, -0.5f, -0.5f,   1.0f, 0.0f, 0.0f,
			//  0.5f,  0.5f, -0.5f,   1.0f, 0.0f, 0.0f,
			//  0.5f,  0.5f, -0.5f,   1.0f, 0.0f, 0.0f,
			// -0.5f,  0.5f, -0.5f,   1.0f, 0.0f, 0.0f,
			// -0.5f, -0.5f, -0.5f,   1.0f, 0.0f, 0.0f,
			//
			// // Voorkant
			// -0.5f, -0.5f,  0.5f,   1.0f, 0.0f, 0.0f,
			//  0.5f, -0.5f,  0.5f,   1.0f, 0.0f, 0.0f,
			//  0.5f,  0.5f,  0.5f,   1.0f, 0.0f, 0.0f,
			//  0.5f,  0.5f,  0.5f,   1.0f, 0.0f, 0.0f,
			// -0.5f,  0.5f,  0.5f,   1.0f, 0.0f, 0.0f,
			// -0.5f, -0.5f,  0.5f,   1.0f, 0.0f, 0.0f,
			//
			// // Linkerkant
			// -0.5f,  0.5f,  0.5f,   1.0f, 0.0f, 0.0f,
			// -0.5f,  0.5f, -0.5f,   1.0f, 0.0f, 0.0f,
			// -0.5f, -0.5f, -0.5f,   1.0f, 0.0f, 0.0f,
			// -0.5f, -0.5f, -0.5f,   1.0f, 0.0f, 0.0f,
			// -0.5f, -0.5f,  0.5f,   1.0f, 0.0f, 0.0f,
			// -0.5f,  0.5f,  0.5f,   1.0f, 0.0f, 0.0f,
			//
			// // Rechterkant
			//  0.5f,  0.5f,  0.5f,   1.0f, 0.0f, 0.0f,
			//  0.5f,  0.5f, -0.5f,   1.0f, 0.0f, 0.0f,
			//  0.5f, -0.5f, -0.5f,   1.0f, 0.0f, 0.0f,
			//  0.5f, -0.5f, -0.5f,   1.0f, 0.0f, 0.0f,
			//  0.5f, -0.5f,  0.5f,   1.0f, 0.0f, 0.0f,
			//  0.5f,  0.5f,  0.5f,   1.0f, 0.0f, 0.0f,
			//
			//  // Onderkant
			//  -0.5f, -0.5f, -0.5f,   1.0f, 0.0f, 0.0f,
			//   0.5f, -0.5f, -0.5f,   1.0f, 0.0f, 0.0f,
			//   0.5f, -0.5f,  0.5f,   1.0f, 0.0f, 0.0f,
			//   0.5f, -0.5f,  0.5f,   1.0f, 0.0f, 0.0f,
			//  -0.5f, -0.5f,  0.5f,   1.0f, 0.0f, 0.0f,
			//  -0.5f, -0.5f, -0.5f,   1.0f, 0.0f, 0.0f,

			 // Bovenkant
			 -0.5f,  0.5f, -0.5f,   1.0f, 0.0f, 0.0f,
			  0.5f,  0.5f, -0.5f,   1.0f, 0.0f, 0.0f,
			  0.5f,  0.5f,  0.5f,   1.0f, 0.0f, 0.0f,
			  0.5f,  0.5f,  0.5f,   1.0f, 0.0f, 0.0f,
			 -0.5f,  0.5f,  0.5f,   1.0f, 0.0f, 0.0f,
			 -0.5f,  0.5f, -0.5f,   1.0f, 0.0f, 0.0f
		};


		glGenVertexArrays(1, &cubeVAO);
		glGenBuffers(1, &cubeVBO);
		glBindVertexArray(cubeVAO);

		glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		// Positie attribuut
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		// Kleur attribuut
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);
	}
}

// void saveAsFileBackground(const std::string& fileName, std::function<void()> callback)
// {
// 	char* data = new char[SHADOW_MAP_WIDTH * SHADOW_MAP_HEIGHT * 4];
// 	glActiveTexture(GL_TEXTURE0);
// 	glBindTexture(GL_TEXTURE_2D, depthMap);
//
// 	glGetTexImage(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, data);
// 	const int rowSize = SHADOW_MAP_WIDTH * 4;
// 	char* row = new char[rowSize];
// 	for (int y = 0; y < SHADOW_MAP_HEIGHT / 2; y++)
// 	{
// 		memcpy(row, data + rowSize * y, rowSize);
// 		memcpy(data + rowSize * y, data + rowSize * (SHADOW_MAP_HEIGHT - 1 - y), rowSize);
// 		memcpy(data + rowSize * (SHADOW_MAP_HEIGHT - 1 - y), row, rowSize);
// 	}
// 	std::thread thread([data, fileName, callback]()
// 		{
// 			if (fileName.substr(fileName.size() - 4) == ".bmp")
// 				stbi_write_bmp(fileName.c_str(), SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT, 4, data);
// 			else if (fileName.substr(fileName.size() - 4) == ".tga")
// 				stbi_write_tga(fileName.c_str(), SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT, 4, data);
// 			else if (fileName.substr(fileName.size() - 4) == ".png")
// 				stbi_write_png(fileName.c_str(), SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT, 4, data, 4 * SHADOW_MAP_WIDTH);
// 			else if (fileName.substr(fileName.size() - 4) == ".jpg")
// 				stbi_write_jpg(fileName.c_str(), SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT, 4, data, 95);
// 			delete[] data;
// 			callback();
// 		});
// 	thread.detach();
// }

void display()


{

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	float near_plane =1.0f, far_plane = 7.5f;

	glm::mat4 lightProjection, lightView;
	glm::mat4 lightSpaceMatrix;

	lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
	lightView = glm::lookAt(lightPos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0, 1.0, 0.0));
	lightSpaceMatrix = lightProjection * lightView;


	shadowMappingDepthShader->use();
	shadowMappingDepthShader->setUniform("lightSpaceMatrix", lightSpaceMatrix);

	glViewport(0, 0, SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT);

	depthMapFBO->bind();
	// glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);


	glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(Vertex), &roomVertices[0].position);
	//geef aan dat de posities op deze locatie zitten
	glVertexAttribPointer(1, 4, GL_FLOAT, false, sizeof(Vertex), &roomVertices[0].color);
	//geef aan dat de kleuren op deze locatie zitten
	glVertexAttribPointer(2, 2, GL_FLOAT, false, sizeof(Vertex), &roomVertices[0].texcoord);
	//geef aan dat de texcoords op deze locatie zitten
	glVertexAttribPointer(3, 3, GL_FLOAT, false, sizeof(Vertex), &roomVertices[0].normal);
	//geef aan dat de normalen op deze locatie zitten
	gridTexture->bind();
	glDrawArrays(GL_QUADS, 0, roomVertices.size());


	glm::mat4 modelMatrix = glm::mat4(1);
	modelMatrix = glm::translate(modelMatrix, glm::vec3(0, 0, 0));
	modelMatrix = glm::rotate(modelMatrix, rotation, glm::vec3(0, 1, 0));

	shadowMappingShader->setUniform("model", modelMatrix);

	models[selectedModel]->draw();


	auto myCallback = []() {
		std::cout << "Afbeelding opgeslagen!" << std::endl;
		// Andere logica kan hier worden toegevoegd
		};

	// saveAsFileBackground("mijnAfbeelding1.png", myCallback);

	depthMapFBO->saveAsFileBackground("test.png", myCallback);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	glViewport(0, 0, screenSize.x, screenSize.y);


	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glm::mat4 projection = glm::perspective(glm::radians(70.0f), screenSize.x / (float)screenSize.y, 0.01f, 300.0f);
	//begin met een perspective matrix
	glm::mat4 view = camera.getMat();


	// mainShader->use();
	// mainShader->setUniform("tex", 1);
	// mainShader->setUniform("modelViewProjectionMatrix", projection * view);

	shadowMappingShader->use(); 
	shadowMappingShader->setUniform("projection", projection);
	shadowMappingShader->setUniform("view", view);
	shadowMappingShader->setUniform("viewPos", camera.position);
	shadowMappingShader->setUniform("lightPos", lightPos);
	shadowMappingShader->setUniform("lightSpaceMatrix", lightSpaceMatrix);

	// glActiveTexture(GL_TEXTURE0);
	// glBindTexture(GL_TEXTURE_2D, depthMapFBO->texid[0]);
	depthMapFBO->use();

	glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(Vertex), &roomVertices[0].position);
	//geef aan dat de posities op deze locatie zitten
	glVertexAttribPointer(1, 4, GL_FLOAT, false, sizeof(Vertex), &roomVertices[0].color);
	//geef aan dat de kleuren op deze locatie zitten
	glVertexAttribPointer(2, 2, GL_FLOAT, false, sizeof(Vertex), &roomVertices[0].texcoord);
	//geef aan dat de texcoords op deze locatie zitten
	glVertexAttribPointer(3, 3, GL_FLOAT, false, sizeof(Vertex), &roomVertices[0].normal);
	//geef aan dat de normalen op deze locatie zitten
	gridTexture->bind();
	glDrawArrays(GL_QUADS, 0, roomVertices.size());


	shadowMappingShader->setUniform("model", modelMatrix);


	models[selectedModel]->draw();
	// renderRedCube(lightPos);
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
	if (key == ' ')
		holdMouse = !holdMouse;

	keys[key] = true;
}

void keyboardUp(unsigned char key, int, int)
{
	keys[key] = false;
}

unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
	if (quadVAO == 0)
	{
		float quadVertices[] = {
			// positions        // texture Coords
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};
		// setup plane VAO
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	}
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
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
	glutCreateWindow("Realtime ShadowMapping");

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutKeyboardUpFunc(keyboardUp);
	glutPassiveMotionFunc(mousePassiveMotion);
	glutIdleFunc(update);

	init();


	glutMainLoop();
}

