#version 330

uniform sampler2D s_texture;
uniform sampler2DShadow s_shadowmap;
uniform sampler2DShadow s_shadowmap1;
uniform sampler2DShadow s_shadowmap2;
uniform sampler2DShadow s_shadowmap3;

in vec2 texCoord;
in vec3 normal;
out vec4 fragColor;
in vec4 shadowPos;
in vec4 shadowPos1;
in vec4 shadowPos2;
in vec4 shadowPos3;

uniform vec3 lightPos;

uniform int amountOfLights;

void main()
{
	

	// perform perspective divide
 //   vec3 projCoords = shadowPos.xyz / shadowPos.w;
    // transform to [0,1] range
 //   projCoords = projCoords * 0.5 + 0.5;

	//Calculate depth
	float bias = 0.005;

	float closestDepth = texture( s_shadowmap, vec3(shadowPos.xy,  (shadowPos.z - bias)/shadowPos.w));

	 float totalShadow = 0.0f;
  for (int i = 0; i < amountOfLights; i++) {
    switch (i) {
    case 0:
      totalShadow += texture(s_shadowmap, vec3(shadowPos.xy, (shadowPos.z - bias) / shadowPos.w));
      break;
    case 1:
      totalShadow += texture(s_shadowmap1, vec3(shadowPos1.xy, (shadowPos1.z - bias) / shadowPos1.w));
      break;
    case 2:
      totalShadow += texture(s_shadowmap2, vec3(shadowPos2.xy, (shadowPos2.z - bias) / shadowPos2.w));
      break;
    case 3:
      totalShadow += texture(s_shadowmap3, vec3(shadowPos3.xy, (shadowPos3.z - bias) / shadowPos3.w));
      break;
    }
  }
  totalShadow /= amountOfLights;
	float currentDepth = shadowPos.z;

//	if(projCoords.z > 1.0)
     //   shadow = 0.0;



	vec4 tex = texture2D(s_texture, texCoord);

	fragColor.rgb = (totalShadow) * tex.rgb;
	fragColor.a = tex.a;
}
