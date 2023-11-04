TextureCube CubeTexture : register(t0);
SamplerState BasicSampler : register(s0);

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

float4 main(VertexToPixel input) : SV_TARGET
{
	//return float4(input.sampleDirection, 1.0f);
	return CubeTexture.Sample(BasicSampler, input.sampleDirection);
}