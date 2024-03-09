#version 330

in vec3 a_position;
in vec3 a_normal;
in vec2 a_texture;

out vec2 texCoord;
out vec3 normal;

uniform mat4 modelMatrix;
uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;

void main()
{
texCoord = a_texture;
	mat3 normalMatrix = mat3(viewMatrix * modelMatrix);
	normalMatrix = transpose(inverse(normalMatrix));

	normal = normalMatrix * a_normal;
	gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(a_position,1.0);
}
