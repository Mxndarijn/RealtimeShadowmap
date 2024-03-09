#version 150

uniform sampler2D s_texture;

in vec2 texCoord;
in vec3 normal;
out vec4 fragColor;


void main()
{


	vec4 tex = texture2D(s_texture, texCoord);

	fragColor.rgba = tex.rgba;
}
