#version 330

in vec3 a_position;
in vec3 a_normal;
in vec2 a_texture;

out vec2 texCoord;
out vec3 normal;
out vec4 shadowPos[4];

uniform mat4 modelMatrix;
uniform mat4 projectionMatrix;
uniform mat4 shadowMatrix[4];
uniform mat4 viewMatrix;

uniform int amountOfLights;

mat4 biasMatrix = mat4(
0.5, 0.0, 0.0, 0.0,
0.0, 0.5, 0.0, 0.0,
0.0, 0.0, 0.5, 0.0,
0.5, 0.5, 0.5, 1.0
);

void main()
{
texCoord = a_texture;
	mat3 normalMatrix = mat3(viewMatrix * modelMatrix);
	normalMatrix = transpose(inverse(normalMatrix));
	for(int i = 0; i < amountOfLights; i++) {
        shadowPos[i] = biasMatrix * shadowMatrix[i] * vec4(a_position, 1.0);
    }
	normal = normalMatrix * a_normal;
	gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(a_position,1.0);
}
