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
#include "Vertex.h"

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

enum class ShadowUniforms
{
	modelMatrix,
	projectionMatrix,
	viewMatrix
};

enum class Uniforms
{
	modelMatrix,
	projectionMatrix,
	viewMatrix,
	shadowMatrix,
	s_texture,
	s_shadowmap,
	diffuseColor,
	textureFactor
};


std::vector<Vertex> roomVertices;
std::vector<ObjModel*> models;
int selectedModel = 0;
Texture* gridTexture;

Shader<Uniforms>* shadowMappingShader;
Shader<ShadowUniforms>* shadowMappingDepthShader;

FBO* depthMapFBO;

glm::ivec2 screenSize;
float rotation;
bool rotating = false;
int lastTime;
bool keys[255];
bool holdMouse = false;


std::vector<Vertex> buildCube(const glm::vec3& p, const glm::vec3& s)
{
	glm::vec4 color;
	color.r = rand() / (float)RAND_MAX;
	color.g = rand() / (float)RAND_MAX;
	color.b = rand() / (float)RAND_MAX;
	color.a = 1;


	std::vector<Vertex> verts;
	verts.reserve(4);

	//bottom
	verts.push_back(Vertex(p + glm::vec3(-s.x, -s.y, -s.z), color, glm::vec2(0, 0), glm::vec3(0, -1, 0)));
	verts.push_back(Vertex(p + glm::vec3(s.x, -s.y, -s.z), color, glm::vec2(1, 0), glm::vec3(0, -1, 0)));
	verts.push_back(Vertex(p + glm::vec3(s.x, -s.y, s.z), color, glm::vec2(1, 1), glm::vec3(0, -1, 0)));
	verts.push_back(Vertex(p + glm::vec3(-s.x, -s.y, s.z), color, glm::vec2(0, 1), glm::vec3(0, -1, 0)));

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

unsigned int cubeVBO, cubeVAO;

void createCubeVAO(const std::vector<Vertex>& verts) {
	// Genereer en bind VBO
	glGenBuffers(1, &cubeVBO);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(Vertex), verts.data(), GL_STATIC_DRAW);

	// Genereer en bind VAO
	glGenVertexArrays(1, &cubeVAO);
	glBindVertexArray(cubeVAO);

	// Zet attribuutpointers op. Hier zijn de locaties aannames.
	// Positie
	glEnableVertexAttribArray(0); // Veronderstelt locatie 0 voor positie
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));

	// Kleur
	glEnableVertexAttribArray(1); // Veronderstelt locatie 1 voor kleur
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));

	// Texture coördinaten
	glEnableVertexAttribArray(2); // Veronderstelt locatie 2 voor texture coördinaten
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texcoord));

	// Normaal
	glEnableVertexAttribArray(3); // Veronderstelt locatie 3 voor normaal
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

	// Unbind VAO
	glBindVertexArray(0);
}

void init()
{
	glewInit();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);

	//Initializing 
	shadowMappingDepthShader = new Shader<ShadowUniforms>("assets/shaders/shadowmap.vert",
	                                                      "assets/shaders/shadowmap.frag");
	shadowMappingDepthShader->bindAttributeLocation("a_position", 0);
	shadowMappingDepthShader->link();
	shadowMappingDepthShader->registerUniform(ShadowUniforms::modelMatrix, "modelMatrix");
	shadowMappingDepthShader->registerUniform(ShadowUniforms::projectionMatrix, "projectionMatrix");
	shadowMappingDepthShader->registerUniform(ShadowUniforms::viewMatrix, "viewMatrix");
	shadowMappingDepthShader->use();

	shadowMappingShader = new Shader<Uniforms>("assets/shaders/default.vert", "assets/shaders/default.frag");
	shadowMappingShader->bindAttributeLocation("a_position", 0);
	shadowMappingShader->bindAttributeLocation("a_normal", 1);
	shadowMappingShader->bindAttributeLocation("a_texcoord", 2);
	shadowMappingShader->link();
	shadowMappingShader->bindFragLocation("fragColor", 0);
	shadowMappingShader->registerUniform(Uniforms::modelMatrix, "modelMatrix");
	shadowMappingShader->registerUniform(Uniforms::projectionMatrix, "projectionMatrix");
	shadowMappingShader->registerUniform(Uniforms::viewMatrix, "viewMatrix");
	shadowMappingShader->registerUniform(Uniforms::shadowMatrix, "shadowMatrix");
	shadowMappingShader->registerUniform(Uniforms::s_texture, "s_texture");
	shadowMappingShader->registerUniform(Uniforms::diffuseColor, "diffuseColor");
	shadowMappingShader->registerUniform(Uniforms::textureFactor, "textureFactor");
	shadowMappingShader->registerUniform(Uniforms::s_shadowmap, "s_shadowmap");
	shadowMappingShader->use();
	shadowMappingShader->setUniform(Uniforms::s_texture, 0);
	shadowMappingShader->setUniform(Uniforms::s_shadowmap, 1);

	depthMapFBO = new FBO(SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT, false, 0, true);


	//	models.push_back(new ObjModel("gamecube/model_n.obj"));
	models.push_back(new ObjModel("statue/statue.obj"));
	models.push_back(new ObjModel("cube/cube.obj"));

	//
	if (glDebugMessageCallback)
	{
		glDebugMessageCallback(&onDebug, NULL);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
		glEnable(GL_DEBUG_OUTPUT);
	}

	rotation = 0;
	lastTime = glutGet(GLUT_ELAPSED_TIME);

	roomVertices = buildCube(glm::vec3(0, 0, 0), glm::vec3(10, 0, 10));
	createCubeVAO(roomVertices);
	gridTexture = new Texture("grid.png");
}

