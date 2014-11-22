attribute vec4 vertPosition;
attribute vec3 vertNormal;
attribute vec2 vertTexCoords;
uniform mat4 P;
uniform mat4 MV;
uniform mat3 T;

varying vec2 fragTexCoords0;
varying vec2 fragTexCoords1;
varying vec3 normal;
varying vec3 objPos;

void main()
{
	gl_Position = P * MV * vertPosition;
	normal = vec3(MV * vec4(vertNormal, 0.0));
	objPos = vec3(MV * vertPosition);
	fragTexCoords0 = vertTexCoords;
	fragTexCoords1 = vec2(T * vec3(vertTexCoords, 1.0));
}