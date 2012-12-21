// GLSL - Toon shader
varying vec3 normal;
varying vec3 lightDir;

void main()
{
	vec4 ecPos;
	vec3 aux;

	/* Compute the light's direction for positional w=0 lights */
	ecPos = gl_ModelViewMatrix * gl_Vertex;
	aux = vec3(gl_LightSource[0].position-ecPos);
	lightDir = normalize(aux);
		
	normal = gl_NormalMatrix * gl_Normal;

	gl_Position = ftransform();
}

