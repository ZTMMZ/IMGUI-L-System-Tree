#version 330 compatibility
uniform int uIterations;
uniform int uPower;
uniform vec3 uColor0;
uniform vec3 uColor1;
uniform float uCX;
uniform float uCY;
uniform int uMORJ;
uniform float uGamma;
uniform float uColorOffset;
in vec2 vST;

vec2 c = vec2(uCX, uCY);
vec2 complexSquare(vec2 c, int power){
	vec2 tmp = c;
	for (int i=2; i<=power; i++){
		tmp = vec2(tmp.x * c.x - tmp.y * c.y, tmp.x * c.y + tmp.y  * c.x);
	}
	return tmp;
}

float fractalM(vec2 c) {
	vec2 z = vec2(0,0);
	int i = 0;
	for (; i < uIterations; i++){
		vec2 nextZ = complexSquare(z,uPower) + c;
		if((nextZ.x * nextZ.x + nextZ.y * nextZ.y) > 4.0) break;
		z=nextZ;
	}
	return float(i)/float(uIterations);
}

float fractalJ(vec2 z){
	int i = 0;
	for(; i<uIterations; i++) {
		vec2 nextZ = complexSquare(z,uPower) + c;
		if((nextZ.x * nextZ.x + nextZ.y * nextZ.y) > 4.0) break;
		z=nextZ;
	}
	return float(i)/float(uIterations);
}

float distance(vec2 a, vec2 b) {
	return sqrt(pow(a[0]-b[0],2)+pow(a[1]-b[1],2));
}

void main( void ) {
	vec2 pos = vec2((vST.s-0.5)*4,(vST.t-0.5)*4);
	if(uMORJ==0){
		float grey = pow((1.0-uColorOffset*distance(vST, vec2(0.5))) * fractalM(pos), 1.0/uGamma);
		gl_FragColor = vec4( mix(uColor0, uColor1, grey), 1.0 );
	} else if (uMORJ==1){
		float grey = pow((1.0-uColorOffset*distance(vST, vec2(0.5))) * fractalJ(pos), 1.0/uGamma);
		gl_FragColor = vec4( mix(uColor0, uColor1, grey), 1.0 );
	}
}
