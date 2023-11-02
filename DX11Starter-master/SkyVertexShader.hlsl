struct VertexToPixel
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
	float4 screenPosition : SV_POSITION;
	float3 sampleDirection : DIRECTION;
};

struct VertexShaderInput
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
	float3 localPosition : POSITION; // XYZ position
	float3 normal : NORMAL;
	float2 uv : TEXCOORD;
	float3 tangent : TANGENT;
};

cbuffer ExternalData : register(b0)
{
	matrix view;
	matrix projection;
}


float4 main(VertexShaderInput input) : SV_POSITION
{
	return input.localPosition;
}