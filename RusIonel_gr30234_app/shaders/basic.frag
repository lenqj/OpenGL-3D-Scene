#version 410 core

in vec3 fPosition;
in vec3 fNormal;
in vec2 fTexCoords;

out vec4 fColor;

//matrices
uniform mat4 model;
uniform mat4 view;
uniform mat3 normalMatrix;
//lighting
uniform vec3 lightDir;
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 lightColor1;
uniform vec3 lightTargetPointPos;
// textures
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
//fog
uniform float fogDensity;

//components
vec3 ambient;
float ambientStrength = 0.1f;
vec3 diffuse;
vec3 specular;
float specularStrength = 0.3f;

float constant = 1.0f;
float linear = 0.025f;
float quadratic = 0.03f;
vec4 fragPosEye = vec4(fPosition, 1.0f);
vec4 fPosEye = view * model * vec4(fPosition, 1.0f);



//lamp lights

uniform vec3 lampLightPos0;
uniform vec3 lampLightColor0;
uniform vec3 lampLightTarget0;
uniform vec3 lampLightPos1;
uniform vec3 lampLightColor1;
uniform vec3 lampLightTarget1;

//shadow
in vec4 fragPosLightSpace;
uniform sampler2D shadowMap;




vec3 computeDirLight()
{
    //compute eye space coordinates

    vec3 normalEye = normalize(normalMatrix * fNormal);

    //normalize light direction
    vec3 lightDirN = vec3(normalize(view * vec4(lightDir, 0.0f)));

    //compute view direction (in eye coordinates, the viewer is situated at the origin
    vec3 viewDir = normalize(- fPosEye.xyz);

    //compute ambient light
    ambient = ambientStrength * lightColor;

    //compute diffuse light
    diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor;

    //compute specular light
    vec3 reflectDir = reflect(-lightDirN, normalEye);
    float specCoeff = pow(max(dot(viewDir, reflectDir), 0.0f), 32);
    specular = specularStrength * specCoeff * lightColor;
    return min((ambient + diffuse) * texture(diffuseTexture, fTexCoords).rgb + specular * texture(specularTexture, fTexCoords).rgb, 1.0f);
}

vec3 computeLampLight0() {
    vec3 normalEye = normalize(normalMatrix * fNormal);
    vec3 lightDirN = normalize(lampLightTarget0 - lampLightPos0);
    vec3 viewDirN = normalize(-fragPosEye.xyz);
    vec3 halfVector = normalize(lightDirN + viewDirN);
    float dist = length(lampLightPos0 - fragPosEye.xyz);
    float att = 1.0 / (constant + linear * dist + quadratic * (dist * dist));
    vec3 ambient = att * ambientStrength * lampLightColor0;
    float diffuseFactor = max(dot(normalEye, lightDirN), 0.0);
    vec3 diffuse = att * diffuseFactor * lampLightColor0;
    float specCoeff = pow(max(dot(normalEye, halfVector), 0.0), 3.0);
    vec3 specular = att * specularStrength * specCoeff * lampLightColor0;
    vec3 finalLight = mix(ambient, diffuse, diffuseFactor);
    finalLight += specular;
    return min(finalLight * texture(diffuseTexture, fTexCoords).rgb, 1.0);
}
vec3 computeLampLight1() {
    vec3 normalEye = normalize(normalMatrix * fNormal);
    vec3 lightDirN = normalize(lampLightTarget1 - lampLightPos1);
    vec3 viewDirN = normalize(-fragPosEye.xyz);
    vec3 halfVector = normalize(lightDirN + viewDirN);
    float dist = length(lampLightPos1 - fragPosEye.xyz);
    float att = 1.0 / (constant + linear * dist + quadratic * (dist * dist));
    vec3 ambient = att * ambientStrength * lampLightColor1;
    float diffuseFactor = max(dot(normalEye, lightDirN), 0.0);
    vec3 diffuse = att * diffuseFactor * lampLightColor1;
    float specCoeff = pow(max(dot(normalEye, halfVector), 0.0), 3.0);
    vec3 specular = att * specularStrength * specCoeff * lampLightColor1;
    vec3 finalLight = mix(ambient, diffuse, diffuseFactor);
    finalLight += specular;
    return min(finalLight * texture(diffuseTexture, fTexCoords).rgb, 1.0);
}

float computeFog()
{
    float fragmentDistance = length(fPosEye);
    float fogFactor = exp(-pow(fragmentDistance * fogDensity, 2));
    return clamp(fogFactor, 0.0f, 1.0f);
}


void main() 
{
    vec3 color = computeDirLight();
    color+=computeLampLight0();
    color+=computeLampLight1();
    
    float fogFactor = computeFog();
    vec4 fogColor = vec4(0.5f, 0.5f, 0.5f, 1.0f);
    fColor = mix(fogColor, vec4(color, 1.0f), fogFactor);
}
