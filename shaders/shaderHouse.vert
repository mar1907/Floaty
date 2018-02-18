#version 410 core

layout(location=0) in vec3 vPosition;
layout(location=1) in vec3 vNormal;
layout(location=2) in vec2 vTexCoords;

out VS_OUT {
    vec2 texCoords;
	vec3 vnormal;
	vec4 vfragPos;
	vec3 vvPositions;
	vec4 vfragPosLightSpace;
} vs_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceTrMatrix;

void main() 
{
	//compute eye space coordinates
	vs_out.vfragPos =  model * vec4(vPosition, 1.0f);
	vs_out.vnormal = vNormal;
	vs_out.vvPositions = vPosition;
	vs_out.vfragPosLightSpace = lightSpaceTrMatrix * model * vec4(vPosition, 1.0f);
	gl_Position = projection * view * model * vec4(vPosition, 1.0f);
	vs_out.texCoords = vTexCoords;
}
