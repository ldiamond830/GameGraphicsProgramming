#include "ShaderUtils.hlsli"

Texture2D SurfaceTexture : register(t0); // "t" registers for textures
SamplerState BasicSampler : register(s0); // "s" registers for sampler

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
	float3 surfaceColor = pow(SurfaceTexture.Sample(BasicSampler, (input.uv + textureOffset) * textureScale).rgb, 2.2f);
	surfaceColor *= colorTint;

	float3 lightSum = CalcAllLights(lights, surfaceColor, input.normal, cameraPosition, input.worldPosition, roughness, 0.0f, 0.0f);

	//calculate final color and gamma correct it to be linear
	float3 finalColor =  pow(lightSum + (ambient * surfaceColor), 1.0f/2.2f);
	
	return float4(finalColor, 1.0f);
}