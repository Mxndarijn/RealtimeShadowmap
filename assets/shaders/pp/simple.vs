#version 330
layout(location=0) in vec3 a_position;
out vec2 texCoord;

void main()
{
	texCoord = (vec2(a_position) + vec2(1, 1)) / 2.0;
	gl_Position = vec4(a_position,1);
}