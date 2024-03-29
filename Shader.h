#pragma once
#include <GL/glew.h>

#include <string>
#include <vector>
#include <map>
#include <glm.hpp>


class ShaderProgram
{
	class Shader
	{
	public:
		GLuint shaderId;
		Shader(std::string fileName, GLenum type);
	};


	GLuint programId;
	std::vector<Shader*> shaders;

	std::map<std::string, int> uniformLocations;

public:
	ShaderProgram(std::string vertShader, std::string fragShader);
	ShaderProgram(std::string vertShader, std::string fragShader, std::string geometryShader);
	~ShaderProgram();

	void addVertexShader(std::string filename);
	void addFragmentShader(std::string filename);
	void addGeometryShader(std::string filename);
	//attributes
	void bindAttributeLocation(std::string name, int position);
	void bindFragLocation(std::string name, int position);
	//uniforms
	GLuint getUniformLocation(const std::string&);
	void setUniformMatrix4(const std::string& name, const glm::mat4& matrix);
	void setUniformMatrix3(const std::string& name, const glm::mat3& matrix);
	void setUniformInt(const std::string& name, int value);
	void setUniformVec2(const std::string& name, const glm::vec2& value);
	void setUniformVec3(const std::string& name, const glm::vec3& value);
	void setUniformVec4(const std::string& name, const glm::vec4& value);
	void setUniformFloat(const std::string& name, float value);
	void setUniformBool(const std::string& name, bool value);

	void link();
	void use();
	static bool hasShaders();
};


class UntypedShader
{
protected:
	GLuint programId;
	std::vector<int> uniformLocations;
	std::vector<std::vector<int>> uniformLocationsArray;
	std::string vertS;

public:
	UntypedShader(std::string vertShader, std::string fragShader);
	UntypedShader(std::string vertShader, std::string fragShader, std::string geoShader);

	virtual ~UntypedShader()
	{
	};
	void addShader(std::string fileName, int shaderType);


	void bindAttributeLocation(std::string name, int position);
	void bindFragLocation(std::string name, int position);

	void link();
	void use();

	void registerUniform(int id, std::string value);
	void registerUniformArray(int id, std::string value, int size);


	template <class T>
	void setUniform(int id, int index, const T& value)
	{
		setUniformRaw(uniformLocationsArray[id][index], value);
	};

	template <class T>
	void setUniform(int id, const T& value)
	{
		setUniformRaw(uniformLocations[id], value);
	};

private:
	void setUniformRaw(int id, int value);
	void setUniformRaw(int id, float value);
	void setUniformRaw(int id, bool value);
	void setUniformRaw(int id, const glm::vec2& value);
	void setUniformRaw(int id, const glm::vec3& value);
	void setUniformRaw(int id, const glm::vec4& value);
	void setUniformRaw(int id, const glm::mat3& value);
	void setUniformRaw(int id, const glm::mat4& value);
	void setUniformRaw(int id, const std::vector<glm::mat4>& value);
};

template <class T>
class Shader : public UntypedShader
{
public:
	Shader(std::string vertShader, std::string fragShader) : UntypedShader(vertShader, fragShader)
	{
	}

	Shader(std::string vertShader, std::string fragShader, std::string geoShader) : UntypedShader(
		vertShader, fragShader, geoShader)
	{
	}

	void registerUniform(T id, std::string value)
	{
		UntypedShader::registerUniform((int)id, value);
	}

	void registerUniformArray(T id, std::string value, int size)
	{
		UntypedShader::registerUniformArray((int)id, value, size);
	}


	template <class V>
	void setUniform(T id, const V& value)
	{
		//assert(uniformLocations[(int)id] != -1);
		UntypedShader::setUniform((int)id, value);
	}

	template <class V>
	void setUniform(T id, int index, const V& value)
	{
		//assert(uniformLocations[(int)id] != -1);
		UntypedShader::setUniform((int)id, index, value);
	}
};
