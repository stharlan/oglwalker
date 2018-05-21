
Texture2D g_Texture2D : register(t0);
sampler TextureSampler : register(s0);
cbuffer VREND_CONST_BUFFER : register(b0)
{
	matrix mWorldViewProj;
};
cbuffer PREND_CONST_BUFFER : register(b1)
{
	float4 f4LightDiffuse;
	float4 f4LightAmbient;
	float3 f3LightDir;
	float f1;
}

//b Constant buffer
//t Texture and texture buffer
//c Buffer offset
//s Sampler
//u Unordered Access View

struct VOut
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
	float3 normal: NORMAL;
	float2 texcoord: TEXCOORD;
};

VOut VShader(float3 position : POSITION, float4 color : COLOR, float3 normal : NORMAL, float2 texcoord : TEXCOORD)
{
	VOut output;
	output.position = mul(float4(position, 1.0f), mWorldViewProj);
	output.color = color;
	output.normal = normal;
	output.texcoord = texcoord;
	return output;
}

//{
	//Filter = MIN_MAG_MIP_LINEAR;
	//AddressU = Wrap;
	//AddressV = Wrap;
//};

float4 PShader(float4 position : SV_POSITION, float4 color : COLOR, float3 normal : NORMAL, float2 texcoord : TEXCOORD) : SV_TARGET
{
	//float4 finalColor = color * f4LightAmbient;
	//finalColor += saturate(dot(f3LightDir, normal) * f4LightDiffuse * color);
	//return finalColor;

	float4 texColor = g_Texture2D.Sample(TextureSampler, texcoord);
	float4 finalColor = texColor * f4LightAmbient;
	finalColor += saturate(dot(f3LightDir, normal) * f4LightDiffuse * texColor);
	return finalColor;
	
	//vec4 TexColor = texture2D(gSampler, TexCoord0.xy);
	//FragColor = (TexColor * Ambient) + clamp(dot(Dir, Normal0) * Diffuse * Color0, 0.0f, 1.0f);


	//return g_Texture2D.Sample(TextureSampler, texcoord);
}
