#include "ShaderUtils.hlsli"

Texture2D Albedo : register(t0); // "t" registers for textures
Texture2D RoughnessMap : register(t1);
Texture2D MetalnessMap : register(t2);
SamplerState SamplerOptions : register(s0); // "s" registers for sampler

cbuffer buffer : register(b0) {
	float3 colorTint;
	float3 cameraPosition;
	float roughness;
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
float4 main(VertexToPixel input) : SV_TARGET
{
	input.normal = normalize(input.normal);

	//"uncorrect" the gamma of the sampled texture color
	float3 surfaceColor = pow(Albedo.Sample(SamplerOptions, (input.uv + textureOffset) * textureScale).rgb, 2.2f);
	surfaceColor *= colorTint;

	float roughness = RoughnessMap.Sample(SamplerOptions, input.uv).r;
	float metalness = MetalnessMap.Sample(SamplerOptions, input.uv).r;
	// Assume albedo texture is actually holding specular color where metalness == 1
	// Note the use of lerp here - metal is generally 0 or 1, but might be in between
	// because of linear texture sampling, so we lerp the specular color to match
	float3 specular = lerp(F0_NON_METAL, surfaceColor.rgb, metalness);

	float3 lightSum = CalcAllLights(lights, surfaceColor, input.normal, cameraPosition, input.worldPosition, roughness, metalness, specular);
	float3 finalColor = pow(lightSum, 1 / 2.2f);
	
	return float4(finalColor, 1.0f);
}