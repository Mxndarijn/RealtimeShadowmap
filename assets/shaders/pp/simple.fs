#version 330
uniform sampler2D s_texture;
uniform sampler2D s_texture2;
uniform sampler2D s_texture3;
uniform sampler2D s_texture4;
uniform float time;

in vec2 texCoord;

void main()
{
    vec4 color = texture2D(s_texture, texCoord);
	gl_FragColor = color;
}