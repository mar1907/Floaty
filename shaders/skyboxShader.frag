#version 410 core

in vec3 textureCoordinates;
out vec4 color;

uniform samplerCube skybox;
uniform int neg;

void main()
{
	if(neg == 1){
		color = vec4(1.0f) - texture(skybox, textureCoordinates);
	}
	else{
		color = texture(skybox, textureCoordinates);
	}
}
