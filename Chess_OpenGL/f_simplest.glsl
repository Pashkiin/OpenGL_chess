#version 330

uniform sampler2D textureMap; 
uniform float mixParam;

out vec4 pixelColor;

in vec4 ic; 
in vec4 n;
in vec4 l;
in vec4 v;
in vec2 iTexCoord;

in vec4 n2;
in vec4 l2;
in vec4 v2;


void main(void) {

	//Znormalizowane interpolowane wektory
	vec4 ml = normalize(l);
	vec4 mn = normalize(n);
	vec4 mv = normalize(v);
	//Wektor odbity
	vec4 mr = reflect(-ml, mn);
	//Obliczenie modelu oświetlenia
	float nl = clamp(dot(mn, ml), 0, 1);
	float rv = pow(clamp(dot(mr, mv), 0, 1), 50);

	//Parametry powierzchni
	vec4 kd = mix(ic, texture(textureMap, iTexCoord), mixParam);
	vec4 ks = vec4(1, 1, 1, 1);
	
	pixelColor = vec4(kd.rgb * nl * vec3(1.0f, 1.0f, 1.0f), kd.a) + vec4(ks.rgb*rv* vec3(1.0f, 1.0f, 1.0f), 0);


	// EXPERIMENTAL SECOND LIGHT
	vec4 ml2 = normalize(l2);
	vec4 mn2 = normalize(n2);
	vec4 mv2 = normalize(v2);
	vec4 mr2 = reflect(-ml2, mn2);
	float nl2 = clamp(dot(mn2, ml2), 0, 1);
	float rv2 = pow(clamp(dot(mr2, mv2), 0, 1), 50);

	vec3 lightColour2 = vec3(1.0f, 0.6f, 0.2f);
	float dimm = 0.2;
	
	pixelColor += vec4(kd.rgb * nl2 * lightColour2 * dimm, kd.a) + vec4(ks.rgb *rv2 * lightColour2 * dimm, 0);
}