bool takeScreenshot = false;

void drawScene(int pass, std::function<void(const glm::mat4& modelMatrix)> modelViewCallback)
{
	glActiveTexture(GL_TEXTURE0);
	modelViewCallback(glm::mat4(1));
	models[selectedModel]->draw();


	modelViewCallback(glm::mat4(1));


	glBindVertexArray(cubeVAO);
	glActiveTexture(GL_TEXTURE0);
	gridTexture->bind();
	glDrawArrays(GL_QUADS, 0, roomVertices.size());
	glBindVertexArray(0);
}


void display()
{
	// std::cout << "Display" << std::endl;
	float fac = 10.0f;
	static float lightDirection = 0;
	lightDirection += 0.001f;

	glm::vec3 lightAngle(cos(lightDirection) * 3, 2, sin(lightDirection) * 3);
	glm::mat4 shadowProjectionMatrix = glm::ortho<float>(-fac, fac, -fac, fac, -5, 20);
	glm::mat4 shadowCameraMatrix = glm::lookAt(lightAngle + glm::vec3(0, 0, 0), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

	glDisable(GL_SCISSOR_TEST);

	shadowMappingDepthShader->use();
	shadowMappingDepthShader->setUniform(ShadowUniforms::projectionMatrix, shadowProjectionMatrix);
	shadowMappingDepthShader->setUniform(ShadowUniforms::viewMatrix, shadowCameraMatrix);
	shadowMappingDepthShader->setUniform(ShadowUniforms::modelMatrix, glm::mat4(1));


	depthMapFBO->bind();
	glViewport(0, 0, depthMapFBO->getWidth(), depthMapFBO->getHeight());
	glClearColor(1, 0, 0, 1);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	drawScene(0, [&](const glm::mat4& modelMatrix)
	{
		// std::cout << "Settings matrix" << std::endl;
		shadowMappingDepthShader->setUniform(ShadowUniforms::modelMatrix, modelMatrix);
	});
	depthMapFBO->unbind();

	glViewport(0, 0, screenSize.x, screenSize.y);
	glEnable(GL_SCISSOR_TEST);
	glClearColor(35 / 255.0f, 145 / 255.0f, 247 / 255.0f, 1.0f);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	glm::mat4 modelMatrix = glm::mat4(1);
	modelMatrix = glm::translate(modelMatrix, glm::vec3(0, 0, 0));
	modelMatrix = glm::rotate(modelMatrix, rotation, glm::vec3(0, 1, 0));

	glm::mat4 projection = glm::perspective(glm::radians(70.0f), screenSize.x / (float)screenSize.y, 0.01f, 300.0f);
	glm::mat4 view = camera.getMat();

	shadowMappingShader->use();
	shadowMappingShader->setUniform(Uniforms::projectionMatrix, projection);
	shadowMappingShader->setUniform(Uniforms::viewMatrix, view);
	shadowMappingShader->setUniform(Uniforms::modelMatrix, modelMatrix);
	shadowMappingShader->setUniform(Uniforms::shadowMatrix, shadowProjectionMatrix * shadowCameraMatrix);
	shadowMappingShader->setUniform(Uniforms::textureFactor, 1.0f);
	shadowMappingShader->setUniform(Uniforms::diffuseColor, glm::vec4(1, 1, 1, 1));

	if (takeScreenshot)
	{
		takeScreenshot = false;
		std::cout << "Taking screenshot" << std::endl;
		const std::function<void()> callback = []()
		{
			std::cout << "Screenshot saved." << std::endl;
		};
		depthMapFBO->saveAsFileBackground("depthMap.png", callback);
	}
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, depthMapFBO->texid[0]);

	drawScene(1, [&](const glm::mat4& modelMatrix)
	{
		shadowMappingShader->setUniform(Uniforms::modelMatrix, modelMatrix);
	});

	glDisable(GL_SCISSOR_TEST);

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

float minTimeBeforeScreenshot = 0;
void update()
{
	float time = glutGet(GLUT_ELAPSED_TIME);
	float deltaTime = (time - lastTime) / 1000.0f;

	if (rotating)
		rotation += deltaTime * 1.0f;

	float speed = 15;
	if(minTimeBeforeScreenshot > 0)
		minTimeBeforeScreenshot -= deltaTime;

	if (keys['d']) camera.move(180, deltaTime * speed);
	if (keys['a']) camera.move(0, deltaTime * speed);
	if (keys['w']) camera.move(90, deltaTime * speed);
	if (keys['s']) camera.move(270, deltaTime * speed);
	if (keys['q']) camera.position.y += deltaTime * speed;
	if (keys['z']) camera.position.y -= deltaTime * speed;
	if (keys['p'])
	{
		if(minTimeBeforeScreenshot <= 0)
		{
			minTimeBeforeScreenshot = 2;
			takeScreenshot = true;
		}
	}
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
