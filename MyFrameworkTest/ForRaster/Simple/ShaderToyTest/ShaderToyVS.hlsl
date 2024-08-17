struct VSInput
{
	float4 Position : POSITION;
	float2 UV : TEXCOORD0;
};

struct VSOutput
{
	float4 Position : SV_POSITION;
	float2 UV : TEXCOORD0;
};

VSOutput main(VSInput In)
{
	VSOutput result = (VSOutput)0;
	result.Position = In.Position;
	result.UV = In.UV;
	return result;
}