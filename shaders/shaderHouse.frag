#version 410 core

in vec3 normal;
in vec4 fragPos;
in vec2 passTexture;
in vec3 vPositions;
in vec4 fragPosLightSpace;

uniform vec3 viewPos;

uniform vec3 lightDir;
uniform vec3 lightColor;

uniform vec3 posLightColor;
uniform vec3 posLightPos;

uniform sampler2D diffuseTexture;
uniform sampler2D shadowMap;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform mat3 normalMatrix;
uniform mat3 lightDirMatrix;

uniform vec3 ambientLight;
uniform vec3 diffuseLight;
uniform vec3 specularLight;

uniform int neg;
uniform int point;

vec3 ambient;
float ambientStrength = 0.5f;
vec3 diffuse;
vec3 specular;
float specularStrength = 0.5f;
vec3 color;
float pointSpecularStrenght = 0.0f;

out vec4 fColor;

vec3 test;

float constant = 1.0f;
float linear = 0.0045f;
float quadratic = 0.0075f;

float pointLinear = 0.7f;
float pointQuadratic = 0.3f;

float computeFog()
{
 float fogDensity = 0.09f;
 float fragmentDistance = length(view * fragPos);
 float fogFactor = exp(-pow(fragmentDistance * fogDensity, 2));

 return clamp(fogFactor, 0.0f, 1.0f);
}


void computePointLight()
{
	vec3 norm = normalize(normalMatrix * normal);
	vec3 lightDir = normalize(posLightPos - fragPos.xyz);

    // diffuse shading
    float diff = max(dot(norm, lightDir), 0.0);

    // specular shading
	vec3 viewDir = normalize(viewPos - fragPos.xyz);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);

    // attenuation
    float distance    = length(posLightPos - fragPos.xyz);
    float attenuation = 1.0 / (constant + pointLinear * distance + 
  			     pointQuadratic * (distance * distance));    

    // combine results
    ambient  += ambientLight * vec3(texture(diffuseTexture, passTexture)) * attenuation;
    diffuse  += diffuseLight * diff * vec3(texture(diffuseTexture, passTexture)) * attenuation;
    specular += specularLight * spec * pointSpecularStrenght * attenuation;
}

void computeLight()
{
	//compute eye space coordinates
	vec4 vertPosEye = view * model * vec4(vPositions, 1.0f);
	vec3 normalEye = normalize(normalMatrix * normal);
	
	//normalize light direction
	vec3 lightDirN = normalize(lightDirMatrix * lightDir);
	
	//compute view direction (in eye coordinates, the viewer is situated at the origin
	vec3 viewDir = normalize(- vertPosEye.xyz);
	
	//compute ambient light
	ambient = ambientStrength * ambientLight;
	
	//compute diffuse light
	diffuse = max(dot(normalEye, lightDirN), 0.0f) * diffuseLight;
	
	//compute specular light
	vec3 reflectDir = reflect(-lightDirN, normalEye);
	float specCoeff = pow(max(dot(viewDir, reflectDir), 0.0f), 32);
	specular = specularStrength * specCoeff * specularLight;
}

float computeShadow()
{	
	// perform perspective divide
    vec3 normalizedCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    if(normalizedCoords.z > 1.0f)
        return 0.0f;
    // Transform to [0,1] range
    normalizedCoords = normalizedCoords * 0.5f + 0.5f;
    // Get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, normalizedCoords.xy).r;    
    // Get depth of current fragment from light's perspective
    float currentDepth = normalizedCoords.z;
    // Check whether current frag pos is in shadow
    float bias = 0.05f;
    float shadow = currentDepth - bias> closestDepth  ? 1.0f : 0.0f;

    return shadow;	
}


void main() 
{
	float shadow = computeShadow();
	computeLight();
	ambient *= texture(diffuseTexture, passTexture);
	diffuse *= texture(diffuseTexture, passTexture);
	if(point==1){
		computePointLight();
	}

	vec3 color = min((ambient + (1.0f - shadow)*diffuse) + (1.0f - shadow)*specular, 1.0f);
	if(neg == 1){
		fColor = vec4(1.0f) - vec4(color, 1.0f);
	}
	else{
		fColor = vec4(color, 1.0f);
	}

	float fogFactor = computeFog();
	vec4 fogColor = vec4(0.5f, 0.5f, 0.5f, 1.0f);
	fColor = mix(fogColor, fColor, fogFactor);
}
