#version 330

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

  //Calculate depth
  float bias = 0.005;

  float closestDepth = texture(s_shadowmap, vec3(shadowPos.xy, (shadowPos.z - bias) / shadowPos.w));
  float currentDepth = shadowPos.z;

  float shadow = 0.0;
  vec2 texelSize = 1.0 / textureSize(s_shadowmap, 0);
  bool showShadow = true;
  for (int i = 0; i < amountOfLights; i++) {
    float s = 0.0;
    for (int x = -1; x <= 1; ++x) {
      for (int y = -1; y <= 1; ++y) {
        switch (i) {
        case 0:
          s += texture(s_shadowmap, vec3(shadowPos.xy + vec2(x, y) * texelSize, (shadowPos.z - bias) / shadowPos.w));
          break;
        case 1:
          s += texture(s_shadowmap1, vec3(shadowPos1.xy + vec2(x, y) * texelSize, (shadowPos1.z - bias) / shadowPos1.w));
          break;
        case 2:
          s += texture(s_shadowmap2, vec3(shadowPos2.xy + vec2(x, y) * texelSize, (shadowPos2.z - bias) / shadowPos2.w));
          break;
        case 3:
          s += texture(s_shadowmap3, vec3(shadowPos3.xy + vec2(x, y) * texelSize, (shadowPos3.z - bias) / shadowPos3.w));
          break;
        }
      }
    }
    s /= 9.0;
    if (s == 1) {
      showShadow = false;
    }
    shadow += s;
  }
  shadow /= amountOfLights;
  vec4 tex = texture2D(s_texture, texCoord);

  if (showShadow)
    fragColor.rgb = (shadow) * tex.rgb;
  else
    fragColor.rgb = tex.rgb;
  fragColor.a = tex.a;
}