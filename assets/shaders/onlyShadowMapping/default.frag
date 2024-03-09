#version 150

uniform sampler2D s_texture;
uniform sampler2DShadow s_shadowmap[4];

uniform int amountOfLights;


in vec2 texCoord;
in vec3 normal;
in vec4 shadowPos[4];
out vec4 fragColor;

uniform vec4 lightPos;

void main()
{


	float totalShadow = 0;
	for(int i = 0; i < amountOfLights; i++) {
		float shadow = texture( s_shadowmap[i], vec3(shadowPos[i].xy,  (shadowPos[i].z)/shadowPos[i].w));

		totalShadow += shadow;
	}
	totalShadow /= amountOfLights;


	vec4 tex = texture2D(s_texture, texCoord);

	fragColor.rgb = (totalShadow) * tex.rgb;
	fragColor.a = tex.a;
}
