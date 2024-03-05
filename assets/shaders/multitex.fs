uniform sampler2D s_texture;
uniform sampler2D s_texture2;
uniform float time;

in vec2 texCoord;
in vec3 color;


void main()
{
	vec4 col1 = texture2D(s_texture, texCoord);
	vec4 col2 = texture2D(s_texture2, texCoord);
	gl_FragColor = mix(col1, col2, clamp( (0.5 + 0.5*sin(time)), 0.0, 1.0));
}