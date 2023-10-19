#include "ShaderUtils.hlsli"
cbuffer colorTint : register(b0) {
	float3 colorTint;
	float3 cameraPosition;
	float roughness;
	float3 ambient;
	Light directionalLight1;
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
	float3 directionToLight = normalize(directionalLight1.direction * -1);
	float3 diffuseColor = Diffuse(input.normal, directionToLight) * directionalLight1.color;
	float3 specularColor = Specular(cameraPosition, input.worldPosition, directionToLight * -1, input.normal, roughness) * directionalLight1.color;;
	float3 finalColor = (colorTint * (diffuseColor + specularColor)) + (ambient * colorTint);

	return float4(finalColor, 1.0f);
}