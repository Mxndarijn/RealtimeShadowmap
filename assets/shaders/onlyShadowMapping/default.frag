#version 150

uniform sampler2D s_texture;
uniform sampler2DShadow s_shadowmap;

in vec2 texCoord;
in vec3 normal;
in vec4 shadowPos;
out vec4 fragColor;

uniform vec4 lightPos;

void main()
{
	float shadow = texture( s_shadowmap, vec3(shadowPos.xy,  (shadowPos.z)/shadowPos.w));


	vec4 tex = texture2D(s_texture, texCoord);

	fragColor.rgb = (shadow) * tex.rgb;
	fragColor.a = tex.a;
}
