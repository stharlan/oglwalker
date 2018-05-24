#version 330

layout(location = 0) in vec3 Position;
layout(location = 1) in vec4 Color;
layout(location = 2) in vec3 Normal;
layout(location = 3) in vec2 TexCoord;

//uniform mat4 gWorld;
//uniform vec3 gEye;
uniform mat4 gPerspectiveViewMatrix;
uniform mat4 gModelMatrix;
uniform vec3 gLightPos;

out vec2 TexCoord0;
out vec4 Color0;
out float LightMag;

float log10( in float n ) { 
	const float kLogBase10 = 1.0 / log2( 10.0 ); 
	return log2( n ) * kLogBase10; 
} 

void main()
{
	// transform the position using the model matrix
	vec4 TransPos4 = gModelMatrix * vec4(Position, 1.0);

	// transform the normal also, 4th dim must be zero on vector transform?
	vec3 TransNormal = normalize((gModelMatrix * vec4(Normal, 0.0)).xyz);

	// get the vector from the light to the transformed position
	vec3 NormLightVector = normalize(gLightPos - (TransPos4).xyz);

	// transform the position to "eye" coords
	gl_Position = gPerspectiveViewMatrix * TransPos4;
	TexCoord0 = TexCoord;
	Color0 = Color;

	// distance between light and pos
	// subtract from 10 to invert
	// clamp 1 to 10
	// log10 to 0 to 1
	float lmult = clamp(log10(20.0f - distance(gLightPos, (TransPos4).xyz)),0.0f,1.0f);

	// if multiple lights, sum the light mag's
	// and max to zero
	// result of dot'ing these two will be -1 to 1
	LightMag = max(dot(NormLightVector, TransNormal),0.0f) * lmult;

}
