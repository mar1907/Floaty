#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in VS_OUT {
    vec2 texCoords;
	vec3 vnormal;
	vec4 vfragPos;
	vec3 vvPositions;
	vec4 vfragPosLightSpace;
} gs_in[];

out vec2 passTexture; 
out vec3 normal;
out vec4 fragPos;
out vec3 vPositions;
out vec4 fragPosLightSpace;

uniform float time;
uniform int exp;

vec4 explode(vec4 position, vec3 normal)
{
    float magnitude = 5.0;
	if(exp==1){
		vec3 direction = normal * ((sin(time) + 1.0) / 2.0) * magnitude; 
		return position + vec4(direction, 0.0);
	}
	return position;
} 

vec3 GetNormal()
{
   vec3 a = vec3(gl_in[0].gl_Position) - vec3(gl_in[1].gl_Position);
   vec3 b = vec3(gl_in[2].gl_Position) - vec3(gl_in[1].gl_Position);
   return normalize(cross(a, b));
} 

void main() {    
    vec3 normal1 = GetNormal();

    gl_Position = explode(gl_in[0].gl_Position, normal1);
    passTexture = gs_in[0].texCoords;
	normal=gs_in[0].vnormal;
	fragPos=gs_in[0].vfragPos;
	vPositions=gs_in[0].vvPositions;
	fragPosLightSpace=gs_in[0].vfragPosLightSpace;
    EmitVertex();
    gl_Position = explode(gl_in[1].gl_Position, normal1);
    passTexture = gs_in[1].texCoords;
	normal=gs_in[1].vnormal;
	fragPos=gs_in[1].vfragPos;
	vPositions=gs_in[1].vvPositions;
	fragPosLightSpace=gs_in[1].vfragPosLightSpace;
    EmitVertex();
    gl_Position = explode(gl_in[2].gl_Position, normal);
    passTexture = gs_in[2].texCoords;
	normal=gs_in[2].vnormal;
	fragPos=gs_in[2].vfragPos;
	vPositions=gs_in[2].vvPositions;
	fragPosLightSpace=gs_in[2].vfragPosLightSpace;
    EmitVertex();
    EndPrimitive();
}  