#version 410 core
out vec4 FragColor;

in vec3 TexCoords;
in vec3 fragNor;
in vec3 lightDir[1];
in vec3 EPos;
in float distance[1];

uniform samplerCube skybox;
uniform float MatShine;
uniform vec3 lightColor[1];
uniform float lightIntensity[1];
uniform int numLights;


void main() {
  vec4 texColor0 = texture(skybox, TexCoords);

  vec3 normal = normalize(fragNor);

  vec3 finalColor = vec3(0.0, 0.0, 0.0);

  for (int i = 0; i < numLights; ++i) {

  vec3 light = normalize(lightDir[i]);

  float dC = max(0.0, dot(normal, light));


  vec3 viewDir = normalize(-EPos);
  vec3 halfDir = normalize(light + viewDir);
  float sC = pow(max(dot(halfDir, normal), 0.0), MatShine);
  vec3 specular = sC * vec3(1.0, 1.0, 1.0) * lightColor[i] * lightIntensity[i];

  vec3 diffuse = dC * texColor0.rgb * lightColor[i] * lightIntensity[i];

  float attenuation = 1.0 / (1.0 + 0.045 * distance[i] + 0.0075 * distance[i] * distance[i]);

  diffuse *= attenuation;
  specular *= attenuation;

  finalColor += diffuse + specular;

  }



  vec3 result = texColor0.rgb;
  FragColor = vec4(result, texColor0.a);


  //FragColor = vec4(TexCoords, 1.0);
}
