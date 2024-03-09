#version 330

uniform sampler2D s_texture;
uniform sampler2DShadow s_shadowmap;

in vec2 texCoord;
in vec3 normal;
in vec4 shadowPos;
out vec4 fragColor;

uniform vec3 lightPos;

void main()
{


	// perform perspective divide
    vec3 projCoords = shadowPos.xyz / shadowPos.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;

	//Calculate depth
	float bias = 0.005;

	float closestDepth = texture( s_shadowmap, vec3(shadowPos.xy,  (shadowPos.z - bias)/shadowPos.w));
	float currentDepth = shadowPos.z;

	float shadow = currentDepth >= closestDepth  ? 1.0 : 0.0;
	if(projCoords.z > 1.0)
        shadow = 0.0;

	vec4 tex = texture2D(s_texture, texCoord);

	fragColor.rgb = (1.0-shadow) * tex.rgb;
	fragColor.a = tex.a;
}
