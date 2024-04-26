
#include "Lighting.hlsli"

// How many lights could we handle?
#define MAX_LIGHTS 128

// Data that can change per material
cbuffer perMaterial : register(b0)
{
	// Surface color
	float3 colorTint;

	// UV adjustments
	float2 uvScale;
	float2 uvOffset;
};

// Data that only changes once per frame
cbuffer perFrame : register(b1)
{
	// An array of light data
	Light lights[MAX_LIGHTS];

	// The amount of lights THIS FRAME
	int lightCount;

	// Needed for specular (reflection) calculation
	float3 cameraPosition;
	
	
	// The number of mip levels in the specular IBL map
    int specularIBLTotalMipLevels;

	// Is indirect lighting on?
    int indirectLightingEnabled;

	// Scaling ibl
    float iblIntensity;
	
    float3 skyColor;
};


// Defines the input to this pixel shader
// - Should match the output of our corresponding vertex shader
struct VertexToPixel
{
	float4 screenPosition	: SV_POSITION;
	float2 uv				: TEXCOORD;
	float3 normal			: NORMAL;
	float3 tangent			: TANGENT;
	float3 worldPos			: POSITION; // The world position of this PIXEL
};


// Texture-related variables
Texture2D Albedo			: register(t0);
Texture2D NormalMap			: register(t1);
Texture2D RoughnessMap		: register(t2);
Texture2D MetalMap			: register(t3);
SamplerState BasicSampler	: register(s0);
SamplerState ClampSampler : register(s1);

struct PS_Output
{
    float4 colorDirect : SV_TARGET0;
    float4 colorIndirect : SV_TARGET1;
    float4 normals : SV_TARGET2;
    float depths : SV_TARGET3;
};

// IBL (indirect PBR) textures not yet implemented
Texture2D BrdfLookUpMap : register(t4);
TextureCube IrradianceIBLMap : register(t5);
TextureCube SpecularIBLMap : register(t6);

float3 IndirectDiffuse(TextureCube irrMap, SamplerState samp, float3 direction)
{
	// Sample in the specified direction - the irradiance map
	// is a pre-computed cube map which represents light
	// coming into this pixel from a particular hemisphere
    float3 diff = irrMap.SampleLevel(samp, direction, 0).rgb;
    return pow(abs(diff), 2.2);

}

float3 IndirectSpecular(TextureCube envMap, int mips, Texture2D brdfLookUp, SamplerState samp, float3 viewRefl, float NdotV, float roughness, float3 specColor)
{
	// Ensure roughness isn't zero
    roughness = max(roughness, MIN_ROUGHNESS);

	// Calculate half of the split-sum approx (this texture is not gamma-corrected, as it just holds raw data)
    float2 indirectBRDF = brdfLookUp.Sample(samp, float2(NdotV, roughness)).rg;
    float3 indSpecFresnel = specColor * indirectBRDF.x + indirectBRDF.y; // Spec color is f0

	// Sample the convolved environment map (other half of split-sum)
    float3 envSample = envMap.SampleLevel(samp, viewRefl, roughness * (mips - 1)).rgb;

	// Adjust environment sample by fresnel
    return pow(abs(envSample), 2.2) * indSpecFresnel;
}

// Entry point for this pixel shader
PS_Output main(VertexToPixel input) : SV_TARGET
{
	// Always re-normalize interpolated direction vectors
	input.normal = normalize(input.normal);
	input.tangent = normalize(input.tangent);

	// Apply the uv adjustments
	input.uv = input.uv * uvScale + uvOffset;

	// Sample various textures
	input.normal = NormalMapping(NormalMap, BasicSampler, input.uv, input.normal, input.tangent);
	float roughness = RoughnessMap.Sample(BasicSampler, input.uv).r;
	float metal = MetalMap.Sample(BasicSampler, input.uv).r;

	// Gamma correct the texture back to linear space and apply the color tint
	float4 surfaceColor = Albedo.Sample(BasicSampler, input.uv);
	surfaceColor.rgb = pow(surfaceColor.rgb, 2.2) * colorTint;

	// Specular color - Assuming albedo texture is actually holding specular color if metal == 1
	// Note the use of lerp here - metal is generally 0 or 1, but might be in between
	// because of linear texture sampling, so we want lerp the specular color to match
	float3 specColor = lerp(F0_NON_METAL.rrr, surfaceColor.rgb, metal);

	// Total color for this pixel
	float3 totalColor = float3(0,0,0);

	// Loop through all lights this frame
	for(int i = 0; i < lightCount; i++)
	{
		// Which kind of light?
		switch (lights[i].Type)
		{
		case LIGHT_TYPE_DIRECTIONAL:
			totalColor += DirLightPBR(lights[i], input.normal, input.worldPos, cameraPosition, roughness, metal, surfaceColor.rgb, specColor);
			break;

		case LIGHT_TYPE_POINT:
			totalColor += PointLightPBR(lights[i], input.normal, input.worldPos, cameraPosition, roughness, metal, surfaceColor.rgb, specColor);
			break;

		case LIGHT_TYPE_SPOT:
			totalColor += SpotLightPBR(lights[i], input.normal, input.worldPos, cameraPosition, roughness, metal, surfaceColor.rgb, specColor);
			break;
		}
	}
	
	// Calculate requisite reflection vectors
    float3 viewToCam = normalize(cameraPosition - input.worldPos);
    float3 viewRefl = normalize(reflect(-viewToCam, input.normal));
    float NdotV = saturate(dot(input.normal, viewToCam));

	// Indirect lighting
    float3 indirectDiffuse = pow(abs(skyColor), 2.2);
	/*
    float3 indirectSpecular = IndirectSpecular(
		SpecularIBLMap, specularIBLTotalMipLevels,
		BrdfLookUpMap, ClampSampler, // MUST use the clamp sampler here!
		viewRefl, NdotV,
		roughness, specColor);
	*/
	// Balance indirect diff/spec
    float3 balancedDiff = DiffuseEnergyConserve(indirectDiffuse, specColor, metal);
    float3 fullIndirect = balancedDiff * surfaceColor.rgb;

    PS_Output output;
    output.colorDirect = float4(totalColor, 1); // No gamma correction yet!
    //output.colorIndirect = float4(fullIndirect * iblIntensity * indirectLightingEnabled, 1);
    output.colorIndirect = float4(skyColor, 1);
    output.normals = float4(input.normal * 0.5f + 0.5f, 1);
    output.depths = input.screenPosition.z;
    return output;
}