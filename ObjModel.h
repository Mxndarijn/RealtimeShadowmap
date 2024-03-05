#pragma once

#include <string>
#include <vector>

#include <windows.h>
#include <gl/GL.h>
#include <glm.hpp>
class Texture;



class ObjModel
{
public:
	struct Vertex
	{
		glm::vec3 position;
		glm::vec4 color;
		glm::vec2 texCoord;
		glm::vec3 normal;
		glm::vec3 tangent;
		glm::vec3 biTangent;
	};
private:
	class MaterialInfo
	{
	public:
		MaterialInfo();
		std::string name;
		Texture* texture;
		Texture* bumpMap;

		bool hasTexture;
	};

	class ObjGroup
	{
	public:
		std::string name;
		int start;
		int end;
		int materialIndex;
	};

	std::vector<ObjGroup*> groups;
	std::vector<MaterialInfo*> materials;
	GLuint _vertexArray;

	void loadMaterialFile(std::string fileName, std::string dirName);
public:
	ObjModel(std::string filename);
	~ObjModel(void);

	void draw(Texture* texOverride = nullptr);
};

