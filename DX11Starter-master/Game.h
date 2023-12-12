#pragma once

#include "DXCore.h"
#include <DirectXMath.h>
#include <wrl/client.h> // Used for ComPtr - a smart pointer for COM objects
#include <vector>
#include "Mesh.h"
#include "Entity.h"
#include "Camera.h"
#include <memory>
#include <DirectXMath.h>
#include "SimpleShader.h"
#include "Material.h"
#include "Light.h"
#include "Sky.h";

class Game
	: public DXCore
{

public:
	Game(HINSTANCE hInstance);
	~Game();

	// Overridden setup and game loop methods, which
	// will be called automatically
	void Init();
	void OnResize();
	void Update(float deltaTime, float totalTime);
	void Draw(float deltaTime, float totalTime);
	void DrawShadowMap();

private:

	// Initialization helper methods - feel free to customize, combine, remove, etc.
	void LoadShaders();
	void CreateGeometry();
	void InitShadowMapResources();

	// Note the usage of ComPtr below
	//  - This is a smart pointer for objects that abide by the
	//     Component Object Model, which DirectX objects do
	//  - More info here: https://github.com/Microsoft/DirectXTK/wiki/ComPtr

	// Buffers to hold actual geometry data
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;

	// Shaders and shader-related constructs
	std::shared_ptr<SimplePixelShader> defaultPixelShader;
	std::shared_ptr<SimplePixelShader> customPixelShader;
	std::shared_ptr<SimplePixelShader> normalMapPixelShader;
	std::shared_ptr<SimplePixelShader> skyPixelShader;
	std::shared_ptr<SimpleVertexShader> vertexShader;
	std::shared_ptr<SimpleVertexShader> normalMapVertexShader;
	std::shared_ptr<SimpleVertexShader> skyVertexShader;
	std::shared_ptr<SimpleVertexShader> shadowVertexShader;
	std::shared_ptr<SimplePixelShader> celPixelShader;

	Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;

	std::vector<std::shared_ptr<Entity>> entityList;
	std::vector<std::shared_ptr<Material>> materialList;
	std::shared_ptr<Camera> currentCamera;
	int mainCameraIndex = 0;
	std::vector<std::shared_ptr<Camera>> cameraList;

	DirectX::XMFLOAT4 colorTint = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	DirectX::XMFLOAT3 offset = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	DirectX::XMFLOAT3 ambientColor = DirectX::XMFLOAT3(0.85f, 0.5f, 0.8f);


	Light sun = {};
	Light directionalLight2 = {};
	Light directionalLight3 = {};

	Light pointLight1 = {};
	Light pointLight2 = {};

	std::vector<Light> lights;
	std::shared_ptr <Sky> skyBox;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState;

	//shadow mapping resources
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> shadowDSV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shadowSRV;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> shadowRasterizer;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> shadowSampler;
	DirectX::XMFLOAT4X4 lightViewMatrix;
	DirectX::XMFLOAT4X4 lightProjectionMatrix;
	float lightProjectionSize = 24.0f;
	int shadowMapResolution = 2048;

	//cel shader resource
	
};

