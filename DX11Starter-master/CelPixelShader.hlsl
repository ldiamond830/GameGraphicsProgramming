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

	float3 lightSum;
	for (int i = 0; i < 5; i++) {
		float diffuse;
		float spec = 0;
		switch (lights[i].type)
		{
		case LIGHT_TYPE_DIRECTIONAL:
			diffuse = Diffuse(input.normal, normalize(-lights[i].direction));
			diffuse = RampTexture.Sample(ClampSampler, float2(diffuse, 0));
			spec = Specular(cameraPosition, input.worldPosition, lights[i].direction, input.normal, 0.01f);
			lightSum += (diffuse * surfaceColor + spec) * lights[i].intensity * lights[i].color;
			
			break;
		case LIGHT_TYPE_POINT:
			float3 direction = normalize(lights[0].position - input.worldPosition);
			diffuse = Diffuse(input.normal, direction);
			diffuse = RampTexture.Sample(ClampSampler, float2(diffuse, 0));

			spec = Specular(cameraPosition, input.worldPosition, normalize(lights[i].position - input.worldPosition), input.normal, 1.0f);

			float attenuation = Attenuate(lights[i], input.worldPosition);
			
			lightSum += ((diffuse * surfaceColor + spec) * lights[i].intensity * lights[0].color) * attenuation;
			break;
		}
		
	}

	//gamma correct
	float3 finalColor = pow(lightSum, 1 / 2.2f);

	return float4(finalColor, 1.0f);
}