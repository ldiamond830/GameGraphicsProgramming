#pragma once
#include <DirectXMath.h>
#include <wrl/client.h>
#include "Material.h"
#include "Transform.h"
#include "Camera.h"

// Helper macro for getting a float between min and max
#define RandomRange(min, max) ((float)rand() / RAND_MAX * (max - min) + min)

class Emitter
{
private:

	struct Particle {
		float emitTime;
		DirectX::XMFLOAT3 startPos;
		DirectX::XMFLOAT3 endPos;
		float pad;
		DirectX::XMFLOAT4 startColor;
		DirectX::XMFLOAT4 endColor;
	};

	int maxParticles = 100;
	Particle* particles;
	int firstLiving = 0;
	int firstDead = 0;
	int numLiving = 0;

	//emission properties
	float maxParticleLifetime = 1.0f;
	int particlesPerSecond;
	float secondsBetweenEmission = 0.5f; //always less than 1
	float timeSinceLastEmit = 0.0f;
	float particleLifetime = 2.0f;
	DirectX::XMFLOAT3 endPos;
	float radius;

	// Rendering
	Microsoft::WRL::ComPtr<ID3D11Device> device;
	Microsoft::WRL::ComPtr<ID3D11Buffer> particleDataBuffer;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> particleDataSRV;
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;

	// Material & transform
	Transform* transform;
	std::shared_ptr<Material> material;

	void UpdateSingleParticle(float currentTime, int index);
	void EmitParticle(float currentTime);
	void CopyParticlesToGPU(Microsoft::WRL::ComPtr<ID3D11DeviceContext> context);
	void CreateParticlesAndGPUResources();

	// Sprite sheet options
	int spriteSheetWidth;
	int spriteSheetHeight;
	float spriteSheetFrameWidth;
	float spriteSheetFrameHeight;
	float spriteSheetSpeedScale;
public:
	Emitter(Microsoft::WRL::ComPtr<ID3D11Device> device, std::shared_ptr<Material> material, int maxParticles, int particlesPerSecond, 
		DirectX::XMFLOAT3 endPos = DirectX::XMFLOAT3(INT16_MAX, INT16_MAX, INT16_MAX), float radius = 1.0f, 
		unsigned int spriteSheetWidth = 1, unsigned int spriteSheetHeight = 1, float spriteSheetSpeedScale = 1.0f);
	void Update(float deltaTime, float currentTime);
	void Draw(Microsoft::WRL::ComPtr<ID3D11DeviceContext> context, std::shared_ptr<Camera> camera, float currentTime);
	Transform* GetTransform();

};

