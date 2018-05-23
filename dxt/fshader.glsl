#version 330

in vec2 TexCoord0;
in vec4 Color0;
in vec3 Normal0;
in float LightMag;

out vec4 FragColor;

uniform sampler2D gSampler;

void main()
{
	//FragColor = texture2D(gSampler, TexCoord0.xy);
	//FragColor = Color0;
	//FragColor = vec4(1,1,1,1);

	//float4 texColor = g_Texture2D.Sample(TextureSampler, texcoord);
	//float4 finalColor = texColor * f4LightAmbient;
	//finalColor += saturate(dot(f3LightDir, normal) * f4LightDiffuse * texColor);
	//return finalColor;

	//FragColor = vec4(1,1,1,1);
	//FragColor = dot(Dir, Normal0) * Diffuse * TexColor;

	vec4 Ambient = vec4(0.5f, 0.5f, 0.5f, 1.0f);
	vec3 Dir = vec3(0.5774f, 0.5774f, 0.5774f); // normalized 1,1,1
	vec4 Diffuse = vec4(1.0f, 1.0f, 1.0f, 1.0f);
	vec4 TexColor = texture2D(gSampler, TexCoord0.xy);
	FragColor = (TexColor * Ambient) + clamp(dot(Dir, Normal0) * Diffuse * Color0, 0.0f, 1.0f);

	//FragColor = LightMag * texture2D(gSampler, TexCoord0.xy);
}
