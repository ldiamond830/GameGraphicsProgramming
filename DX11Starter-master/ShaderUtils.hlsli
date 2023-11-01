#ifndef __GGP_SHADER_UTILS__ // Each .hlsli file needs a unique identifier!
#define __GGP_SHADER_UTILS__
#define LIGHT_TYPE_DIRECTIONAL 0
#define LIGHT_TYPE_POINT 1
#define LIGHT_TYPE_SPOT 2
#define MAX_SPECULAR_EXPONENT 256.0f

//Struct representing the data we expect to receive from earlier pipeline stages
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
	float2 uv				: TEXCOORD;
    float3 normal			: NORMAL;
    float3 worldPosition	: POSITION;
};

struct VertexToPixelNormalMap
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
    float4 screenPosition : SV_POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float3 worldPosition : POSITION;
    float3 tangent : TANGENT;
};

// Struct representing a single vertex worth of data
// - This should match the vertex definition in our C++ code
// - By "match", I mean the size, order and number of members
// - The name of the struct itself is unimportant, but should be descriptive
// - Each variable must have a semantic, which defines its usage
struct VertexShaderInput
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
    float3 localPosition : POSITION; // XYZ position
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
    float3 tangent : TANGENT;
};

struct Light
{
    int type;
    float3 direction;
    float range;
    float3 position;
    float intensity;
    float3 color;
    float spotFalloff;
    float3 padding;
};

float Diffuse(float3 normal, float3 dirToLight)
{
    return saturate(dot(normal, dirToLight));
}

float Specular(float3 cameraPosition, float3 pixelWorldPosition, float3 incomingLightDirection, float3 normal, float roughness)
{
    
    float specExponent = (1.0f - roughness) * MAX_SPECULAR_EXPONENT;
    if (specExponent >= 0.05)
    {
        float3 view = normalize(cameraPosition - pixelWorldPosition);
        float3 reflection = reflect(incomingLightDirection, normal);
        return pow(saturate(dot(reflection, view)), specExponent);
    }
    else
    {
        return 0;
    }
}

float Attenuate(Light light, float3 worldPos)
{
    float dist = distance(light.position, worldPos);
    float att = saturate(1.0f - (dist * dist / (light.range * light.range)));
    return att * att;
}

float3 DirectionalLight(Light light, float3 surfaceColor, float3 normal, float3 cameraPosition, float3 pixelWorldPosition, float roughness)
{
    float diffuseColor = Diffuse(normal, normalize(light.direction * -1));
    float specularColor = Specular(cameraPosition, pixelWorldPosition, light.direction, normal, roughness);
    
    return (surfaceColor * (diffuseColor + specularColor)) * light.intensity * light.color;
}

float3 PointLight(Light light, float3 surfaceColor, float3 normal, float3 cameraPosition, float3 pixelWorldPosition, float roughness)
{
    float3 direction = normalize(pixelWorldPosition - light.position);
    float diffuseColor = Diffuse(normal, normalize(direction * -1));
    float specularColor = Specular(cameraPosition, pixelWorldPosition, direction, normal, roughness);
    float attenuation = Attenuate(light, pixelWorldPosition);
    return ((surfaceColor * (diffuseColor + specularColor)) * light.intensity * light.color) * attenuation;
}

#endif