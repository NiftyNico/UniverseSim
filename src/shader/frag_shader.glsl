uniform sampler2D texture0;
uniform sampler2D texture1;
uniform sampler2D texture2;
uniform vec3 lightPositions[100];
uniform int numLights;
uniform float lightDistanceModifier;
uniform int isSun;

varying vec2 fragTexCoords0;
varying vec2 fragTexCoords1;
varying vec3 normal;
varying vec3 objPos;

void main()
{
	vec3 kd = texture2D(texture0, fragTexCoords0).rgb;
	vec3 atmosKd = texture2D(texture2, fragTexCoords1).rgb;
	vec4 theColor = vec4(0.0, 0.0, 0.0, 0.0);

	if (isSun != 0) {
		theColor = vec4(kd + atmosKd, 1.0);		
	} else {			
		vec3 n = normalize(normal);
		int i;
		for(i = 0; i < numLights; i++) {
	    	vec3 l = normalize(lightPositions[i] - objPos);
		    vec3 e = normalize(vec3(0.0, 0.0, 0.0) - objPos);
		    vec3 h = normalize(l + e);

			vec3 ks = texture2D(texture1, fragTexCoords0).rgb;

			float distL = abs(distance(objPos, l));
			float lightingMod = //1.0 / (1.5 + 0.2 * distL + lightDistanceModifier * pow(distL, 2.0));
			 distL / lightDistanceModifier;
			lightingMod = min(1.0, lightingMod);
			float lighting = lightingMod * max(dot(l, n), 0.0);

			vec4 atmosColor = vec4(atmosKd, 1.0) * lighting;
			vec4 mainColor = vec4(kd, 1.0) * lighting + vec4(ks, 1.0) * pow(max(dot(h, n), 0.0), 20.0);

			vec4 newColor = atmosColor + mainColor;
			float fNumInts = float(numLights);
			theColor += vec4(newColor.r / fNumInts, newColor.g / fNumInts, newColor.b / fNumInts, newColor.a / fNumInts);
		}
	}

	gl_FragColor = theColor;

}
