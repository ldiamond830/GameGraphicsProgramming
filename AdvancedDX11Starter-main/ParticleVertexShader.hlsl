cbuffer externalData : register(b0)
{
    matrix view;
    matrix projection;
    float currentTime;
    float particleLifetime;
};

struct Particle
{
    float emitTime;
    float3 startPos;
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
    
    float3 pos = p.startPos;
    pos += float3(view._11, view._12, view._13) * offsets[cornerID].x;
    pos += float3(view._21, view._22, view._23) * offsets[cornerID].y;
    
    // Calculate output position
    matrix viewProj = mul(projection, view);
    output.position = mul(viewProj, float4(pos, 1.0f));
    
    float2 uvs[4];
	/* TL */
    uvs[0] = float2(0, 0);
	/* TR */
    uvs[1] = float2(1, 0);
	/* BR */
    uvs[2] = float2(1, 1);
	/* BL */
    uvs[3] = float2(0, 1);
	
	// Finalize output
    output.uv = uvs[cornerID];
    output.colorTint = lerp(p.startColor, p.endColor, age);
	return output;
}