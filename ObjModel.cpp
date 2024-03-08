#include <gl/glew.h>
#include "ObjModel.h"
#include "Texture.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <algorithm>
#include <glm.hpp>


#define BUFFER_OFFSET(vrt, attribute) (void*)((char*)(&attribute) - (char*)(&vrt))

std::string replace(std::string str, const std::string &toReplace, const std::string &replacement)
{
	size_t index = 0;
	while (true) 
	{
		 index = str.find(toReplace, index);
		 if (index == std::string::npos) 
			 break;
		 str.replace(index, toReplace.length(), replacement);
		 ++index;
	}
	return str;
}

std::vector<std::string> split(const std::string &str, const std::string &sep)
{
	std::vector<std::string> ret;
	int oldIndex = 0;
	while(true)
	{
		size_t index = str.find(sep, oldIndex);
		if(index == std::string::npos)
			break;
		ret.push_back(str.substr(oldIndex, index - oldIndex));
		oldIndex = index+1;
	}
	ret.push_back(str.substr(oldIndex));
	return ret;
}

inline std::string toLower(std::string data)
{
	std::transform(data.begin(), data.end(), data.begin(), ::tolower);
	return data;
}

inline bool isWhiteSpace(char c)
{
	return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

inline std::string trim(std::string data)
{
	while (data.length() > 0 && isWhiteSpace(data[0]))
		data = data.substr(1);
	while (data.length() > 0 && isWhiteSpace(data[data.length() - 1]))
		data = data.substr(0, data.size() - 1);
	return data;
}



glm::vec4 calcTangentVector(
	glm::vec3 pos1,		glm::vec3 pos2,		glm::vec3 pos3, 
	glm::vec2 texCoord1,	glm::vec2 texCoord2,	glm::vec2 texCoord3, glm::vec3 normal)
{
	// Given the 3 vertices (position and texture coordinates) of a triangle
	// calculate and return the triangle's tangent vector.

	// Create 2 vectors in object space.
	//
	// edge1 is the vector from vertex positions pos1 to pos2.
	// edge2 is the vector from vertex positions pos1 to pos3.
	glm::vec3 edge1(pos2-pos1);//Vector3 edge1(pos2[0] - pos1[0], pos2[1] - pos1[1], pos2[2] - pos1[2]);
	glm::vec3 edge2(pos3-pos1);//Vector3 edge2(pos3[0] - pos1[0], pos3[1] - pos1[1], pos3[2] - pos1[2]);

	edge1 = glm::normalize(edge1);
	edge2 = glm::normalize(edge2);

	// Create 2 vectors in tangent (texture) space that point in the same
	// direction as edge1 and edge2 (in object space).
	//
	// texEdge1 is the vector from texture coordinates texCoord1 to texCoord2.
	// texEdge2 is the vector from texture coordinates texCoord1 to texCoord3.
	glm::vec2 texEdge1(texCoord2 - texCoord1);
	glm::vec2 texEdge2(texCoord3 - texCoord1);

	texEdge1 = glm::normalize(texEdge1);
	texEdge2 = glm::normalize(texEdge2);

	// These 2 sets of vectors form the following system of equations:
	//
	//  edge1 = (texEdge1.x * tangent) + (texEdge1.y * bitangent)
	//  edge2 = (texEdge2.x * tangent) + (texEdge2.y * bitangent)
	//
	// Using matrix notation this system looks like:
	//
	//  [ edge1 ]     [ texEdge1.x  texEdge1.y ]  [ tangent   ]
	//  [       ]  =  [                        ]  [           ]
	//  [ edge2 ]     [ texEdge2.x  texEdge2.y ]  [ bitangent ]
	//
	// The solution is:
	//
	//  [ tangent   ]        1     [ texEdge2.y  -texEdge1.y ]  [ edge1 ]
	//  [           ]  =  -------  [                         ]  [       ]
	//  [ bitangent ]      det A   [-texEdge2.x   texEdge1.x ]  [ edge2 ]
	//
	//  where:
	//        [ texEdge1.x  texEdge1.y ]
	//    A = [                        ]
	//        [ texEdge2.x  texEdge2.y ]
	//
	//    det A = (texEdge1.x * texEdge2.y) - (texEdge1.y * texEdge2.x)
	//
	// From this solution the tangent space basis vectors are:
	//
	//    tangent = (1 / det A) * ( texEdge2.y * edge1 - texEdge1.y * edge2)
	//  bitangent = (1 / det A) * (-texEdge2.x * edge1 + texEdge1.x * edge2)
	//     normal = cross(tangent, bitangent)

	glm::vec3 t;
	glm::vec3 b;
	

	float det = (texEdge1[0] * texEdge2[1]) - (texEdge1[1] * texEdge2[0]);

	if (det == 0)
	{
		t = glm::vec3(1.0f, 0.0f, 0.0f);
		b = glm::vec3(0.0f, 1.0f, 0.0f);
	}
	else
	{
		det = 1.0f / det;

		t[0] = (texEdge2[1] * edge1[0] - texEdge1[1] * edge2[0]) * det;
		t[1] = (texEdge2[1] * edge1[1] - texEdge1[1] * edge2[0]) * det;
		t[2] = (texEdge2[1] * edge1[2] - texEdge1[1] * edge2[0]) * det;

		b[0] = (-texEdge2[0] * edge1[0] + texEdge1[0] * edge2[0]) * det;
		b[1] = (-texEdge2[0] * edge1[1] + texEdge1[0] * edge2[1]) * det;
		b[2] = (-texEdge2[0] * edge1[2] + texEdge1[0] * edge2[2]) * det;

		t = normalize(t);
		b = normalize(b);
	}

	// Calculate the handedness of the local tangent space.
	// The bitangent vector is the cross product between the triangle face
	// normal vector and the calculated tangent vector. The resulting bitangent
	// vector should be the same as the bitangent vector calculated from the
	// set of linear equations above. If they point in different directions
	// then we need to invert the cross product calculated bitangent vector. We
	// store this scalar multiplier in the tangent vector's 'w' component so
	// that the correct bitangent vector can be generated in the normal mapping
	// shader's vertex shader.

	glm::vec3 bitangent = glm::cross(normal, t);
	float handedness = (glm::dot(bitangent, b) < 0.0f) ? -1.0f : 1.0f;

	return glm::vec4(t[0], t[1], t[2], handedness);
}




ObjModel::ObjModel(std::string fileName)
{
	fileName = "assets/models/" + fileName;
	std::string dirName = fileName;
	if(dirName.rfind("/") != std::string::npos)
		dirName = dirName.substr(0, dirName.rfind("/"));
	if(dirName.rfind("\\") != std::string::npos)
		dirName = dirName.substr(0, dirName.rfind("\\"));
	if(fileName == dirName)
		dirName = "";


	std::ifstream pFile(fileName.c_str());
	if (!pFile.is_open())
	{
		std::cout << "Error opening " << fileName << std::endl;
		return;
	}

	std::vector<glm::vec3>	vertices;
	std::vector<glm::vec3>	normals;
	std::vector<glm::vec2>	texcoords;

	std::vector<Vertex> finalVertices;


	ObjGroup* currentGroup = new ObjGroup();
	currentGroup->end = -1;
	currentGroup->start = 0;
	currentGroup->materialIndex = -1;


	while(!pFile.eof())
	{
		std::string line;
		std::getline(pFile, line);
		
		line = replace(line, "\t", " ");
		while(line.find("  ") != std::string::npos)
			line = replace(line, "  ", " ");
		if(line == "")
			continue;
		if(line[0] == ' ')
			line = line.substr(1);
		if(line == "")
			continue;
		if(line[line.length()-1] == ' ')
			line = line.substr(0, line.length()-1);
		if(line == "")
			continue;
		if(line[0] == '#')
			continue;

		std::vector<std::string> params = split(line, " ");
		params[0] = toLower(params[0]);

		if(params[0] == "v")
		{
			vertices.push_back(glm::vec3(
				atof(params[1].c_str()),
				atof(params[2].c_str()),
				atof(params[3].c_str())));
		}
		else if(params[0] == "vn")
		{
			normals.push_back(glm::vec3(
				atof(params[1].c_str()),
				atof(params[2].c_str()),
				atof(params[3].c_str())));
		}
		else if(params[0] == "vt")
		{
			texcoords.push_back(glm::vec2(
				atof(params[1].c_str()),
				atof(params[2].c_str())));
		}
		else if(params[0] == "f")
		{
			std::vector<std::string> indices1 = split(params[1], "/");
			std::vector<std::string> indices2 = split(params[2], "/");
			std::vector<std::string> indices3 = split(params[3], "/");
			glm::vec3 &p1 = vertices[(atoi(indices1[0].c_str()) - 1)];
			glm::vec3 &p2 = vertices[(atoi(indices2[0].c_str()) - 1)];
			glm::vec3 &p3 = vertices[(atoi(indices3[0].c_str()) - 1)];
			glm::vec2 &t1 = texcoords[(atoi(indices1[1].c_str()) - 1)];
			glm::vec2 &t2 = texcoords[(atoi(indices2[1].c_str()) - 1)];
			glm::vec2 &t3 = texcoords[(atoi(indices3[1].c_str()) - 1)];
			glm::vec3 n = normals[(atoi(indices1[2].c_str()) - 1)]; //TODO: don't only use the first normal

			Vertex v;
			glm::vec4 tangent(calcTangentVector(p1,p2,p3,t1,t2,t3,n));

			v.tangent = glm::vec3(tangent);
			v.biTangent = glm::cross(v.tangent, v.normal);
			v.color = glm::vec4(rand() / (float)RAND_MAX, rand() / (float)RAND_MAX, rand() / (float)RAND_MAX, 1);
			for(size_t ii = 4; ii <= params.size(); ii++)
			{
				for(size_t i = ii-3; i < ii; i++)
				{
					std::vector<std::string> indices = split(params[i == (ii-3) ? 1 : i], "/");

					if(indices.size() >= 1)
						v.position = vertices[(atoi(indices[0].c_str())-1)];
					if(indices.size() == 2) //texture 
						v.texCoord = texcoords[(atoi(indices[1].c_str())-1)];
					if(indices.size() == 3)
					{
						if( indices[1] != "")
							v.texCoord = texcoords[(atoi(indices[1].c_str())-1)];
						v.normal = normals[(atoi(indices[2].c_str())-1)];
					}

					finalVertices.push_back(v);
					currentGroup->end = finalVertices.size();
				}
			}
		}
		else if(params[0] == "s")
		{
		}
        else if(params[0] == "mtllib")
        {
            loadMaterialFile(dirName + "/" + params[1], dirName);
        }
		else if(params[0] == "usemtl")
		{
			if(currentGroup->end != -1)
				groups.push_back(currentGroup);
			currentGroup = new ObjGroup();
			currentGroup->start = finalVertices.size();
			currentGroup->materialIndex = -1;

			for(size_t i = 0; i < materials.size(); i++)
			{
				MaterialInfo* info = materials[i];
				if(info->name == params[1])
				{
					currentGroup->materialIndex = i;
					break;
				}
			}
			if(currentGroup->materialIndex == -1)
				std::cout<<"Could not find material name "<<params[1]<<std::endl;
		}
		//else
		//	std::cout<<line<<std::endl;

	}

	groups.push_back(currentGroup);


    glGenVertexArrays(1, &_vertexArray);
    glBindVertexArray(_vertexArray);
        
    GLuint _vertexBuffer;
    glGenBuffers(1, &_vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, finalVertices.size()*sizeof(Vertex), &finalVertices[0], GL_STATIC_DRAW);
        

	Vertex v;
    glEnableVertexAttribArray(0); //position
	glEnableVertexAttribArray(1); //
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);
	glEnableVertexAttribArray(4);
	glEnableVertexAttribArray(5);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), BUFFER_OFFSET(v, v.position));
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), BUFFER_OFFSET(v, v.color));
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), BUFFER_OFFSET(v, v.texCoord));
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), BUFFER_OFFSET(v, v.normal));
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), BUFFER_OFFSET(v, v.tangent));
	glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), BUFFER_OFFSET(v, v.biTangent));

    glBindVertexArray(0);        
	glBindBuffer(GL_ARRAY_BUFFER, 0);


}


