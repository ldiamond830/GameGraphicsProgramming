#include "ShaderUtils.hlsli"

cbuffer externalData : register(b0)
{
	matrix world;
	matrix view;
	matrix projection;
};

// A simplified vertex shader only used for the shadow map
float4 main(VertexShaderInput input) : SV_POSITION
{
matrix wvp = mul(projection, mul(view, world));
return mul(wvp, float4(input.localPosition, 1.0f));
}