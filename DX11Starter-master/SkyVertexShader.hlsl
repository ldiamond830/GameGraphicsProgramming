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


VertexToPixel main(VertexShaderInput input) 
{
	VertexToPixel output;

	matrix viewNoTranslation = view;
	viewNoTranslation._14 = 0;
	viewNoTranslation._24 = 0;
	viewNoTranslation._34 = 0;

	matrix viewProjection = mul(projection, viewNoTranslation);
	output.screenPosition = mul(viewProjection, float4(input.localPosition, 1.0f));
	output.screenPosition.z = output.screenPosition.w;

	output.sampleDirection = input.localPosition;

	return output;
}