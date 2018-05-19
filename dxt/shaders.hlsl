
struct VOut
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
	float3 normal: NORMAL;
};

cbuffer REND_CONST_BUFFER : register(b0)
{
	matrix mWorldViewProj;
	float4 f4LightDiffuse;
	float4 f4LightAmbient;
	float3 f3LightDir;
	float f1;
};

VOut VShader(float3 position : POSITION, float4 color : COLOR, float3 normal : NORMAL)
{
	VOut output;
	float4 finalColor = color * f4LightAmbient;

	finalColor += saturate(dot(f3LightDir, normal) * f4LightDiffuse * color);
	output.position = mul(float4(position, 1.0f), mWorldViewProj);
	output.color = finalColor;
	output.normal = normal;
	return output;
}


float4 PShader(float4 position : SV_POSITION, float4 color : COLOR, float3 normal : NORMAL) : SV_TARGET
{
	return color;
}
