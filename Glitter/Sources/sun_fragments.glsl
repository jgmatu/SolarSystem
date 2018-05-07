#version 330 core

in vec3 normal;
in vec3 fragPos;
in vec2 texCoord;

uniform sampler2D diffuseTexture;

out vec4 fragColor;

void main()
{
      vec3 lightColor = vec3(1.0, 1.0, 0.0); // Es una variable unform que va reflejado en la tierra y en la luna como el color del sol.
      vec3 lightPos = vec3(0.0, 0.0, 0.0); // 0 0 0 La luz solar
      vec3 viewPos = vec3(0.0, 0.0, -4.0); // Esto debe recogerlo de la camara...
      float ambientStrength = 1.2;
      vec3 ambient = ambientStrength * lightColor;

      // Diffuse...
      vec3 norm = normalize(normal);
      vec3 lightDir = normalize(lightPos - fragPos);
      float diff = max(dot(norm, lightDir), 0.0);
      vec3 diffuse = diff * lightColor;

      // Result fragments...
      vec3 result = (ambient + diffuse) * texture(diffuseTexture, texCoord).rgb;
      fragColor = vec4(result, 1.0);
}
