#include "ShaderUtils.hlsli"

Texture2D SurfaceTexture : register(t0); // "t" registers for textures
SamplerState BasicSampler : register(s0); // "s" registers for sampler

cbuffer colorTint : register(b0) {
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

	float3 surfaceColor = SurfaceTexture.Sample(BasicSampler, input.uv).rgb;
	surfaceColor *= colorTint;

	float3 lightSum = 0;
	for (int i = 0; i < 5; i++) {
		switch (lights[i].type) {
		case LIGHT_TYPE_DIRECTIONAL:
			lightSum += DirectionalLight(lights[i], surfaceColor, input.normal, cameraPosition, input.worldPosition, roughness);
			break;
		case LIGHT_TYPE_POINT:
			lightSum += PointLight(lights[i], surfaceColor, input.normal, cameraPosition, input.worldPosition, roughness);
			break;
		}
	}
	/*
		float3 sumDirectionalLights = DirectionalLight(directionalLight1, colorTint, input.normal, cameraPosition, input.worldPosition, roughness) +
		DirectionalLight(directionalLight2, colorTint, input.normal, cameraPosition, input.worldPosition, roughness) +
		DirectionalLight(directionalLight3, colorTint, input.normal, cameraPosition, input.worldPosition, roughness);

		float3 sumPointLights = PointLight(pointLight1, colorTint, input.normal, cameraPosition, input.worldPosition, roughness) +
			PointLight(pointLight2, colorTint, input.normal, cameraPosition, input.worldPosition, roughness);
			*/
	float3 finalColor =  lightSum + (ambient * surfaceColor);

	return float4(finalColor, 1.0f);
}