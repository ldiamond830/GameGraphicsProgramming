cbuffer externalData : register(b0)
{
    matrix view;
    matrix projection;
    float currentTime;
    float particleLifetime;
    
    int spriteSheetWidth;
    int spriteSheetHeight;
    float spriteSheetFrameWidth;
    float spriteSheetFrameHeight;
    float spriteSheetSpeedScale;
};

struct Particle
{
    float emitTime;
    float3 startPos;
    float3 endPos;
    float pad;
    float4 startColor;
    float4 endColor;
};

StructuredBuffer<Particle> ParticleData : register(t0);

struct VertexToPixel
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
    float4 colorTint : COLOR;
};

VertexToPixel main(uint id : SV_VertexID)
{
    VertexToPixel output;
    
    // Get id info
    uint particleID = id / 4; // Every group of 4 verts are ONE particle!
    uint cornerID = id % 4; // 0,1,2,3 = the corner of the particle "quad"

	// Grab one particle and its starting position
    Particle p = ParticleData.Load(particleID);

	// Calculate the age and age "percentage" (0 to 1)
    float age = currentTime - p.emitTime;
    float agePercent = age / particleLifetime;
    
     // Offsets for the 4 corners of a quad - we'll only
	// use one for each vertex, but which one depends
	// on the cornerID above.
    float2 offsets[4];
    offsets[0] = float2(-1.0f, +1.0f); // TL
    offsets[1] = float2(+1.0f, +1.0f); // TR
    offsets[2] = float2(+1.0f, -1.0f); // BR
    offsets[3] = float2(-1.0f, -1.0f); // BL    
    
    float3 pos = lerp(p.startPos, p.endPos, age);
    pos += float3(view._11, view._12, view._13) * offsets[cornerID].x;
    pos += float3(view._21, view._22, view._23) * offsets[cornerID].y;
    
    // Calculate output position
    matrix viewProj = mul(projection, view);
    output.position = mul(viewProj, float4(pos, 1.0f));
    
    float animPercent = fmod(agePercent * spriteSheetSpeedScale, 1.0f);
    uint ssIndex = (uint) floor(animPercent * (spriteSheetWidth * spriteSheetHeight));

	// Get the U/V indices (basically column & row index across the sprite sheet)
    uint uIndex = ssIndex % spriteSheetWidth;
    uint vIndex = ssIndex / spriteSheetHeight; // Integer division is important here!

	// Convert to a top-left corner in uv space (0-1)
    float u = uIndex / (float) spriteSheetWidth;
    float v = vIndex / (float) spriteSheetHeight;

    float2 uvs[4];
	/* TL */
    uvs[0] = float2(u, v);
	/* TR */
    uvs[1] = float2(u + spriteSheetFrameWidth, v);
	/* BR */
    uvs[2] = float2(u + spriteSheetFrameWidth, v + spriteSheetFrameHeight);
	/* BL */
    uvs[3] = float2(u, v + spriteSheetFrameHeight);
	
	// Finalize output
    output.uv = saturate(uvs[cornerID]);
    output.colorTint = lerp(p.startColor, p.endColor, age);
    return output;
}