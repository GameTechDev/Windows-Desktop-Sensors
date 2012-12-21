// minimal.vert
#version 150 core

uniform mat4 ProjectionMatrix;
uniform mat4 ModelViewMatrix;

in vec3 in_Position;
in vec2 in_UV;

out vec2 texCoord0;

void main(void)
{

	  gl_Position =  ProjectionMatrix * ModelViewMatrix * vec4(in_Position, 1.0);
	  texCoord0 = in_UV;
}