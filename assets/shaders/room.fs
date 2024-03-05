#version 330
uniform sampler2D s_texture;
uniform int tex = 1;

in vec2 texCoord;
in vec3 color;


layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec4 fragColor2;


void main()
{
	vec4 texColor = texture2D(s_texture, texCoord) * tex + (1-tex) * vec4(1,1,1,1);
	fragColor = texColor;
	fragColor2 = vec4(0,0,0,1);
}