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

uniform vec3 lightPos;

uniform int amountOfLights;

void main() {

  //Calculate depth
  float bias = 0.005;

  float closestDepth = texture(s_shadowmap, vec3(shadowPos.xy, (shadowPos.z - bias) / shadowPos.w));
  float currentDepth = shadowPos.z;

  float shadow = 0.0;
  vec2 texelSize = 1.0 / textureSize(s_shadowmap, 0);
  for (int i = 0; i < amountOfLights; i++) {
    for (int x = -1; x <= 1; ++x) {
      for (int y = -1; y <= 1; ++y) {
        switch (i) {
        case 0:
          shadow += texture(s_shadowmap, vec3(shadowPos.xy + vec2(x, y) * texelSize, (shadowPos.z - bias) / shadowPos.w));
          break;
        case 1:
          shadow += texture(s_shadowmap1, vec3(shadowPos1.xy + vec2(x, y) * texelSize, (shadowPos1.z - bias) / shadowPos1.w));
          break;
        case 2:
          shadow += texture(s_shadowmap2, vec3(shadowPos2.xy + vec2(x, y) * texelSize, (shadowPos2.z - bias) / shadowPos2.w));
          break;
        case 3:
          shadow += texture(s_shadowmap3, vec3(shadowPos3.xy + vec2(x, y) * texelSize, (shadowPos3.z - bias) / shadowPos3.w));
          break;
        }
      }
    }
  }
  shadow /= 9.0 * amountOfLights;
  vec4 tex = texture2D(s_texture, texCoord);

  fragColor.rgb = (shadow) * tex.rgb;
  fragColor.a = tex.a;
}