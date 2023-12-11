#include "ShaderUtils.hlsli"

Texture2D Albedo : register(t0); // "t" registers for textures
Texture2D NormalMap : register(t1);
Texture2D RoughnessMap : register(t2);
Texture2D MetalnessMap : register(t3);
Texture2D ShadowMap : register(t4);
Texture2D RampTexture : register(t5);
SamplerState SamplerOptions : register(s0); // "s" registers for sampler
SamplerState ClampSampler : register(s1); // "s" registers for sampler
SamplerComparisonState ShadowSampler : register(s1);

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
	//return float4(RampTexture.Sample(SamplerOptions, input.uv).r, 0, 0, 1);
	// Perform the perspective divide (divide by W) ourselves
	input.shadowMapPos /= input.shadowMapPos.w;
	// Convert the normalized device coordinates to UVs for sampling
	float2 shadowUV = input.shadowMapPos.xy * 0.5f + 0.5f;
	shadowUV.y = 1 - shadowUV.y; // Flip the Y
	// Grab the distances we need: light-to-pixel and closest-surface
	float distToLight = input.shadowMapPos.z;

	// Get a ratio of comparison results using SampleCmpLevelZero()
	float shadowAmount = ShadowMap.SampleCmpLevelZero(ShadowSampler, shadowUV, distToLight).r;

	float3 surfaceColor = pow(Albedo.Sample(SamplerOptions, (input.uv + textureOffset) * textureScale).rgb, 2.2f);
	surfaceColor *= colorTint;

	float3 unpackedNormal = NormalMap.Sample(SamplerOptions, input.uv).rgb * 2 - 1;
	unpackedNormal = normalize(unpackedNormal);

	float3 N = normalize(input.normal); // Must be normalized here or before
	float3 T = normalize(input.tangent); // Must be normalized here or before
	T = normalize(T - N * dot(T, N)); // Gram-Schmidt assumes T&N are normalized!
	float3 B = cross(T, N);
	float3x3 TBN = float3x3(T, B, N);

	input.normal = mul(unpackedNormal, TBN);

	float roughness = RoughnessMap.Sample(SamplerOptions, input.uv).r;
	float metalness = MetalnessMap.Sample(SamplerOptions, input.uv).r;
	// Assume albedo texture is actually holding specular color where metalness == 1
	// Note the use of lerp here - metal is generally 0 or 1, but might be in between
	// because of linear texture sampling, so we lerp the specular color to match
	float3 specular = lerp(F0_NON_METAL, surfaceColor.rgb, metalness);

	//float3 lightSum = CalcAllLightsCel(lights, surfaceColor, input.normal, cameraPosition, input.worldPosition, roughness, metalness, specular, shadowAmount, RampTexture, ClampSampler);
	float diffuse = Diffuse(input.normal, normalize(-lights[0].direction));
	diffuse = RampTexture.Sample(ClampSampler, float2(diffuse, 0));
	float spec = Specular(cameraPosition, input.worldPosition, normalize(cameraPosition - input.worldPosition), input.normal, 0.5f);
	float3 lightSum = (diffuse * surfaceColor + spec) * lights[0].intensity * lights[0].color;
	float3 finalColor = pow(lightSum, 1 / 2.2f);

	return float4(finalColor, 1.0f);
}