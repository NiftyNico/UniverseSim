uniform sampler2D texture0;
uniform sampler2D texture1;
uniform sampler2D texture2;
uniform vec3 lightPositions[10];

varying vec2 fragTexCoords0;
varying vec2 fragTexCoords1;
varying vec3 normal;
varying vec3 objPos;

void main()
{
	vec3 n = normalize(normal);
    vec3 l = normalize(lightPositions[0] - objPos);
    vec3 e = normalize(vec3(0.0, 0.0, 0.0) - objPos);
    vec3 h = normalize(l + e);

	vec3 earthKd = texture2D(texture0, fragTexCoords0).rgb;
	vec3 earthKs = texture2D(texture1, fragTexCoords0).rgb;
	vec3 cloudKd = texture2D(texture2, fragTexCoords1).rgb;
	vec4 cloudColor = vec4(cloudKd, 1.0);
	vec4 planetColor = vec4(earthKd, 1.0) * max(dot(l, n), 0.0) + vec4(earthKs, 1.0) * pow(max(dot(h, n), 0.0), 20.0);

	gl_FragColor = cloudColor + planetColor;
}
