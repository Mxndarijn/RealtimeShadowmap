#include <GL/glew.h>
#include "shader.h"
#include <fstream>
#include <gtc/type_ptr.hpp>
#include <iostream>


void printShaderInfoLog(std::string fileName, GLuint obj)
{
	int infologLength = 0;
	int charsWritten = 0;
	char* infoLog;

	glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &infologLength);

	if (infologLength > 0)
	{
		infoLog = (char*)malloc(infologLength);
		glGetShaderInfoLog(obj, infologLength, &charsWritten, infoLog);
		if (infoLog[0] != '\0')
		{
			printf("%s\n", fileName.c_str());
			printf("%s\n", infoLog);
			//	getchar();
		}
		free(infoLog);
	}
}

void printProgramInfoLog(std::string fileName, GLuint obj)
{
	int infologLength = 0;
	int charsWritten = 0;
	char* infoLog;

	glGetProgramiv(obj, GL_INFO_LOG_LENGTH, &infologLength);

	if (infologLength > 0)
	{
		infoLog = (char*)malloc(infologLength);
		glGetProgramInfoLog(obj, infologLength, &charsWritten, infoLog);
		if (infoLog[0] != '\0')
		{
			printf("%s\n", fileName.c_str());
			printf("%s\n", infoLog);
			//	getchar();
		}
		free(infoLog);
	}
}


ShaderProgram::ShaderProgram(std::string vertShader, std::string fragShader)
{
	programId = glCreateProgram();
	addVertexShader(vertShader);
	addFragmentShader(fragShader);
}

ShaderProgram::ShaderProgram(std::string vertShader, std::string fragShader, std::string geoShader)
{
	programId = glCreateProgram();
	addVertexShader(vertShader);
	addFragmentShader(fragShader);
	addGeometryShader(geoShader);
}

ShaderProgram::~ShaderProgram(void)
{
}

void ShaderProgram::bindAttributeLocation(std::string name, int position)
{
	glBindAttribLocation(programId, position, name.c_str());
}

void ShaderProgram::bindFragLocation(std::string name, int position)
{
	if (glBindFragDataLocation != NULL)
		glBindFragDataLocation(programId, position, name.c_str());
}

void ShaderProgram::link()
{
	glLinkProgram(programId);
}

void ShaderProgram::use()
{
	glUseProgram(programId);
}

void ShaderProgram::addVertexShader(std::string filename)
{
	Shader* s = new Shader(filename, GL_VERTEX_SHADER);
	shaders.push_back(s);
	glAttachShader(programId, s->shaderId);
}

void ShaderProgram::addFragmentShader(std::string filename)
{
	Shader* s = new Shader(filename, GL_FRAGMENT_SHADER);
	shaders.push_back(s);
	glAttachShader(programId, s->shaderId);
}

void ShaderProgram::addGeometryShader(std::string filename)
{
	Shader* s = new Shader(filename, GL_GEOMETRY_SHADER);
	shaders.push_back(s);
	glAttachShader(programId, s->shaderId);
}

void ShaderProgram::setUniformMatrix4(const std::string& name, const glm::mat4& matrix)
{
	glUniformMatrix4fv(getUniformLocation(name), 1, 0, glm::value_ptr(matrix));
}

void ShaderProgram::setUniformMatrix3(const std::string& name, const glm::mat3& matrix)
{
	glUniformMatrix3fv(getUniformLocation(name), 1, 0, glm::value_ptr(matrix));
}

void ShaderProgram::setUniformInt(const std::string& name, int value)
{
	glUniform1i(getUniformLocation(name), value);
}

void ShaderProgram::setUniformFloat(const std::string& name, float value)
{
	glUniform1f(getUniformLocation(name), value);
}

void ShaderProgram::setUniformBool(const std::string& name, bool value)
{
	glUniform1i(getUniformLocation(name), value ? 1 : 0);
}


void ShaderProgram::setUniformVec2(const std::string& name, const glm::vec2& value)
{
	glUniform2fv(getUniformLocation(name), 1, glm::value_ptr(value));
}

void ShaderProgram::setUniformVec3(const std::string& name, const glm::vec3& value)
{
	glUniform3fv(getUniformLocation(name), 1, glm::value_ptr(value));
}

void ShaderProgram::setUniformVec4(const std::string& name, const glm::vec4& value)
{
	glUniform4fv(getUniformLocation(name), 1, glm::value_ptr(value));
}


GLuint ShaderProgram::getUniformLocation(const std::string& name)
{
	if (uniformLocations.find(name) == uniformLocations.end())
		uniformLocations[name] = glGetUniformLocation(programId, name.c_str());
	return uniformLocations[name];
}

bool ShaderProgram::hasShaders()
{
	return GLEW_VERSION_1_5 == GL_TRUE;
}


