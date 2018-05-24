#version 330

layout(location = 0) in vec3 Position;
layout(location = 1) in vec4 Color;
layout(location = 2) in vec3 Normal;
layout(location = 3) in vec2 TexCoord;

uniform mat4 gWorld;
uniform vec3 gEye;

out vec2 TexCoord0;
out vec4 Color0;
out vec3 Normal0;
out float LightMag;

void main()
{
	gl_Position = gWorld * vec4(Position, 1.0);
	TexCoord0 = TexCoord;
	Color0 = Color;
	Normal0 = Normal;
	LightMag = clamp(dot(Normal0, gEye), 0.5f, 1.0f);
}
