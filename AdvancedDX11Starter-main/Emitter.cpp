#include "Emitter.h"

Emitter::Emitter(Microsoft::WRL::ComPtr<ID3D11Device> _device, std::shared_ptr<Material> _material, int _maxParticles, int _particlesPerSecond)
{
	device = _device;
	material = _material;
	maxParticles = _maxParticles;
	particlesPerSecond = _particlesPerSecond;
	particles = new Particle[maxParticles];
	transform = new Transform();
	CreateParticlesAndGPUResources();
}

void Emitter::Update(float deltaTime, float currentTime)
{
	// Anything to update?
	if (numLiving > 0)
	{
		// Update all particles - Check cyclic buffer first
		if (firstLiving < firstDead)
		{
			// First alive is BEFORE first dead, so the "living" particles are contiguous
			// 
			// 0 -------- FIRST ALIVE ----------- FIRST DEAD -------- MAX
			// |    dead    |            alive       |         dead    |

			// First alive is before first dead, so no wrapping
			for (int i = firstLiving; i < firstDead; i++)
				UpdateSingleParticle(currentTime, i);
		}
		else if (firstDead < firstLiving)
		{
			// First alive is AFTER first dead, so the "living" particles wrap around
			// 
			// 0 -------- FIRST DEAD ----------- FIRST ALIVE -------- MAX
			// |    alive    |            dead       |         alive   |

			// Update first half (from firstAlive to max particles)
			for (int i = firstLiving; i < maxParticles; i++)
				UpdateSingleParticle(currentTime, i);

			// Update second half (from 0 to first dead)
			for (int i = 0; i < firstDead; i++)
				UpdateSingleParticle(currentTime, i);
		}
		else
		{
			// First alive is EQUAL TO first dead, so they're either all alive or all dead
			// - Since we know at least one is alive, they should all be
			//
			//            FIRST ALIVE
			// 0 -------- FIRST DEAD -------------------------------- MAX
			// |    alive     |                   alive                |
			for (int i = 0; i < maxParticles; i++)
				UpdateSingleParticle(currentTime, i);
		}
	}

	// Add to the time
	timeSinceLastEmit += deltaTime;

	// Enough time to emit?
	while (timeSinceLastEmit > secondsBetweenEmission)
	{
		EmitParticle(currentTime);
		timeSinceLastEmit = 0;
	}
}

void Emitter::Draw(Microsoft::WRL::ComPtr<ID3D11DeviceContext> context, std::shared_ptr<Camera> camera, float currentTime)
{
	CopyParticlesToGPU(context);

	// Set up buffers - note that we're NOT using a vertex buffer!
	// When we draw, we'll calculate the number of vertices we expect
	// to have given how many particles are currently alive.  We'll
	// construct the actual vertex data on the fly in the shader.
	UINT stride = 0;
	UINT offset = 0;
	ID3D11Buffer* nullBuffer = 0;
	context->IASetVertexBuffers(0, 1, &nullBuffer, &stride, &offset);
	context->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	// Set particle-specific data and let the
	// material take care of the rest
	material->PrepareMaterial(transform, camera);

	// Vertex data
	std::shared_ptr<SimpleVertexShader> vs = material->GetVertexShader();
	vs->SetMatrix4x4("view", camera->GetView());
	vs->SetMatrix4x4("projection", camera->GetProjection());
	vs->SetFloat("currentTime", currentTime);
	vs->SetFloat("particleLifetime", particleLifetime);
	vs->SetShaderResourceView("ParticleData", particleDataSRV);
	vs->CopyAllBufferData();

	std::shared_ptr<SimplePixelShader> ps = material->GetPixelShader();
	ps->SetShaderResourceView("Particle", material->GetTextureSRV("Particle"));
	//ps->SetSamplerState("BasicSampler" material)
	context->DrawIndexed(numLiving * 6, 0, 0);
}

void Emitter::UpdateSingleParticle(float currentTime, int index)
{
	float age = currentTime - particles[index].emitTime;

	// Update and check for death
	if (age > particleLifetime)
	{
		// Recent death, so retire by moving alive count (and wrap)
		firstLiving++;
		firstLiving %= maxParticles;
		numLiving--;
	}
}