std::string getFileData(const std::string& fileName)
{
	std::string data = "";
	std::string line;
	std::ifstream pFile(fileName);
	while (!pFile.eof() && pFile.good())
	{
		std::getline(pFile, line);
		if (line.substr(0, 8) == "#include")
		{
			std::string path = "";
			if (fileName.find("/") != std::string::npos)
				path = fileName.substr(0, fileName.rfind("/") + 1);
			if (fileName.find("\\") != std::string::npos)
				path = fileName.substr(0, fileName.rfind("\\") + 1);

			std::string includeFile = "";
			includeFile = line.substr(10, line.size() - 11); //heel vies
			line = getFileData(path + includeFile);
		}
		data += line + "\n";
	}
	return data;
}

ShaderProgram::Shader::Shader(std::string fileName, GLenum type)
{
	std::string data = getFileData(fileName);
	shaderId = glCreateShader(type);
	const char* d2 = data.c_str();
	glShaderSource(shaderId, 1, &d2, NULL);
	glCompileShader(shaderId);
	printShaderInfoLog(fileName, shaderId);
}


void UntypedShader::addShader(std::string fileName, int shaderType)
{
	std::string data = getFileData(fileName);
	GLuint shaderId = glCreateShader(shaderType);
	const char* d2 = data.c_str();
	glShaderSource(shaderId, 1, &d2, NULL);
	glCompileShader(shaderId);
	printShaderInfoLog(fileName, shaderId);

	glAttachShader(programId, shaderId);
}

UntypedShader::UntypedShader(std::string vertShader, std::string fragShader)
{
	programId = glCreateProgram();
	addShader(vertShader, GL_VERTEX_SHADER);
	addShader(fragShader, GL_FRAGMENT_SHADER);
	vertS = vertShader;
	printProgramInfoLog(vertShader + "/" + fragShader, programId);
}

UntypedShader::UntypedShader(std::string vertShader, std::string fragShader, std::string geoShader)
{
	programId = glCreateProgram();
	addShader(vertShader, GL_VERTEX_SHADER);
	addShader(fragShader, GL_FRAGMENT_SHADER);
	addShader(geoShader, GL_GEOMETRY_SHADER);
	printProgramInfoLog(vertShader + "/" + fragShader + "/" + geoShader, programId);
}

void UntypedShader::bindAttributeLocation(std::string name, int position)
{
	glBindAttribLocation(programId, position, name.c_str());
}

void UntypedShader::bindFragLocation(std::string name, int position)
{
	glBindFragDataLocation(programId, position, name.c_str());
}

void UntypedShader::link()
{
	glLinkProgram(programId);
}

void UntypedShader::use()
{
	glUseProgram(programId);
}

void UntypedShader::registerUniform(int id, std::string value)
{
	if ((int)uniformLocations.size() <= id)
		uniformLocations.resize(id + 1, -1);
	uniformLocations[id] = glGetUniformLocation(programId, value.c_str());
	if (uniformLocations[id] < 0)
		std::cout << "Error registering uniform " << value << std::endl;
}


void UntypedShader::registerUniformArray(int id, std::string value, int size)
{
	if ((int)uniformLocationsArray.size() <= id)
		uniformLocationsArray.resize(id + 1);
	for (int i = 0; i < size; i++)
		uniformLocationsArray[id].push_back(
			glGetUniformLocation(programId, (value + "[" + std::to_string(i) + "]").c_str()));
}

void UntypedShader::setUniformRaw(int id, int value)
{
	glUniform1i(id, value);
}

void UntypedShader::setUniformRaw(int id, bool value)
{
	glUniform1i(id, value ? 1 : 0);
}

void UntypedShader::setUniformRaw(int id, float value)
{
	glUniform1f(id, value);
}

void UntypedShader::setUniformRaw(int id, const glm::vec2& value)
{
	glUniform2fv(id, 1, glm::value_ptr(value));
}

void UntypedShader::setUniformRaw(int id, const glm::vec3& value)
{
	glUniform3fv(id, 1, glm::value_ptr(value));
}

void UntypedShader::setUniformRaw(int id, const glm::vec4& value)
{
	glUniform4fv(id, 1, glm::value_ptr(value));
}

void UntypedShader::setUniformRaw(int id, const glm::mat3& value)
{
	glUniformMatrix3fv(id, 1, 0, glm::value_ptr(value));
}

void UntypedShader::setUniformRaw(int id, const glm::mat4& value)
{
	glUniformMatrix4fv(id, 1, 0, glm::value_ptr(value));
}

void UntypedShader::setUniformRaw(int id, const std::vector<glm::mat4>& values)
{
	float* data = new float[values.size() * 16];
	for (size_t i = 0; i < values.size(); i++)
		memcpy(&data[i * 16], glm::value_ptr(values[i]), 4 * 4 * 4);

	glUniformMatrix4fv(id, values.size(), 0, data);
	delete[] data;
}
