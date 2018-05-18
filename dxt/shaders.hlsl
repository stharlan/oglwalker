struct VOut
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
};

cbuffer REND_CONST_BUFFER : register(b0)
{
	matrix mWorldViewProj;
};

VOut VShader(float4 position : POSITION, float4 color : COLOR)
{
	VOut output;

	//output.position = position;
	output.position = mul(position, mWorldViewProj);
	output.color = color;

	return output;
}


float4 PShader(float4 position : SV_POSITION, float4 color : COLOR) : SV_TARGET
{
	return color;
}
