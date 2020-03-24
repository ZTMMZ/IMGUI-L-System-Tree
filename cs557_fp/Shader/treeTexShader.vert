#version 330 compatibility
uniform float uLightX, uLightY, uLightZ;
flat out vec3 vNf;
out vec3 vNs;
flat out vec3 vLf;
out vec3 vLs;
flat out vec3 vEf;
out vec3 vEs;
out vec2 vST;

out vec4 vECposition;
out vec3 vLightPos;

const float PI = 3.14159265;
const float DELTA = 1./500.;
void main( ) {
	
	vLightPos = vec3( uLightX, uLightY, uLightZ );

	// Vertex in Eye Coordination
	vECposition = gl_ModelViewMatrix * gl_Vertex;
	
	// Vector from the Point to the Light Position
	vLf = vLightPos - vECposition.xyz;
	vLs = vLf;
	
	// Vector from the Point to the Eye Position 
	vEf = vec3( 0., 0., 0. ) - vECposition.xyz;
	vEs = vEf;
	
	// Vertex Color
	vST = gl_MultiTexCoord0.st;

	// Surface normal T
	vNf = normalize( gl_NormalMatrix * gl_Normal );	
	vNs = vNf;


	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;

}
