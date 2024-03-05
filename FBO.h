#pragma once

#include <string>
#include <functional>

#include <GL/glew.h>
#include <windows.h>

class FBO
{
	int width;
	int height;
	GLuint depthBuffer;
	GLuint depthTexture;
	int textureCount;
public:
	GLuint fboId;
	GLint oldFBO;
	enum Type
	{
		Color,
		Normal,
		Position,
		Depth,
		ShadowCube,
		None,
	};

	FBO(int width, int height, bool depth = false, int textureCount = 1, bool hasDepthTexture = false);
	FBO(int width, int height, bool hasDepthTexture, Type buf1, Type buf2 = None, Type buf3 = None, Type buf4 = None);

	~FBO();

	void bind();
	void bind(int index);
	void unbind();


	void use(int offset = 0);

	Type types[5];
	GLuint texid[5];	//5 should be big enough

	int getHeight();
	int getWidth();

	void saveAsFile(const std::string &fileName);
	void saveAsFileBackground(const std::string &fileName, std::function<void()> callback);

};
