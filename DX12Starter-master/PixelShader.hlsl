#include "Lighting.hlsli"

Texture2D Albedo : register(t0); // "t" registers for textures
Texture2D NormalMap : register(t1);
Texture2D RoughnessMap : register(t2);
Texture2D MetalnessMap : register(t3);
SamplerState Sampler : register(s0); // "s" registers for sampler

cbuffer ExternalData : register(b0)
{
    float2 uvScale;
    float2 uvOffset;
    float3 cameraPosition;
    int lightCount;
    Light lights[MAX_LIGHT_COUNT];
}

// Struct representing the data we expect to receive from earlier pipeline stages
// - Should match the output of our corresponding vertex shader
// - The name of the struct itself is unimportant
// - The variable names don't have to match other shaders (just the semantics)
// - Each variable must have a semantic, which defines its usage
struct VertexToPixel
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
	float4 screenPosition	: SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
    float3 tangent : TANGENT;
    float3 worldPosition : POSITION;
};

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

    // Scale and offset uv as necessary
    input.uv = input.uv * uvScale + uvOffset;
    float3 surfaceColor = pow(Albedo.Sample(Sampler, input.uv).rgb, 2.2f);
    //surfaceColor *= colorTint;
	
    float3 unpackedNormal = NormalMap.Sample(Sampler, input.uv).rgb * 2 - 1;
    unpackedNormal = normalize(unpackedNormal);

    float3 N = normalize(input.normal); // Must be normalized here or before
    float3 T = normalize(input.tangent); // Must be normalized here or before
    T = normalize(T - N * dot(T, N)); // Gram-Schmidt assumes T&N are normalized!
    float3 B = cross(T, N);
    float3x3 TBN = float3x3(T, B, N);

    input.normal = mul(unpackedNormal, TBN);

    float roughness = RoughnessMap.Sample(Sampler, input.uv).r;
    float metalness = MetalnessMap.Sample(Sampler, input.uv).r;
	// Assume albedo texture is actually holding specular color where metalness == 1
	// Note the use of lerp here - metal is generally 0 or 1, but might be in between
	// because of linear texture sampling, so we lerp the specular color to match
    float3 specular = lerp(F0_NON_METAL.rrr, surfaceColor.rgb, metalness);

    float3 lightSum = CalcAllLights(lights, lightCount, surfaceColor, input.normal, cameraPosition, input.worldPosition, roughness, metalness, specular);
    float3 finalColor = pow(lightSum, 1 / 2.2f);

    return float4(finalColor, 1.0f);
}