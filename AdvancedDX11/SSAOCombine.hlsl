struct VertexToPixel
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

Texture2D SceneColorsNoAmbient : register(t0);
Texture2D Ambient : register(t1);
Texture2D SSAOBlur : register(t2);
SamplerState BasicSampler : register(s0);

float4 main(VertexToPixel input) : SV_TARGET
{
// get original color, ambient color, and blurred occlusion color
    float3 sceneColors = SceneColorsNoAmbient.Sample(BasicSampler, input.uv).rgb;
    //currently unused due to lack of ibl to work with pbr render
    //float3 ambient = Ambient.Sample(BasicSampler, input.uv).rgb;
    float ao = SSAOBlur.Sample(BasicSampler, input.uv).r;
// Final combine
    return pow(float4(ao * sceneColors, 1), 1/2.2f);
}