#version 330 compatibility

uniform float uKa, uKd, uKs;
uniform vec4 uSpecularColor;
uniform vec4 uAmbientColor;
uniform vec4 uDiffuseColor;
uniform float uShininess;
uniform bool uFlat;
uniform sampler2D uTex;
uniform float uAttenuation;

flat in vec3 vNf;
in vec3 vNs;
flat in vec3 vLf;
in vec3 vLs;
flat in vec3 vEf;
in vec3 vEs;
in vec2 vST;
in vec4  vECposition;
in vec3  vLightPos;

void main( ) {

	gl_FragColor = texture2D(uTex, vST);
	if (gl_FragColor.a==0.) discard;
	//Lighting
	vec3 Normal;
	vec3 Light;
	vec3 Eye;

	if( uFlat ) {
		Normal = normalize(vNf);
		Light = normalize(vLf);
		Eye = normalize(vEf);
	} else {
		Normal = normalize(vNs);
		Light = normalize(vLs);
		Eye = normalize(vEs);
	}

	vec4 ambient = uKa * gl_FragColor * uAmbientColor;

	float attenuation = uAttenuation * (1.0 / distance(vLightPos,vECposition.xyz));
	float d = max( dot(Normal,Light), 0. );
	vec4 diffuse = uKd * d * gl_FragColor * uDiffuseColor * attenuation;

	float sp = 0.;
	if( dot(Normal,Light) > 0. ) {
		vec3 ref = normalize( 2. * Normal * dot(Normal,Light) - Light );
		sp = pow( max( dot(Eye,ref),0. ), uShininess );
	}
	vec4 specular = uKs * sp * uSpecularColor;

	gl_FragColor = vec4( ambient.rgb + diffuse.rgb + specular.rgb, gl_FragColor.a );

}