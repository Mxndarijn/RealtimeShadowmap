#version 330

in vec3 a_position;
in vec3 a_normal;
in vec2 a_texture;

out vec2 texCoord;
out vec3 normal;
out vec4 shadowPos;
out vec4 shadowPos1;
out vec4 shadowPos2;
out vec4 shadowPos3;

uniform mat4 modelMatrix;
uniform mat4 projectionMatrix;
uniform mat4 shadowMatrix;
uniform mat4 shadowMatrix1;
uniform mat4 shadowMatrix2;
uniform mat4 shadowMatrix3;
uniform mat4 viewMatrix;

uniform int amountOfLights;

mat4 biasMatrix = mat4(
  0.5, 0.0, 0.0, 0.0,
  0.0, 0.5, 0.0, 0.0,
  0.0, 0.0, 0.5, 0.0,
  0.5, 0.5, 0.5, 1.0
);

void main() {
    texCoord = a_texture;
    mat3 normalMatrix = mat3(viewMatrix * modelMatrix);
    normalMatrix = transpose(inverse(normalMatrix));
    for (int i = 0; i < amountOfLights; i++) {
      switch (i) {
      case 0:
        shadowPos = biasMatrix * shadowMatrix * vec4(a_position, 1.0);
        break;
      case 1:
       shadowPos1 = biasMatrix * shadowMatrix1 * vec4(a_position, 1.0);
        break;
      case 2:
        shadowPos2 = biasMatrix * shadowMatrix2 * vec4(a_position, 1.0);
        break;
      case 3:
        shadowPos3 = biasMatrix * shadowMatrix3 * vec4(a_position, 1.0);
        break;
      }
      normal = normalMatrix * a_normal;
      gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(a_position, 1.0);
    }
    }