void Emitter::EmitParticle(float currentTime)
{
	// Any left to spawn?
	if (numLiving == maxParticles)
		return;

	// Which particle is spawning?

	// Update the spawn time
	particles[firstDead].emitTime = currentTime;

	// Adjust the particle start position based on the random range (box shape)
	particles[firstDead].startPos = transform->GetPosition();
	particles[firstDead].startPos.x += ((float)rand() / RAND_MAX * (1.0f - -1.0f) + -1.0f);
	particles[firstDead].startPos.y += ((float)rand() / RAND_MAX * (1.0f - -1.0f) + -1.0f);
	particles[firstDead].startPos.z += ((float)rand() / RAND_MAX * (1.0f - -1.0f) + -1.0f);

	particles[firstDead].startColor = DirectX::XMFLOAT4(material->GetColorTint().x, material->GetColorTint().y, material->GetColorTint().z, 1.0f);
	particles[firstDead].endColor = particles[firstDead].startColor;
	particles[firstDead].endColor.x += RandomRange(-1.0f, 1.0f);
	particles[firstDead].endColor.y += RandomRange(-1.0f, 1.0f);
	particles[firstDead].endColor.z += RandomRange(-1.0f, 1.0f);
	/*
	// Adjust particle start velocity based on random range
	particles[spawnedIndex].StartVelocity = startVelocity;
	particles[spawnedIndex].StartVelocity.x += velocityRandomRange.x * RandomRange(-1.0f, 1.0f);
	particles[spawnedIndex].StartVelocity.y += velocityRandomRange.y * RandomRange(-1.0f, 1.0f);
	particles[spawnedIndex].StartVelocity.z += velocityRandomRange.z * RandomRange(-1.0f, 1.0f);

	// Adjust start and end rotation values based on range
	particles[spawnedIndex].StartRotation = RandomRange(rotationStartMinMax.x, rotationStartMinMax.y);
	particles[spawnedIndex].EndRotation = RandomRange(rotationEndMinMax.x, rotationEndMinMax.y);
	*/
	// Increment the first dead particle (since it's now alive)
	firstDead++;
	firstDead %= maxParticles; // Wrap

	// One more living particle
	numLiving++;
}

void Emitter::CopyParticlesToGPU(Microsoft::WRL::ComPtr<ID3D11DeviceContext> context)
{
	// Now that we have emit and updated all particles for this frame, 
	// we can copy them to the GPU as either one big chunk or two smaller chunks

	// Map the buffer
	D3D11_MAPPED_SUBRESOURCE mapped = {};
	context->Map(particleDataBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);

	// How are living particles arranged in the buffer?
	if (firstLiving < firstDead)
	{
		// Only copy from FirstAlive -> FirstDead
		memcpy(
			mapped.pData, // Destination = start of particle buffer
			particles + firstLiving, // Source = particle array, offset to first living particle
			sizeof(Particle) * numLiving); // Amount = number of particles (measured in BYTES!)
	}
	else
	{
		// Copy from 0 -> FirstDead 
		memcpy(
			mapped.pData, // Destination = start of particle buffer
			particles, // Source = start of particle array
			sizeof(Particle) * firstDead); // Amount = particles up to first dead (measured in BYTES!)

		// ALSO copy from FirstAlive -> End
		memcpy(
			(void*)((Particle*)mapped.pData + firstDead), // Destination = particle buffer, AFTER the data we copied in previous memcpy()
			particles + firstLiving,  // Source = particle array, offset to first living particle
			sizeof(Particle) * (maxParticles - firstLiving)); // Amount = number of living particles at end of array (measured in BYTES!)
	}

	// Unmap now that we're done copying
	context->Unmap(particleDataBuffer.Get(), 0);
}

void Emitter::CreateParticlesAndGPUResources()
{
	// Delete and release existing resources
	if (particles) delete[] particles;
	indexBuffer.Reset();
	particleDataBuffer.Reset();
	particleDataSRV.Reset();

	// Set up the particle array
	particles = new Particle[maxParticles];
	ZeroMemory(particles, sizeof(Particle) * maxParticles);

	// Create an index buffer for particle drawing
	// indices as if we had two triangles per particle
	int numIndices = maxParticles * 6;
	unsigned int* indices = new unsigned int[numIndices];
	int indexCount = 0;
	for (int i = 0; i < maxParticles * 4; i += 4)
	{
		indices[indexCount++] = i;
		indices[indexCount++] = i + 1;
		indices[indexCount++] = i + 2;
		indices[indexCount++] = i;
		indices[indexCount++] = i + 2;
		indices[indexCount++] = i + 3;
	}
	D3D11_SUBRESOURCE_DATA indexData = {};
	indexData.pSysMem = indices;

	// Regular (static) index buffer
	D3D11_BUFFER_DESC ibDesc = {};
	ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibDesc.CPUAccessFlags = 0;
	ibDesc.Usage = D3D11_USAGE_DEFAULT;
	ibDesc.ByteWidth = sizeof(unsigned int) * maxParticles * 6;
	device->CreateBuffer(&ibDesc, &indexData, indexBuffer.GetAddressOf());
	delete[] indices; // Sent to GPU already

	// Make a dynamic buffer to hold all particle data on GPU
	// Note: We'll be overwriting this every frame with new lifetime data
	D3D11_BUFFER_DESC allParticleBufferDesc = {};
	allParticleBufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	allParticleBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	allParticleBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	allParticleBufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	allParticleBufferDesc.StructureByteStride = sizeof(Particle);
	allParticleBufferDesc.ByteWidth = sizeof(Particle) * maxParticles;
	device->CreateBuffer(&allParticleBufferDesc, 0, particleDataBuffer.GetAddressOf());

	// Create an SRV that points to a structured buffer of particles
	// so we can grab this data in a vertex shader
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.NumElements = maxParticles;
	device->CreateShaderResourceView(particleDataBuffer.Get(), &srvDesc, particleDataSRV.GetAddressOf());
}
Transform* Emitter::GetTransform()
{
	return transform;
}
