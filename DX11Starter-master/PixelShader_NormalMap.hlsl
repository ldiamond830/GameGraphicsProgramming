#include "ShaderUtils.hlsli"

Texture2D Albedo : register(t0); // "t" registers for textures
Texture2D NormalMap : register(t1);
Texture2D RoughnessMap : register(t2);
Texture2D MetalnesMap : register(t3);
SamplerState BasicSampler : register(s0); // "s" registers for sampler

cbuffer buffer : register(b0) {
	float3 colorTint;
	float3 cameraPosition;
	float3 ambient;
	Light directionalLight1;
	Light directionalLight2;
	Light directionalLight3;
	Light pointLight1;
	Light pointLight2;
	Light lights[5];
	float textureScale;
	float textureOffset;
}


// --------------------------------------------------------
// The entry point (main method) for our pixel shader
// 
// - Input is the data coming down the pipeline (defined by the struct)
// - Output is a single color (float4)
// - Has a special semantic (SV_TARGET), which means 
//    "put the output of this into the current render target"
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
float4 main(VertexToPixelNormalMap input) : SV_TARGET
{
	

	float3 surfaceColor = pow(Albedo.Sample(BasicSampler, (input.uv + textureOffset) * textureScale).rgb, 2.2f);
	surfaceColor *= colorTint;

	float3 unpackedNormal = NormalMap.Sample(BasicSampler, input.uv).rgb * 2 - 1;
	unpackedNormal = normalize(unpackedNormal);

	float3 N = normalize(input.normal); // Must be normalized here or before
	float3 T = normalize(input.tangent); // Must be normalized here or before
	T = normalize(T - N * dot(T, N)); // Gram-Schmidt assumes T&N are normalized!
	float3 B = cross(T, N);
	float3x3 TBN = float3x3(T, B, N);

	input.normal = mul(unpackedNormal, TBN);

	float roughness = RoughnessMap.Sample(SamplerOptions, input.uv).r;
	float metalness = MetalnessMap.Sample(SamplerOptions, input.uv).r;

	float3 lightSum = CalcAllLights(lights, surfaceColor, input.normal, cameraPosition, input.worldPosition, roughness);
	float3 finalColor = pow(lightSum + (ambient * surfaceColor), 1/2.2f);

	return float4(finalColor, 1.0f);
}