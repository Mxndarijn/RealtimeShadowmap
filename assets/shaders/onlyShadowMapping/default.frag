#version 150

uniform sampler2D s_texture;
uniform sampler2DShadow s_shadowmap;
uniform sampler2DShadow s_shadowmap1;
uniform sampler2DShadow s_shadowmap2;
uniform sampler2DShadow s_shadowmap3;

in vec2 texCoord;
in vec3 normal;
in vec4 shadowPos;
in vec4 shadowPos1;
in vec4 shadowPos2;
in vec4 shadowPos3;

out vec4 fragColor;

uniform int amountOfLights;

void main() {
  float totalShadow = 0.0f;
  for (int i = 0; i < amountOfLights; i++) {
    switch (i) {
    case 0:
      totalShadow += texture(s_shadowmap, vec3(shadowPos.xy, (shadowPos.z) / shadowPos.w));
      break;
    case 1:
      totalShadow += texture(s_shadowmap1, vec3(shadowPos1.xy, (shadowPos1.z) / shadowPos1.w));
      break;
    case 2:
      totalShadow += texture(s_shadowmap2, vec3(shadowPos2.xy, (shadowPos2.z) / shadowPos2.w));
      break;
    case 3:
      totalShadow += texture(s_shadowmap3, vec3(shadowPos3.xy, (shadowPos3.z) / shadowPos3.w));
      break;
    }
  }
  totalShadow /= amountOfLights;

  vec4 tex = texture2D(s_texture, texCoord);

  fragColor.rgb = (totalShadow) * tex.rgb;
  fragColor.a = tex.a;
}