ObjModel::~ObjModel(void)
{
}




void ObjModel::draw(Texture* texOverride)
{
    glBindVertexArray(_vertexArray);
	for(size_t i = 0; i < groups.size(); i++)
	{
		ObjGroup* group = groups[i];
		MaterialInfo* material = materials[group->materialIndex];
		if(material->hasTexture)
			material->texture->bind();
		glActiveTexture(GL_TEXTURE0);
		if (texOverride != nullptr)
			texOverride->bind();
		else if(material->bumpMap != NULL)
			material->bumpMap->bind();
		else
			material->texture->bind();
		glActiveTexture(GL_TEXTURE0);

		glDrawArrays(GL_TRIANGLES, group->start, group->end - group->start);
	}
	glBindVertexArray(0);

}

void ObjModel::loadMaterialFile( std::string fileName, std::string dirName )
{
	std::ifstream pFile(fileName.c_str());
	if (!pFile.is_open())
	{
		std::cout << "Error opening material file " << dirName << "/" << fileName << std::endl;
	}

	MaterialInfo* currentMaterial = NULL;

	while(!pFile.eof())
	{
		std::string line;
		std::getline(pFile, line);
		
		line = replace(line, "\t", " ");
		while(line.find("  ") != std::string::npos)
			line = replace(line, "  ", " ");
		if(line == "")
			continue;
		if(line[0] == ' ')
			line = line.substr(1);
		if(line == "")
			continue;
		if(line[line.length()-1] == ' ')
			line = line.substr(0, line.length()-1);
		if(line == "")
			continue;
		if(line[0] == '#')
			continue;

		std::vector<std::string> params = split(line, " ");
		params[0] = toLower(params[0]);

		if(params[0] == "newmtl")
		{
			if(currentMaterial != NULL)
			{
				materials.push_back(currentMaterial);
			}
			currentMaterial = new MaterialInfo();
			currentMaterial->name = params[1];
		}
		else if(params[0] == "map_kd")
		{
			currentMaterial->hasTexture = true;
			currentMaterial->texture = new Texture(trim(dirName + "/" + params[1]));
		}
		else if(params[0] == "map_bump")
		{
			currentMaterial->bumpMap = new Texture(trim(dirName + "/" + params[1]));
		}
//		else
//			std::cout<<"Didn't parse "<<params[0]<<" in material file"<<std::endl;
	}
	if(currentMaterial != NULL)
		materials.push_back(currentMaterial);

}

ObjModel::MaterialInfo::MaterialInfo()
{
	texture = NULL;
	bumpMap = NULL;
	hasTexture = false;
}
