#include "Shader.h"

#include <iostream>
#include <fstream>

#include <gtc/type_ptr.hpp>

bool checkShaderErrors(GLuint shaderId)
{
	GLint status;
	glGetShaderiv(shaderId, GL_COMPILE_STATUS, &status);					//kijk of het compileren is gelukt
	if (status == GL_FALSE)
	{
		int length, charsWritten;
		glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &length);				//haal de lengte van de foutmelding op
		char* infolog = new char[length + 1];
		memset(infolog, 0, length + 1);
		glGetShaderInfoLog(shaderId, length, &charsWritten, infolog);		//en haal de foutmelding zelf op
		std::cout << "Error compiling shader:\n" << infolog << std::endl;
		delete[] infolog;
		return true;
	}
	return false;
}



Shader::Shader(const std::string & filename)
{
	this->name = filename;
	checkForUpdate();
}

void Shader::checkForUpdate()
{
	bool update = false;
	struct stat result;
	if (stat(("assets/shaders/" + name + ".vs").c_str(), &result) == 0)
	{
		auto mod_time = result.st_mtime;
		if (mod_time > vsUpdate)
			update = true;
		vsUpdate = mod_time;
	}
	if (stat(("assets/shaders/" + name + ".fs").c_str(), &result) == 0)
	{
		auto mod_time = result.st_mtime;
		if (mod_time > fsUpdate)
			update = true;
		fsUpdate = mod_time;
	}
	if (update)
	{
		std::cout << "Reloading shader " << name << std::endl;
		reloadShaders();
	}
}

void Shader::reloadShaders()
{
	if (programId != -1)
		glDeleteProgram(programId);

	uniforms.clear();

	std::ifstream vertexShaderFile("assets/shaders/" + name + ".vs");
	std::string vertexShaderData((std::istreambuf_iterator<char>(vertexShaderFile)), std::istreambuf_iterator<char>());
	const char* cvertexShaderData = vertexShaderData.c_str();

	std::ifstream fragShaderFile("assets/shaders/" + name + ".fs");
	std::string fragShaderData((std::istreambuf_iterator<char>(fragShaderFile)), std::istreambuf_iterator<char>());
	const char* cfragShaderData = fragShaderData.c_str();

	programId = glCreateProgram();							// maak een shaderprogramma aan

	GLuint vertexId = glCreateShader(GL_VERTEX_SHADER);		// maak vertex shader aan
	glShaderSource(vertexId, 1, &cvertexShaderData, NULL);		// laat opengl de shader uit de variabele 'vertexShader' halen
	glCompileShader(vertexId);								// compileer de shader
	if (checkShaderErrors(vertexId))
	{
		glDeleteProgram(programId);
		programId = -1;
		return;
	}
	glAttachShader(programId, vertexId);					// hang de shader aan het shaderprogramma


	GLuint fragmentId = glCreateShader(GL_FRAGMENT_SHADER);	// maak fragment shader aan
	glShaderSource(fragmentId, 1, &cfragShaderData, NULL);	// laat opengl de shader uit de variabele 'fragmentShader' halen
	glCompileShader(fragmentId);							// compileer de shader
	if (checkShaderErrors(fragmentId))
	{
		glDeleteProgram(programId);
		programId = -1;
		return;
	}

	glAttachShader(programId, fragmentId);					// hang de shader aan het shaderprogramma

	glBindAttribLocation(programId, 0, "a_position");		// zet de positie op vertex attribuut 0
	glBindAttribLocation(programId, 1, "a_color");			// zet de kleur op vertex attribuut 1
	glBindAttribLocation(programId, 2, "a_texcoord");		// zet de texcoord op vertex attribuut 2
	glBindAttribLocation(programId, 3, "a_normal");			// zet de normaal op vertex attribuut 3
	glLinkProgram(programId);								// link het programma, zorg dat alle attributes en varying gelinked zijn
	glUseProgram(programId);								// Zet dit als actieve programma
}

void Shader::use()
{
	glUseProgram(programId);								// Zet dit als actieve programma
}

GLuint Shader::getUniform(const std::string & name)
{
	auto it = uniforms.find(name);
	if (it != uniforms.end())
		return it->second;
	GLuint location = glGetUniformLocation(programId, name.c_str());
	uniforms[name] = location;
	return location;
}

void Shader::setUniform(const std::string & name, const glm::mat4 & mat)
{
	glUniformMatrix4fv(getUniform(name), 1, GL_FALSE, glm::value_ptr(mat));
}

void Shader::setUniform(const std::string & name, int val)
{
	glUniform1i(getUniform(name), val);
}
