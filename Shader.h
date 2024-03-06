#pragma once

#include <map>
#include <string>

#include <GL/glew.h>
#include <glm.hpp>

class Shader
{
	std::map<std::string, GLuint> uniforms;
	GLuint programId = -1;
	time_t vsUpdate = 0;
	time_t fsUpdate = 0;
public:
	std::string name;

	Shader(const std::string &filename);

	inline bool isValid()	{		return programId != -1;	}

	void checkForUpdate();

	void reloadShaders();

	void use();

	GLuint getUniform(const std::string &name);

	void setUniform(const std::string &name, const glm::mat4 &mat);
	void setUniform(const std::string &name, int val);
	void setUniform(const std::string &name, const glm::vec3 &value);
};